#ifndef SORBET_FOUND_DEFINITIONS_H
#define SORBET_FOUND_DEFINITIONS_H

#include "common/common.h"
#include "core/ArityHash.h"
#include "core/Loc.h"
#include "core/NameRef.h"
#include "core/ParsedArg.h"
#include "core/SymbolRef.h"
#include <vector>

namespace sorbet::core {

class FoundDefinitions;

struct FoundClass;
struct FoundStaticField;
struct FoundTypeMember;
struct FoundMethod;
struct FoundField;

class FoundDefinitionRef final {
public:
    enum class Kind : uint8_t {
        Empty = 0,
        Class = 1,
        // ClassRef = 2,
        Method = 3,
        StaticField = 4,
        TypeMember = 5,
        Symbol = 6, // stores ClassOrModuleRef IDs
        Field = 7,
    };
    CheckSize(Kind, 1, 1);

private:
    struct Storage {
        Kind kind : 4;
        // When kind != Symbol, `id` stores indices into the assorted vectors on `FoundDefitions`.
        // The 28-bit limit means that a single file cannot have more than 2^28 definitions in that file.
        //
        // When kind == Symbol, `id` stores ClassOrModule IDs. This means that a FoundDefinitionRef
        // can only store a max ID of 2^28 (instead of the 2^32 currently allowed for
        // ClassOrModuleRefs). But the interplay between findSymbols and defineSymbols means that
        // any SymbolRef discovered in an AST must have been placed there by a phase that came
        // before namer. This means that _technically_ we could have IDs as large as the largest
        // ClassOrModuleRef ID in the payload (if we had code to directly look up symbols by name in
        // GlobalState), but in practice (and ENFORCE'd below) the largest ID is the largest
        // synthetic ClassOrModuleRef defined in SymbolRef.h.
        //
        // 2^28 ≈ 268 million
        uint32_t id : 28;
        //
    } _storage;
    CheckSize(Storage, 4, 4);

public:
    FoundDefinitionRef(FoundDefinitionRef::Kind kind, uint32_t idx) : _storage({kind, idx}) {
        ENFORCE(this->_storage.kind != Kind::Symbol || idx < Symbols::MAX_SYNTHETIC_CLASS_SYMBOLS);
    }
    FoundDefinitionRef() : FoundDefinitionRef(FoundDefinitionRef::Kind::Empty, 0) {}
    FoundDefinitionRef(const FoundDefinitionRef &nm) = default;
    FoundDefinitionRef(FoundDefinitionRef &&nm) = default;
    FoundDefinitionRef &operator=(const FoundDefinitionRef &rhs) = default;

    static FoundDefinitionRef root() {
        return FoundDefinitionRef(FoundDefinitionRef::Kind::Symbol, core::Symbols::root().id());
    }

    FoundDefinitionRef::Kind kind() const {
        return _storage.kind;
    }

    bool exists() const {
        return _storage.id > 0;
    }

    uint32_t idx() const {
        return _storage.id;
    }

    FoundClass &klass(FoundDefinitions &foundDefs);
    const FoundClass &klass(const FoundDefinitions &foundDefs) const;

    FoundMethod &method(FoundDefinitions &foundDefs);
    const FoundMethod &method(const FoundDefinitions &foundDefs) const;

    FoundStaticField &staticField(FoundDefinitions &foundDefs);
    const FoundStaticField &staticField(const FoundDefinitions &foundDefs) const;

    FoundTypeMember &typeMember(FoundDefinitions &foundDefs);
    const FoundTypeMember &typeMember(const FoundDefinitions &foundDefs) const;

    FoundField &field(FoundDefinitions &foundDefs);
    const FoundField &field(const FoundDefinitions &foundDefs) const;

    core::ClassOrModuleRef symbol() const;

    static std::string kindToString(Kind kind);

    std::string toString(const core::GlobalState &gs, const FoundDefinitions &foundDefs) const;
};
CheckSize(FoundDefinitionRef, 4, 4);

struct FoundClass final {
    FoundDefinitionRef owner;
    core::NameRef name;
    core::LocOffsets loc;
    core::LocOffsets declLoc;

    enum class Kind : uint8_t {
        Unknown,
        Module,
        Class,
    };
    Kind classKind;

    std::string toString(const core::GlobalState &gs, const FoundDefinitions &foundDefs, uint32_t id) const;
};
CheckSize(FoundClass, 28, 4);

struct FoundStaticField final {
    FoundDefinitionRef owner;
    core::NameRef name;
    core::LocOffsets asgnLoc;
    core::LocOffsets lhsLoc;
    bool isTypeAlias = false;

    std::string toString(const core::GlobalState &gs, const FoundDefinitions &foundDefs, uint32_t id) const;
};
CheckSize(FoundStaticField, 28, 4);

struct FoundTypeMember final {
    FoundDefinitionRef owner;
    core::NameRef name;
    core::LocOffsets asgnLoc;
    core::LocOffsets nameLoc;
    core::LocOffsets litLoc;
    core::NameRef varianceName;
    bool isFixed = false;
    bool isTypeTemplate = false;

    std::string toString(const core::GlobalState &gs, const FoundDefinitions &foundDefs, uint32_t id) const;
};
CheckSize(FoundTypeMember, 40, 4);

struct FoundMethod final {
    FoundDefinitionRef owner;
    core::NameRef name;
    core::LocOffsets loc;
    core::LocOffsets declLoc;
    std::vector<core::ParsedArg> parsedArgs;
    core::ArityHash arityHash;
    struct Flags {
        bool isSelfMethod : 1;
        bool isRewriterSynthesized : 1;
        bool isAttrReader : 1;
        bool discardDef : 1;
        bool genericPropGetter : 1;

