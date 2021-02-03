#include "main/autogen/autoloader.h"
#include "absl/strings/match.h"
#include "common/FileOps.h"
#include "common/Timer.h"
#include "common/formatting.h"
#include "common/sort.h"
#include "core/GlobalState.h"
#include "core/Names.h"

#include "absl/strings/str_join.h"
#include "absl/strings/str_replace.h"
#include "absl/strings/str_split.h"

using namespace std;
namespace sorbet::autogen {

// Like the rest of autogen, the `autoloader` pass works by walking an existing tree of data and converting it to a
// different representation before using it. In this case, it takes the `autogen::ParsedFile` representation and
// converts it to a `DefTree`, and then emits the autoloader files based on passed-in string fragments

bool AutoloaderConfig::include(const NamedDefinition &nd) const {
    return !nd.qname.nameParts.empty() &&
           topLevelNamespaceRefs.find(nd.qname.nameParts[0]) != topLevelNamespaceRefs.end();
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

string_view AutoloaderConfig::normalizePath(const core::GlobalState &gs, core::FileRef file) const {
    auto path = file.data(gs).path();
    for (const auto &prefix : stripPrefixes) {
        if (absl::StartsWith(path, prefix)) {
            return path.substr(prefix.size());
        }
    }
    return path;
}

AutoloaderConfig AutoloaderConfig::enterConfig(core::GlobalState &gs, const realmain::options::AutoloaderConfig &cfg) {
    AutoloaderConfig out;
    out.rootDir = cfg.rootDir;
    out.preamble = cfg.preamble;
    out.registryModule = cfg.registryModule;
    out.rootObject = cfg.rootObject;
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
    out.packagedAutoloader = cfg.packagedAutoloader;
    return out;
}

NamedDefinition NamedDefinition::fromDef(const core::GlobalState &gs, ParsedFile &parsedFile, DefinitionRef def) {
    QualifiedName parentName;
    if (def.data(parsedFile).parent_ref.exists()) {
        auto parentRef = def.data(parsedFile).parent_ref.data(parsedFile);
        if (!parentRef.resolved.empty()) {
            parentName = parentRef.resolved;
        } else {
            parentName = parentRef.name;
        }
    }
    const auto &pathStr = parsedFile.tree.file.data(gs).path();
    u4 pathDepth = count(pathStr.begin(), pathStr.end(), '/'); // Pre-compute for comparison

    auto fullName = parsedFile.showQualifiedName(gs, def);

    return {def.data(parsedFile), fullName, parentName, parsedFile.requires, parsedFile.tree.file, pathDepth};
}

bool NamedDefinition::preferredTo(const core::GlobalState &gs, const NamedDefinition &lhs, const NamedDefinition &rhs) {
    ENFORCE(lhs.qname == rhs.qname, "Can only compare definitions with same name");
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
                       fmt::map_join(qname.nameParts, "::", [&](core::NameRef nr) -> string { return nr.show(gs); }));
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
    return qname.empty();
}

core::NameRef DefTree::name() const {
    ENFORCE(!qname.empty());
    return qname.name();
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
        fullName = root() ? alCfg.rootObject
                          : fmt::format("{}", fmt::map_join(qname.nameParts, "::", [&](const auto &nr) -> string {
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
        ENFORCE(qname.size() > 1);
        casgnArg = fmt::format(", [{}, :{}]",
                               fmt::map_join(qname.nameParts.begin(), --qname.nameParts.end(),
                                             "::", [&](const auto &nr) -> string { return nr.show(gs); }),
                               qname.name().show(gs));
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
            fmt::format_to(buf, " < {}", fmt::map_join(def.parentName.nameParts, "::", [&](const auto &nr) -> string {
                               return nr.show(gs);
                           }));
        }
    } else {
        fmt::format_to(buf, "\nmodule {}", fullName);
    }
    // TODO aliases? casgn?
    fmt::format_to(buf, "\nend\n");
}

string DefTree::path(const core::GlobalState &gs) const {
    auto toPath = [&](const auto &fr) -> string { return fr.show(gs); };
    return fmt::format("{}.rb", fmt::map_join(qname.nameParts, "/", toPath));
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
    for (const auto &part : ndef.qname.nameParts) {
        auto &child = node->children[part];
        if (!child) {
            child = make_unique<DefTree>();
            child->qname.package = ndef.qname.package;
            child->qname.nameParts = node->qname.nameParts;
            child->qname.nameParts.emplace_back(part);
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
    ENFORCE(lhs.qname == rhs.qname, "Name mismatch for DefTreeBuilder::merge");
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
    if (!alCfg.sameFileCollapsable(root.qname.nameParts)) {
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
    if (!alCfg.packagedAutoloader || !node.root()) {
        FileOps::writeIfDifferent(filePath, node.renderAutoloadSrc(gs, alCfg));
    }
    toDelete.erase(filePath);
    if (!node.children.empty()) {
        auto subdir = join(path, node.root() ? "" : name);
        if (!node.root() && !FileOps::dirExists(subdir)) {
            FileOps::createDir(subdir);
        }
        for (auto &[_, child] : node.children) {
            if (alCfg.packagedAutoloader && node.root()) {
                // in a packaged context, we want to make sure that these constants are also put in their packages
                ENFORCE(child->qname.package);
                auto namespaceName = child->qname.package->show(gs);
                // this package name will include the suffix _Package on the end, and we want to remove that
                constexpr int suffixLen = char_traits<char>::length("_Package");
                ENFORCE(namespaceName.size() > suffixLen);
                string packageName = namespaceName.substr(0, namespaceName.size() - suffixLen);
                auto pkgSubdir = join(subdir, packageName);
                FileOps::ensureDir(pkgSubdir);
                write(gs, alCfg, pkgSubdir, toDelete, *child);
            } else {
                write(gs, alCfg, subdir, toDelete, *child);
            }
        }
    }
}

namespace {
string renderPackageAutoloadSrc(const core::GlobalState &gs, const AutoloaderConfig &alCfg, const Package &pkg,
                                const string_view mangledName) {
    fmt::memory_buffer buf;
    fmt::format_to(buf, "{}\n", alCfg.preamble);

    // TODO: what will the export_methods autoload actually look like? no idea, but this shows that the data is there
    if (pkg.exportMethods) {
        fmt::format_to(buf, "require_relative \"{}/{}.rb\"\n", mangledName, pkg.exportMethods->join(gs, "/"));
        fmt::format_to(buf, "module ::PackageRoot::{}\n", mangledName);
        fmt::format_to(buf, "  extend ::PackageRoot::{}::{}\n", mangledName, pkg.exportMethods->join(gs, "::"));
        fmt::format_to(buf, "end\n");
    }
    fmt::format_to(buf, "{}.autoload_map(::PackageRoot::{}, {{\n", alCfg.registryModule, mangledName);
    for (auto expt : pkg.exports) {
        auto name = expt.join(gs, "::");
        fmt::format_to(buf, "  {}: \"{}/{}.rb\",\n", name, mangledName, name);
    }
    fmt::format_to(buf, "}})\n");

    return to_string(buf);
}
} // namespace

void AutoloadWriter::writePackageAutoloads(const core::GlobalState &gs, const AutoloaderConfig &alCfg,
                                           const std::string &path, const vector<Package> &packages) {
    // we're going to be building up the root package map as we walk all the other packages, so make that first
    fmt::memory_buffer buf;
    fmt::format_to(buf, "{}\n", alCfg.preamble);
    fmt::format_to(buf, "{}.autoload_map(::PackageRoot, {{\n", alCfg.registryModule);

    // walk over all the packages
    for (auto &pkg : packages) {
        auto pkgName = fmt::format(
            "{}", fmt::map_join(pkg.package, "::", [&](core::NameRef nr) -> string { return nr.show(gs); }));
        auto mangledName =
            fmt::format("{}", fmt::map_join(pkg.package, "_", [&](core::NameRef nr) -> string { return nr.show(gs); }));
        auto source = renderPackageAutoloadSrc(gs, alCfg, pkg, mangledName);

        FileOps::ensureDir(join(path, mangledName));
        auto targetPath = join(path, join(mangledName, "_root.rb"));

        // write the package autoload into the appropriate file
        FileOps::writeIfDifferent(targetPath, source);

        // and add the entry for this file to the root autoload
        fmt::format_to(buf, "  {}: \"{}\",\n", pkgName, join(mangledName, "_root.rb"));
    }

    fmt::format_to(buf, "}})\n");
    auto rootSrc = to_string(buf);
    FileOps::writeIfDifferent(join(path, "_root.rb"), rootSrc);
}

} // namespace sorbet::autogen
