#ifndef SORBET_TYPES_H
#define SORBET_TYPES_H

#include "absl/base/casts.h"
#include "absl/types/span.h"
#include "common/counters/Counters.h"
#include "core/Context.h"
#include "core/Error.h"
#include "core/ParsedArg.h"
#include "core/ShowOptions.h"
#include "core/SymbolRef.h"
#include "core/TypeConstraint.h"
#include <memory>
#include <optional>
#include <string>
namespace sorbet::core {
/** Dmitry: unlike in Dotty, those types are always dealiased. For now */
class AppliedType;
class IntrinsicMethod;
class TypeConstraint;
struct DispatchArgs;
struct DispatchComponent;
struct DispatchResult;
struct CallLocs;
class ClassOrModule;
class TypeVar;
class SendAndBlockLink;
class TypeAndOrigins;

class ArgInfo {
public:
    using ArgFlags = core::ParsedArg::ArgFlags;
    ArgFlags flags;
    NameRef name;
    // Stores the `.bind(...)` symbol if the `&blk` arg's type had one
    ClassOrModuleRef rebind;
    Loc loc;
    TypePtr type;

    /*
     * Return type as observed by the method
     * body that defined that argument
     */
    TypePtr argumentTypeAsSeenByImplementation(Context ctx, core::TypeConstraint &constr) const;

    bool isSyntheticBlockArgument() const;
    std::string toString(const GlobalState &gs) const;
    std::string show(const GlobalState &gs) const;
    std::string_view argumentName(const GlobalState &gs) const;
    ArgInfo(const ArgInfo &) = delete;
    ArgInfo() = default;
    ArgInfo(ArgInfo &&) noexcept = default;
    ArgInfo &operator=(ArgInfo &&) noexcept = default;
    ArgInfo deepCopy() const;
};
CheckSize(ArgInfo, 32, 8);

template <class T, class... Args> TypePtr make_type(Args &&...args) {
    static_assert(!TypePtr::TypeToIsInlined<T>::value, "Inlined types must specialize `make_type` for each combination "
                                                       "of argument types; is one specialization missing?");
    return TypePtr(TypePtr::TypeToTag<T>::value, new T(std::forward<Args>(args)...));
}

enum class UntypedMode {
    AlwaysCompatible = 1,
    AlwaysIncompatible = 2,
};

class Types final {
public:
    /** Greater lower bound: the widest type that is subtype of both t1 and t2 */
    static TypePtr all(const GlobalState &gs, const TypePtr &t1, const TypePtr &t2);

    /** Lower upper bound: the narrowest type that is super type of both t1 and t2 */
    static TypePtr any(const GlobalState &gs, const TypePtr &t1, const TypePtr &t2);

    /**
     * is every instance of  t1 an  instance of t2?
     *
     * The parameter `mode` controls whether or not `T.untyped` is
     * considered to be a super type or subtype of all other types */

    /**
     * The `errorDetailsCollector` parameter is used to pass additional details out of isSubType
     * about why subtyping failed, which can then be shown to the user. See ErrorSection::Collector
     * in core/Error.h for the API.
     *
     * If this call is going to be used to determine if an error should be shown, you should pass in
     * an instance of ErrorSection::Collector. Otherwise, you should pass in
     * ErrorSection::Collector::NO_OP, as passing an ErrorSection::Collector will slow down subtype
     * checking to collect the additional information.
     */
    template <class T>
    static bool isSubTypeUnderConstraint(const GlobalState &gs, TypeConstraint &constr, const TypePtr &t1,
                                         const TypePtr &t2, UntypedMode mode, T &errorDetailsCollector);

    /** is every instance of  t1 an  instance of t2 when not allowed to modify constraint */
    template <class T>
    static bool isSubType(const GlobalState &gs, const TypePtr &t1, const TypePtr &t2, T &errorDetailsCollector);
    /** is every instance of  t1 an  instance of t2 when not allowed to modify constraint */
    static bool isSubType(const GlobalState &gs, const TypePtr &t1, const TypePtr &t2) {
        return isSubType(gs, t1, t2, ErrorSection::Collector::NO_OP);
    };
    static bool equiv(const GlobalState &gs, const TypePtr &t1, const TypePtr &t2);
    template <class T>
    static bool equivUnderConstraint(const GlobalState &gs, TypeConstraint &constr, const TypePtr &t1,
                                     const TypePtr &t2, T &errorDetailsCollector);

    /** check that t1 <: t2, but do not consider `T.untyped` as super type or a subtype of all other types */
    static bool isAsSpecificAs(const GlobalState &gs, const TypePtr &t1, const TypePtr &t2);
    static bool equivNoUntyped(const GlobalState &gs, const TypePtr &t1, const TypePtr &t2);

    template <class T>
    static bool equivNoUntypedUnderConstraint(const GlobalState &gs, TypeConstraint &constr, const TypePtr &t1,
                                              const TypePtr &t2, T &errorDetailsCollector);

    static TypePtr top();
    static TypePtr bottom();
    static TypePtr nilClass();
    static TypePtr untyped(core::SymbolRef blame);
    static TypePtr untypedUntracked();
    static TypePtr void_();
    static TypePtr trueClass();
    static TypePtr falseClass();
    static TypePtr Integer();
    static TypePtr String();
    static TypePtr Symbol();
    static TypePtr Float();
    static TypePtr Boolean();
    static TypePtr Object();
    static TypePtr arrayOfUntyped(sorbet::core::SymbolRef blame);
    static TypePtr rangeOfUntyped(sorbet::core::SymbolRef blame);
    static TypePtr hashOfUntyped();
    static TypePtr hashOfUntyped(sorbet::core::SymbolRef blame);
    static TypePtr procClass();
    static TypePtr nilableProcClass();
    static TypePtr declBuilderForProcsSingletonClass();
    static TypePtr falsyTypes();
    static absl::Span<const ClassOrModuleRef> falsySymbols();
    static TypePtr todo();

    static TypePtr dropSubtypesOf(const GlobalState &gs, const TypePtr &from,
                                  absl::Span<const ClassOrModuleRef> klasses);
    static TypePtr approximateSubtract(const GlobalState &gs, const TypePtr &from, const TypePtr &what);
    static bool canBeTruthy(const GlobalState &gs, const TypePtr &what);
    static bool canBeFalsy(const GlobalState &gs, const TypePtr &what);
    enum class Combinator { OR, AND };

