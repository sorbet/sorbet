#ifndef SORBET_FOUND_DEFINITIONS_H
#define SORBET_FOUND_DEFINITIONS_H

#include "common/common.h"
#include "core/Loc.h"
#include "core/NameRef.h"
#include "core/ParsedArg.h"
#include "core/SymbolRef.h"
#include <vector>

namespace sorbet::core {

class FoundDefinitions;

struct FoundClassRef;
struct FoundClass;
struct FoundStaticField;
struct FoundTypeMember;
struct FoundMethod;

class FoundDefinitionRef final {
public:
    enum class Kind : uint8_t {
        Empty = 0,
        Class = 1,
        ClassRef = 2,
        Method = 3,
        StaticField = 4,
        TypeMember = 5,
        Symbol = 6,
    };
    CheckSize(Kind, 1, 1);

private:
    struct Storage {
        Kind kind;
        uint32_t id : 24; // We only support 2^24 (≈ 16M) definitions of any kind in a single file.
    } _storage;
    CheckSize(Storage, 4, 4);

public:
    FoundDefinitionRef(FoundDefinitionRef::Kind kind, uint32_t idx) : _storage({kind, idx}) {}
    FoundDefinitionRef() : FoundDefinitionRef(FoundDefinitionRef::Kind::Empty, 0) {}
    FoundDefinitionRef(const FoundDefinitionRef &nm) = default;
    FoundDefinitionRef(FoundDefinitionRef &&nm) = default;
    FoundDefinitionRef &operator=(const FoundDefinitionRef &rhs) = default;

    static FoundDefinitionRef root() {
        return FoundDefinitionRef(FoundDefinitionRef::Kind::Symbol, core::SymbolRef(core::Symbols::root()).rawId());
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

    FoundClassRef &klassRef(FoundDefinitions &foundDefs);
    const FoundClassRef &klassRef(const FoundDefinitions &foundDefs) const;

    FoundClass &klass(FoundDefinitions &foundDefs);
    const FoundClass &klass(const FoundDefinitions &foundDefs) const;

    FoundMethod &method(FoundDefinitions &foundDefs);
    const FoundMethod &method(const FoundDefinitions &foundDefs) const;

    FoundStaticField &staticField(FoundDefinitions &foundDefs);
    const FoundStaticField &staticField(const FoundDefinitions &foundDefs) const;

    FoundTypeMember &typeMember(FoundDefinitions &foundDefs);
    const FoundTypeMember &typeMember(const FoundDefinitions &foundDefs) const;

    core::SymbolRef symbol() const;

    static std::string kindToString(Kind kind);

    std::string toString(const core::GlobalState &gs, const FoundDefinitions &foundDefs) const;
};
CheckSize(FoundDefinitionRef, 4, 4);

struct FoundClassRef final {
    core::NameRef name;
    core::LocOffsets loc;
    // If !owner.exists(), owner is determined by reference site.
    FoundDefinitionRef owner;

    std::string toString(const core::GlobalState &gs, const FoundDefinitions &foundDefs, uint32_t id) const;
};
CheckSize(FoundClassRef, 16, 4);

struct FoundClass final {
    FoundDefinitionRef owner;
    FoundDefinitionRef klass;
    core::LocOffsets loc;
    core::LocOffsets declLoc;

    enum class Kind : uint8_t {
        Module,
        Class,
    };
    Kind classKind;

    std::string toString(const core::GlobalState &gs, const FoundDefinitions &foundDefs, uint32_t id) const;
};
CheckSize(FoundClass, 28, 4);

struct FoundStaticField final {
    FoundDefinitionRef owner;
    FoundDefinitionRef klass;
    core::NameRef name;
    core::LocOffsets asgnLoc;
    core::LocOffsets lhsLoc;
    bool isTypeAlias = false;

    std::string toString(const core::GlobalState &gs, const FoundDefinitions &foundDefs, uint32_t id) const;
};
CheckSize(FoundStaticField, 32, 4);

struct FoundTypeMember final {
    FoundDefinitionRef owner;
    core::NameRef name;
    core::LocOffsets asgnLoc;
    core::LocOffsets nameLoc;
    core::LocOffsets litLoc;
    core::NameRef varianceName;
    bool isFixed = false;
    bool isTypeTemplete = false;

    std::string toString(const core::GlobalState &gs, const FoundDefinitions &foundDefs, uint32_t id) const;
};
CheckSize(FoundTypeMember, 40, 4);

struct FoundMethod final {
    FoundDefinitionRef owner;
    core::NameRef name;
    core::LocOffsets loc;
    core::LocOffsets declLoc;
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
    std::vector<core::ParsedArg> parsedArgs;
    std::vector<uint32_t> argsHash;

    std::string toString(const core::GlobalState &gs, const FoundDefinitions &foundDefs, uint32_t id) const;
};
CheckSize(FoundMethod, 80, 8);

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

class FoundDefinitions final {
    // Contains references to items in _klasses, _methods, _staticFields, and _typeMembers.
    // Used to determine the order in which symbols are defined in SymbolDefiner.
    std::vector<FoundDefinitionRef> _definitions;
    // Contains references to classes in general. Separate from `FoundClass` because we sometimes need to define class
    // Symbols for classes that are referenced from but not present in the given file.
    std::vector<FoundClassRef> _klassRefs;
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

    FoundDefinitionRef addDefinition(FoundDefinitionRef ref) {
        DEBUG_ONLY(switch (ref.kind()) {
            case FoundDefinitionRef::Kind::Class:
            case FoundDefinitionRef::Kind::Method:
            case FoundDefinitionRef::Kind::StaticField:
            case FoundDefinitionRef::Kind::TypeMember:
                break;
            case FoundDefinitionRef::Kind::ClassRef:
            case FoundDefinitionRef::Kind::Empty:
            case FoundDefinitionRef::Kind::Symbol:
                ENFORCE(false, "Attempted to give unexpected FoundDefinitionRef kind to addDefinition");
        });
        _definitions.emplace_back(ref);
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
        return addDefinition(FoundDefinitionRef(FoundDefinitionRef::Kind::Class, idx));
    }

    FoundDefinitionRef addClassRef(FoundClassRef &&klassRef) {
        const uint32_t idx = _klassRefs.size();
        _klassRefs.emplace_back(std::move(klassRef));
        return FoundDefinitionRef(FoundDefinitionRef::Kind::ClassRef, idx);
    }

    FoundDefinitionRef addMethod(FoundMethod &&method) {
        const uint32_t idx = _methods.size();
        _methods.emplace_back(std::move(method));
        return addDefinition(FoundDefinitionRef(FoundDefinitionRef::Kind::Method, idx));
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

    FoundDefinitionRef addSymbol(core::SymbolRef symbol) {
        return FoundDefinitionRef(FoundDefinitionRef::Kind::Symbol, symbol.rawId());
    }

    void addModifier(FoundModifier &&mod) {
        _modifiers.emplace_back(std::move(mod));
    }

    // See documentation on _definitions
    const std::vector<FoundDefinitionRef> &definitions() const {
        return _definitions;
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

    friend FoundDefinitionRef;
};

} // namespace sorbet::core

#endif
