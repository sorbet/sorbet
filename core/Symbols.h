#ifndef SORBET_SYMBOLS_H
#define SORBET_SYMBOLS_H

#include "common/common.h"
#include "core/ArityHash.h"
#include "core/Loc.h"
#include "core/Names.h"
#include "core/Polarity.h"
#include "core/SymbolRef.h"
#include "core/Types.h"
#include "core/packages/MangledName.h"
#include <memory>
#include <tuple>
#include <vector>

namespace sorbet {
class Levenstein;
}

namespace sorbet::core {
class ClassOrModule;
class GlobalState;
struct LocalSymbolTableHashes;
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
    // A list of method names that this intrinsic dispatches to.
    // Used to ensure that calls involving intrinsics are correctly typechecked on the fast path.
    //
    // If your intrinsic does not dispatch to another method, the default implementation returns an
    // empty vector, so you can elide an implementation.
    //
    // If your intrinsic cannot declare the list of methods it might dispatch to (like
    // `<call-with-splat>`, etc.), you will have to edit Substitute.cc.
    virtual std::vector<NameRef> dispatchesTo() const;

    virtual void apply(const GlobalState &gs, const DispatchArgs &args, DispatchResult &res) const = 0;
};

enum class Visibility : uint8_t {
    Public = 1,
    Protected,
    Private,
};

class Method final {
    friend class ClassOrModule;
    friend class GlobalState;
    friend class serialize::SerializerImpl;

    // This is to allow updating `GlobalState::methods` in place with a new method,
    // over top of an existing method
    Method &operator=(Method &&) = default;

public:
    Method(const Method &) = delete;
    Method &operator=(const Method &) = delete;
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
        // It might have been nice to be able to separate out the kinds of incompatible overrides.
        // This boolean silences all override-related errors.
        bool allowIncompatibleOverrideAll : 1;
        bool allowIncompatibleOverrideVisibility : 1;
        bool isPackagePrivate : 1;

        constexpr static uint16_t NUMBER_OF_FLAGS = 12;
        constexpr static uint16_t VALID_BITS_MASK = (1 << NUMBER_OF_FLAGS) - 1;
        Flags() noexcept
            : isRewriterSynthesized(false), isProtected(false), isPrivate(false), isOverloaded(false),
              isAbstract(false), isGenericMethod(false), isOverridable(false), isFinal(false), isOverride(false),
              allowIncompatibleOverrideAll(false), allowIncompatibleOverrideVisibility(false), isPackagePrivate(false) {
        }

        uint16_t serialize() const {
            static_assert(sizeof(Flags) == sizeof(uint16_t));
            // Can replace this with std::bit_cast in C++20
            auto rawBits = *reinterpret_cast<const uint16_t *>(this);

            // We need to mask the valid bits since uninitialized memory isn't zeroed in C++.
            return rawBits & VALID_BITS_MASK;
        }
    };
    CheckSize(Flags, 2, 1);

    Loc loc() const;
    absl::Span<const Loc> locs() const;
    void addLoc(const core::GlobalState &gs, core::Loc loc);
    void removeLocsForFile(core::FileRef file);
    uint32_t hash(const GlobalState &gs) const;
    uint32_t methodShapeHash(const GlobalState &gs) const;
    ArityHash methodArityHash(const GlobalState &gs) const;

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

    using ParametersStore = InlinedVector<ParamInfo, core::SymbolRef::EXPECTED_METHOD_PARAMS_COUNT>;

    // if dealiasing fails here, then we return a bad alias method stub instead
    MethodRef dealiasMethod(const GlobalState &gs, int depthLimit = 42) const;

    // TODO(dmitry) perf: most calls to this method could be eliminated as part of perf work.
    MethodRef ref(const GlobalState &gs) const;

    Method deepCopy(const GlobalState &to) const;
    void sanityCheck(const GlobalState &gs) const;
    bool ignoreInHashing(const GlobalState &gs) const;
    bool isPrintable(const GlobalState &gs) const;

    // Equivalent to `getIntrinsic() != nullptr`, but potentially more efficient.
    bool hasIntrinsic() const;
    // All `IntrinsicMethod`s in sorbet should be statically allocated, which is
    // why raw pointers are safe.
    const IntrinsicMethod *getIntrinsic() const;

    ClassOrModuleRef owner;
    NameRef name;
    ClassOrModuleRef rebind;
    Flags flags;
    // We store an offset into the intrinsic table used by calls.cc; the only
    // wrinkle is that our offset here is the offset of the intrinsic + 1, so
    // zero represents "no intrinsic".  This storage system assumes we will
    // always have < 2^16 intrinsics, which ought to be enough for anybody.
    const static uint16_t INVALID_INTRINSIC_OFFSET = 0;
    const static uint16_t FIRST_VALID_INTRINSIC_OFFSET = 1;
    uint16_t intrinsicOffset = INVALID_INTRINSIC_OFFSET;
    TypePtr resultType;
    ParametersStore parameters;

    InlinedVector<TypeParameterRef, 4> &getOrCreateTypeParameters() {
        if (typeParams) {
            return *typeParams;
        }
        typeParams = std::make_unique<InlinedVector<TypeParameterRef, 4>>();
        return *typeParams;
    }

    absl::Span<const TypeParameterRef> typeParameters() const {
        if (typeParams) {
            return *typeParams;
        }
        return {};
    }

    InlinedVector<TypeParameterRef, 4> &existingTypeParameters() {
        ENFORCE(typeParams != nullptr);
        return *typeParams;
    }