    static TypePtr resultTypeAsSeenFrom(const GlobalState &gs, const TypePtr &what, ClassOrModuleRef fromWhat,
                                        ClassOrModuleRef inWhat, const std::vector<TypePtr> &targs);

    static InlinedVector<TypeMemberRef, 4> alignBaseTypeArgs(const GlobalState &gs, ClassOrModuleRef what,
                                                             const std::vector<TypePtr> &targs, ClassOrModuleRef asIf);
    // Extract the return value type from a proc.
    static TypePtr getProcReturnType(const GlobalState &gs, const TypePtr &procType);
    static TypePtr instantiate(const GlobalState &gs, const TypePtr &what, absl::Span<const TypeMemberRef> params,
                               const std::vector<TypePtr> &targs);
    /** Replace all type variables in `what` with their instantiations.
     * Requires that `tc` has already been solved.
     */
    static TypePtr instantiate(const GlobalState &gs, const TypePtr &what, const TypeConstraint &tc);

    static TypePtr replaceSelfType(const GlobalState &gs, const TypePtr &what, const TypePtr &receiver);
    /** Get rid of type variables in `what` and return a type that we deem close enough to continue
     * typechecking. We should be careful to only used this type when we are trying to guess a type.
     * We should do proper instantiation and subtype test after we have guessed type variables with
     * tc.solve(). If the constraint has already been solved, use `instantiate` instead.
     */
    static TypePtr approximate(const GlobalState &gs, const TypePtr &what, const TypeConstraint &tc);
    static TypePtr dropLiteral(const GlobalState &gs, const TypePtr &tp);

    /** Internal implementation. You should probably use all(). */
    static TypePtr glb(const GlobalState &gs, const TypePtr &t1, const TypePtr &t2);
    /** Internal implementation. You should probably use any(). */
    static TypePtr lub(const GlobalState &gs, const TypePtr &t1, const TypePtr &t2);

    static TypePtr lubAll(const GlobalState &gs, const std::vector<TypePtr> &elements);
    static TypePtr arrayOf(const GlobalState &gs, const TypePtr &elem);
    static TypePtr rangeOf(const GlobalState &gs, const TypePtr &elem);
    static TypePtr hashOf(const GlobalState &gs, const TypePtr &elem);
    static TypePtr setOf(const TypePtr &elem);
    static TypePtr tClass(const TypePtr &attachedClass);
    static TypePtr dropNil(const GlobalState &gs, const TypePtr &from);

    /** Recursively replaces proxies with their underlying types */
    static TypePtr widen(const GlobalState &gs, const TypePtr &type);
    static std::optional<int> getProcArity(const AppliedType &type);

    /** Unwrap SelfTypeParam instances that belong to the given owner, to a bare LambdaParam */
    static TypePtr unwrapSelfTypeParam(Context ctx, const TypePtr &ty);

    // Given a type, return a SymbolRef for the Ruby class that has that type, or no symbol if no such class exists.
    // This is an internal method for implementing intrinsics. In the future we should make all updateKnowledge methods
    // be intrinsics so that this can become an anonymous helper function in calls.cc.
    static core::ClassOrModuleRef getRepresentedClass(const GlobalState &gs, const core::TypePtr &ty);

    /**
     * unwrapType is used to take an expression that's parsed at the value-level,
     * and turn it into a type. For example, consider the following two expressions:
     *
     * > Integer.sqrt 10
     * > T::Array[Integer].new
     *
     * In both lines, `Integer` is initially resolved as the singleton class of
     * `Integer`. This is because it's not immediately clear if we want to refer
     * to the type `Integer` or if we want the singleton class of Integer for
     * calling singleton methods. In the first line this was the correct choice, as
     * we're just invoking the singleton method `sqrt`. In the second case we need
     * to fix up the `Integer` sub-expression, and turn it back into the type of
     * integer values. This is what `unwrapType` does, it turns the value-level
     * expression back into a type-level one.
     */
    static TypePtr unwrapType(const GlobalState &gs, Loc loc, const TypePtr &tp);

