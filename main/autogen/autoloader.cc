#include "main/autogen/autoloader.h"
#include "absl/strings/match.h"
#include "common/FileOps.h"
#include "common/Timer.h"
#include "common/formatting.h"
#include "common/sort.h"
#include "core/GlobalState.h"
#include "core/Names.h"

using namespace std;
namespace sorbet::autogen {

// Like the rest of autogen, the `autoloader` pass works by walking an existing tree of data and converting it to a
// different representation before using it. In this case, it takes the `autogen::ParsedFile` representation and
// converts it to a `DefTree`, and then emits the autoloader files based on passed-in string fragments

// `true` if the definition should have autoloads generated for it based on the `AutoloaderConfig`
bool AutoloaderConfig::include(const NamedDefinition &nd) const {
    return !nd.nameParts.empty() && topLevelNamespaceRefs.find(nd.nameParts[0]) != topLevelNamespaceRefs.end();
}

// `true` if the file should have autoloads generated for it (i.e. it's a ruby source file that's not ignored)
bool AutoloaderConfig::includePath(string_view path) const {
    return absl::EndsWith(path, ".rb") &&
           !sorbet::FileOps::isFileIgnored("", fmt::format("/{}", path), absoluteIgnorePatterns,
                                           relativeIgnorePatterns);
}

// `true` if the file should be required
bool AutoloaderConfig::includeRequire(core::NameRef req) const {
    return excludedRequireRefs.find(req) == excludedRequireRefs.end();
}

// `true` if collapsible (???)
bool AutoloaderConfig::sameFileCollapsable(const vector<core::NameRef> &module) const {
    return nonCollapsableModuleNames.find(module) == nonCollapsableModuleNames.end();
}

// normalize the path relative to the prefixes (???)
string_view AutoloaderConfig::normalizePath(const core::GlobalState &gs, core::FileRef file) const {
    auto path = file.data(gs).path();
    for (const auto &prefix : stripPrefixes) {
        if (absl::StartsWith(path, prefix)) {
            return path.substr(prefix.size());
        }
    }
    return path;
}

// Convert the autoloader config passed in from `realmain` to this `AutoloaderConfig`
AutoloaderConfig AutoloaderConfig::enterConfig(core::GlobalState &gs, const realmain::options::AutoloaderConfig &cfg) {
    AutoloaderConfig out;
    out.rootDir = cfg.rootDir;
    out.preamble = cfg.preamble;
    out.registryModule = cfg.registryModule;
    for (auto &str : cfg.modules) {
        out.topLevelNamespaceRefs.emplace(gs.enterNameConstant(str));
    }
    for (auto &str : cfg.requireExcludes) {
        out.excludedRequireRefs.emplace(gs.enterNameUTF8(str));
    }
    for (auto &nameParts : cfg.sameFileModules) {
        vector<core::NameRef> refs;
        for (auto &name : nameParts) {
            refs.emplace_back(gs.enterNameConstant(name));
        }
        out.nonCollapsableModuleNames.emplace(refs);
    }
    out.absoluteIgnorePatterns = cfg.absoluteIgnorePatterns;
    out.relativeIgnorePatterns = cfg.relativeIgnorePatterns;
    out.stripPrefixes = cfg.stripPrefixes;
    return out;
}

// Takes a name and, if it is a packaged name, "unpackages" it by modifying it in-place and returning the package name
optional<core::NameRef> unpackageName(vector<core::NameRef> &name) {
    if (name.front() != core::Names::Constants::PackageRegistry()) {
        return nullopt;
    }
    auto pkgName = std::optional<core::NameRef>{name[1]};
    // TODO(gdritter): do this more efficiently!
    name.erase(name.begin());
    name.erase(name.begin());
    return pkgName;
}

// Convert an `autogen::DefinitionRef` to a `NamedDefinition`: this pulls the name, the parent definitions' name, the
// requirements, the path, and the _depth_ of the path (which can short-circuit comparison against another path)
NamedDefinition NamedDefinition::fromDef(const core::GlobalState &gs, ParsedFile &parsedFile, DefinitionRef def) {
    vector<core::NameRef> parentName;
    if (def.data(parsedFile).parent_ref.exists()) {
        auto parentRef = def.data(parsedFile).parent_ref.data(parsedFile);
        if (!parentRef.resolved.empty()) {
            parentName = parentRef.resolved;
        } else {
            parentName = parentRef.name;
        }

        unpackageName(parentName);
    }
    const auto &pathStr = parsedFile.tree.file.data(gs).path();
    u4 pathDepth = count(pathStr.begin(), pathStr.end(), '/'); // Pre-compute for comparison

    auto fullName = parsedFile.showFullName(gs, def);
    auto packageName = unpackageName(fullName);

    return {def.data(parsedFile), fullName,  parentName, parsedFile.requires,
            parsedFile.tree.file, pathDepth, packageName};
}

// Used for sorting `NamedDefinition`
bool NamedDefinition::preferredTo(const core::GlobalState &gs, const NamedDefinition &lhs, const NamedDefinition &rhs) {
    ENFORCE(lhs.nameParts == rhs.nameParts, "Can only compare definitions with same name");
    // Load defs with a parent name first since others will tend to depend on them.
    // Secondarily, the same idea for defs with less nesting in their path.
    // Finally, sort alphabetically by path to break any ties.
    if (lhs.parentName.empty() != rhs.parentName.empty()) {
        return rhs.parentName.empty();
    }
    if (lhs.pathDepth != rhs.pathDepth) {
        return lhs.pathDepth < rhs.pathDepth;
    }
    return lhs.fileRef.data(gs).path() < rhs.fileRef.data(gs).path();
}

void showHelper(const core::GlobalState &gs, fmt::memory_buffer &buf, const DefTree &node, int level) {
    auto fileRefToString = [&](const NamedDefinition &nd) -> string_view { return nd.fileRef.data(gs).path(); };
    fmt::format_to(buf, "{} [{}]\n", node.root() ? "<ROOT>" : node.name().show(gs),
                   fmt::map_join(node.namedDefs, ", ", fileRefToString));
    for (const auto &[name, tree] : node.children) {
        for (int i = 0; i < level; ++i) {
            fmt::format_to(buf, "  ");
        }
        showHelper(gs, buf, *tree, level + 1);
    }
}

string DefTree::show(const core::GlobalState &gs, int level) const {
    fmt::memory_buffer buf;
    showHelper(gs, buf, *this, 0);
    return to_string(buf);
}

string DefTree::fullName(const core::GlobalState &gs) const {
    return fmt::format("{}",
                       fmt::map_join(nameParts, "::", [&](core::NameRef nr) -> string_view { return nr.show(gs); }));
}

string join(string path, string file) {
    if (file.empty()) {
        return path;
    }
    return fmt::format("{}/{}", path, file);
}

bool visitDefTree(const DefTree &tree, std::function<bool(const DefTree &)> visit) {
    bool descend = visit(tree);
    if (!descend) {
        return false;
    }
    for (const auto &[_, child] : tree.children) {
        descend = visitDefTree(*child, visit);
        if (!descend) {
            return false;
        }
    }
    return true;
}

core::FileRef DefTree::file() const {
    core::FileRef ref;
    if (!namedDefs.empty()) {
        // TODO what if there are more than one?
        ref = namedDefs[0].fileRef;
    } else if (nonBehaviorDef != nullptr) {
        ref = nonBehaviorDef->fileRef;
    }
    return ref;
}

bool DefTree::hasDifferentFile(core::FileRef file) const {
    bool res = false;
    auto visit = [&](const DefTree &node) -> bool {
        auto f = node.file();
        if (file != f && f.exists()) {
            res = true;
            return false;
        }
        return true;
    };
    visitDefTree(*this, visit);
    return res;
}

bool DefTree::root() const {
    return nameParts.empty();
}

core::NameRef DefTree::name() const {
    ENFORCE(!nameParts.empty());
    return nameParts.back();
}

string DefTree::renderAutoloadSrc(const core::GlobalState &gs, const AutoloaderConfig &alCfg) const {
    fmt::memory_buffer buf;
    fmt::format_to(buf, "{}\n", alCfg.preamble);

    core::FileRef definingFile;
    if (!namedDefs.empty()) {
        definingFile = file();
    } else if (children.empty() && hasDef()) {
        definingFile = file();
    }
    if (definingFile.exists()) {
        requires(gs, alCfg, buf);
    }

    string fullName = "nil";
    string casgnArg;
    auto type = definitionType(gs);
    if (type == Definition::Type::Module || type == Definition::Type::Class) {
        fullName = root() ? "Object" : fmt::format("{}", fmt::map_join(nameParts, "::", [&](const auto &nr) -> string {
                                                       return nr.show(gs);
                                                   }));
        if (!root()) {
            fmt::format_to(buf, "{}.on_autoload('{}')\n", alCfg.registryModule, fullName);
            predeclare(gs, fullName, buf);
        }
        if (!children.empty()) {
            fmt::format_to(buf, "\n{}.autoload_map({}, {{\n", alCfg.registryModule, fullName);
            vector<pair<core::NameRef, string>> childNames;
            std::transform(children.begin(), children.end(), back_inserter(childNames),
                           [&gs](const auto &pair) { return make_pair(pair.first, pair.first.show(gs)); });
            fast_sort(childNames, [](const auto &lhs, const auto &rhs) -> bool { return lhs.second < rhs.second; });
            for (const auto &pair : childNames) {
                fmt::format_to(buf, "  {}: \"{}/{}\",\n", pair.second, alCfg.rootDir,
                               children.at(pair.first)->path(gs));
            }
            fmt::format_to(buf, "}})\n", fullName);
        }
    } else if (type == Definition::Type::Casgn || type == Definition::Type::Alias) {
        ENFORCE(nameParts.size() > 1);
        casgnArg = fmt::format(", [{}, :{}]",
                               fmt::map_join(nameParts.begin(), --nameParts.end(),
                                             "::", [&](const auto &nr) -> string { return nr.show(gs); }),
                               nameParts.back().show(gs));
    }

    if (definingFile.exists()) {
        fmt::format_to(buf, "\n{}.for_autoload({}, \"{}\"{})\n", alCfg.registryModule, fullName,
                       alCfg.normalizePath(gs, definingFile), casgnArg);
    }
    return to_string(buf);
}

void DefTree::requires(const core::GlobalState &gs, const AutoloaderConfig &alCfg, fmt::memory_buffer &buf) const {
    if (root() || !hasDef()) {
        return;
    }
    auto &ndef = definition(gs);
    vector<string> reqs;
    for (auto reqRef : ndef.requires) {
        if (alCfg.includeRequire(reqRef)) {
            string req = reqRef.show(gs);
            reqs.emplace_back(req);
        }
    }
    fast_sort(reqs);
    auto last = unique(reqs.begin(), reqs.end());
    for (auto it = reqs.begin(); it != last; ++it) {
        fmt::format_to(buf, "require '{}'\n", *it);
    }
}

void DefTree::predeclare(const core::GlobalState &gs, string_view fullName, fmt::memory_buffer &buf) const {
    if (hasDef() && definitionType(gs) == Definition::Type::Class) {
        fmt::format_to(buf, "\nclass {}", fullName);
        auto &def = definition(gs);
        if (!def.parentName.empty()) {
            fmt::format_to(buf, " < {}",
                           fmt::map_join(def.parentName, "::", [&](const auto &nr) -> string { return nr.show(gs); }));
        }
    } else {
        fmt::format_to(buf, "\nmodule {}", fullName);
    }
    // TODO aliases? casgn?
    fmt::format_to(buf, "\nend\n");
}

string DefTree::path(const core::GlobalState &gs) const {
    auto toPath = [&](const auto &fr) -> string { return fr.show(gs); };
    return fmt::format("{}.rb", fmt::map_join(nameParts, "/", toPath));
}

Definition::Type DefTree::definitionType(const core::GlobalState &gs) const {
    if (!hasDef()) {
        return Definition::Type::Module;
    }
    return definition(gs).def.type;
}

bool DefTree::hasDef() const {
    return (nonBehaviorDef != nullptr) || !namedDefs.empty();
}

const NamedDefinition &DefTree::definition(const core::GlobalState &gs) const {
    if (!namedDefs.empty()) {
        ENFORCE(namedDefs.size() == 1, "Cannot determine definitions for '{}' (size={})", fullName(gs),
                namedDefs.size());
        return namedDefs[0];
    } else {
        ENFORCE(nonBehaviorDef != nullptr, "Could not find any definitions for '{}'", fullName(gs));
        return *nonBehaviorDef;
    }
}

void DefTreeBuilder::addParsedFileDefinitions(const core::GlobalState &gs, const AutoloaderConfig &alConfig,
                                              std::unique_ptr<DefTree> &root, ParsedFile &pf) {
    ENFORCE(root->root());
    if (!alConfig.includePath(pf.path)) {
        return;
    }
    for (auto &def : pf.defs) {
        if (def.id.id() == 0) {
            continue;
        }
        addSingleDef(gs, alConfig, root, NamedDefinition::fromDef(gs, pf, def.id));
    }
}

void DefTreeBuilder::addSingleDef(const core::GlobalState &gs, const AutoloaderConfig &alCfg,
                                  std::unique_ptr<DefTree> &root, NamedDefinition ndef) {
    if (!alCfg.include(ndef)) {
        return;
    }

    DefTree *node = root.get();
    for (const auto &part : ndef.nameParts) {
        auto &child = node->children[part];
        if (!child) {
            child = make_unique<DefTree>();
            child->nameParts = node->nameParts;
            child->nameParts.emplace_back(part);
        }
        node = child.get();
    }
    if (ndef.def.defines_behavior) {
        node->namedDefs.emplace_back(move(ndef));
    } else {
        updateNonBehaviorDef(gs, *node, move(ndef));
    }
}

DefTree DefTreeBuilder::merge(const core::GlobalState &gs, DefTree lhs, DefTree rhs) {
    ENFORCE(lhs.nameParts == rhs.nameParts, "Name mismatch for DefTreeBuilder::merge");
    lhs.namedDefs.insert(lhs.namedDefs.end(), make_move_iterator(rhs.namedDefs.begin()),
                         make_move_iterator(rhs.namedDefs.end()));
    if (rhs.nonBehaviorDef) {
        updateNonBehaviorDef(gs, lhs, move(*rhs.nonBehaviorDef.get()));
    }
    for (auto &[rname, rchild] : rhs.children) {
        auto lchild = lhs.children.find(rname);
        if (lchild == lhs.children.end()) {
            lhs.children[rname] = move(rchild);
        } else {
            lhs.children[rname] = make_unique<DefTree>(merge(gs, move(*lchild->second), move(*rchild)));
        }
    }
    return lhs;
}

void DefTreeBuilder::updateNonBehaviorDef(const core::GlobalState &gs, DefTree &node, NamedDefinition ndef) {
    if (!node.namedDefs.empty()) {
        // Non behavior-defining definitions do not matter for nodes that have behavior. There is no
        // need to continue tracking it.
        return;
    }
    if ((node.nonBehaviorDef == nullptr) || NamedDefinition::preferredTo(gs, ndef, *node.nonBehaviorDef)) {
        node.nonBehaviorDef = make_unique<NamedDefinition>(move(ndef));
    }
}

void DefTreeBuilder::collapseSameFileDefs(const core::GlobalState &gs, const AutoloaderConfig &alCfg, DefTree &root) {
    core::FileRef definingFile;
    if (!root.namedDefs.empty()) {
        definingFile = root.file();
    }
    if (!alCfg.sameFileCollapsable(root.nameParts)) {
        return;
    }

    for (auto it = root.children.begin(); it != root.children.end(); /*nothing*/) {
        auto copyIt =
            it++; // see
                  // https://github.com/abseil/abseil-cpp/blob/62f05b1f57ad660e9c09e02ce7d591dcc4d0ca08/absl/container/internal/raw_hash_set.h#L1157-L1169
                  // for why
        auto &child = copyIt->second;

        if (child->hasDifferentFile(definingFile)) {
            collapseSameFileDefs(gs, alCfg, *child);
        } else {
            root.children.erase(copyIt);
        }
    }
}

void AutoloadWriter::writeAutoloads(const core::GlobalState &gs, const AutoloaderConfig &alCfg, const std::string &path,
                                    const DefTree &root) {
    UnorderedSet<string> toDelete; // Remove from this set as we write files
    if (FileOps::exists(path)) {
        vector<string> existingFiles = FileOps::listFilesInDir(path, {".rb"}, true, {}, {});
        toDelete.insert(make_move_iterator(existingFiles.begin()), make_move_iterator(existingFiles.end()));
    }
    write(gs, alCfg, path, toDelete, root);
    for (const auto &file : toDelete) {
        FileOps::removeFile(file);
    }
}

void AutoloadWriter::write(const core::GlobalState &gs, const AutoloaderConfig &alCfg, const std::string &path,
                           UnorderedSet<std::string> &toDelete, const DefTree &node) {
    string name = node.root() ? "root" : node.name().show(gs);
    string filePath = join(path, fmt::format("{}.rb", name));
    FileOps::writeIfDifferent(filePath, node.renderAutoloadSrc(gs, alCfg));
    toDelete.erase(filePath);
    if (!node.children.empty()) {
        auto subdir = join(path, node.root() ? "" : name);
        if (!node.root() && !FileOps::dirExists(subdir)) {
            FileOps::createDir(subdir);
        }
        for (auto &[_, child] : node.children) {
            write(gs, alCfg, subdir, toDelete, *child);
        }
    }
}

} // namespace sorbet::autogen