private:
    SymbolRef::LOC_store locs_;
    std::unique_ptr<InlinedVector<TypeParameterRef, 4>> typeParams;
};
CheckSize(Method, 136, 8);

// Contains a field or a static field
class Field final {
    friend class GlobalState;
    friend class serialize::SerializerImpl;

    // This is to allow updating `GlobalState::fields` in place with a new field, over top of an existing field
    Field &operator=(Field &&) = default;

public:
    Field(const Field &) = delete;
    Field &operator=(const Field &) = delete;
    Field() = default;
    Field(Field &&) noexcept = default;

    class Flags {
    public:
        bool isField : 1;
        bool isStaticField : 1;

        // Static Field flags
        bool isStaticFieldTypeAlias : 1;
        bool isStaticFieldPrivate : 1;

        // Can only export a static field
        bool isExported : 1;

        constexpr static uint8_t NUMBER_OF_FLAGS = 5;
        constexpr static uint8_t VALID_BITS_MASK = (1 << NUMBER_OF_FLAGS) - 1;

        Flags() noexcept
            : isField(false), isStaticField(false), isStaticFieldTypeAlias(false), isStaticFieldPrivate(false),
              isExported(false) {}

        uint8_t serialize() const {
            static_assert(sizeof(Flags) == sizeof(uint8_t));
            // Can replace this with std::bit_cast in C++20
            auto rawBits = *reinterpret_cast<const uint8_t *>(this);
            // We need to mask the valid bits since uninitialized memory isn't zeroed in C++.
            return rawBits & VALID_BITS_MASK;
        }
    };
    CheckSize(Flags, 1, 1);

    Loc loc() const;
    absl::Span<const Loc> locs() const;
    void addLoc(const core::GlobalState &gs, core::Loc loc);
    void removeLocsForFile(core::FileRef file);

    bool isClassAlias() const;

    uint32_t hash(const GlobalState &gs) const;
    uint32_t fieldShapeHash(const GlobalState &gs) const;

    void sanityCheck(const GlobalState &gs) const;

    Field deepCopy(const GlobalState &to) const;

    bool isPrintable(const GlobalState &gs) const;

    FieldRef ref(const GlobalState &gs) const;

    SymbolRef dealias(const GlobalState &gs, int depthLimit = 42) const;

    NameRef name;
    ClassOrModuleRef owner;
    TypePtr resultType;

private:
    SymbolRef::LOC_store locs_;

public:
    Flags flags;
};
CheckSize(Field, 56, 8);

class TypeParameter final {
    friend class GlobalState;
    friend class serialize::SerializerImpl;

    // This is to allow updating `GlobalState::typeArguments` in place with a new type argument,
    // over top of an existing method
    TypeParameter &operator=(TypeParameter &&) = default;

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

