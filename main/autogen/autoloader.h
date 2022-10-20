#ifndef AUTOGEN_AUTOLOADER_H
#define AUTOGEN_AUTOLOADER_H
#include "ast/ast.h"
#include "main/autogen/data/definitions.h"
#include "main/options/options.h"
#include <string_view>

namespace sorbet {
class WorkerPool;
}

namespace sorbet::autogen {

// Contains same information as `realmain::options::AutoloaderConfig` except with `core::NameRef`s
// instead of strings.
struct AutoloaderConfig {
    // Convert the autoloader config passed in from `realmain` to this `AutoloaderConfig`. Much of this is about
    // converting `string`s to `NameRef`s
    static AutoloaderConfig enterConfig(core::GlobalState &gs, const realmain::options::AutoloaderConfig &cfg);

    // `true` if the definition should have autoloads generated for it based on the `AutoloaderConfig`
    bool include(const NamedDefinition &) const;
    // `true` if the file should have autoloads generated for it (i.e. it's a ruby source file that's not ignored)
    bool includePath(std::string_view path) const;
    // `true` if the file should be required based on the provided configuration
    bool includeRequire(core::NameRef req) const;
    // Should definitions in this namespace be collapsed into their
    // parent if they all are from the same file?
    bool sameFileCollapsable(const std::vector<core::NameRef> &module) const;
    // This package is registered for path-based autoloading
    bool registeredForPBAL(const std::vector<core::NameRef> &pkgParts) const;
    // normalize the path relative to the provided prefixes
    std::string_view normalizePath(const core::GlobalState &gs, core::FileRef file) const;

    std::string rootDir;
    std::string preamble;
    std::string registryModule;
    std::string rootObject;
    UnorderedSet<core::NameRef> topLevelNamespaceRefs;
    UnorderedSet<core::NameRef> excludedRequireRefs;
    UnorderedSet<std::vector<core::NameRef>> nonCollapsableModuleNames;
    UnorderedSet<std::vector<core::NameRef>> pbalNamespaces;
    std::vector<std::string> absoluteIgnorePatterns;
    std::vector<std::string> relativeIgnorePatterns;
    std::vector<std::string> stripPrefixes;

    AutoloaderConfig() = default;
    AutoloaderConfig(const AutoloaderConfig &) = delete;
    AutoloaderConfig(AutoloaderConfig &&) = default;
    AutoloaderConfig &operator=(const AutoloaderConfig &) = delete;
    AutoloaderConfig &operator=(AutoloaderConfig &&) = default;
};

struct NamedDefinition {
    // Convert an `autogen::DefinitionRef` to a `NamedDefinition`: this pulls the name, the parent definitions' name,
    // the requirements, the path, and the _depth_ of the path (which can short-circuit comparison against another path)
    static NamedDefinition fromDef(const core::GlobalState &, ParsedFile &, DefinitionRef);
    // Used for sorting `NamedDefinition`
    static bool preferredTo(const core::GlobalState &gs, const NamedDefinition &lhs, const NamedDefinition &rhs);

    Definition def;
    QualifiedName qname;
    QualifiedName parentName;
    std::vector<core::NameRef> requireStatements;
    core::FileRef fileRef;
    uint32_t pathDepth;

    NamedDefinition() = default;
    NamedDefinition(Definition def, QualifiedName qname, QualifiedName parentName,
                    std::vector<core::NameRef> requireStatements, core::FileRef fileRef, uint32_t pathDepth)
        : def(def), qname(std::move(qname)), parentName(std::move(parentName)),
          requireStatements(std::move(requireStatements)), fileRef(fileRef), pathDepth(pathDepth) {}
    NamedDefinition(const NamedDefinition &) = delete;
    NamedDefinition(NamedDefinition &&) = default;
    NamedDefinition &operator=(const NamedDefinition &) = delete;
    NamedDefinition &operator=(NamedDefinition &&) = default;
};

class DefTree {
public:
    UnorderedMap<core::NameRef, std::unique_ptr<DefTree>> children;

    // For definitions that define behavior we enforce that it is only from a single code location.
    // However some nodes may represent a name that is used in many places but where none define
    // behavior (e.g. a module that is only used for namespacing). In that case, deterministically
    // pick a single file to use for the definition based on NamedDefinition::preferredTo precedence
    // rules.
    std::vector<NamedDefinition> namedDefs;
    std::unique_ptr<NamedDefinition> nonBehaviorDef;
    QualifiedName qname;
    core::NameRef pkgName;

    bool root() const;
    core::NameRef name() const;
    std::string path(const core::GlobalState &gs) const;
    std::string show(const core::GlobalState &gs, int level = 0) const; // Render the entire tree
    std::string fullName(const core::GlobalState &) const;

    std::string renderAutoloadSrc(const core::GlobalState &gs, const AutoloaderConfig &) const;

    DefTree() = default;
    DefTree(const DefTree &) = delete;
    DefTree(DefTree &&) = default;
    DefTree &operator=(const DefTree &) = delete;
    DefTree &operator=(DefTree &&) = default;

private:
    core::FileRef file() const;
    void predeclare(const core::GlobalState &gs, std::string_view fullName, fmt::memory_buffer &buf) const;
    void requireStatements(const core::GlobalState &gs, const AutoloaderConfig &, fmt::memory_buffer &buf) const;
    bool hasDifferentFile(core::FileRef) const;
    bool hasDef() const;
    const NamedDefinition &definition(const core::GlobalState &) const;
    Definition::Type definitionType(const core::GlobalState &) const;
    void markPackageNamespace(core::NameRef mangledName, const std::vector<core::NameRef> &nameParts);
    DefTree *findNode(const std::vector<core::NameRef> &nameParts);

    friend class DefTreeBuilder;
};

class DefTreeBuilder {
public:
    // Add all definitions in a parsed file to a `DefTree` root.
    static void addParsedFileDefinitions(const core::GlobalState &, const AutoloaderConfig &,
                                         std::unique_ptr<DefTree> &root, ParsedFile &);
    static void addSingleDef(const core::GlobalState &, const AutoloaderConfig &, std::unique_ptr<DefTree> &root,
                             NamedDefinition);

    static DefTree merge(const core::GlobalState &gs, DefTree lhs, DefTree rhs);
    static void markPackages(const core::GlobalState &gs, DefTree &root, const AutoloaderConfig &autoloaderConfig);
    static void collapseSameFileDefs(const core::GlobalState &gs, const AutoloaderConfig &, DefTree &root);

private:
    static void updateNonBehaviorDef(const core::GlobalState &gs, DefTree &node, NamedDefinition ndef);
};

class AutoloadWriter {
public:
    static void writeAutoloads(const core::GlobalState &gs, WorkerPool &workers, const AutoloaderConfig &,
                               const std::string &path, const DefTree &root);

    static void writePackageAutoloads(const core::GlobalState &gs, const AutoloaderConfig &, const std::string &path,
                                      const std::vector<Package> &packages);
};

} // namespace sorbet::autogen
#endif // AUTOGEN_AUTOLOADER_H
