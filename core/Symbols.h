#ifndef SORBET_SYMBOLS_H
#define SORBET_SYMBOLS_H

#include "common/common.h"
#include "core/Loc.h"
#include "core/Names.h"
#include "core/SymbolRef.h"
#include "core/Types.h"
#include <memory>
#include <tuple>
#include <vector>

namespace sorbet::core {
class Symbol;
class GlobalState;
struct GlobalStateHash;
class Type;
class MutableContext;
class Context;
class TypeAndOrigins;
class SendAndBlockLink;
struct DispatchArgs;
struct CallLocs;

namespace serialize {
class SerializerImpl;
}
class IntrinsicMethod {
public:
    virtual void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const = 0;
};

enum class Variance { CoVariant = 1, ContraVariant = -1, Invariant = 0 };

enum class Visibility : uint8_t {
    Public = 1,
    Protected,
    Private,
};

class Method final {
public:
    friend class Symbol;
    friend class serialize::SerializerImpl;

    Method(const Method &) = delete;
    Method() = default;
    Method(Method &&) noexcept = default;
    class Flags {
    public:
        // Synthesized by C++ code in a Rewriter pass
        bool isRewriterSynthesized : 1;
        bool isProtected : 1;
        bool isPrivate : 1;
        bool isOverloaded : 1;
        bool isAbstract : 1;
        bool isGenericMethod : 1;
        bool isOverridable : 1;
        bool isFinal : 1;
        bool isOverride : 1;
        bool isIncompatibleOverride : 1;
        bool isPackagePrivate : 1;

        constexpr static uint16_t NUMBER_OF_FLAGS = 11;
        constexpr static uint16_t VALID_BITS_MASK = (1 << NUMBER_OF_FLAGS) - 1;
        Flags() noexcept
            : isRewriterSynthesized(false), isProtected(false), isPrivate(false), isOverloaded(false),
              isAbstract(false), isGenericMethod(false), isOverridable(false), isFinal(false), isOverride(false),
              isIncompatibleOverride(false), isPackagePrivate(false) {}

        uint16_t serialize() const {
            ENFORCE(sizeof(Flags) == sizeof(uint16_t));
            // Can replace this with std::bit_cast in C++20
            auto rawBits = *reinterpret_cast<const uint16_t *>(this);

            // We need to mask the valid bits since uninitialized memory isn't zeroed in C++.
            return rawBits & VALID_BITS_MASK;
        }
    };
    CheckSize(Flags, 2, 1);

    Loc loc() const;
    const InlinedVector<Loc, 2> &locs() const;
    void addLoc(const core::GlobalState &gs, core::Loc loc);
    uint32_t hash(const GlobalState &gs) const;
    uint32_t methodShapeHash(const GlobalState &gs) const;
    std::vector<uint32_t> methodArgumentHash(const GlobalState &gs) const;

    inline void setMethodPublic() {
        flags.isPrivate = false;
        flags.isProtected = false;
    }

    void setMethodVisibility(Visibility visibility) {
        switch (visibility) {
            case Visibility::Public:
                this->setMethodPublic();
                break;
            case Visibility::Protected:
                this->flags.isProtected = true;
                break;
            case Visibility::Private:
                this->flags.isPrivate = true;
                break;
        }
    }

    inline bool isMethodPublic() const {
        return !flags.isProtected && !flags.isPrivate;
    }

    Visibility methodVisibility() const {
        if (this->isMethodPublic()) {
            return Visibility::Public;
        } else if (this->flags.isProtected) {
            return Visibility::Protected;
        } else if (this->flags.isPrivate) {
            return Visibility::Private;
        } else {
            Exception::raise("Expected method to have visibility");
        }
    }

    bool hasSig() const {
        return resultType != nullptr;
    }

    using ArgumentsStore = InlinedVector<ArgInfo, core::SymbolRef::EXPECTED_METHOD_ARGS_COUNT>;

    // if dealiasing fails here, then we return a bad alias method stub instead
    MethodRef dealiasMethod(const GlobalState &gs, int depthLimit = 42) const;

    // TODO(dmitry) perf: most calls to this method could be eliminated as part of perf work.
    MethodRef ref(const GlobalState &gs) const;