        constexpr static uint8_t NUMBER_OF_FLAGS = 6;
        constexpr static uint8_t VALID_BITS_MASK = (1 << NUMBER_OF_FLAGS) - 1;

        Flags() noexcept
            : isTypeArgument(false), isTypeMember(false), isCovariant(false), isInvariant(false),
              isContravariant(false), isFixed(false) {}

        uint8_t serialize() const {
            static_assert(sizeof(Flags) == sizeof(uint8_t));
            // Can replace this with std::bit_cast in C++20
            auto rawBits = *reinterpret_cast<const uint8_t *>(this);
            // Mask the valid bits since uninitialized bits can be any value.
            return rawBits & VALID_BITS_MASK;
        }

        bool hasFlags(const Flags other) {
            const auto otherSerialized = other.serialize();
            return (this->serialize() & otherSerialized) == otherSerialized;
        }
    };
    CheckSize(Flags, 1, 1);

    Loc loc() const;
    absl::Span<const Loc> locs() const;
    void addLoc(const core::GlobalState &gs, core::Loc loc);
    void removeLocsForFile(core::FileRef file);

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
        Exception::raise("None of the variance-related flags are set. Call to variance() before resolver?");
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
    SymbolRef::LOC_store locs_;
};
CheckSize(TypeParameter, 56, 8);

class ClassOrModule final {
public:
    ClassOrModule(const ClassOrModule &) = delete;
    ClassOrModule() = default;
    ClassOrModule(ClassOrModule &&) noexcept = default;

    class Flags {
    public:
        bool isClass : 1;
        bool isModule : 1;
        bool isAbstract : 1;
        bool isInterface : 1;
        bool isLinearizationComputed : 1;
        bool isFinal : 1;
        bool isSealed : 1;
        bool isPrivate : 1;
        bool isDeclared : 1;
        bool isExported : 1;
        bool isBehaviorDefining : 1;

        constexpr static uint16_t NUMBER_OF_FLAGS = 11;
        constexpr static uint16_t VALID_BITS_MASK = (1 << NUMBER_OF_FLAGS) - 1;

        Flags() noexcept
            : isClass(false), isModule(false), isAbstract(false), isInterface(false), isLinearizationComputed(false),
              isFinal(false), isSealed(false), isPrivate(false), isDeclared(false), isExported(false),
              isBehaviorDefining(false) {}

        uint16_t serialize() const {
            static_assert(sizeof(Flags) == sizeof(uint16_t));
            // Can replace this with std::bit_cast in C++20
            auto rawBits = *reinterpret_cast<const uint16_t *>(this);
            // Mask the valid bits since uninitialized bits can be any value.
            return rawBits & VALID_BITS_MASK;
        }
    };
    CheckSize(Flags, 2, 1);

    Loc loc() const;
    absl::Span<const Loc> locs() const;
    void addLoc(const core::GlobalState &gs, core::Loc loc);
    void removeLocsForFile(core::FileRef file);

    uint32_t hash(const GlobalState &gs, bool includeTypeMemberNames) const;
    uint32_t classOrModuleShapeHash(const GlobalState &gs) const {
        auto skipTypeMemberNames = true;
        return hash(gs, skipTypeMemberNames);
    }

    std::vector<TypePtr> selfTypeArgs(const GlobalState &gs) const;

    // selfType and externalType return the type of an instance of this Symbol
    // if instantiated without specific type parameters, as seen from inside or
    // outside of the class, respectively.
    TypePtr selfType(const GlobalState &gs) const;
    TypePtr externalType() const;

    // !! THREAD UNSAFE !! operation that computes the external type of this symbol.
    // Do not call this method from multi-threaded contexts (which, honestly, shouldn't
    // have access to a mutable GlobalState and thus shouldn't be able to call it).
    TypePtr unsafeComputeExternalType(GlobalState &gs);

    inline InlinedVector<ClassOrModuleRef, 4> &mixins() {
        return mixins_;
    }

    inline const InlinedVector<ClassOrModuleRef, 4> &mixins() const {
        return mixins_;
    }

    // Attempts to add the given mixin to the symbol. If the mixin is invalid because it is not a module, it returns
    // `false` (but still adds the mixin for processing during linearization) and the caller should report an error.
    [[nodiscard]] bool addMixin(const GlobalState &gs, ClassOrModuleRef sym,
                                std::optional<uint16_t> index = std::nullopt);

