#include "main/autogen/autoloader.h"
#include "absl/strings/match.h"
#include "common/FileOps.h"
#include "common/Timer.h"
#include "core/GlobalState.h"
#include "core/Names.h"

using namespace std;
namespace sorbet::autogen {

bool AutoloaderConfig::include(const NamedDefinition &nd) const {
    return !nd.nameParts.empty() && topLevelNamespaceRefs.find(nd.nameParts[0]) != topLevelNamespaceRefs.end();
}

bool AutoloaderConfig::includePath(string_view path) const {
    return absl::EndsWith(path, ".rb") &&
           !sorbet::FileOps::isFileIgnored("", fmt::format("/{}", path), absoluteIgnorePatterns,
                                           relativeIgnorePatterns);
}

bool AutoloaderConfig::includeRequire(core::NameRef req) const {
    return excludedRequireRefs.find(req) == excludedRequireRefs.end();
}

bool AutoloaderConfig::sameFileCollapsable(const vector<core::NameRef> &module) const {
    return nonCollapsableModuleNames.find(module) == nonCollapsableModuleNames.end();
}

AutoloaderConfig AutoloaderConfig::enterConfig(core::GlobalState &gs, const realmain::options::AutoloaderConfig &cfg) {
    AutoloaderConfig out;
    out.rootDir = cfg.rootDir;
    out.preamble = cfg.preamble;
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
    return out;
}

NamedDefinition NamedDefinition::fromDef(core::Context ctx, ParsedFile &parsedFile, DefinitionRef def) {
    vector<core::NameRef> parentName;
    if (def.data(parsedFile).parent_ref.exists()) {
        auto parentRef = def.data(parsedFile).parent_ref.data(parsedFile);
        if (!parentRef.resolved.empty()) {
            parentName = parentRef.resolved;
        } else {
            parentName = parentRef.name;
        }
    }
    const auto &pathStr = parsedFile.tree.file.data(ctx).path();
    u4 pathDepth = count(pathStr.begin(), pathStr.end(), '/'); // Pre-compute for comparison
    return {def.data(parsedFile), parsedFile.showFullName(ctx, def),
            parentName,           parsedFile.requires,
            parsedFile.tree.file, pathDepth};
}

bool NamedDefinition::preferredTo(core::Context ctx, const NamedDefinition &lhs, const NamedDefinition &rhs) {
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
    return lhs.fileRef.data(ctx).path() < rhs.fileRef.data(ctx).path();
}

void showHelper(core::Context ctx, fmt::memory_buffer &buf, const DefTree &node, int level) {
    auto fileRefToString = [&](const NamedDefinition &nd) -> string_view { return nd.fileRef.data(ctx).path(); };
    fmt::format_to(buf, "{} [{}]\n", node.root() ? "<ROOT>" : node.name().show(ctx),
                   fmt::map_join(node.namedDefs, ", ", fileRefToString));
    for (const auto &[name, tree] : node.children) {
        for (int i = 0; i < level; ++i) {
            fmt::format_to(buf, "  ");
        }
        showHelper(ctx, buf, *tree, level + 1);
    }
}

string DefTree::show(core::Context ctx, int level) {
    fmt::memory_buffer buf;
    showHelper(ctx, buf, *this, 0);
    return to_string(buf);
}

string DefTree::fullName(core::Context ctx) const {
    return fmt::format("{}",
                       fmt::map_join(nameParts, "::", [&](core::NameRef nr) -> string_view { return nr.show(ctx); }));
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

string DefTree::renderAutoloadSrc(core::Context ctx, const AutoloaderConfig &alCfg) const {
    fmt::memory_buffer buf;
    fmt::format_to(buf, "{}\n", alCfg.preamble);

    core::FileRef definingFile;
    if (!namedDefs.empty()) {
        definingFile = file();
    } else if (children.empty() && hasDef()) {
        definingFile = file();
    }
    if (definingFile.exists()) {
        requires(ctx, alCfg, buf);
    }

    string fullName = "nil";
    auto type = definitionType(ctx);
    if (type == Definition::Module || type == Definition::Class) {
        fullName = root() ? "Object" : fmt::format("{}", fmt::map_join(nameParts, "::", [&](const auto &nr) -> string {
                                                       return nr.show(ctx);
                                                   }));
        if (!root()) {
            fmt::format_to(buf, "Opus::Require.on_autoload('{}')\n", fullName);
            predeclare(ctx, fullName, buf);
        }
        if (!children.empty()) {
            fmt::format_to(buf, "\nOpus::Require.autoload_map({}, {{\n", fullName);
            vector<pair<core::NameRef, string>> childNames;
            std::transform(children.begin(), children.end(), back_inserter(childNames),
                           [ctx](const auto &pair) { return make_pair(pair.first, pair.first.show(ctx)); });
            fast_sort(childNames, [](const auto &lhs, const auto &rhs) -> bool { return lhs.second < rhs.second; });
            for (const auto &pair : childNames) {
                fmt::format_to(buf, "  {}: \"autoloader/{}\",\n", pair.second, children.at(pair.first)->path(ctx));
            }
            fmt::format_to(buf, "}})\n", fullName);
        }
    }

    if (definingFile.exists()) {
        fmt::format_to(buf, "\nOpus::Require.for_autoload({}, \"{}\")\n", fullName, definingFile.data(ctx).path());
    }
    return to_string(buf);
}

void DefTree::requires(core::Context ctx, const AutoloaderConfig &alCfg, fmt::memory_buffer &buf) const {
    if (root() || !hasDef()) {
        return;
    }
    auto &ndef = definition(ctx);
    vector<string> reqs;
    for (auto reqRef : ndef.requires) {
        if (alCfg.includeRequire(reqRef)) {
            string req = reqRef.show(ctx);
            reqs.emplace_back(req);
        }
    }
    fast_sort(reqs);
    auto last = unique(reqs.begin(), reqs.end());
    for (auto it = reqs.begin(); it != last; ++it) {
        fmt::format_to(buf, "require '{}'\n", *it);
    }
}

void DefTree::predeclare(core::Context ctx, string_view fullName, fmt::memory_buffer &buf) const {
    if (hasDef() && definitionType(ctx) == Definition::Class) {
        fmt::format_to(buf, "\nclass {}", fullName);
        auto &def = definition(ctx);
        if (!def.parentName.empty()) {
            fmt::format_to(buf, " < {}",
                           fmt::map_join(def.parentName, "::", [&](const auto &nr) -> string { return nr.show(ctx); }));
        }
    } else {
        fmt::format_to(buf, "\nmodule {}", fullName);
    }
    // TODO aliases? casgn?
    fmt::format_to(buf, "\nend\n");
}

string DefTree::path(core::Context ctx) const {
    auto toPath = [&](const auto &fr) -> string { return fr.show(ctx); };
    return fmt::format("{}.rb", fmt::map_join(nameParts, "/", toPath));
}

Definition::Type DefTree::definitionType(core::Context ctx) const {
    if (!hasDef()) {
        return Definition::Module;
    }
    return definition(ctx).def.type;
}

bool DefTree::hasDef() const {
    return (nonBehaviorDef != nullptr) || !namedDefs.empty();
}

const NamedDefinition &DefTree::definition(core::Context ctx) const {
    if (!namedDefs.empty()) {
        ENFORCE(namedDefs.size() == 1, "Cannot determine definitions for '{}' (size={})", fullName(ctx),
                namedDefs.size());
        return namedDefs[0];
    } else {
        ENFORCE(nonBehaviorDef != nullptr, "Could not find any definitions for '{}'", fullName(ctx));
        return *nonBehaviorDef;
    }
}

void DefTreeBuilder::addParsedFileDefinitions(core::Context ctx, const AutoloaderConfig &alConfig,
                                              std::unique_ptr<DefTree> &root, ParsedFile &pf) {
    ENFORCE(root->root());
    if (!alConfig.includePath(pf.path)) {
        return;
    }
    for (auto &def : pf.defs) {
        if (def.id.id() == 0) {
            continue;
        }
        addSingleDef(ctx, alConfig, root, NamedDefinition::fromDef(ctx, pf, def.id));
    }
}

void DefTreeBuilder::addSingleDef(core::Context ctx, const AutoloaderConfig &alCfg, std::unique_ptr<DefTree> &root,
                                  NamedDefinition ndef) {
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
        updateNonBehaviorDef(ctx, *node, move(ndef));
    }
}

DefTree DefTreeBuilder::merge(core::Context ctx, DefTree lhs, DefTree rhs) {
    ENFORCE(lhs.nameParts == rhs.nameParts, "Name mismatch for DefTreeBuilder::merge");
    lhs.namedDefs.insert(lhs.namedDefs.end(), make_move_iterator(rhs.namedDefs.begin()),
                         make_move_iterator(rhs.namedDefs.end()));
    if (rhs.nonBehaviorDef) {
        updateNonBehaviorDef(ctx, lhs, move(*rhs.nonBehaviorDef.get()));
    }
    for (auto &[rname, rchild] : rhs.children) {
        auto lchild = lhs.children.find(rname);
        if (lchild == lhs.children.end()) {
            lhs.children[rname] = move(rchild);
        } else {
            lhs.children[rname] = make_unique<DefTree>(merge(ctx, move(*lchild->second), move(*rchild)));
        }
    }
    return lhs;
}

void DefTreeBuilder::updateNonBehaviorDef(core::Context ctx, DefTree &node, NamedDefinition ndef) {
    if (!node.namedDefs.empty()) {
        // Non behavior-defining definitions do not matter for nodes that have behavior. There is no
        // need to continue tracking it.
        return;
    }
    if ((node.nonBehaviorDef == nullptr) || NamedDefinition::preferredTo(ctx, ndef, *node.nonBehaviorDef)) {
        node.nonBehaviorDef = make_unique<NamedDefinition>(move(ndef));
    }
}

void DefTreeBuilder::collapseSameFileDefs(core::Context ctx, const AutoloaderConfig &alCfg, DefTree &root) {
    core::FileRef definingFile;
    if (!root.namedDefs.empty()) {
        definingFile = root.file();
    }
    if (!alCfg.sameFileCollapsable(root.nameParts)) {
        return;
    }

    for (auto it = root.children.begin(); it != root.children.end(); ++it) {
        auto &child = it->second;
        if (child->hasDifferentFile(definingFile)) {
            collapseSameFileDefs(ctx, alCfg, *child);
        } else {
            root.children.erase(it);
        }
    }
}

void AutoloadWriter::writeAutoloads(core::Context ctx, const AutoloaderConfig &alCfg, const std::string &path,
                                    const DefTree &root) {
    UnorderedSet<string> toDelete; // Remove from this set as we write files
    if (FileOps::exists(path)) {
        vector<string> existingFiles = FileOps::listFilesInDir(path, {".rb"}, true, {}, {});
        toDelete.insert(make_move_iterator(existingFiles.begin()), make_move_iterator(existingFiles.end()));
    }
    write(ctx, alCfg, path, toDelete, root);
    for (const auto &file : toDelete) {
        FileOps::removeFile(file);
    }
}

void AutoloadWriter::write(core::Context ctx, const AutoloaderConfig &alCfg, const std::string &path,
                           UnorderedSet<std::string> &toDelete, const DefTree &node) {
    string name = node.root() ? "root" : node.name().show(ctx);
    string filePath = join(path, fmt::format("{}.rb", name));
    FileOps::writeIfDifferent(filePath, node.renderAutoloadSrc(ctx, alCfg));
    toDelete.erase(filePath);
    if (!node.children.empty()) {
        auto subdir = join(path, node.root() ? "" : name);
        if (!node.root()) {
            FileOps::createDir(subdir);
        }
        for (auto &[_, child] : node.children) {
            write(ctx, alCfg, subdir, toDelete, *child);
        }
    }
}

} // namespace sorbet::autogen