    Method deepCopy(const GlobalState &to) const;
    void sanityCheck(const GlobalState &gs) const;
    bool ignoreInHashing(const GlobalState &gs) const;
    bool isPrintable(const GlobalState &gs) const;

    ClassOrModuleRef owner;
    NameRef name;
    ClassOrModuleRef rebind;
    TypePtr resultType;
    // All `IntrinsicMethod`s in sorbet should be statically-allocated, which is
    // why raw pointers are safe.
    const IntrinsicMethod *intrinsic = nullptr;
    ArgumentsStore arguments;
    InlinedVector<TypeArgumentRef, 4> typeArguments;

private:
    InlinedVector<Loc, 2> locs_;

public:
    Flags flags;
};
CheckSize(Method, 192, 8);

// Contains a field or a static field
class Field final {
    friend class serialize::SerializerImpl;

public:
    Field(const Field &) = delete;
    Field() = default;
    Field(Field &&) noexcept = default;

    class Flags {
    public:
        bool isField : 1;
        bool isStaticField : 1;

        // Static Field flags
        bool isStaticFieldTypeAlias : 1;
        bool isStaticFieldPrivate : 1;

        constexpr static uint8_t NUMBER_OF_FLAGS = 4;
        constexpr static uint8_t VALID_BITS_MASK = (1 << NUMBER_OF_FLAGS) - 1;

        Flags() noexcept
            : isField(false), isStaticField(false), isStaticFieldTypeAlias(false), isStaticFieldPrivate(false) {}

        uint8_t serialize() const {
            ENFORCE(sizeof(Flags) == sizeof(uint8_t));
            // Can replace this with std::bit_cast in C++20
            auto rawBits = *reinterpret_cast<const uint8_t *>(this);
            // We need to mask the valid bits since uninitialized memory isn't zeroed in C++.
            return rawBits & VALID_BITS_MASK;
        }
    };
    CheckSize(Flags, 1, 1);

    Loc loc() const;
    const InlinedVector<Loc, 2> &locs() const;
    void addLoc(const core::GlobalState &gs, core::Loc loc);

    uint32_t hash(const GlobalState &gs) const;

    void sanityCheck(const GlobalState &gs) const;

    Field deepCopy(const GlobalState &to) const;

    bool isPrintable(const GlobalState &gs) const;

    FieldRef ref(const GlobalState &gs) const;

    SymbolRef dealias(const GlobalState &gs, int depthLimit = 42) const;

    NameRef name;
    ClassOrModuleRef owner;
    TypePtr resultType;

private:
    InlinedVector<Loc, 2> locs_;

public:
    Flags flags;
};
CheckSize(Field, 64, 8);

class TypeParameter final {
    friend class serialize::SerializerImpl;

public:
    TypeParameter(const TypeParameter &) = delete;
    TypeParameter() = default;
    TypeParameter(TypeParameter &&) noexcept = default;

    class Flags {
    public:
        bool isTypeArgument : 1;
        bool isTypeMember : 1;

        // Type flags
        bool isCovariant : 1;
        bool isInvariant : 1;
        bool isContravariant : 1;
        bool isFixed : 1;

        Flags() noexcept
            : isTypeArgument(false), isTypeMember(false), isCovariant(false), isInvariant(false),
              isContravariant(false), isFixed(false) {}

        uint8_t serialize() const {
            // Can replace this with std::bit_cast in C++20
            return *reinterpret_cast<const uint8_t *>(this);
        }

        bool hasFlags(const Flags other) {
            if (other.isTypeArgument && !this->isTypeArgument) {
                return false;
            }

            if (other.isTypeMember && !this->isTypeMember) {
                return false;
            }

            if (other.isCovariant && !this->isCovariant) {
                return false;
            }

            if (other.isInvariant && !this->isInvariant) {
                return false;
            }

            if (other.isContravariant && !this->isContravariant) {
                return false;
            }

            if (other.isFixed && !this->isFixed) {
                return false;
            }

            return true;
        }
    };
    CheckSize(Flags, 1, 1);

    Loc loc() const;
    const InlinedVector<Loc, 2> &locs() const;
    void addLoc(const core::GlobalState &gs, core::Loc loc);