    // Converts type syntax like `GenericClass[Arg0, Arg1]` into a TypePtr.
    //
    // Called both from type_syntax.cc during sig parsing and from infer after encountering
    // something that look like type syntax in a method body.
    static TypePtr applyTypeArguments(const GlobalState &gs, const CallLocs &locs, uint16_t numPosArgs,
                                      const InlinedVector<const TypeAndOrigins *, 2> &args,
                                      ClassOrModuleRef genericClass, bool inResolver);
};

struct Intrinsic {
    enum class Kind : uint8_t {
        Instance = 1,
        Singleton = 2,
    };
    const ClassOrModuleRef symbol;
    const Kind singleton;
    const NameRef method;
    const IntrinsicMethod *impl;
};
absl::Span<const Intrinsic> intrinsicMethods();
using IntrinsicMethodsDispatchMap = UnorderedMap<NameRef, const std::vector<NameRef>>;
const IntrinsicMethodsDispatchMap &intrinsicMethodsDispatchMap();

template <class To> bool isa_type(const TypePtr &what) {
    return what != nullptr && what.tag() == TypePtr::TypeToTag<To>::value;
}

// Specializations to handle the class hierarchy.
class ClassType;
template <> inline bool isa_type<ClassType>(const TypePtr &what) {
    if (what == nullptr) {
        return false;
    }
    switch (what.tag()) {
        case TypePtr::Tag::ClassType:
        case TypePtr::Tag::BlamedUntyped:
        case TypePtr::Tag::UnresolvedClassType:
        case TypePtr::Tag::UnresolvedAppliedType:
            return true;
        default:
            return false;
    }
}

inline bool is_ground_type(const TypePtr &what) {
    if (what == nullptr) {
        return false;
    }
    switch (what.tag()) {
        case TypePtr::Tag::ClassType:
        case TypePtr::Tag::BlamedUntyped:
        case TypePtr::Tag::UnresolvedClassType:
        case TypePtr::Tag::UnresolvedAppliedType:
        case TypePtr::Tag::OrType:
        case TypePtr::Tag::AndType:
            return true;
        case TypePtr::Tag::NamedLiteralType:
        case TypePtr::Tag::IntegerLiteralType:
        case TypePtr::Tag::FloatLiteralType:
        case TypePtr::Tag::ShapeType:
        case TypePtr::Tag::TupleType:
        case TypePtr::Tag::MetaType:
        case TypePtr::Tag::LambdaParam:
        case TypePtr::Tag::SelfTypeParam:
        case TypePtr::Tag::SelfType:
        case TypePtr::Tag::AliasType:
        case TypePtr::Tag::AppliedType:
        case TypePtr::Tag::TypeVar:
            return false;
    }
}

inline bool is_proxy_type(const TypePtr &what) {
    if (what == nullptr) {
        return false;
    }
    switch (what.tag()) {
        case TypePtr::Tag::NamedLiteralType:
        case TypePtr::Tag::IntegerLiteralType:
        case TypePtr::Tag::FloatLiteralType:
        case TypePtr::Tag::ShapeType:
        case TypePtr::Tag::TupleType:
        case TypePtr::Tag::MetaType:
            return true;
        case TypePtr::Tag::ClassType:
        case TypePtr::Tag::BlamedUntyped:
        case TypePtr::Tag::UnresolvedClassType:
        case TypePtr::Tag::UnresolvedAppliedType:
        case TypePtr::Tag::OrType:
        case TypePtr::Tag::AndType:
        case TypePtr::Tag::LambdaParam:
        case TypePtr::Tag::SelfTypeParam:
        case TypePtr::Tag::SelfType:
        case TypePtr::Tag::AliasType:
        case TypePtr::Tag::AppliedType:
        case TypePtr::Tag::TypeVar:
            return false;
    }
}

template <class To> To const *cast_type(const TypePtr &what) {
    static_assert(TypePtr::TypeToIsInlined<To>::value == false,
                  "Cast inlined type objects with `cast_type_nonnull`, and use `isa_type` to check if the TypePtr "
                  "contains the type you expect.");
    if (isa_type<To>(what)) {
        return reinterpret_cast<To const *>(what.get());
    } else {
        return nullptr;
    }
}

template <class To> To *cast_type(TypePtr &what) {
    // const To* -> To* to avoid reimplementing the same logic twice.
    return const_cast<To *>(cast_type<To>(static_cast<const TypePtr &>(what)));
}

template <class To> To *cast_type(TypePtr &&what) = delete;

template <class To>
typename TypePtr::TypeToCastType<To, TypePtr::TypeToIsInlined<To>::value>::type cast_type_nonnull(const TypePtr &what) {
    ENFORCE_NO_TIMER(isa_type<To>(what));
    return *reinterpret_cast<const To *>(what.get());
}

template <class To>
typename TypePtr::TypeToCastType<To, TypePtr::TypeToIsInlined<To>::value>::type
cast_type_nonnull(TypePtr &&what) = delete;

// Simple forwarders defined on TypePtr which make `typecase` work.
template <class To> inline bool TypePtr::isa(const TypePtr &what) {
    return isa_type<To>(what);
}

template <class To>
inline typename TypePtr::TypeToCastType<To, TypePtr::TypeToIsInlined<To>::value>::type
TypePtr::cast(const TypePtr &what) {
    return cast_type_nonnull<To>(what);
}

template <> inline bool TypePtr::isa<TypePtr>(const TypePtr &what) {
    return true;
}

// Required to support cast<TypePtr> specialization.
template <> struct TypePtr::TypeToIsInlined<TypePtr> {
    static constexpr bool value = false;
};

template <> inline TypePtr const &TypePtr::cast<TypePtr>(const TypePtr &what) {
    return what;
}

#define TYPE_IMPL(name, isInlined)                                \
    class name;                                                   \
    template <> struct TypePtr::TypeToTag<name> {                 \
        static constexpr TypePtr::Tag value = TypePtr::Tag::name; \
    };                                                            \
    template <> struct TypePtr::TypeToIsInlined<name> {           \
        static constexpr bool value = isInlined;                  \
    };                                                            \
    class __attribute__((aligned(8))) name

#define TYPE(name) TYPE_IMPL(name, false)

#define TYPE_INLINED(name) TYPE_IMPL(name, true)

TYPE_INLINED(ClassType) {
public:
    // `const` because this is an inlined type; the symbol is inlined into the TypePtr. The TypePtr must be modified
    // to update the type.
    const ClassOrModuleRef symbol;
    ClassType(ClassOrModuleRef symbol);

    std::string toStringWithTabs(const GlobalState &gs, int tabs = 0) const;
    std::string show(const GlobalState &gs) const {
        return show(gs, {});
    };
    std::string show(const GlobalState &gs, ShowOptions options) const;
    uint32_t hash(const GlobalState &gs) const;
    DispatchResult dispatchCall(const GlobalState &gs, const DispatchArgs &args) const;

    TypePtr getCallArguments(const GlobalState &gs, NameRef name) const;
    bool derivesFrom(const GlobalState &gs, ClassOrModuleRef klass) const;
    void _sanityCheck(const GlobalState &gs) const;
};
CheckSize(ClassType, 8, 8);

template <> inline TypePtr make_type<ClassType, core::ClassOrModuleRef>(core::ClassOrModuleRef &&ref) {
    return TypePtr(TypePtr::Tag::ClassType, ref.id());
}

template <> inline TypePtr make_type<ClassType, core::ClassOrModuleRef &>(core::ClassOrModuleRef &ref) {
    return TypePtr(TypePtr::Tag::ClassType, ref.id());
}

template <> inline ClassType cast_type_nonnull<ClassType>(const TypePtr &what) {
    ENFORCE_NO_TIMER(isa_type<ClassType>(what));
    if (what.tag() == TypePtr::Tag::ClassType) {
        return ClassType(core::ClassOrModuleRef::fromRaw(what.inlinedValue()));
    } else {
        // Subclasses of ClassType contain untyped with metadata.
        return ClassType(core::Symbols::untyped());
    }
}

/*
 * This is the type used to represent a use of a type_member or type_template in
 * a signature.
 */
TYPE(LambdaParam) final : public Refcounted {
public:
    TypeMemberRef definition;

    // The type bounds provided in the definition of the type_member or
    // type_template.
    TypePtr lowerBound;
    TypePtr upperBound;

    LambdaParam(TypeMemberRef definition, TypePtr lower, TypePtr upper);
    LambdaParam(const LambdaParam &) = delete;
    LambdaParam &operator=(const LambdaParam &) = delete;
    std::string toStringWithTabs(const GlobalState &gs, int tabs = 0) const;
    std::string show(const GlobalState &gs) const {
        return show(gs, {});
    };
    std::string show(const GlobalState &gs, ShowOptions options) const;
    uint32_t hash(const GlobalState &gs) const;

    bool derivesFrom(const GlobalState &gs, ClassOrModuleRef klass) const;

    void _sanityCheck(const GlobalState &gs) const;

    TypePtr _instantiate(const GlobalState &gs, absl::Span<const TypeMemberRef> params,
                         const std::vector<TypePtr> &targs) const;
};
CheckSize(LambdaParam, 24, 8);

TYPE_INLINED(SelfTypeParam) final {
public:
    const SymbolRef definition;

    SelfTypeParam(const SymbolRef definition);
    std::string toStringWithTabs(const GlobalState &gs, int tabs = 0) const;
    std::string show(const GlobalState &gs) const {
        return show(gs, {});
    };
    std::string show(const GlobalState &gs, ShowOptions options) const;
    uint32_t hash(const GlobalState &gs) const;

    bool derivesFrom(const GlobalState &gs, ClassOrModuleRef klass) const;

    DispatchResult dispatchCall(const GlobalState &gs, const DispatchArgs &args) const;
    void _sanityCheck(const GlobalState &gs) const;
};
CheckSize(SelfTypeParam, 8, 8);

template <> inline TypePtr make_type<SelfTypeParam, core::SymbolRef &>(core::SymbolRef &definition) {
    return TypePtr(TypePtr::Tag::SelfTypeParam, definition.rawId());
}

template <> inline TypePtr make_type<SelfTypeParam, core::TypeArgumentRef &>(core::TypeArgumentRef &definition) {
    auto sym = SymbolRef(definition);
    return make_type<SelfTypeParam>(sym);
}

template <>
inline TypePtr make_type<SelfTypeParam, const core::TypeMemberRef &>(const core::TypeMemberRef &definition) {
    auto sym = SymbolRef(definition);
    return make_type<SelfTypeParam>(sym);
}

template <> inline SelfTypeParam cast_type_nonnull<SelfTypeParam>(const TypePtr &what) {
    ENFORCE_NO_TIMER(isa_type<SelfTypeParam>(what));
    return SelfTypeParam(core::SymbolRef::fromRaw(what.inlinedValue()));
}

TYPE_INLINED(AliasType) final {
public:
    AliasType(SymbolRef other);
    std::string toStringWithTabs(const GlobalState &gs, int tabs = 0) const;
    std::string show(const GlobalState &gs) const {
        return show(gs, {});
    };
    std::string show(const GlobalState &gs, ShowOptions options) const;
    uint32_t hash(const GlobalState &gs) const;
    bool derivesFrom(const GlobalState &gs, ClassOrModuleRef klass) const;

    const SymbolRef symbol;
    void _sanityCheck(const GlobalState &gs) const;
};
CheckSize(AliasType, 8, 8);

template <> inline TypePtr make_type<AliasType, core::SymbolRef &>(core::SymbolRef &other) {
    return TypePtr(TypePtr::Tag::AliasType, other.rawId());
}

template <> inline TypePtr make_type<AliasType, core::SymbolRef>(core::SymbolRef &&other) {
    return TypePtr(TypePtr::Tag::AliasType, other.rawId());
}

template <> inline AliasType cast_type_nonnull<AliasType>(const TypePtr &what) {
    ENFORCE_NO_TIMER(isa_type<AliasType>(what));
    return AliasType(core::SymbolRef::fromRaw(what.inlinedValue()));
}

/** This is a specific kind of self-type that should only be used in method return position.
 * It indicates that the method may(or will) return `self` or type that behaves equivalently
 * to self(e.g. in case of `.clone`).
 */
TYPE_INLINED(SelfType) final {
public:
    SelfType();
    std::string toStringWithTabs(const GlobalState &gs, int tabs = 0) const;
    std::string show(const GlobalState &gs) const {
        return show(gs, {});
    };
    std::string show(const GlobalState &gs, ShowOptions options) const;
    std::string showValue(const GlobalState &gs) const;
    uint32_t hash(const GlobalState &gs) const;

    TypePtr _replaceSelfType(const GlobalState &gs, const TypePtr &receiver) const;

    void _sanityCheck(const GlobalState &gs) const;
    bool derivesFrom(const GlobalState &gs, ClassOrModuleRef klass) const;
};
CheckSize(SelfType, 8, 8);

template <> inline TypePtr make_type<SelfType>() {
    // static_cast required to disambiguate TypePtr constructor.
    return TypePtr(TypePtr::Tag::SelfType, uint64_t(0));
}

template <> inline SelfType cast_type_nonnull<SelfType>(const TypePtr &what) {
    ENFORCE_NO_TIMER(isa_type<SelfType>(what));
    return SelfType();
}

TYPE_INLINED(NamedLiteralType) final {
    const NameRef name;

public:
    enum class LiteralTypeKind : uint8_t { String, Symbol };
    const LiteralTypeKind literalKind;
    NamedLiteralType(ClassOrModuleRef klass, NameRef val);
    TypePtr underlying(const GlobalState &gs) const;
    bool derivesFrom(const GlobalState &gs, ClassOrModuleRef klass) const;
    DispatchResult dispatchCall(const GlobalState &gs, const DispatchArgs &args) const;
    int64_t asInteger() const;
    core::NameRef asName() const;
    core::NameRef unsafeAsName() const;

    std::string toStringWithTabs(const GlobalState &gs, int tabs = 0) const;
    std::string show(const GlobalState &gs) const {
        return show(gs, {});
    };
    std::string show(const GlobalState &gs, ShowOptions options) const;
    std::string showValue(const GlobalState &gs) const;
    uint32_t hash(const GlobalState &gs) const;

    bool equals(const NamedLiteralType &rhs) const;
    void _sanityCheck(const GlobalState &gs) const;
};
CheckSize(NamedLiteralType, 8, 8);

// TODO(froydnj) it would be more work, but maybe it would be cleaner to split this
// type into distinct types, rather than doing this manual tagging.
template <>
inline TypePtr make_type<NamedLiteralType, ClassOrModuleRef, NameRef &>(ClassOrModuleRef &&klass, NameRef &val) {
    NamedLiteralType type(klass, val);
    return TypePtr(TypePtr::Tag::NamedLiteralType,
                   (uint64_t(val.rawId()) << 8) | static_cast<uint64_t>(type.literalKind));
}

template <>
inline TypePtr make_type<NamedLiteralType, ClassOrModuleRef, NameRef>(ClassOrModuleRef &&klass, NameRef &&val) {
    NamedLiteralType type(klass, val);
    return TypePtr(TypePtr::Tag::NamedLiteralType,
                   (uint64_t(val.rawId()) << 8) | static_cast<uint64_t>(type.literalKind));
}

template <> inline NamedLiteralType cast_type_nonnull<NamedLiteralType>(const TypePtr &what) {
    ENFORCE_NO_TIMER(isa_type<NamedLiteralType>(what));
    uint64_t tagged = what.inlinedValue();
    uint32_t id = static_cast<uint32_t>(tagged >> 8);
    auto literalKind = static_cast<NamedLiteralType::LiteralTypeKind>(tagged & 0xff);
    switch (literalKind) {
        case NamedLiteralType::LiteralTypeKind::String:
            return NamedLiteralType(Symbols::String(), NameRef::fromRawUnchecked(id));
        case NamedLiteralType::LiteralTypeKind::Symbol:
            return NamedLiteralType(Symbols::Symbol(), NameRef::fromRawUnchecked(id));
    }
}

TYPE(IntegerLiteralType) final : public Refcounted {
public:
    const int64_t value;

    IntegerLiteralType(int64_t val);
    TypePtr underlying(const GlobalState &gs) const;
    bool derivesFrom(const GlobalState &gs, ClassOrModuleRef klass) const;
    DispatchResult dispatchCall(const GlobalState &gs, const DispatchArgs &args) const;

    std::string toStringWithTabs(const GlobalState &gs, int tabs = 0) const;
    std::string show(const GlobalState &gs) const {
        return show(gs, {});
    };
    std::string show(const GlobalState &gs, ShowOptions options) const;
    std::string showValue(const GlobalState &gs) const;
    uint32_t hash(const GlobalState &gs) const;

    bool equals(const IntegerLiteralType &rhs) const;
    void _sanityCheck(const GlobalState &gs) const;
};
CheckSize(IntegerLiteralType, 16, 8);

template <> inline TypePtr make_type<IntegerLiteralType, int64_t>(int64_t &&val) {
    return make_type<IntegerLiteralType>(absl::bit_cast<uint64_t>(val));
}

template <> inline TypePtr make_type<IntegerLiteralType, long &>(long &val) {
    return make_type<IntegerLiteralType>(static_cast<int64_t>(val));
}

template <> inline TypePtr make_type<IntegerLiteralType, long long &>(long long &val) {
    return make_type<IntegerLiteralType>(static_cast<int64_t>(val));
}

TYPE(FloatLiteralType) final : public Refcounted {
public:
    const double value;

    FloatLiteralType(double val);
    TypePtr underlying(const GlobalState &gs) const;
    bool derivesFrom(const GlobalState &gs, ClassOrModuleRef klass) const;
    DispatchResult dispatchCall(const GlobalState &gs, const DispatchArgs &args) const;

    std::string toStringWithTabs(const GlobalState &gs, int tabs = 0) const;
    std::string show(const GlobalState &gs) const {
        return show(gs, {});
    };
    std::string show(const GlobalState &gs, ShowOptions options) const;
    std::string showValue(const GlobalState &gs) const;
    uint32_t hash(const GlobalState &gs) const;

    bool equals(const FloatLiteralType &rhs) const;
    void _sanityCheck(const GlobalState &gs) const;
};
CheckSize(FloatLiteralType, 16, 8);

template <> inline TypePtr make_type<FloatLiteralType, double &&>(double &&val) {
    return make_type<FloatLiteralType>(val);
}

template <> inline TypePtr make_type<FloatLiteralType, float &&>(float &&val) {
    return make_type<FloatLiteralType>(static_cast<double>(val));
}

/*
 * TypeVars are the used for the type parameters of generic methods.
 * Note: These are mutated post-construction and cannot be inlined.
 */
TYPE(TypeVar) final : public Refcounted {
public:
    TypeArgumentRef sym;
    TypeVar(TypeArgumentRef sym);
    TypeVar(const TypeVar &) = delete;
    TypeVar &operator=(const TypeVar &) = delete;
    std::string toStringWithTabs(const GlobalState &gs, int tabs = 0) const;
    std::string show(const GlobalState &gs) const {
        return show(gs, {});
    };
    std::string show(const GlobalState &gs, ShowOptions options) const;
    uint32_t hash(const GlobalState &gs) const;
    void _sanityCheck(const GlobalState &gs) const;

    bool derivesFrom(const GlobalState &gs, ClassOrModuleRef klass) const;

    TypePtr _approximate(const GlobalState &gs, const TypeConstraint &tc, core::Polarity polarity) const;
    TypePtr _instantiate(const GlobalState &gs, const TypeConstraint &tc) const;
};
CheckSize(TypeVar, 8, 8);

TYPE(OrType) final : public Refcounted {
public:
    TypePtr left;
    TypePtr right;

    OrType(const OrType &) = delete;
    OrType &operator=(const OrType &) = delete;
    std::string toStringWithTabs(const GlobalState &gs, int tabs = 0) const;
    std::string show(const GlobalState &gs) const {
        return show(gs, {});
    };
    std::string show(const GlobalState &gs, ShowOptions options) const;
    uint32_t hash(const GlobalState &gs) const;
    DispatchResult dispatchCall(const GlobalState &gs, const DispatchArgs &args) const;
    TypePtr getCallArguments(const GlobalState &gs, NameRef name) const;
    bool derivesFrom(const GlobalState &gs, ClassOrModuleRef klass) const;
    void _sanityCheck(const GlobalState &gs) const;
    TypePtr _instantiate(const GlobalState &gs, absl::Span<const TypeMemberRef> params,
                         const std::vector<TypePtr> &targs) const;
    TypePtr _approximate(const GlobalState &gs, const TypeConstraint &tc, core::Polarity polarity) const;
    TypePtr _instantiate(const GlobalState &gs, const TypeConstraint &tc) const;
    TypePtr _replaceSelfType(const GlobalState &gs, const TypePtr &receiver) const;

private:
    /*
     * You probably want Types::any() instead.
     */
    OrType(const TypePtr &left, const TypePtr &right);

    /*
     * These implementation methods are allowed to directly instantiate
     * `OrType`. Because `make_shared<OrType>` is not compatible with private
     * constructors + friend declarations (it's `shared_ptr` calling the
     * constructor, so the friend declaration doesn't work), we provide the
     * `make_shared` helper here.
     */
    friend TypePtr Types::nilableProcClass();
    friend TypePtr Types::falsyTypes();
    friend TypePtr Types::Boolean();
    friend class NameSubstitution;
    friend class serialize::SerializerImpl;
    template <class T>
    friend bool Types::isSubTypeUnderConstraint(const GlobalState &gs, TypeConstraint &constr, const TypePtr &t1,
                                                const TypePtr &t2, UntypedMode mode, T &errorDetailsCollector);
    friend TypePtr lubDistributeOr(const GlobalState &gs, const TypePtr &t1, const TypePtr &t2);
    friend TypePtr lubGround(const GlobalState &gs, const TypePtr &t1, const TypePtr &t2);
    friend TypePtr Types::lub(const GlobalState &gs, const TypePtr &t1, const TypePtr &t2);
    friend TypePtr Types::glb(const GlobalState &gs, const TypePtr &t1, const TypePtr &t2);
    friend TypePtr filterOrComponents(const TypePtr &originalType, const InlinedVector<TypePtr, 4> &typeFilter);
    friend TypePtr Types::dropSubtypesOf(const GlobalState &gs, const TypePtr &from,
                                         absl::Span<const ClassOrModuleRef> klasses);
    friend TypePtr Types::unwrapSelfTypeParam(Context ctx, const TypePtr &t1);
    friend class ClassOrModule; // the actual method is `recordSealedSubclass(Mutableconst GlobalState &gs, SymbolRef
                                // subclass)`, but referring to it introduces a cycle

    static TypePtr make_shared(const TypePtr &left, const TypePtr &right);
};
CheckSize(OrType, 24, 8);

TYPE(AndType) final : public Refcounted {
public:
    TypePtr left;
    TypePtr right;

    AndType(const AndType &) = delete;
    AndType &operator=(const AndType &) = delete;
    std::string toStringWithTabs(const GlobalState &gs, int tabs = 0) const;
    std::string show(const GlobalState &gs) const {
        return show(gs, {});
    };
    std::string show(const GlobalState &gs, ShowOptions options) const;
    uint32_t hash(const GlobalState &gs) const;
    DispatchResult dispatchCall(const GlobalState &gs, const DispatchArgs &args) const;

    TypePtr getCallArguments(const GlobalState &gs, NameRef name) const;
    bool derivesFrom(const GlobalState &gs, ClassOrModuleRef klass) const;
    void _sanityCheck(const GlobalState &gs) const;
    TypePtr _instantiate(const GlobalState &gs, absl::Span<const TypeMemberRef> params,
                         const std::vector<TypePtr> &targs) const;
    TypePtr _replaceSelfType(const GlobalState &gs, const TypePtr &receiver) const;
    TypePtr _approximate(const GlobalState &gs, const TypeConstraint &tc, core::Polarity polarity) const;
    TypePtr _instantiate(const GlobalState &gs, const TypeConstraint &tc) const;

private:
    /*
     * You probably want Types::all() instead.
     */
    AndType(const TypePtr &left, const TypePtr &right);

    friend class NameSubstitution;
    friend class serialize::SerializerImpl;
    friend class TypeConstraint;

    template <class T>
    friend bool Types::isSubTypeUnderConstraint(const GlobalState &gs, TypeConstraint &constr, const TypePtr &t1,
                                                const TypePtr &t2, UntypedMode mode, T &errorDetailsCollector);
    friend TypePtr lubGround(const GlobalState &gs, const TypePtr &t1, const TypePtr &t2);
    friend TypePtr glbDistributeAnd(const GlobalState &gs, const TypePtr &t1, const TypePtr &t2);
    friend TypePtr glbGround(const GlobalState &gs, const TypePtr &t1, const TypePtr &t2);
    friend TypePtr Types::lub(const GlobalState &gs, const TypePtr &t1, const TypePtr &t2);
    friend TypePtr Types::glb(const GlobalState &gs, const TypePtr &t1, const TypePtr &t2);
    friend TypePtr Types::unwrapSelfTypeParam(Context ctx, const TypePtr &t1);

    static TypePtr make_shared(const TypePtr &left, const TypePtr &right);
};
CheckSize(AndType, 24, 8);

TYPE(ShapeType) final : public Refcounted {
public:
    std::vector<TypePtr> keys; // TODO: store sorted by whatever
    std::vector<TypePtr> values;
    ShapeType(std::vector<TypePtr> keys, std::vector<TypePtr> values);
    ShapeType(const ShapeType &) = delete;
    ShapeType &operator=(const ShapeType &) = delete;

    std::string toStringWithTabs(const GlobalState &gs, int tabs = 0) const;
    std::string show(const GlobalState &gs) const {
        return show(gs, {});
    };
    std::string show(const GlobalState &gs, ShowOptions options) const;
    uint32_t hash(const GlobalState &gs) const;
    std::string showWithMoreInfo(const GlobalState &gs) const;
    DispatchResult dispatchCall(const GlobalState &gs, const DispatchArgs &args) const;
    void _sanityCheck(const GlobalState &gs) const;
    TypePtr _instantiate(const GlobalState &gs, absl::Span<const TypeMemberRef> params,
                         const std::vector<TypePtr> &targs) const;
    TypePtr _approximate(const GlobalState &gs, const TypeConstraint &tc, core::Polarity polarity) const;
    TypePtr _instantiate(const GlobalState &gs, const TypeConstraint &tc) const;
    TypePtr underlying(const GlobalState &gs) const;
    bool derivesFrom(const GlobalState &gs, core::ClassOrModuleRef klass) const;

    std::optional<size_t> indexForKey(const TypePtr &t) const;
    std::optional<size_t> indexForKey(const NamedLiteralType &lit) const;
    std::optional<size_t> indexForKey(const IntegerLiteralType &lit) const;
    std::optional<size_t> indexForKey(const FloatLiteralType &lit) const;
};
CheckSize(ShapeType, 56, 8);

TYPE(TupleType) final : public Refcounted {
private:
    TupleType() = delete;

public:
    std::vector<TypePtr> elems;

    TupleType(std::vector<TypePtr> elements);
    TupleType(const TupleType &) = delete;
    TupleType &operator=(const TupleType &) = delete;

    std::string toStringWithTabs(const GlobalState &gs, int tabs = 0) const;
    std::string show(const GlobalState &gs) const {
        return show(gs, {});
    };
    std::string show(const GlobalState &gs, ShowOptions options) const;
    std::string showWithMoreInfo(const GlobalState &gs) const;
    uint32_t hash(const GlobalState &gs) const;
    void _sanityCheck(const GlobalState &gs) const;
    TypePtr _instantiate(const GlobalState &gs, absl::Span<const TypeMemberRef> params,
                         const std::vector<TypePtr> &targs) const;
    DispatchResult dispatchCall(const GlobalState &gs, const DispatchArgs &args) const;
    TypePtr _approximate(const GlobalState &gs, const TypeConstraint &tc, core::Polarity polarity) const;
    TypePtr _instantiate(const GlobalState &gs, const TypeConstraint &tc) const;

    // Return the type of the underlying array that this tuple decays into
    TypePtr elementType(const GlobalState &gs) const;
    TypePtr underlying(const GlobalState &gs) const;
    bool derivesFrom(const GlobalState &gs, core::ClassOrModuleRef klass) const;
};
CheckSize(TupleType, 32, 8);

TYPE(AppliedType) final : public Refcounted {
public:
    ClassOrModuleRef klass;
    std::vector<TypePtr> targs;
    AppliedType(ClassOrModuleRef klass, std::vector<TypePtr> targs);
    AppliedType(const AppliedType &) = delete;
    AppliedType &operator=(const AppliedType &) = delete;

    std::string toStringWithTabs(const GlobalState &gs, int tabs = 0) const;
    std::string show(const GlobalState &gs) const {
        return show(gs, {});
    };
    std::string show(const GlobalState &gs, ShowOptions options) const;
    uint32_t hash(const GlobalState &gs) const;
    DispatchResult dispatchCall(const GlobalState &gs, const DispatchArgs &args) const;
    void _sanityCheck(const GlobalState &gs) const;
    TypePtr _instantiate(const GlobalState &gs, absl::Span<const TypeMemberRef> params,
                         const std::vector<TypePtr> &targs) const;

    TypePtr getCallArguments(const GlobalState &gs, NameRef name) const;

    bool derivesFrom(const GlobalState &gs, ClassOrModuleRef klass) const;
    TypePtr _approximate(const GlobalState &gs, const TypeConstraint &tc, core::Polarity polarity) const;
    TypePtr _instantiate(const GlobalState &gs, const TypeConstraint &tc) const;
};
CheckSize(AppliedType, 32, 8);

// MetaType is the type of a Type. You can think of it as generalization of
// Ruby's singleton classes to Types; Just as `A.singleton_class` is the *type*
// of the *value* `A`, MetaType[T] is the *type* of a *value* that holds the
// type T during execution. For instance, the type of `T.untyped` is
// `MetaType(Types::untyped())`.
//
// These are used within the inferencer in places where we need to track
// user-written types in the source code.
TYPE(MetaType) final : public Refcounted {
public:
    TypePtr wrapped;

    MetaType(const TypePtr &wrapped);
    MetaType(const MetaType &) = delete;
    MetaType &operator=(const MetaType &) = delete;

    std::string toStringWithTabs(const GlobalState &gs, int tabs = 0) const;
    std::string show(const GlobalState &gs) const {
        return show(gs, {});
    };
    std::string show(const GlobalState &gs, ShowOptions options) const;
    uint32_t hash(const GlobalState &gs) const;

    bool derivesFrom(const GlobalState &gs, ClassOrModuleRef klass) const;

    DispatchResult dispatchCall(const GlobalState &gs, const DispatchArgs &args) const;
    void _sanityCheck(const GlobalState &gs) const;

    TypePtr _approximate(const GlobalState &gs, const TypeConstraint &tc, core::Polarity polarity) const;
    TypePtr underlying(const GlobalState &gs) const;
};
CheckSize(MetaType, 16, 8);

class SendAndBlockLink {
    SendAndBlockLink(const SendAndBlockLink &) = default;

public:
    SendAndBlockLink(SendAndBlockLink &&) = default;
    std::vector<ArgInfo::ArgFlags> argFlags;
    core::NameRef fun;
    int rubyRegionId;
    std::shared_ptr<DispatchResult> result;