        // In C++20 we can replace this with bit field initialzers
        Flags()
            : isSelfMethod(false), isRewriterSynthesized(false), isAttrReader(false), discardDef(false),
              genericPropGetter(false) {}
    };
    Flags flags;
    CheckSize(Flags, 1, 1);

    std::string toString(const core::GlobalState &gs, const FoundDefinitions &foundDefs, uint32_t id) const;
};
CheckSize(FoundMethod, 56, 8);

struct FoundModifier {
    enum class Kind : uint8_t {
        Class = 0,
        Method = 1,
        ClassOrStaticField = 2,
    };
    Kind kind;
    FoundDefinitionRef owner;
    core::LocOffsets loc;
    // The name of the modification.
    core::NameRef name;
    // For methods: The name of the method being modified.
    // For constants: The name of the constant being modified.
    core::NameRef target;

    FoundModifier withTarget(core::NameRef target) {
        return FoundModifier{this->kind, this->owner, this->loc, this->name, target};
    }
};
CheckSize(FoundModifier, 24, 4);

struct FoundField {
    enum class Kind : uint8_t {
        ClassVariable,
        InstanceVariable,
    };
    Kind kind;
    bool onSingletonClass;
    bool fromWithinMethod;
    FoundDefinitionRef owner;
    core::LocOffsets loc;
    core::NameRef name;

    std::string toString(const core::GlobalState &gs, const FoundDefinitions &foundDefs, uint32_t id) const;
};
CheckSize(FoundField, 20, 4);

class FoundDefinitions final {
    // Contains references to items in _staticFields and _typeMembers.
    // Used so there is a consistent definition & redefinition ordering.
    std::vector<FoundDefinitionRef> _deletableDefinitions;
    // Contains all classes defined in the file.
    std::vector<FoundClass> _klasses;
    // Contains all methods defined in the file.
    std::vector<FoundMethod> _methods;
    // Contains all static fields defined in the file.
    std::vector<FoundStaticField> _staticFields;
    // Contains all type members defined in the file.
    std::vector<FoundTypeMember> _typeMembers;
    // Contains all method and class modifiers (e.g. private/public/protected).
    std::vector<FoundModifier> _modifiers;
    // Contains all class and instance variables defined in the file.
    std::vector<FoundField> _fields;

    FoundDefinitionRef addDefinition(FoundDefinitionRef ref) {
        DEBUG_ONLY(switch (ref.kind()) {
            case FoundDefinitionRef::Kind::StaticField:
            case FoundDefinitionRef::Kind::TypeMember:
                break;
            case FoundDefinitionRef::Kind::Class:
            case FoundDefinitionRef::Kind::Method:
            case FoundDefinitionRef::Kind::Field:
            case FoundDefinitionRef::Kind::Empty:
            case FoundDefinitionRef::Kind::Symbol:
                ENFORCE(false, "Attempted to give unexpected FoundDefinitionRef kind to addDefinition");
        });
        _deletableDefinitions.emplace_back(ref);
        return ref;
    }

public:
    FoundDefinitions() = default;
    FoundDefinitions(FoundDefinitions &&names) = default;
    FoundDefinitions(const FoundDefinitions &names) = delete;
    ~FoundDefinitions() = default;

    FoundDefinitionRef addClass(FoundClass &&klass) {
        const uint32_t idx = _klasses.size();
        _klasses.emplace_back(std::move(klass));
        return FoundDefinitionRef(FoundDefinitionRef::Kind::Class, idx);
    }

    FoundDefinitionRef addMethod(FoundMethod &&method) {
        const uint32_t idx = _methods.size();
        _methods.emplace_back(std::move(method));
        return FoundDefinitionRef(FoundDefinitionRef::Kind::Method, idx);
    }

    FoundDefinitionRef addStaticField(FoundStaticField &&staticField) {
        const uint32_t idx = _staticFields.size();
        _staticFields.emplace_back(std::move(staticField));
        return addDefinition(FoundDefinitionRef(FoundDefinitionRef::Kind::StaticField, idx));
    }

    FoundDefinitionRef addTypeMember(FoundTypeMember &&typeMember) {
        const uint32_t idx = _typeMembers.size();
        _typeMembers.emplace_back(std::move(typeMember));
        return addDefinition(FoundDefinitionRef(FoundDefinitionRef::Kind::TypeMember, idx));
    }

    FoundDefinitionRef addField(FoundField &&field) {
        const uint32_t idx = _fields.size();
        _fields.emplace_back(std::move(field));
        return FoundDefinitionRef(FoundDefinitionRef::Kind::Field, idx);
    }

    FoundDefinitionRef addSymbol(core::ClassOrModuleRef symbol) {
        return FoundDefinitionRef(FoundDefinitionRef::Kind::Symbol, symbol.id());
    }

    void addModifier(FoundModifier &&mod) {
        _modifiers.emplace_back(std::move(mod));
    }

    // See documentation on _deletableDefinitions
    const std::vector<FoundDefinitionRef> &deletableDefinitions() const {
        return _deletableDefinitions;
    }

    // See documentation on _klasses
    const std::vector<FoundClass> &klasses() const {
        return _klasses;
    }

    // See documentation on _methods
    const std::vector<FoundMethod> &methods() const {
        return _methods;
    }

    // See documentation on _modifiers
    const std::vector<FoundModifier> &modifiers() const {
        return _modifiers;
    }

    // See documentation on _fields
    const std::vector<FoundField> &fields() const {
        return _fields;
    }

    // See documentation on _staticFields
    const std::vector<FoundStaticField> &staticFields() const {
        return _staticFields;
    }

    // See documentation on _typeMembers
    const std::vector<FoundTypeMember> &typeMembers() const {
        return _typeMembers;
    }

    friend FoundDefinitionRef;
};

} // namespace sorbet::core

#endif