    uint32_t hash(const GlobalState &gs) const;

    bool isPrintable(const GlobalState &gs) const;

    SymbolRef ref(const GlobalState &gs) const;

    Variance variance() const {
        if (flags.isInvariant) {
            return Variance::Invariant;
        }
        if (flags.isCovariant) {
            return Variance::CoVariant;
        }
        if (flags.isContravariant) {
            return Variance::ContraVariant;
        }
        Exception::raise("Should not happen");
    }

    SymbolRef dealias(const GlobalState &gs, int depthLimit = 42) const;

    void sanityCheck(const GlobalState &gs) const;

    TypeParameter deepCopy(const GlobalState &gs) const;

    Flags flags;
    // Method for TypeArgument, ClassOrModule for TypeMember.
    SymbolRef owner;
    NameRef name;
    TypePtr resultType;

private:
    InlinedVector<Loc, 2> locs_;
};

class Symbol final {
public:
    Symbol(const Symbol &) = delete;
    Symbol() = default;
    Symbol(Symbol &&) noexcept = default;

    class Flags {
    public:
        static constexpr uint32_t NONE = 0;

        // We're packing three different kinds of flags into separate ranges with the uint32_t's below:
        //
        // 0x0000'0000
        //   ├▶    ◀┤└─ Applies to all types of symbol
        //   │      │
        //   │      └─ For our current symbol type, what flags does it have?
        //   │         (New flags grow up towards MSB)
        //   │
        //   └─ What type of symbol is this?
        //      (New flags grow down towards LSB)
        //

        // --- What type of symbol is this? ---
        static constexpr uint32_t CLASS_OR_MODULE = 0x8000'0000;

        // --- Applies to all types of Symbols ---

        // Synthesized by C++ code in a Rewriter pass
        static constexpr uint32_t REWRITER_SYNTHESIZED = 0x0000'0001;

        // --- For our current symbol type, what flags does it have?

        // Class flags
        static constexpr uint32_t CLASS_OR_MODULE_CLASS = 0x0000'0010;
        static constexpr uint32_t CLASS_OR_MODULE_MODULE = 0x0000'0020;
        static constexpr uint32_t CLASS_OR_MODULE_ABSTRACT = 0x0000'0040;
        static constexpr uint32_t CLASS_OR_MODULE_INTERFACE = 0x0000'0080;
        static constexpr uint32_t CLASS_OR_MODULE_LINEARIZATION_COMPUTED = 0x0000'0100;
        static constexpr uint32_t CLASS_OR_MODULE_FINAL = 0x0000'0200;
        static constexpr uint32_t CLASS_OR_MODULE_SEALED = 0x0000'0400;
        static constexpr uint32_t CLASS_OR_MODULE_PRIVATE = 0x0000'0800;
        static constexpr uint32_t CLASS_OR_MODULE_UNDECLARED = 0x0000'1000;
    };

    Loc loc() const;
    const InlinedVector<Loc, 2> &locs() const;
    void addLoc(const core::GlobalState &gs, core::Loc loc);

    uint32_t hash(const GlobalState &gs) const;

    std::vector<TypePtr> selfTypeArgs(const GlobalState &gs) const;

    // selfType and externalType return the type of an instance of this Symbol
    // (which must be isClassOrModule()), if instantiated without specific type
    // parameters, as seen from inside or outside of the class, respectively.
    TypePtr selfType(const GlobalState &gs) const;
    TypePtr externalType() const;

    // !! THREAD UNSAFE !! operation that computes the external type of this symbol.
    // Do not call this method from multi-threaded contexts (which, honestly, shouldn't
    // have access to a mutable GlobalState and thus shouldn't be able to call it).
    TypePtr unsafeComputeExternalType(GlobalState &gs);

    inline InlinedVector<ClassOrModuleRef, 4> &mixins() {
        ENFORCE_NO_TIMER(isClassOrModule());
        return mixins_;
    }

    inline const InlinedVector<ClassOrModuleRef, 4> &mixins() const {
        ENFORCE_NO_TIMER(isClassOrModule());
        return mixins_;
    }