    SendAndBlockLink(NameRef fun, std::vector<ArgInfo::ArgFlags> &&argFlags, int rubyRegionId);
    std::optional<int> fixedArity() const;
    std::shared_ptr<SendAndBlockLink> duplicate();
};

class TypeAndOrigins final {
public:
    TypePtr type;
    InlinedVector<Loc, 1> origins;

    std::vector<ErrorLine> origins2Explanations(const GlobalState &gs, Loc originForUninitialized) const;

    static ErrorSection explainExpected(const GlobalState &gs, TypePtr type, Loc origin, const std::string &for_);
    ErrorSection explainExpected(const GlobalState &gs, const std::string &for_, Loc originForUninitialized) const;
    std::optional<ErrorSection> explainGot(const GlobalState &gs, Loc originForUninitialized) const;

    ~TypeAndOrigins() noexcept;
    TypeAndOrigins() = default;
    TypeAndOrigins(TypePtr type, Loc origin);
    TypeAndOrigins(TypePtr type, InlinedVector<Loc, 1> origins) : type(std::move(type)), origins(std::move(origins)) {}
    TypeAndOrigins(const TypeAndOrigins &) = default;
    TypeAndOrigins(TypeAndOrigins &&) = default;
    TypeAndOrigins &operator=(const TypeAndOrigins &) = default;
    TypeAndOrigins &operator=(TypeAndOrigins &&) = default;
};
CheckSize(TypeAndOrigins, 32, 8);

struct CallLocs final {
    FileRef file;
    LocOffsets call;
    LocOffsets receiver;
    LocOffsets fun;
    InlinedVector<LocOffsets, 2> &args;
};

struct DispatchArgs {
    /*
     * A note on selfType vs fullType: Because of proxies, AndType, and OrType,
     * dispatching a call can recurse into sub-components of an outer type. At
     * top level, when invoked from inference, they will match, and be the same
     * as `this`.
     *
     * As we recurse down into structured types, `fullType` continues to refer
     * to the outermost, top-level type, while `selfType` refers to the *most
     * specific* known type for `self` in this particular invocation. In
     * particular, "most specific" means that it will still carry the `AndType`s
     * and `ProxyType`s, even as we peel them off and call `dispatchCall` on
     * inner types.
     *
     * `fullType` should be exclusively used for generating error
     * messages. `selfType` is primarily used to implement `T.self_type`.
     */

