#ifndef RUBY_TYPER_AUTOGEN_AUTOLOADER_H
#define RUBY_TYPER_AUTOGEN_AUTOLOADER_H

#include "ast/ast.h"
#include "main/autogen/autogen.h"
#include "main/options/options.h"
#include <string_view>

namespace sorbet::autogen {

// Contains same information as `realmain::options::AutoloaderConfig` except with `core::NameRef`s
// instead of strings.
struct AutoloaderConfig {
    static AutoloaderConfig enterConfig(core::GlobalState &gs, const realmain::options::AutoloaderConfig &cfg);

    bool include(const NamedDefinition &) const;
    bool includePath(std::string_view path) const;
    bool includeRequire(core::NameRef req) const;

    std::string rootDir;
    std::string preamble;
    UnorderedSet<core::NameRef> topLevelNamespaceRefs;
    UnorderedSet<core::NameRef> excludedRequireRefs;
    std::vector<std::vector<core::NameRef>> sameFileModuleNames;
    std::vector<std::string> absoluteIgnorePatterns;
    std::vector<std::string> relativeIgnorePatterns;

    AutoloaderConfig() = default;
    AutoloaderConfig(const AutoloaderConfig &) = delete;
    AutoloaderConfig(AutoloaderConfig &&) = default;
    AutoloaderConfig &operator=(const AutoloaderConfig &) = delete;
    AutoloaderConfig &operator=(AutoloaderConfig &&) = default;
};

struct NamedDefinition {
    static NamedDefinition fromDef(core::Context, ParsedFile &, DefinitionRef);

    Definition def;
    std::vector<core::NameRef> nameParts;
    std::vector<core::NameRef> parentName;
    std::vector<core::NameRef> requires;
    core::FileRef fileRef;

    NamedDefinition() = default;
    NamedDefinition(const NamedDefinition &) = delete;
    NamedDefinition(NamedDefinition &&) = default;
    NamedDefinition &operator=(const NamedDefinition &) = delete;
    NamedDefinition &operator=(NamedDefinition &&) = default;
};

class DefTree {
public:
    UnorderedMap<core::NameRef, std::unique_ptr<DefTree>> children;
    std::vector<NamedDefinition> namedDefs;
    std::vector<NamedDefinition> nonBehaviorDefs;
    std::vector<core::NameRef> nameParts;

    bool root() const;
    core::NameRef name() const;
    void addSingleDef(core::Context, const AutoloaderConfig &, NamedDefinition);
    void prettyPrint(core::Context ctx, int level = 0);
    std::string fullName(core::Context) const;

    void writeAutoloads(core::Context ctx, const AutoloaderConfig &, std::string path,
                        std::shared_ptr<spdlog::logger> logger);
    std::string autoloads(core::Context ctx, const AutoloaderConfig &, std::shared_ptr<spdlog::logger> logger);

    std::string path(core::Context ctx);

    void prune(core::Context, const AutoloaderConfig &);
    bool prunable(const AutoloaderConfig &) const;

    void merge(DefTree rhs);

    DefTree() = default;
    DefTree(const DefTree &) = delete;
    DefTree(DefTree &&) = default;
    DefTree &operator=(const DefTree &) = delete;
    DefTree &operator=(DefTree &&) = default;

private:
    core::FileRef file() const;
    void predeclare(core::Context ctx, std::string_view fullName, fmt::memory_buffer &buf);
    void requires(core::Context ctx, const AutoloaderConfig &, fmt::memory_buffer &buf);
    bool hasDifferentFile(core::FileRef) const;
    bool hasDef() const;
    NamedDefinition &definition(core::Context);
    Definition::Type definitionType(core::Context);
};

// Add all definitions in a parsed file to a `DefTree` root.
void addAutoloaderDefinitions(core::Context, const AutoloaderConfig &, ParsedFile &, DefTree &);

} // namespace sorbet::autogen
#endif // RUBY_TYPER_AUTOGEN_AUTOLOADER_H
