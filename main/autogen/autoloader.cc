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
    return {def.data(parsedFile), parsedFile.showFullName(ctx, def), parentName, parsedFile.requires,
            parsedFile.tree.file};
}

void DefTree::show(core::Context ctx, int level) {
    auto fileRefToString = [&](const NamedDefinition &nd) -> string_view { return nd.fileRef.data(ctx).path(); };
    fmt::print("{} [{}]\n", name().show(ctx), fmt::map_join(namedDefs, ", ", fileRefToString));
    for (auto &[name, tree] : children) {
        for (int i = 0; i < level; ++i) {
            fmt::print("  ");
        }
        tree->show(ctx, level + 1);
    }
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
    } else if (!nonBehaviorDefs.empty()) {
        // TODO this lacks sorting from `definition_sort_key`
        ref = nonBehaviorDefs[0].fileRef;
    }
    return ref;
}

static const core::FileRef EMPTY_FILE;
void DefTree::collapseSameFileDefs(core::Context ctx, const AutoloaderConfig &alCfg) {
    core::FileRef definingFile = EMPTY_FILE;
    if (!namedDefs.empty()) {
        definingFile = file();
    }
    if (!alCfg.sameFileCollapsable(nameParts)) {
        return;
    }

    for (auto it = children.begin(); it != children.end(); ++it) {
        auto &child = it->second;
        if (child->hasDifferentFile(definingFile)) {
            child->collapseSameFileDefs(ctx, alCfg);
        } else {
            children.erase(it);
        }
    }
}

bool DefTree::hasDifferentFile(core::FileRef file) const {
    bool res = false;
    auto visit = [&](const DefTree &node) -> bool {
        auto f = node.file();
        if (file != f && f != EMPTY_FILE) {
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

void DefTree::writeAutoloads(core::Context ctx, const AutoloaderConfig &alCfg, std::string path,
                             shared_ptr<spdlog::logger> logger) {
    string name = root() ? "root" : this->name().show(ctx);
    FileOps::write(join(path, fmt::format("{}.rb", name)), autoloads(ctx, alCfg, logger));
    if (!children.empty()) {
        auto subdir = join(path, root() ? "" : name);
        if (!root()) {
            FileOps::createDir(subdir);
        }
        for (auto &[_, child] : children) {
            child->writeAutoloads(ctx, alCfg, subdir, logger);
        }
    }
}

string DefTree::autoloads(core::Context ctx, const AutoloaderConfig &alCfg, shared_ptr<spdlog::logger> logger) {
    Timer timeit(logger, "autogenAutoloaderAutoloads");
    fmt::memory_buffer buf;
    fmt::format_to(buf, "{}\n", alCfg.preamble);

    core::FileRef definingFile = EMPTY_FILE;
    if (!namedDefs.empty()) {
        definingFile = file();
    } else if (children.empty() && hasDef()) {
        definingFile = file();
    }
    if (definingFile != EMPTY_FILE) {
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
                fmt::format_to(buf, "  {}: \"autoloader/{}\",\n", pair.second, children[pair.first]->path(ctx));
            }
            fmt::format_to(buf, "}})\n", fullName);
        }
    }

    if (definingFile != EMPTY_FILE) {
        fmt::format_to(buf, "\nOpus::Require.for_autoload({}, \"{}\")\n", fullName, definingFile.data(ctx).path());
    }
    return to_string(buf);
}

void DefTree::requires(core::Context ctx, const AutoloaderConfig &alCfg, fmt::memory_buffer &buf) {
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

void DefTree::predeclare(core::Context ctx, string_view fullName, fmt::memory_buffer &buf) {
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

string DefTree::path(core::Context ctx) {
    auto toPath = [&](const auto &fr) -> string { return fr.show(ctx); };
    return fmt::format("{}.rb", fmt::map_join(nameParts, "/", toPath));
}

Definition::Type DefTree::definitionType(core::Context ctx) {
    if (!hasDef()) {
        return Definition::Module;
    }
    return definition(ctx).def.type;
}

bool DefTree::hasDef() const {
    return !(namedDefs.empty() && nonBehaviorDefs.empty());
}

NamedDefinition &DefTree::definition(core::Context ctx) {
    if (!namedDefs.empty()) {
        ENFORCE(namedDefs.size() == 1, "Cannot determine definitions for '{}' (size={})", fullName(ctx),
                namedDefs.size());
        return namedDefs[0];
    } else {
        ENFORCE(!nonBehaviorDefs.empty(), "Could not find any defintions for '{}'", fullName(ctx));
        return nonBehaviorDefs[0];
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

void DefTreeBuilder::addSingleDef(core::Context, const AutoloaderConfig &alCfg, std::unique_ptr<DefTree> &root,
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
        node->nonBehaviorDefs.emplace_back(move(ndef));
    }
}

DefTree DefTreeBuilder::merge(DefTree lhs, DefTree rhs) {
    ENFORCE(lhs.nameParts == rhs.nameParts, "Name mismatch for DefTree::merge");
    lhs.namedDefs.insert(lhs.namedDefs.end(), make_move_iterator(rhs.namedDefs.begin()),
                         make_move_iterator(rhs.namedDefs.end()));
    lhs.nonBehaviorDefs.insert(lhs.nonBehaviorDefs.end(), make_move_iterator(rhs.nonBehaviorDefs.begin()),
                               make_move_iterator(rhs.nonBehaviorDefs.end()));
    for (auto &[rname, rchild] : rhs.children) {
        auto lchild = lhs.children.find(rname);
        if (lchild == lhs.children.end()) {
            lhs.children[rname] = move(rchild);
        } else {
            lhs.children[rname] = make_unique<DefTree>(merge(move(*lchild->second), move(*rchild)));
        }
    }
    return lhs;
}

} // namespace sorbet::autogen
