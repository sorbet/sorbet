#include "core/FoundDefinitions.h"

using namespace std;

namespace sorbet::core {

FoundClassRef &FoundDefinitionRef::klassRef(FoundDefinitions &foundDefs) {
    ENFORCE(kind() == FoundDefinitionRef::Kind::ClassRef);
    ENFORCE(foundDefs._klassRefs.size() > idx());
    return foundDefs._klassRefs[idx()];
}
const FoundClassRef &FoundDefinitionRef::klassRef(const FoundDefinitions &foundDefs) const {
    ENFORCE(kind() == FoundDefinitionRef::Kind::ClassRef);
    ENFORCE(foundDefs._klassRefs.size() > idx());
    return foundDefs._klassRefs[idx()];
}

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

core::SymbolRef FoundDefinitionRef::symbol() const {
    ENFORCE(kind() == FoundDefinitionRef::Kind::Symbol);
    return core::SymbolRef::fromRaw(_storage.id);
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
        case FoundDefinitionRef::Kind::ClassRef:
            return "FoundClassRef";
        case FoundDefinitionRef::Kind::Empty:
            return "FoundEmpty";
        case FoundDefinitionRef::Kind::Symbol:
            return "FoundSymbol";
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
        case FoundDefinitionRef::Kind::ClassRef:
            result = this->klassRef(foundDefs).toString(gs, foundDefs, this->idx());
            break;
        case FoundDefinitionRef::Kind::Empty:
            result = "{}";
            break;
        case FoundDefinitionRef::Kind::Symbol:
            result = fmt::format("{{ symbol = {} }}", this->symbol().show(gs));
            break;
    }
    return fmt::format("{} {}", kindToString(this->kind()), result);
}

string FoundClassRef::toString(const core::GlobalState &gs, const FoundDefinitions &foundDefs, uint32_t id) const {
    return fmt::format("{{ id = {}, name = {}, owner = {} }}", id, name.toString(gs), owner.idx());
}

string FoundClass::toString(const core::GlobalState &gs, const FoundDefinitions &foundDefs, uint32_t id) const {
    return fmt::format("{{ id = {}, owner = {}, klass = {}, classKind = {} }}", id, owner.idx(), klass.idx(),
                       classKind == Kind::Module ? "module" : "class");
}

string FoundStaticField::toString(const core::GlobalState &gs, const FoundDefinitions &foundDefs, uint32_t id) const {
    return fmt::format("{{ id = {}, owner = {}, klass = {}, name = {} }}", id, owner.idx(), klass.idx(),
                       name.toString(gs));
}

string FoundTypeMember::toString(const core::GlobalState &gs, const FoundDefinitions &foundDefs, uint32_t id) const {
    return fmt::format("{{ id = {}, owner = {}, name = {} }}", id, owner.idx(), name.toString(gs));
}

std::string FoundMethod::toString(const core::GlobalState &gs, const FoundDefinitions &foundDefs, uint32_t id) const {
    return fmt::format("{{ id = {}, owner = {}, name = {} }}", id, owner.idx(), name.toString(gs));
}

} // namespace sorbet::core
