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

} // namespace sorbet::core
