#ifndef AUTOGEN_H
#define AUTOGEN_H

#include "ast/ast.h"
#include "main/options/options.h"

namespace sorbet::autogen {

const u4 NONE_ID = (u4)-1;

struct ParsedFile;
struct Reference;
struct Definition;

struct AutoloaderConfig;
struct NamedDefinition;
class DefTree;

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
    enum Type { Module, Class, Casgn, Alias };

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

    std::string toString(core::Context ctx) const;
    std::string toMsgpack(core::Context ctx, int version);
    std::vector<core::NameRef> showFullName(core::Context ctx, DefinitionRef id) const;
    std::vector<std::string> listAllClasses(core::Context ctx);
};

class Autogen final {
public:
    static ParsedFile generate(core::Context ctx, ast::ParsedFile tree);
    Autogen() = delete;
};

} // namespace sorbet::autogen
#endif // AUTOGEN_H
