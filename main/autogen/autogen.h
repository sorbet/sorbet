#include "ast/ast.h"
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
    std::string name;
    std::vector<core::NameRef> nameParts;
    std::vector<core::NameRef> parentName;
    std::vector<core::NameRef> requires;
    core::FileRef fileRef;

    std::string_view toString(core::Context ctx) const;

    // NamedDefinition() = default;
    // NamedDefinition(const NamedDefinition &) = delete;
    // NamedDefinition(NamedDefinition &&) = default;
    // NamedDefinition &operator=(const NamedDefinition &) = delete;
    // NamedDefinition &operator=(NamedDefinition &&) = delete;
};

/*
relative == 'extn.rb' ||
relative.start_with?('extn/') ||
# Contains C extensions (which the autoloader doesn't know about). These
# have to be excluded via path rather than extension due to shenanigans
# in `ignore_require?` (see comments there for details).
relative.start_with?('build/cext/') ||
# Contain wrappers for the C extensions above. These wrappers don't
# define behavior, so we need to be able to manually require them.
relative.start_with?('cext/') ||
# The autoloader system files aren't in the dependency graph, so they
# should require normally.
relative.start_with?('build/autoloader/') ||
# dev/lib code doesn't use autoloaders because it's used
# by several things (e.g. `pay`) that don't share the
# pay-server Gemfile and thus can't necessarily load extn.
relative.start_with?('dev/lib/') ||
# rubocop plugins need to run quickly without depending on the build
relative.include?('/rubocop/') ||
# TODO: stop excluding `scripts` and `bin` directories from autoloading.
# # All we need to do is expand `NoBadMigrationDeps` to include stuff in these directories.
relative.start_with?('scripts/') ||
relative.include?('/scripts/') ||
relative.start_with?('bin/') ||
relative.include?('/bin/') ||
# Vendored gems should use their own requires normally.
relative.start_with?('vendor/') ||
# This is for ruby scripts without an extension, .erb files, and the .yml
# files we put into our ruby-analyze data, all of which are impossible to
# require.
!relative.end_with?('.rb') ||
# This contains a vendored copy of (some of) RDL with a lot of
# top-level code
relative.start_with?('lib/ruby-types/rdl-types/')
*/

struct AutoloaderConfig { // TODO dynamic loading
    UnorderedSet<std::string> topLevelNamespaces{"Opus", "Critic", "Chalk", "T", "Foo", "Yabba"}; // TODO TODO
    std::string_view rootDir = "autoloader";
    std::vector<std::regex> excludePatterns = {
        std::regex(R"(^bin/)"),        std::regex(R"(^build/autoloader/)"),
        std::regex(R"(^build/cext/)"), std::regex(R"(^cext/)"),
        std::regex(R"(^dev/lib/)"),    std::regex(R"(^extn/)"),
        std::regex(R"(^extn\.rb$)"),   std::regex(R"(^lib/ruby-types/rdl-types/)"),
        std::regex(R"(^scripts/)"),    std::regex(R"(^vendor/)"),
        std::regex(R"(/bin/)"),        std::regex(R"(/rubocop/)"),
        std::regex(R"(/scripts/)"),
    };
    UnorderedSet<std::string> excludedRequires = {
        // These are referred to in scripts that aren't usable with our
        // normal Gemfile. Many of them should probably be removed to
        // make all executables reproducible.
        "/deploy/batch-srv/current/extn",
        "activemerchant",
        "gmail",
        "luhn",
        "pdfkit",
        "perftools",
        "rugged",

        // Only in the smartsync Gemfile. This should likely get reconciled with
        // the main Gemfile eventually.
        "listen",

        // zookeeper group in Gemfile, normally excluded because compilation can be hairy
        "zeke",

        // ci_ignore group in Gemfile, not bundled in CI
        "pry-rescue",
        "pry-byebug",
        "byebug",
        "byebug/core",
    };

    std::vector<std::vector<std::string>> sameFileModules = {
        {"Opus", "Autogen", "Event"}, // TODO how do I compare refs?
    };

    bool include(core::Context, const NamedDefinition &) const;
    bool includePath(std::string_view path) const;
    bool includeRequire(const std::string &require) const;

    AutoloaderConfig() = default;
    AutoloaderConfig(const AutoloaderConfig &) = delete;
    AutoloaderConfig(AutoloaderConfig &&) = default;
    AutoloaderConfig &operator=(const AutoloaderConfig &) = delete;
    AutoloaderConfig &operator=(AutoloaderConfig &&) = delete;
};

class DefTree {
public:
    std::string name; // TODO switch to refs
    UnorderedMap<std::string, std::unique_ptr<DefTree>> children;
    std::vector<NamedDefinition> namedDefs;
    std::vector<NamedDefinition> nonBehaviorDefs;
    std::vector<core::NameRef> nameParts;

    bool root() const;
    void addDef(core::Context, const AutoloaderConfig &, const NamedDefinition &);
    void prettyPrint(core::Context ctx, int level = 0);

    void writeAutoloads(core::Context ctx, const AutoloaderConfig &, std::string path);
    std::string autoloads(core::Context ctx, const AutoloaderConfig &);

    std::string path(core::Context ctx);

    void prune(core::Context, const AutoloaderConfig &);
    bool prunable(core::Context, const AutoloaderConfig &) const;

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
    NamedDefinition &definition();
    Definition::Type definitionType();
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
