#include "main/autogen/autoloader.h"
#include "absl/strings/match.h"
#include "common/FileOps.h"
#include "common/Timer.h"
#include "common/concurrency/ConcurrentQueue.h"
#include "common/concurrency/WorkerPool.h"
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
    return !nd.qname.nameParts.empty() && topLevelNamespaceRefs.contains(nd.qname.nameParts[0]);
}

bool AutoloaderConfig::includePath(string_view path) const {
    return absl::EndsWith(path, ".rb") &&
           !sorbet::FileOps::isFileIgnored("", fmt::format("/{}", path), absoluteIgnorePatterns,
                                           relativeIgnorePatterns);
}

bool AutoloaderConfig::includeRequire(core::NameRef req) const {
    return !excludedRequireRefs.contains(req);
}

bool AutoloaderConfig::sameFileCollapsable(const vector<core::NameRef> &module) const {
    return !nonCollapsableModuleNames.contains(module);
}

bool AutoloaderConfig::registeredForPBAL(const vector<core::NameRef> &pkgParts) const {
    return pbalNamespaces.empty() || (absl::c_any_of(pbalNamespaces, [&pkgParts](auto &pbalNamespace) {
               return pbalNamespace.size() <= pkgParts.size() &&
                      std::equal(pbalNamespace.begin(), pbalNamespace.end(), pkgParts.begin());
           }));
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
    for (auto &nameParts : cfg.pbalNamespaces) {
        vector<core::NameRef> refs;
        for (auto &name : nameParts) {
            refs.emplace_back(gs.enterNameConstant(name));
        }
        out.pbalNamespaces.emplace(refs);
    }
    out.absoluteIgnorePatterns = cfg.absoluteIgnorePatterns;
    out.relativeIgnorePatterns = cfg.relativeIgnorePatterns;
    out.stripPrefixes = cfg.stripPrefixes;
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
    uint32_t pathDepth = count(pathStr.begin(), pathStr.end(), '/'); // Pre-compute for comparison

    auto fullName = parsedFile.showQualifiedName(gs, def);

    return {def.data(parsedFile),         move(fullName),       move(parentName),
            parsedFile.requireStatements, parsedFile.tree.file, pathDepth};
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
    fmt::format_to(std::back_inserter(buf), "{} [{}]\n", node.root() ? "<ROOT>" : node.name().show(gs),
                   fmt::map_join(node.namedDefs, ", ", fileRefToString));
    for (const auto &[name, tree] : node.children) {
        for (int i = 0; i < level; ++i) {
            fmt::format_to(std::back_inserter(buf), "  ");
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

string join(string_view path, string file) {
    if (file.empty()) {
        return string(path);
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
    fmt::format_to(std::back_inserter(buf), "{}\n", alCfg.preamble);

    core::FileRef definingFile;
    if (!namedDefs.empty()) {
        definingFile = file();
    } else if (children.empty() && hasDef()) {
        definingFile = file();
    }
    if (definingFile.exists()) {
        requireStatements(gs, alCfg, buf);
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
            fmt::format_to(std::back_inserter(buf), "{}.on_autoload('{}')\n", alCfg.registryModule, fullName);
            predeclare(gs, fullName, buf);
        }

        if (pkgName.exists()) {
            ENFORCE(!gs.packageDB().empty());

            auto &pkg = gs.packageDB().getPackageInfo(pkgName);

            // First path prefix is guaranteed to be the directory location of the package
            const string_view pathPrefix = pkg.pathPrefixes()[0];

            fmt::format_to(std::back_inserter(buf), "\n{}.pbal_register_package({}, '{}')\n", alCfg.registryModule,
                           fullName, pathPrefix);
        } else if (!children.empty()) {
            fmt::format_to(std::back_inserter(buf), "\n{}.autoload_map({}, {{\n", alCfg.registryModule, fullName);
            vector<pair<core::NameRef, string>> childNames;
            std::transform(children.begin(), children.end(), back_inserter(childNames),
                           [&gs](const auto &pair) { return make_pair(pair.first, pair.first.show(gs)); });
            fast_sort(childNames, [](const auto &lhs, const auto &rhs) -> bool { return lhs.second < rhs.second; });
            for (const auto &pair : childNames) {
                fmt::format_to(std::back_inserter(buf), "  {}: \"{}/{}\",\n", pair.second, alCfg.rootDir,
                               children.at(pair.first)->path(gs));
            }
            fmt::format_to(std::back_inserter(buf), "}})\n", fullName);
        }
    } else if (type == Definition::Type::Casgn || type == Definition::Type::Alias ||
               type == Definition::Type::TypeAlias) {
        ENFORCE(qname.size() > 1);
        casgnArg = fmt::format(", [{}, :{}]",
                               fmt::map_join(qname.nameParts.begin(), --qname.nameParts.end(),
                                             "::", [&](const auto &nr) -> string { return nr.show(gs); }),
                               qname.name().show(gs));
    }

    if (definingFile.exists()) {
        fmt::format_to(std::back_inserter(buf), "\n{}.for_autoload({}, \"{}\"{})\n", alCfg.registryModule, fullName,
                       alCfg.normalizePath(gs, definingFile), casgnArg);
    }
    return to_string(buf);
}

void DefTree::requireStatements(const core::GlobalState &gs, const AutoloaderConfig &alCfg,
                                fmt::memory_buffer &buf) const {
    if (root() || !hasDef()) {
        return;
    }
    auto &ndef = definition(gs);
    vector<string> reqs;
    for (auto reqRef : ndef.requireStatements) {
        if (alCfg.includeRequire(reqRef)) {
            string req = reqRef.show(gs);
            reqs.emplace_back(req);
        }
    }
    fast_sort(reqs);
    auto last = unique(reqs.begin(), reqs.end());
    for (auto it = reqs.begin(); it != last; ++it) {
        fmt::format_to(std::back_inserter(buf), "require '{}'\n", *it);
    }
}

void DefTree::predeclare(const core::GlobalState &gs, string_view fullName, fmt::memory_buffer &buf) const {
    if (hasDef() && definitionType(gs) == Definition::Type::Class) {
        fmt::format_to(std::back_inserter(buf), "\nclass {}", fullName);
        auto &def = definition(gs);
        if (!def.parentName.empty()) {
            fmt::format_to(
                std::back_inserter(buf), " < {}",
                fmt::map_join(def.parentName.nameParts, "::", [&](const auto &nr) -> string { return nr.show(gs); }));
        }
    } else {
        fmt::format_to(std::back_inserter(buf), "\nmodule {}", fullName);
    }
    // TODO aliases? casgn?
    fmt::format_to(std::back_inserter(buf), "\nend\n");
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

DefTree *DefTree::findNode(const vector<core::NameRef> &nameParts) {
    DefTree *node = this;
    for (auto nr : nameParts) {
        auto it = node->children.find(nr);
        if (it == node->children.end()) {
            return nullptr;
        }
        node = it->second.get();
    }

    return node;
}

void DefTree::markPackageNamespace(core::NameRef mangledName, const vector<core::NameRef> &nameParts) {
    DefTree *node = this->findNode(nameParts);
    if (node == nullptr) {
        return;
    }

    ENFORCE(!(node->pkgName.exists()), "Package name should not be already set");
    node->pkgName = mangledName;
}

void DefTreeBuilder::markPackages(const core::GlobalState &gs, DefTree &root, const AutoloaderConfig &alCfg) {
    auto testRoot = root.findNode({core::Names::Constants::Test()});

    for (auto nr : gs.packageDB().packages()) {
        auto &pkg = gs.packageDB().getPackageInfo(nr);
        if (pkg.strictAutoloaderCompatibility()) {
            // Only mark strictly path-based autoload compatible packages for now to reduce
            // computation / code generation, given this is the only current use-case for registering
            // packages in this context in the Stripe codebase.

            // Additionally this package must be registed for path-based autoloading.
            // TODO: (aadi-stripe, 10/24/2022) Remove this functionality once we no longer require
            // special registration.
            auto &pkgFullName = pkg.fullName();
            if (!alCfg.registeredForPBAL(pkgFullName)) {
                continue;
            }

            root.markPackageNamespace(pkg.mangledName(), pkgFullName);
            if (testRoot != nullptr) {
                testRoot->markPackageNamespace(pkg.mangledName(), pkgFullName);
            }
        }
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

        if (child->pkgName.exists() || child->hasDifferentFile(definingFile)) {
            collapseSameFileDefs(gs, alCfg, *child);
        } else {
            root.children.erase(copyIt);
        }
    }
}

namespace {
struct RenderAutoloadTask {
    string filePath;
    const DefTree &node;
};

struct ModificationState {
    bool modified;
};

// This function has two duties:
// * It creates autoload rendering tasks which will occur in a later parallel phase.
// * It creates subdirectories when needed, as they are required to write the autoloader output.
void populateAutoloadTasksAndCreateDirectories(const core::GlobalState &gs, vector<RenderAutoloadTask> &tasks,
                                               const AutoloaderConfig &alCfg, string_view path, const DefTree &node) {
    string name = node.root() ? "root" : node.name().show(gs);
    string filePath = join(path, fmt::format("{}.rb", name));
    tasks.emplace_back(RenderAutoloadTask{filePath, node});

    // Generate autoloads for child nodes if they exist and pkgName is not present (since the latter indicates
    // path-based autoloading for the package).
    if (!node.children.empty() && !node.pkgName.exists()) {
        auto subdir = join(path, node.root() ? "" : name);
        if (!node.root()) {
            FileOps::ensureDir(subdir);
        }
        for (auto &[_, child] : node.children) {
            populateAutoloadTasksAndCreateDirectories(gs, tasks, alCfg, subdir, *child);
        }
    }
}
}; // namespace

void AutoloadWriter::writeAutoloads(const core::GlobalState &gs, WorkerPool &workers, const AutoloaderConfig &alCfg,
                                    const std::string &path, const DefTree &root) {
    vector<RenderAutoloadTask> tasks;
    populateAutoloadTasksAndCreateDirectories(gs, tasks, alCfg, path, root);

    auto modificationState = ModificationState{false};
    std::mutex modificationMutex;

    if (FileOps::exists(path)) {
        // Clear out files that we do not plan to write.
        vector<string> existingFiles = FileOps::listFilesInDir(path, {".rb"}, true, {}, {});
        UnorderedSet<string> existingFilesSet(make_move_iterator(existingFiles.begin()),
                                              make_move_iterator(existingFiles.end()));
        for (auto &task : tasks) {
            existingFilesSet.erase(task.filePath);
        }
        for (const auto &file : existingFilesSet) {
            FileOps::removeFile(file);
        }
    }

    // Parallelize writing the files.
    auto inputq = make_shared<ConcurrentBoundedQueue<int>>(tasks.size());
    auto outputq = make_shared<BlockingBoundedQueue<CounterState>>(tasks.size());
    for (int i = 0; i < tasks.size(); ++i) {
        inputq->push(i, 1);
    }

    workers.multiplexJob(
        "runAutogenWriteAutoloads", [&gs, &tasks, &alCfg, inputq, outputq, &modificationState, &modificationMutex]() {
            int n = 0;
            {
                Timer timeit(gs.tracer(), "autogenWriteAutoloadsWorker");
                int idx = 0;

                for (auto result = inputq->try_pop(idx); !result.done(); result = inputq->try_pop(idx)) {
                    ++n;
                    auto &task = tasks[idx];
                    bool rewritten = FileOps::writeIfDifferent(task.filePath, task.node.renderAutoloadSrc(gs, alCfg));

                    // Initial read should be cheap, read outside mutex
                    if (rewritten && !modificationState.modified) {
                        modificationMutex.lock();
                        // Re-test inside mutex
                        if (!modificationState.modified) {
                            modificationState.modified = true;
                        }
                        modificationMutex.unlock();
                    }
                }
            }

            outputq->push(getAndClearThreadCounters(), n);
        });

    CounterState out;
    for (auto res = outputq->wait_pop_timed(out, WorkerPool::BLOCK_INTERVAL(), gs.tracer()); !res.done();
         res = outputq->wait_pop_timed(out, WorkerPool::BLOCK_INTERVAL(), gs.tracer())) {
        if (!res.gotItem()) {
            continue;
        }
        counterConsume(move(out));
    }

    const std::string mtimeFile = join(path, "_mtime_stamp");
    if (!FileOps::exists(mtimeFile) || modificationState.modified) {
        FileOps::write(mtimeFile, to_string(std::time(0)));
    }
}

} // namespace sorbet::autogen