    // Attempts to add the given mixin to the symbol. If the mixin is invalid because it is not a module, it returns
    // `false` (but still adds the mixin for processing during linearization) and the caller should report an error.
    [[nodiscard]] bool addMixin(const GlobalState &gs, ClassOrModuleRef sym,
                                std::optional<uint16_t> index = std::nullopt);

    // Add a placeholder for a mixin and return index in mixins()
    uint16_t addMixinPlaceholder(const GlobalState &gs);

    inline InlinedVector<TypeMemberRef, 4> &typeMembers() {
        ENFORCE(isClassOrModule());
        return typeParams;
    }

    inline const InlinedVector<TypeMemberRef, 4> &typeMembers() const {
        ENFORCE(isClassOrModule());
        return typeParams;
    }

    // Return the number of type parameters that must be passed to instantiate
    // this generic type. May differ from typeMembers.size() if some type
    // members have fixed values.
    int typeArity(const GlobalState &gs) const;

    bool derivesFrom(const GlobalState &gs, ClassOrModuleRef sym) const;

    // TODO(dmitry) perf: most calls to this method could be eliminated as part of perf work.
    SymbolRef ref(const GlobalState &gs) const;

    inline bool isClassOrModule() const {
        return (flags & Symbol::Flags::CLASS_OR_MODULE) != 0;
    }

    bool isSingletonClass(const GlobalState &gs) const;

    inline bool isClassOrModuleModule() const {
        ENFORCE_NO_TIMER(isClassOrModule());
        if (flags & Symbol::Flags::CLASS_OR_MODULE_MODULE) {
            return true;
        }
        if (flags & Symbol::Flags::CLASS_OR_MODULE_CLASS) {
            return false;
        }
        Exception::raise("Should never happen");
    }

    inline bool isClassModuleSet() const {
        ENFORCE_NO_TIMER(isClassOrModule());
        return flags & (Symbol::Flags::CLASS_OR_MODULE_MODULE | Symbol::Flags::CLASS_OR_MODULE_CLASS);
    }

    inline bool isClassOrModuleClass() const {
        return !isClassOrModuleModule();
    }

    inline bool isClassOrModuleAbstract() const {
        ENFORCE(isClassOrModule());
        return (flags & Symbol::Flags::CLASS_OR_MODULE_ABSTRACT) != 0;
    }

    inline bool isClassOrModuleInterface() const {
        ENFORCE_NO_TIMER(isClassOrModule());
        return (flags & Symbol::Flags::CLASS_OR_MODULE_INTERFACE) != 0;
    }

    inline bool isClassOrModuleLinearizationComputed() const {
        ENFORCE_NO_TIMER(isClassOrModule());
        return (flags & Symbol::Flags::CLASS_OR_MODULE_LINEARIZATION_COMPUTED) != 0;
    }

    inline bool isClassOrModuleFinal() const {
        ENFORCE_NO_TIMER(isClassOrModule());
        return (flags & Symbol::Flags::CLASS_OR_MODULE_FINAL) != 0;
    }

    inline bool isClassOrModuleSealed() const {
        ENFORCE_NO_TIMER(isClassOrModule());
        return (flags & Symbol::Flags::CLASS_OR_MODULE_SEALED) != 0;
    }

    inline bool isClassOrModulePrivate() const {
        ENFORCE_NO_TIMER(isClassOrModule());
        return (flags & Symbol::Flags::CLASS_OR_MODULE_PRIVATE) != 0;
    }

    inline bool isClassOrModuleUndeclared() const {
        ENFORCE_NO_TIMER(isClassOrModule());

        // Represents "filler definitions", which are not explicitly declared. Pre-resolver, these have a class/module
        // type, but neither class or module flag should be set. Post-resolver, we will use the "undeclared" bit to
        // identify them (this is useful for LSP fast-path checks).

        return ((flags & (Symbol::Flags::CLASS_OR_MODULE_MODULE | Symbol::Flags::CLASS_OR_MODULE_CLASS)) == 0) ||
               ((flags & Symbol::Flags::CLASS_OR_MODULE_UNDECLARED) != 0);
    }

    inline void setClassOrModule() {
        flags |= Symbol::Flags::CLASS_OR_MODULE;
    }

