#include "ast/ast.h"
#include "main/options/options.h"
#include <regex>

namespace sorbet::autogen {

const u4 NONE_ID = (u4)-1;

struct ParsedFile;
struct Reference;
struct Definition;

struct DefinitionRef {
    u4 _id;

    DefinitionRef() : _id(NONE_ID){};
    DefinitionRef(u4 id) : _id(id) {}

    u4 id() {
        return _id;
    }

    bool exists() {
        return _id != NONE_ID;
    }

    Definition &data(ParsedFile &pf);
};

struct ReferenceRef {
    u4 _id;
    ReferenceRef() : _id(NONE_ID){};
    ReferenceRef(u4 id) : _id(id) {}

    u4 id() {
        return _id;
    }

    bool exists() {
        return _id != NONE_ID;
    }

    Reference &data(ParsedFile &pf);
};

struct Definition {
    enum Type { Module, Class, Casgn, Alias };

    DefinitionRef id;

    Type type;
    bool defines_behavior;
    bool is_empty;

    ReferenceRef parent_ref;
    ReferenceRef aliased_ref;
    ReferenceRef defining_ref;
};

struct NamedDefinition {
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

class DefTree {
public:
    UnorderedMap<core::NameRef, std::unique_ptr<DefTree>> children;
    std::vector<NamedDefinition> namedDefs;
    std::vector<NamedDefinition> nonBehaviorDefs;
    std::vector<core::NameRef> nameParts;

    bool root() const;
    core::NameRef name() const;
    void addDef(core::Context, const AutoloaderConfig &, NamedDefinition);
    void prettyPrint(core::Context ctx, int level = 0);
    std::string fullName(core::Context) const;

    void writeAutoloads(core::Context ctx, const AutoloaderConfig &, std::string path);
    std::string autoloads(core::Context ctx, const AutoloaderConfig &);

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

struct Reference {
    ReferenceRef id;

    DefinitionRef scope;
    std::vector<core::NameRef> name;
    std::vector<DefinitionRef> nesting;
    std::vector<core::NameRef> resolved;

    core::Loc loc;
    core::Loc definitionLoc;
    bool is_resolved_statically;
    bool is_defining_ref;

    DefinitionRef parent_of;
};

struct ParsedFile {
    ast::ParsedFile tree;
    u4 cksum;
    std::string path;
    std::vector<Definition> defs;
    std::vector<Reference> refs;
    std::vector<core::NameRef> requires;

    std::string toString(core::Context ctx);
    std::string toMsgpack(core::Context ctx, int version);
    void classlist(core::Context ctx, std::vector<std::string> &out);
    NamedDefinition toNamed(core::Context ctx, DefinitionRef def);
    void addDefinitions(core::Context ctx, const AutoloaderConfig &alConfig, DefTree &root);

private:
    std::vector<core::NameRef> showFullName(core::Context ctx, DefinitionRef id);
    friend class MsgpackWriter;
};

class Autogen final {
public:
    static ParsedFile generate(core::Context ctx, ast::ParsedFile tree);
    Autogen() = delete;
};
} // namespace sorbet::autogen