    // Add a placeholder for a mixin and return index in mixins()
    uint16_t addMixinPlaceholder(const GlobalState &gs);

    inline InlinedVector<TypeMemberRef, 4> &getOrCreateTypeMembers() {
        if (typeParams) {
            return *typeParams;
        }
        typeParams = std::make_unique<InlinedVector<TypeMemberRef, 4>>();
        return *typeParams;
    }

    inline absl::Span<const TypeMemberRef> typeMembers() const {
        if (typeParams) {
            return *typeParams;
        }
        return {};
    }

    inline InlinedVector<TypeMemberRef, 4> &existingTypeMembers() {
        ENFORCE(typeParams != nullptr);
        return *typeParams;
    }

    // Return the number of type parameters that must be passed to instantiate
    // this generic type. May differ from typeMembers.size() if some type
    // members have fixed values.
    int typeArity(const GlobalState &gs) const;

    bool derivesFrom(const GlobalState &gs, ClassOrModuleRef sym) const;

    // TODO(dmitry) perf: most calls to this method could be eliminated as part of perf work.
    ClassOrModuleRef ref(const GlobalState &gs) const;

    bool isSingletonClass(const GlobalState &gs) const;

    inline bool isModule() const {
        if (flags.isModule) {
            return true;
        }

        if (flags.isClass) {
            return false;
        }

        Exception::raise("isModule() but neither flags.isModule nor flags.isClass. Call to isModule before resolver?");
    }

    inline bool isClassModuleSet() const {
        return flags.isModule || flags.isClass;
    }

    inline bool isClass() const {
        return !isModule();
    }

    inline void setIsModule(bool isModule) {
        if (isModule) {
            ENFORCE(!flags.isClass);
            flags.isModule = true;
        } else {
            ENFORCE(!flags.isModule);
            flags.isClass = true;
        }
    }

    inline bool isDeclared() const {
        return flags.isDeclared;
    }

    inline void setDeclared() {
        ENFORCE(isClassModuleSet());

        if (!flags.isDeclared) {
            flags.isDeclared = true;
        }
    }

    inline void unsetClassOrModuleLinearizationComputed() {
        flags.isLinearizationComputed = false;
    }

    SymbolRef findMember(const GlobalState &gs, NameRef name) const;
    MethodRef findMethod(const GlobalState &gs, NameRef name) const;
    SymbolRef findMemberNoDealias(NameRef name) const;
    MethodRef findMethodNoDealias(NameRef name) const;
    SymbolRef findMemberTransitive(const GlobalState &gs, NameRef name) const;
    SymbolRef findMemberTransitiveNoDealias(const GlobalState &gs, NameRef name) const;
    MethodRef findMethodTransitive(const GlobalState &gs, NameRef name) const;
    // A version of findMemberTransitive that skips looking in the members of the current symbol,
    // instead looking only in the members of any parent.
    MethodRef findParentMethodTransitive(const GlobalState &gs, NameRef name) const;
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

    void recordSealedSubclass(GlobalState &gs, ClassOrModuleRef subclass);

    // Returns the locations that are allowed to subclass the sealed class.
    absl::Span<const Loc> sealedLocs(const GlobalState &gs) const;

    TypePtr sealedSubclassesToUnion(const GlobalState &gs) const;

    bool hasSingleSealedSubclass(const GlobalState &gs) const;

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

    // A link to the corresponding spot in the `<PackageSpecRegistry>` hierarchy.
    //
    // Might correspond to an intermediate `<PackageSpecRegistry>` namespace (not a package), but at least
    // will always correspond to the tightest possible namespace in the `<PackageSpecRegistry>` subtree.
    //
    // Only set if `gs.packageDB().enabled()`
    //
    // - Given ::<root>, contains ::<PackageSpecRegistry>
    // - Given ::Opus::MyPkg, contains ::<PackageSpecRegistry>::Opus::MyPkg
    // - Given ::Opus::MyPkg::Foo, contains ::<PackageSpecRegistry>::Opus::MyPkg::Foo
    // - Given ::Opus::MyPkg::Foo::InnerPkg, contains ::<PackageSpecRegistry>::Opus::MyPkg::Foo::InnerPkg
    // - Given ::Test, contains ::<PackageSpecRegistry>
    // - Given ::Test::Opus::MyPkg, contains ::<PackageSpecRegistry>::Opus::MyPkg
    //
    // When set to `::<none>`, inherit whatever our owner has for `package`.
    ClassOrModuleRef packageRegistryOwner = core::Symbols::PackageSpecRegistry();

