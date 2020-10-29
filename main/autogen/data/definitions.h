#ifndef AUTOGEN_DEFINITIONS_H
#define AUTOGEN_DEFINITIONS_H

#include "ast/ast.h"

namespace sorbet::autogen {

// The types defined here are simplified views of class and constant definitions in a Ruby codebase, which we use to
// create static output information and autoloader files

const u4 NONE_ID = (u4)-1;

struct ParsedFile;
struct Reference;
struct Definition;

struct DefinitionRef;
struct ReferenceRef;

struct AutoloaderConfig;
struct NamedDefinition;
class DefTree;

enum class ClassKind { Class, Module };

struct DefinitionRef {
    u4 _id;

    DefinitionRef() : _id(NONE_ID){};
    DefinitionRef(u4 id) : _id(id) {}

    u4 id() const {
        return _id;
    }

    bool exists() const {
        return _id != NONE_ID;
    }

    const Definition &data(const ParsedFile &pf) const;
};

struct ReferenceRef {
    u4 _id;
    ReferenceRef() : _id(NONE_ID){};
    ReferenceRef(u4 id) : _id(id) {}

    u4 id() const {
        return _id;
    }

    bool exists() const {
        return _id != NONE_ID;
    }

    const Reference &data(const ParsedFile &pf) const;
};

struct Definition {
    enum class Type : u8 { Module, Class, Casgn, Alias };

    DefinitionRef id;

    Type type;
    bool defines_behavior;
    bool is_empty;

    ReferenceRef parent_ref;
    ReferenceRef aliased_ref;
    ReferenceRef defining_ref;
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

    ClassKind parentKind = ClassKind::Module;

    DefinitionRef parent_of;
};

struct ParsedFile {
    friend class MsgpackWriter;

    ast::ParsedFile tree;
    u4 cksum;
    std::string path;
    std::vector<Definition> defs;
    std::vector<Reference> refs;
    std::vector<core::NameRef> requires;

    std::string toString(const core::GlobalState &gs) const;
    std::string toMsgpack(core::Context ctx, int version);
    std::vector<core::NameRef> showFullName(const core::GlobalState &gs, DefinitionRef id) const;
    std::vector<std::string> listAllClasses(core::Context ctx);
};

} // namespace sorbet::autogen
#endif // AUTOGEN_DEFINITIONS_H