    inline void setIsModule(bool isModule) {
        ENFORCE(isClassOrModule());
        if (isModule) {
            ENFORCE((flags & Symbol::Flags::CLASS_OR_MODULE_CLASS) == 0);
            flags |= Symbol::Flags::CLASS_OR_MODULE_MODULE;
        } else {
            ENFORCE((flags & Symbol::Flags::CLASS_OR_MODULE_MODULE) == 0);
            flags |= Symbol::Flags::CLASS_OR_MODULE_CLASS;
        }
    }

    inline void setClassModuleUndeclared() {
        ENFORCE(isClassOrModule());
        flags |= Symbol::Flags::CLASS_OR_MODULE_UNDECLARED;
    }

    inline void setClassOrModuleAbstract() {
        ENFORCE(isClassOrModule());
        flags |= Symbol::Flags::CLASS_OR_MODULE_ABSTRACT;
    }

    inline void setClassOrModuleInterface() {
        ENFORCE(isClassOrModule());
        flags |= Symbol::Flags::CLASS_OR_MODULE_INTERFACE;
    }

    inline void setClassOrModuleLinearizationComputed() {
        ENFORCE(isClassOrModule());
        flags |= Symbol::Flags::CLASS_OR_MODULE_LINEARIZATION_COMPUTED;
    }

    inline void setClassOrModuleFinal() {
        ENFORCE(isClassOrModule());
        flags |= Symbol::Flags::CLASS_OR_MODULE_FINAL;
    }

    inline void setClassOrModuleSealed() {
        ENFORCE(isClassOrModule());
        flags |= Symbol::Flags::CLASS_OR_MODULE_SEALED;
    }

    inline void setClassOrModulePrivate() {
        ENFORCE(isClassOrModule());
        flags |= Symbol::Flags::CLASS_OR_MODULE_PRIVATE;
    }

    inline void setRewriterSynthesized() {
        flags |= Symbol::Flags::REWRITER_SYNTHESIZED;
    }
    inline bool isRewriterSynthesized() {
        return (flags & Symbol::Flags::REWRITER_SYNTHESIZED) != 0;
    }

    SymbolRef findMember(const GlobalState &gs, NameRef name) const;
    MethodRef findMethod(const GlobalState &gs, NameRef name) const;
    SymbolRef findMemberNoDealias(const GlobalState &gs, NameRef name) const;
    MethodRef findMethodNoDealias(const GlobalState &gs, NameRef name) const;
    SymbolRef findMemberTransitive(const GlobalState &gs, NameRef name) const;
    MethodRef findMethodTransitive(const GlobalState &gs, NameRef name) const;
    MethodRef findConcreteMethodTransitive(const GlobalState &gs, NameRef name) const;

    /* transitively finds a member with the most similar name */

    struct FuzzySearchResult {
        SymbolRef symbol;
        NameRef name;
        int distance;
    };

    std::vector<FuzzySearchResult> findMemberFuzzyMatch(const GlobalState &gs, NameRef name, int betterThan = -1) const;

    // Returns true if the symbol or any of its children are not in the symbol table. False otherwise.
    bool isPrintable(const GlobalState &gs) const;

    // Returns the singleton class for this class, lazily instantiating it if it
    // doesn't exist.
    ClassOrModuleRef singletonClass(GlobalState &gs);

    // Returns the singleton class or noSymbol
    ClassOrModuleRef lookupSingletonClass(const GlobalState &gs) const;

    // Returns attached class or noSymbol if it does not exist
    ClassOrModuleRef attachedClass(const GlobalState &gs) const;

    ClassOrModuleRef topAttachedClass(const GlobalState &gs) const;

    void recordSealedSubclass(MutableContext ctx, ClassOrModuleRef subclass);

    // Returns the locations that are allowed to subclass the sealed class.
    const InlinedVector<Loc, 2> &sealedLocs(const GlobalState &gs) const;

    TypePtr sealedSubclassesToUnion(const GlobalState &ctx) const;

    bool hasSingleSealedSubclass(const GlobalState &ctx) const;

    // Record a required ancestor for this class of module
    void recordRequiredAncestor(GlobalState &gs, ClassOrModuleRef ancestor, Loc loc);