    NameRef name;
    const CallLocs &locs;
    uint16_t numPosArgs;
    InlinedVector<const TypeAndOrigins *, 2> &args;
    const TypePtr &selfType;
    const TypeAndOrigins fullType;
    const TypePtr &thisType;
    const std::shared_ptr<const SendAndBlockLink> &block;
    Loc originForUninitialized;
    bool isPrivateOk;
    // Do not produce dispatch-related errors while evaluating the call. This is a performance optimization, as there
    // are cases where we call dispatchCall with no intention of showing the errors to the user. Producing those
    // unreported errors is expensive!
    bool suppressErrors;
    NameRef enclosingMethodForSuper;

    DispatchArgs(const DispatchArgs &) = delete;
    DispatchArgs &operator=(const DispatchArgs &) = delete;

    Loc callLoc() const {
        return core::Loc(locs.file, locs.call);
    }
    Loc receiverLoc() const {
        return core::Loc(locs.file, locs.receiver);
    }
    Loc funLoc() const {
        return core::Loc(locs.file, locs.fun);
    }
    Loc argLoc(size_t i) const {
        return core::Loc(locs.file, locs.args[i]);
    }
    Loc argsLoc() const {
        if (!locs.args.empty()) {
            return core::Loc(locs.file, locs.args.front().join(locs.args.back()));
        }

        if (this->block != nullptr) {
            if (locs.fun.exists()) {
                return funLoc().copyEndWithZeroLength();
            }
            return callLoc().copyEndWithZeroLength();
        }

        if (locs.fun.exists()) {
            return core::Loc(locs.file, locs.fun.endPos(), locs.call.endPos());
        }

        return callLoc().copyEndWithZeroLength();
    }
    Loc blockLoc(const GlobalState &gs) const;

