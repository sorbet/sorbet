#include "core/FoundDefinitions.h"

using namespace std;

namespace sorbet::core {

FoundClass &FoundDefinitionRef::klass(FoundDefinitions &foundDefs) {
    ENFORCE(kind() == FoundDefinitionRef::Kind::Class);
    ENFORCE(foundDefs._klasses.size() > idx());
    return foundDefs._klasses[idx()];
}
const FoundClass &FoundDefinitionRef::klass(const FoundDefinitions &foundDefs) const {
    ENFORCE(kind() == FoundDefinitionRef::Kind::Class);
    ENFORCE(foundDefs._klasses.size() > idx());
    return foundDefs._klasses[idx()];
}

FoundMethod &FoundDefinitionRef::method(FoundDefinitions &foundDefs) {
    ENFORCE(kind() == FoundDefinitionRef::Kind::Method);
    ENFORCE(foundDefs._methods.size() > idx());
    return foundDefs._methods[idx()];
}
const FoundMethod &FoundDefinitionRef::method(const FoundDefinitions &foundDefs) const {
    ENFORCE(kind() == FoundDefinitionRef::Kind::Method);
    ENFORCE(foundDefs._methods.size() > idx());
    return foundDefs._methods[idx()];
}

FoundStaticField &FoundDefinitionRef::staticField(FoundDefinitions &foundDefs) {
    ENFORCE(kind() == FoundDefinitionRef::Kind::StaticField);
    ENFORCE(foundDefs._staticFields.size() > idx());
    return foundDefs._staticFields[idx()];
}
const FoundStaticField &FoundDefinitionRef::staticField(const FoundDefinitions &foundDefs) const {
    ENFORCE(kind() == FoundDefinitionRef::Kind::StaticField);
    ENFORCE(foundDefs._staticFields.size() > idx());
    return foundDefs._staticFields[idx()];
}

FoundTypeMember &FoundDefinitionRef::typeMember(FoundDefinitions &foundDefs) {
    ENFORCE(kind() == FoundDefinitionRef::Kind::TypeMember);
    ENFORCE(foundDefs._typeMembers.size() > idx());
    return foundDefs._typeMembers[idx()];
}
const FoundTypeMember &FoundDefinitionRef::typeMember(const FoundDefinitions &foundDefs) const {
    ENFORCE(kind() == FoundDefinitionRef::Kind::TypeMember);
    ENFORCE(foundDefs._typeMembers.size() > idx());
    return foundDefs._typeMembers[idx()];
}

FoundField &FoundDefinitionRef::field(FoundDefinitions &foundDefs) {
    ENFORCE(kind() == FoundDefinitionRef::Kind::Field);
    ENFORCE(foundDefs._fields.size() > idx());
    return foundDefs._fields[idx()];
}
const FoundField &FoundDefinitionRef::field(const FoundDefinitions &foundDefs) const {
    ENFORCE(kind() == FoundDefinitionRef::Kind::Field);
    ENFORCE(foundDefs._fields.size() > idx());
    return foundDefs._fields[idx()];
}

core::ClassOrModuleRef FoundDefinitionRef::symbol() const {
    ENFORCE(kind() == FoundDefinitionRef::Kind::Symbol);
    return core::ClassOrModuleRef::fromRaw(_storage.id);
}

string FoundDefinitionRef::kindToString(Kind kind) {
    switch (kind) {
        case FoundDefinitionRef::Kind::Class:
            return "FoundClass";
        case FoundDefinitionRef::Kind::Method:
            return "FoundMethod";
        case FoundDefinitionRef::Kind::StaticField:
            return "FoundStaticField";
        case FoundDefinitionRef::Kind::TypeMember:
            return "FoundTypeMember";
        case FoundDefinitionRef::Kind::Empty:
            return "FoundEmpty";
        case FoundDefinitionRef::Kind::Symbol:
            return "FoundSymbol";
        case FoundDefinitionRef::Kind::Field:
            return "FoundField";
    }
}

string FoundDefinitionRef::toString(const core::GlobalState &gs, const FoundDefinitions &foundDefs) const {
    string result;
    switch (this->kind()) {
        case FoundDefinitionRef::Kind::Class:
            result = this->klass(foundDefs).toString(gs, foundDefs, this->idx());
            break;
        case FoundDefinitionRef::Kind::Method:
            result = this->method(foundDefs).toString(gs, foundDefs, this->idx());
            break;
        case FoundDefinitionRef::Kind::StaticField:
            result = this->staticField(foundDefs).toString(gs, foundDefs, this->idx());
            break;
        case FoundDefinitionRef::Kind::TypeMember:
            result = this->typeMember(foundDefs).toString(gs, foundDefs, this->idx());
            break;
        case FoundDefinitionRef::Kind::Empty:
            result = "{}";
            break;
        case FoundDefinitionRef::Kind::Symbol:
            result = fmt::format("{{ symbol = {} }}", this->symbol().show(gs));
            break;
        case FoundDefinitionRef::Kind::Field:
            result = this->field(foundDefs).toString(gs, foundDefs, this->idx());
            break;
    }
    return fmt::format("{} {}", kindToString(this->kind()), result);
}

string FoundClass::toString(const core::GlobalState &gs, const FoundDefinitions &foundDefs, uint32_t id) const {
    string_view classKindStr;
    switch (classKind) {
        case Kind::Unknown:
            classKindStr = "unknown"sv;
            break;
        case Kind::Module:
            classKindStr = "module"sv;
            break;
        case Kind::Class:
            classKindStr = "class"sv;
            break;
    }
    return fmt::format("{{ id = {}, owner = {}, name = {}, classKind = {} }}", id, owner.idx(), name.show(gs),
                       classKindStr);
}

string FoundStaticField::toString(const core::GlobalState &gs, const FoundDefinitions &foundDefs, uint32_t id) const {
    return fmt::format("{{ id = {}, owner = {}, name = {} }}", id, owner.idx(), name.toString(gs));
}

string FoundTypeMember::toString(const core::GlobalState &gs, const FoundDefinitions &foundDefs, uint32_t id) const {
    return fmt::format("{{ id = {}, owner = {}, name = {} }}", id, owner.idx(), name.toString(gs));
}

std::string FoundMethod::toString(const core::GlobalState &gs, const FoundDefinitions &foundDefs, uint32_t id) const {
    return fmt::format("{{ id = {}, owner = {}, name = {} }}", id, owner.idx(), name.toString(gs));
}

string FoundField::toString(const core::GlobalState &gs, const FoundDefinitions &foundDefs, uint32_t id) const {
    return fmt::format("{{ id = {}, owner = {}, name = {} }}", id, owner.idx(), name.toString(gs));
}

} // namespace sorbet::core