    // The package that this symbol belongs to.
    //
    // Only set if `gs.packageDB().enabled()`
    //
    // If `!package.exists()`, then this symbol is "unpackaged."
    // This is the case for all definitions in Sorbet's payload.
    //
    // TODO(jez): If someone creates a `__package.rb` for something in the standard library after
    // the payload has already loaded, we won't retroactively determine the package for those
    // constants, which could cause problems. Probably we should ban that? Because e.g. we won't
    // have checked the implementation of the payload RBI files against whatever constraints the
    // __package.rb declares (e.g. imports).
    //
    // TODO(jez) Ban defining a package called `Test`
    packages::MangledName package;

    // The class or module that this class or module is nested inside of.
    //
    // Given `::A::B`, the owner is `::A`
    // Given `::A`, the owner is `::<root>`
    // Given `::<root>`, the owner is `::<root>`
    ClassOrModuleRef owner;

    ClassOrModuleRef superClass_;

    inline ClassOrModuleRef superClass() const {
        return superClass_;
    }

    inline void setSuperClass(ClassOrModuleRef klass) {
        superClass_ = klass;
    }

    Flags flags;
    NameRef name; // todo: move out? it should not matter but it's important for name resolution
    TypePtr resultType;

    UnorderedMap<NameRef, SymbolRef> members_;

    UnorderedMap<NameRef, SymbolRef> &members() {
        return members_;
    };
    const UnorderedMap<NameRef, SymbolRef> &members() const {
        return members_;
    };

    template <typename P>
    std::vector<std::pair<NameRef, SymbolRef>> membersStableOrderSlowPredicate(const GlobalState &gs,
                                                                               P predicate) const {
        std::vector<std::pair<NameRef, SymbolRef>> result;

        for (const auto &e : this->members()) {
            if (predicate(e.first, e.second)) {
                result.emplace_back(e);
            }
        }

        sortMembersStableOrder(gs, result);

        return result;
    }

    std::vector<std::pair<NameRef, SymbolRef>> membersStableOrderSlow(const GlobalState &gs) const;

    ClassOrModule deepCopy(const GlobalState &to, bool keepGsId = false) const;
    void sanityCheck(const GlobalState &gs) const;

private:
    static void sortMembersStableOrder(const GlobalState &gs, std::vector<std::pair<NameRef, SymbolRef>> &out);

    friend class serialize::SerializerImpl;
    friend class GlobalState;

    FuzzySearchResult findMemberFuzzyMatchUTF8(const GlobalState &gs, NameRef name, Levenstein &levenstein,
                                               int betterThan = -1) const;
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
    std::unique_ptr<InlinedVector<TypeMemberRef, 4>> typeParams;
    SymbolRef::LOC_store locs_;

    // Record a required ancestor for this class of module in a magic property
    void recordRequiredAncestorInternal(GlobalState &gs, RequiredAncestor &ancestor, NameRef prop);

    // Read required ancestors for this class of module from a magic property
    std::vector<RequiredAncestor> readRequiredAncestorsInternal(const GlobalState &gs, NameRef prop) const;

    std::vector<RequiredAncestor> requiredAncestorsTransitiveInternal(GlobalState &gs,
                                                                      std::vector<ClassOrModuleRef> &seen);

    SymbolRef findMemberTransitiveInternal(const GlobalState &gs, NameRef name, int maxDepth, bool dealias) const;
    SymbolRef findParentMemberTransitiveInternal(const GlobalState &gs, NameRef name, int maxDepth, bool dealias) const;

    void addMixinAt(ClassOrModuleRef sym, std::optional<uint16_t> index);
};
CheckSize(ClassOrModule, 128, 8);

} // namespace sorbet::core
#endif // SORBET_SYMBOLS_H