    Loc errLoc() const {
        auto funLoc = this->funLoc();
        auto recvLoc = this->receiverLoc();
        if (funLoc.exists() && !funLoc.empty()) {
            return funLoc;
        } else if (this->name == Names::squareBrackets() && recvLoc.exists() && !recvLoc.empty()) {
            return core::Loc(this->locs.file, recvLoc.endPos(), this->callLoc().endPos());
        } else {
            return this->callLoc();
        }
    }

    DispatchArgs withSelfAndThisRef(const TypePtr &newSelfRef) const;
    DispatchArgs withThisRef(const TypePtr &newThisRef) const;
    DispatchArgs withErrorsSuppressed() const;
};

struct DispatchComponent {
    TypePtr receiver;
    MethodRef method;
    std::vector<std::unique_ptr<Error>> errors;
    TypePtr sendTp;
    TypePtr blockReturnType;
    TypePtr blockPreType;
    ClassOrModuleRef rebind;
    Loc rebindLoc;
    std::unique_ptr<TypeConstraint> constr;
};

struct DispatchResult {
    enum class Combinator { OR, AND };
    TypePtr returnType;
    DispatchComponent main;
    std::unique_ptr<DispatchResult> secondary;
    Combinator secondaryKind;

    DispatchResult() = default;
    DispatchResult(TypePtr returnType, TypePtr receiverType, core::MethodRef method)
        : returnType(returnType),
          main(DispatchComponent{
              std::move(receiverType), method, {}, std::move(returnType), nullptr, nullptr, {}, {}, nullptr}){};
    DispatchResult(TypePtr returnType, DispatchComponent comp)
        : returnType(std::move(returnType)), main(std::move(comp)){};
    DispatchResult(TypePtr returnType, DispatchComponent comp, std::unique_ptr<DispatchResult> secondary,
                   Combinator secondaryKind)
        : returnType(std::move(returnType)), main(std::move(comp)), secondary(std::move(secondary)),
          secondaryKind(secondaryKind){};