    // Associate a required ancestor with the loc it's required at
    struct RequiredAncestor {
        ClassOrModuleRef origin; // The class or module that required `symbol`
        ClassOrModuleRef symbol; // The symbol required
        Loc loc;                 // The location it was required at

        RequiredAncestor(ClassOrModuleRef origin, ClassOrModuleRef symbol, Loc loc)
            : origin(origin), symbol(symbol), loc(loc) {}
    };

    void computeRequiredAncestorLinearization(GlobalState &gs);

    // Locally required ancestors by this class or module
    std::vector<RequiredAncestor> requiredAncestors(const GlobalState &gs) const;

    // All required ancestors by this class or module
    std::vector<RequiredAncestor> requiredAncestorsTransitive(const GlobalState &gs) const;

    // if dealiasing fails here, then we return Untyped instead
    SymbolRef dealias(const GlobalState &gs, int depthLimit = 42) const;

    bool ignoreInHashing(const GlobalState &gs) const;

    SymbolRef owner;
    ClassOrModuleRef superClass_;

    inline ClassOrModuleRef superClass() const {
        ENFORCE_NO_TIMER(isClassOrModule());
        return superClass_;
    }

    inline void setSuperClass(ClassOrModuleRef claz) {
        ENFORCE(isClassOrModule());
        superClass_ = claz;
    }

    uint32_t flags = Flags::NONE;
    NameRef name; // todo: move out? it should not matter but it's important for name resolution
    TypePtr resultType;

    UnorderedMap<NameRef, SymbolRef> members_;

    UnorderedMap<NameRef, SymbolRef> &members() {
        return members_;
    };
    const UnorderedMap<NameRef, SymbolRef> &members() const {
        return members_;
    };

    std::vector<std::pair<NameRef, SymbolRef>> membersStableOrderSlow(const GlobalState &gs) const;

    Symbol deepCopy(const GlobalState &to, bool keepGsId = false) const;
    void sanityCheck(const GlobalState &gs) const;

private:
    friend class serialize::SerializerImpl;
    friend class GlobalState;

    FuzzySearchResult findMemberFuzzyMatchUTF8(const GlobalState &gs, NameRef name, int betterThan = -1) const;
    std::vector<FuzzySearchResult> findMemberFuzzyMatchConstant(const GlobalState &gs, NameRef name,
                                                                int betterThan = -1) const;

    /*
     * mixins and superclasses: `superClass` is *not* included in the
     *   `argumentsOrMixins` list. `superClass` may not exist even if
     *   `isClassOrModule()`, which implies that this symbol is either a module or one
     *   of our magic synthetic classes. During parsing+naming, `superClass ==
     *   todo()` iff every definition we've seen for this class has had an
     *   implicit superclass (`class Foo` with no `< Parent`); Once we hit
     *   Resolver::finalize(), these will be rewritten to `Object()`.
     */
    InlinedVector<ClassOrModuleRef, 4> mixins_;

    /** For Class or module - ordered type members of the class,
     */
    InlinedVector<TypeMemberRef, 4> typeParams;
    InlinedVector<Loc, 2> locs_;

    // Record a required ancestor for this class of module in a magic property
    void recordRequiredAncestorInternal(GlobalState &gs, RequiredAncestor &ancestor, NameRef prop);

    // Read required ancestors for this class of module from a magic property
    std::vector<RequiredAncestor> readRequiredAncestorsInternal(const GlobalState &gs, NameRef prop) const;

    std::vector<RequiredAncestor> requiredAncestorsTransitiveInternal(GlobalState &gs,
                                                                      std::vector<ClassOrModuleRef> &seen);

    SymbolRef findMemberTransitiveInternal(const GlobalState &gs, NameRef name, int maxDepth = 100) const;

    inline void unsetClassOrModuleLinearizationComputed() {
        ENFORCE(isClassOrModule());
        flags &= ~Symbol::Flags::CLASS_OR_MODULE_LINEARIZATION_COMPUTED;
    }

    void addMixinAt(ClassOrModuleRef sym, std::optional<uint16_t> index);
};
// CheckSize(Symbol, 144, 8); // This is under too much churn to be worth checking

} // namespace sorbet::core
#endif // SORBET_SYMBOLS_H
