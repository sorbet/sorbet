#include "ast/ast.h"

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

typedef std::pair<std::string, Definition::Type> AutogenSubclassEntry;
typedef std::set<AutogenSubclassEntry> AutogenSubclassSet;
typedef std::map<std::string, AutogenSubclassSet> AutogenSubclassMap;

struct ParsedFile {
    ast::ParsedFile tree;
    u4 cksum;
    std::string path;
    std::vector<Definition> defs;
    std::vector<Reference> refs;
    std::vector<core::NameRef> requires;

    std::string toString(core::Context ctx);
    std::string toMsgpack(core::Context ctx, int version);
    std::vector<std::string> classlist(core::Context ctx);
    std::optional<AutogenSubclassMap> subclasses(core::Context ctx, std::vector<std::string> &absolutePathsToIgnore,
                                                 std::vector<std::string> &relativePathsToIgnore);

private:
    std::vector<core::NameRef> showFullName(core::Context ctx, DefinitionRef id);
    friend class MsgpackWriter;
};

void maybeInsertChild(const std::string &parentName, const AutogenSubclassSet &children, AutogenSubclassMap &out);
void patchChildMap(AutogenSubclassMap &childMap);
void descendantsOf(const AutogenSubclassMap &childMap, const std::string &parent, AutogenSubclassSet &out);
std::unique_ptr<std::vector<std::string>> serializeSubclassMap(AutogenSubclassMap &descendantsMap,
                                                               std::vector<std::string> &parentNames);

class Autogen final {
public:
    static ParsedFile generate(core::Context ctx, ast::ParsedFile tree);
    Autogen() = delete;
};
} // namespace sorbet::autogen