    // Combine two dispatch results, preferring the left as the `main`.
    static DispatchResult merge(const GlobalState &gs, Combinator kind, DispatchResult &&left, DispatchResult &&right);
};

TYPE_INLINED(BlamedUntyped) final : public ClassType {
public:
    const core::SymbolRef blame;
    BlamedUntyped(SymbolRef whoToBlame) : ClassType(core::Symbols::untyped()), blame(whoToBlame){};

    TypePtr getCallArguments(const GlobalState &gs, NameRef name) const;
};

template <> inline TypePtr make_type<BlamedUntyped, core::SymbolRef &>(core::SymbolRef &whoToBlame) {
    return TypePtr(TypePtr::Tag::BlamedUntyped, whoToBlame.rawId());
}

template <> inline BlamedUntyped cast_type_nonnull<BlamedUntyped>(const TypePtr &what) {
    ENFORCE_NO_TIMER(isa_type<BlamedUntyped>(what));
    return BlamedUntyped(core::SymbolRef::fromRaw(what.inlinedValue()));
}

TYPE(UnresolvedClassType) final : public Refcounted, public ClassType {
public:
    const core::SymbolRef scope;
    const std::vector<core::NameRef> names;
    UnresolvedClassType(SymbolRef scope, std::vector<core::NameRef> names)
        : ClassType(core::Symbols::untyped()), scope(scope), names(std::move(names)){};
    UnresolvedClassType(const UnresolvedClassType &) = delete;
    UnresolvedClassType &operator=(const UnresolvedClassType &) = delete;
    std::string toStringWithTabs(const GlobalState &gs, int tabs = 0) const;
    std::string show(const GlobalState &gs) const {
        return show(gs, {});
    };
    std::string show(const GlobalState &gs, ShowOptions options) const;
    uint32_t hash(const GlobalState &gs) const;
};

TYPE(UnresolvedAppliedType) final : public Refcounted, public ClassType {
public:
    const core::ClassOrModuleRef klass;
    const std::vector<TypePtr> targs;
    UnresolvedAppliedType(ClassOrModuleRef klass, std::vector<TypePtr> targs)
        : ClassType(core::Symbols::untyped()), klass(klass), targs(std::move(targs)){};
    UnresolvedAppliedType(const UnresolvedAppliedType &) = delete;
    UnresolvedAppliedType &operator=(const UnresolvedAppliedType &) = delete;
    std::string toStringWithTabs(const GlobalState &gs, int tabs = 0) const;
    std::string show(const GlobalState &gs) const {
        return show(gs, {});
    };
    std::string show(const GlobalState &gs, ShowOptions options) const;
    uint32_t hash(const GlobalState &gs) const;
};

} // namespace sorbet::core
#endif // SORBET_TYPES_H
