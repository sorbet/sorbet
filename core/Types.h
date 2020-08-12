#ifndef SORBET_TYPES_H
#define SORBET_TYPES_H

#include "common/Counters.h"
#include "core/Context.h"
#include "core/Error.h"
#include "core/SymbolRef.h"
#include "core/TypeConstraint.h"
#include <memory>
#include <optional>
#include <string>
namespace sorbet::core {
/** Dmitry: unlike in Dotty, those types are always dealiased. For now */
class Type;
class AppliedType;
class IntrinsicMethod;
class TypeConstraint;
struct DispatchArgs;
struct DispatchComponent;
struct DispatchResult;
struct CallLocs;
class Symbol;
class TypeVar;
class SendAndBlockLink;
class TypeAndOrigins;

class ArgInfo {
public:
    struct ArgFlags {
        bool isKeyword = false;
        bool isRepeated = false;
        bool isDefault = false;
        bool isShadow = false;
        bool isBlock = false;

    private:
        friend class Symbol;
        friend class serialize::SerializerImpl;
        void setFromU1(u1 flags);
        u1 toU1() const;
    };
    ArgFlags flags;
    NameRef name;
    SymbolRef rebind;
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
    std::string argumentName(const GlobalState &gs) const;
    ArgInfo(const ArgInfo &) = delete;
    ArgInfo() = default;
    ArgInfo(ArgInfo &&) noexcept = default;
    ArgInfo &operator=(ArgInfo &&) noexcept = default;
    ArgInfo deepCopy() const;
};
CheckSize(ArgInfo, 48, 8);

template <class T, class... Args> TypePtr make_type(Args &&... args) {
    return TypePtr(std::make_shared<T>(std::forward<Args>(args)...));
}

enum class UntypedMode {
    AlwaysCompatible = 1,
    AlwaysIncompatible = 2,
};

class Types final {
public:
    /** Greater lower bound: the widest type that is subtype of both t1 and t2 */
    static TypePtr all(const GlobalState &gs, const TypePtr &t1, const TypePtr &t2);

    /** Lower upper bound: the narrowest type that is supper type of both t1 and t2 */
    static TypePtr any(const GlobalState &gs, const TypePtr &t1, const TypePtr &t2);

    /**
     * is every instance of  t1 an  instance of t2?
     *
     * The parameter `mode` controls whether or not `T.untyped` is
     * considered to be a super type or subtype of all other types */
    static bool isSubTypeUnderConstraint(const GlobalState &gs, TypeConstraint &constr, const TypePtr &t1,
                                         const TypePtr &t2, UntypedMode mode);

    /** is every instance of  t1 an  instance of t2 when not allowed to modify constraint */
    static bool isSubType(const GlobalState &gs, const TypePtr &t1, const TypePtr &t2);
    static bool equiv(const GlobalState &gs, const TypePtr &t1, const TypePtr &t2);

    /** check that t1 <: t2, but do not consider `T.untyped` as super type or a subtype of all other types */
    static bool isAsSpecificAs(const GlobalState &gs, const TypePtr &t1, const TypePtr &t2);
    static bool equivNoUntyped(const GlobalState &gs, const TypePtr &t1, const TypePtr &t2);

    static TypePtr top();
    static TypePtr bottom();
    static TypePtr nilClass();
    static TypePtr untyped(const core::GlobalState &gs, core::SymbolRef blame);
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
    static TypePtr arrayOfUntyped();
    static TypePtr rangeOfUntyped();
    static TypePtr hashOfUntyped();
    static TypePtr procClass();
    static TypePtr classClass();
    static TypePtr declBuilderForProcsSingletonClass();
    static TypePtr falsyTypes();

    static TypePtr dropSubtypesOf(const GlobalState &gs, const TypePtr &from, SymbolRef klass);
    static TypePtr approximateSubtract(const GlobalState &gs, const TypePtr &from, const TypePtr &what);
    static bool canBeTruthy(const GlobalState &gs, const TypePtr &what);
    static bool canBeFalsy(const GlobalState &gs, const TypePtr &what);
    enum class Combinator { OR, AND };

    static TypePtr resultTypeAsSeenFrom(const GlobalState &gs, TypePtr what, SymbolRef fromWhat, SymbolRef inWhat,
                                        const std::vector<TypePtr> &targs);

    static InlinedVector<SymbolRef, 4> alignBaseTypeArgs(const GlobalState &gs, SymbolRef what,
                                                         const std::vector<TypePtr> &targs, SymbolRef asIf);
    // Extract the return value type from a proc.
    static TypePtr getProcReturnType(const GlobalState &gs, const TypePtr &procType);
    static TypePtr instantiate(const GlobalState &gs, const TypePtr &what, const InlinedVector<SymbolRef, 4> &params,
                               const std::vector<TypePtr> &targs);
    /** Replace all type variables in `what` with their instantiations.
     * Requires that `tc` has already been solved.
     */
    static TypePtr instantiate(const GlobalState &gs, const TypePtr &what, const TypeConstraint &tc);

    static TypePtr replaceSelfType(const GlobalState &gs, const TypePtr &what, const TypePtr &receiver);
    /** Get rid of type variables in `what` and return a type that we deem close enough to continue
     * typechecking. We should be careful to only used this type when we are trying to guess a type.
     * We should do proper instatiation and subtype test after we have guessed type variables with
     * tc.solve(). If the constraint has already been solved, use `instantiate` instead.
     */
    static TypePtr approximate(const GlobalState &gs, const TypePtr &what, const TypeConstraint &tc);
    static TypePtr dispatchCallWithoutBlock(const GlobalState &gs, const TypePtr &recv, DispatchArgs args);
    static TypePtr dropLiteral(const TypePtr &tp);

    /** Internal implementation. You should probably use all(). */
    static TypePtr glb(const GlobalState &gs, const TypePtr &t1, const TypePtr &t2);
    /** Internal implementation. You should probably use any(). */
    static TypePtr lub(const GlobalState &gs, const TypePtr &t1, const TypePtr &t2);

    static TypePtr lubAll(const GlobalState &gs, std::vector<TypePtr> &elements);
    static TypePtr arrayOf(const GlobalState &gs, const TypePtr &elem);
    static TypePtr rangeOf(const GlobalState &gs, const TypePtr &elem);
    static TypePtr hashOf(const GlobalState &gs, const TypePtr &elem);
    static TypePtr dropNil(const GlobalState &gs, const TypePtr &from);

    /** Recursively replaces proxies with their underlying types */
    static TypePtr widen(const GlobalState &gs, const TypePtr &type);
    static std::optional<int> getProcArity(const AppliedType &type);

    /** Unwrap SelfTypeParam instances that belong to the given owner, to a bare LambdaParam */
    static TypePtr unwrapSelfTypeParam(Context ctx, const TypePtr &ty);

    // Given a type, return a SymbolRef for the Ruby class that has that type, or no symbol if no such class exists.
    // This is an internal method for implementing intrinsics. In the future we should make all updateKnowledge methods
    // be intrinsics so that this can become an anonymous helper function in calls.cc.
    static core::SymbolRef getRepresentedClass(const GlobalState &gs, const core::Type *ty);
};

struct Intrinsic {
    enum class Kind : u1 {
        Instance = 1,
        Singleton = 2,
    };
    const SymbolRef symbol;
    const Kind singleton;
    const NameRef method;
    const IntrinsicMethod *impl;
};
extern const std::vector<Intrinsic> intrinsicMethods;

class Type {
public:
    Type() = default;
    Type(const Type &obj) = delete;
    virtual ~Type() = default;
    // Internal printer.
    virtual std::string toStringWithTabs(const GlobalState &gs, int tabs = 0) const = 0;
    std::string toString(const GlobalState &gs) const {
        return toStringWithTabs(gs);
    }
    // User visible type. Should exactly match what the user can write.
    virtual std::string show(const GlobalState &gs) const = 0;
    // Like show, but can include extra info. Does not necessarily match what the user can type.
    virtual std::string showWithMoreInfo(const GlobalState &gs) const {
        return show(gs);
    }

    virtual std::string typeName() const = 0;
    virtual TypePtr _instantiate(const GlobalState &gs, const InlinedVector<SymbolRef, 4> &params,
                                 const std::vector<TypePtr> &targs) = 0;
    virtual TypePtr _instantiate(const GlobalState &gs, const TypeConstraint &tc);

    virtual TypePtr _replaceSelfType(const GlobalState &gs, const TypePtr &receiver);

    virtual DispatchResult dispatchCall(const GlobalState &gs, DispatchArgs args) = 0;
    virtual TypePtr getCallArguments(const GlobalState &gs, NameRef name) = 0;
    virtual bool derivesFrom(const GlobalState &gs, SymbolRef klass) const = 0;
    virtual void _sanityCheck(const GlobalState &gs) = 0;
    void sanityCheck(const GlobalState &gs) {
        if (!debug_mode)
            return;
        _sanityCheck(gs);
    }

    bool isUntyped() const;
    bool isNilClass() const;
    core::SymbolRef untypedBlame() const;
    bool isBottom() const;
    virtual bool hasUntyped();
    virtual bool isFullyDefined() = 0;
    virtual int kind() = 0;
    virtual TypePtr _approximate(const GlobalState &gs, const TypeConstraint &tc);
    unsigned int hash(const GlobalState &gs) const;
};
CheckSize(Type, 8, 8);

template <class To> To *cast_type(Type *what) {
    static_assert(!std::is_pointer<To>::value, "To has to be a pointer");
    static_assert(std::is_assignable<Type *&, To *>::value, "Ill Formed To, has to be a subclass of Type");
    return fast_cast<Type, To>(what);
}

template <class To> const To *cast_type(const Type *what) {
    static_assert(!std::is_pointer<To>::value, "To has to be a pointer");
    static_assert(std::is_assignable<Type *&, To *>::value, "Ill Formed To, has to be a subclass of Type");
    return fast_cast<const Type, const To>(what);
}

template <class To> bool isa_type(Type *what) {
    return cast_type<To>(what) != nullptr;
}

class GroundType : public Type {};

class ProxyType : public Type {
public:
    // TODO: use shared pointers that use inline counter
    virtual TypePtr underlying() const = 0;
    ProxyType() = default;

    virtual DispatchResult dispatchCall(const GlobalState &gs, DispatchArgs args) override;
    virtual TypePtr getCallArguments(const GlobalState &gs, NameRef name) override;
    virtual bool derivesFrom(const GlobalState &gs, SymbolRef klass) const override;

    void _sanityCheck(const GlobalState &gs) override;
};
CheckSize(ProxyType, 8, 8);

class ClassType : public GroundType {
public:
    SymbolRef symbol;
    ClassType(SymbolRef symbol);
    virtual int kind() final;

    virtual std::string toStringWithTabs(const GlobalState &gs, int tabs = 0) const override;
    virtual std::string show(const GlobalState &gs) const override;
    virtual std::string typeName() const override;
    virtual DispatchResult dispatchCall(const GlobalState &gs, DispatchArgs args) override final;

    virtual TypePtr getCallArguments(const GlobalState &gs, NameRef name) final;
    virtual bool derivesFrom(const GlobalState &gs, SymbolRef klass) const final;
    void _sanityCheck(const GlobalState &gs) final;
    virtual TypePtr _instantiate(const GlobalState &gs, const InlinedVector<SymbolRef, 4> &params,
                                 const std::vector<TypePtr> &targs) override final;
    virtual bool isFullyDefined() final;
    virtual bool hasUntyped() override final;
};
CheckSize(ClassType, 16, 8);

/*
 * This is the type used to represent a use of a type_member or type_template in
 * a signature.
 */
class LambdaParam final : public Type {
public:
    SymbolRef definition;

    // The type bounds provided in the definition of the type_member or
    // type_template.
    TypePtr lowerBound;
    TypePtr upperBound;

    LambdaParam(SymbolRef definition, TypePtr lower, TypePtr upper);
    virtual std::string toStringWithTabs(const GlobalState &gs, int tabs = 0) const final;
    virtual std::string show(const GlobalState &gs) const final;
    virtual std::string typeName() const final;

    virtual bool derivesFrom(const GlobalState &gs, SymbolRef klass) const final;

    virtual DispatchResult dispatchCall(const GlobalState &gs, DispatchArgs args) final;
    virtual TypePtr getCallArguments(const GlobalState &gs, NameRef name) final;
    void _sanityCheck(const GlobalState &gs) final;
    virtual bool isFullyDefined() final;

    virtual TypePtr _instantiate(const GlobalState &gs, const InlinedVector<SymbolRef, 4> &params,
                                 const std::vector<TypePtr> &targs) override;
    virtual int kind() final;
};
CheckSize(LambdaParam, 48, 8);

class SelfTypeParam final : public Type {
public:
    SymbolRef definition;

    SelfTypeParam(const SymbolRef definition);
    virtual std::string toStringWithTabs(const GlobalState &gs, int tabs = 0) const final;
    virtual std::string show(const GlobalState &gs) const final;
    virtual std::string typeName() const final;

    virtual bool derivesFrom(const GlobalState &gs, SymbolRef klass) const final;

    virtual DispatchResult dispatchCall(const GlobalState &gs, DispatchArgs args) final;
    virtual TypePtr getCallArguments(const GlobalState &gs, NameRef name) final;
    void _sanityCheck(const GlobalState &gs) final;
    virtual bool isFullyDefined() final;

    virtual TypePtr _instantiate(const GlobalState &gs, const InlinedVector<SymbolRef, 4> &params,
                                 const std::vector<TypePtr> &targs) override;
    virtual int kind() final;
};
CheckSize(SelfTypeParam, 16, 8);

class AliasType final : public Type {
public:
    AliasType(SymbolRef other);
    virtual std::string toStringWithTabs(const GlobalState &gs, int tabs = 0) const final;
    virtual std::string show(const GlobalState &gs) const final;
    virtual std::string typeName() const final;
    virtual DispatchResult dispatchCall(const GlobalState &gs, DispatchArgs args) final;
    virtual TypePtr getCallArguments(const GlobalState &gs, NameRef name) final;
    virtual bool derivesFrom(const GlobalState &gs, SymbolRef klass) const final;

    SymbolRef symbol;
    void _sanityCheck(const GlobalState &gs) final;
    virtual bool isFullyDefined() final;

    virtual TypePtr _instantiate(const GlobalState &gs, const InlinedVector<SymbolRef, 4> &params,
                                 const std::vector<TypePtr> &targs) override;
    virtual int kind() final;
};
CheckSize(AliasType, 16, 8);

/** This is a specific kind of self-type that should only be used in method return position.
 * It indicates that the method may(or will) return `self` or type that behaves equivalently
 * to self(e.g. in case of `.clone`).
 */
class SelfType final : public Type {
public:
    SelfType();
    virtual std::string toStringWithTabs(const GlobalState &gs, int tabs = 0) const final;
    virtual std::string show(const GlobalState &gs) const final;
    virtual std::string showValue(const GlobalState &gs) const final;
    virtual std::string typeName() const override;
    virtual bool isFullyDefined() final;

    virtual TypePtr _instantiate(const GlobalState &gs, const InlinedVector<SymbolRef, 4> &params,
                                 const std::vector<TypePtr> &targs) override;
    virtual int kind() final;
    virtual TypePtr _replaceSelfType(const GlobalState &gs, const TypePtr &receiver) override;

    virtual DispatchResult dispatchCall(const GlobalState &gs, DispatchArgs args) final;
    void _sanityCheck(const GlobalState &gs) final;
    virtual TypePtr getCallArguments(const GlobalState &gs, NameRef name) final;
    virtual bool derivesFrom(const GlobalState &gs, SymbolRef klass) const final;
};
CheckSize(SelfType, 8, 8);

class LiteralType final : public ProxyType {
public:
    union {
        const int64_t value;
        const double floatval;
    };

    enum class LiteralTypeKind : u1 { Integer, String, Symbol, True, False, Float };
    const LiteralTypeKind literalKind;
    LiteralType(int64_t val);
    LiteralType(double val);
    LiteralType(SymbolRef klass, NameRef val);
    LiteralType(bool val);
    virtual TypePtr underlying() const override;

    virtual std::string toStringWithTabs(const GlobalState &gs, int tabs = 0) const final;
    virtual std::string show(const GlobalState &gs) const final;
    virtual std::string showValue(const GlobalState &gs) const final;
    virtual std::string typeName() const override;
    virtual bool isFullyDefined() final;

    bool equals(const LiteralType &rhs) const;

    virtual TypePtr _instantiate(const GlobalState &gs, const InlinedVector<SymbolRef, 4> &params,
                                 const std::vector<TypePtr> &targs) override;
    virtual int kind() final;
};
CheckSize(LiteralType, 24, 8);

/*
 * TypeVars are the used for the type parameters of generic methods.
 */
class TypeVar final : public Type {
public:
    SymbolRef sym;
    TypeVar(SymbolRef sym);
    virtual std::string toStringWithTabs(const GlobalState &gs, int tabs = 0) const final;
    virtual std::string show(const GlobalState &gs) const final;
    virtual std::string typeName() const final;
    virtual DispatchResult dispatchCall(const GlobalState &gs, DispatchArgs args) final;
    void _sanityCheck(const GlobalState &gs) final;
    virtual bool isFullyDefined() final;
    virtual TypePtr getCallArguments(const GlobalState &gs, NameRef name) final;
    virtual bool derivesFrom(const GlobalState &gs, SymbolRef klass) const final;

    virtual TypePtr _instantiate(const GlobalState &gs, const InlinedVector<SymbolRef, 4> &params,
                                 const std::vector<TypePtr> &targs) override;
    virtual int kind() final;
    virtual TypePtr _approximate(const GlobalState &gs, const TypeConstraint &tc) override;
    virtual TypePtr _instantiate(const GlobalState &gs, const TypeConstraint &tc) override;
};
CheckSize(TypeVar, 16, 8);

class OrType final : public GroundType {
public:
    TypePtr left;
    TypePtr right;
    virtual int kind() final;

    virtual std::string toStringWithTabs(const GlobalState &gs, int tabs = 0) const final;
    virtual std::string show(const GlobalState &gs) const final;
    virtual std::string typeName() const final;
    virtual DispatchResult dispatchCall(const GlobalState &gs, DispatchArgs args) final;
    virtual TypePtr getCallArguments(const GlobalState &gs, NameRef name) final;
    virtual bool derivesFrom(const GlobalState &gs, SymbolRef klass) const final;
    void _sanityCheck(const GlobalState &gs) final;
    virtual bool isFullyDefined() final;
    virtual TypePtr _instantiate(const GlobalState &gs, const InlinedVector<SymbolRef, 4> &params,
                                 const std::vector<TypePtr> &targs) override;
    virtual bool hasUntyped() override;
    virtual TypePtr _approximate(const GlobalState &gs, const TypeConstraint &tc) override;
    virtual TypePtr _instantiate(const GlobalState &gs, const TypeConstraint &tc) override;
    virtual TypePtr _replaceSelfType(const GlobalState &gs, const TypePtr &receiver) override;

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
    friend TypePtr Types::falsyTypes();
    friend TypePtr Types::Boolean();
    friend class GlobalSubstitution;
    friend class serialize::SerializerImpl;
    friend bool Types::isSubTypeUnderConstraint(const GlobalState &gs, TypeConstraint &constr, const TypePtr &t1,
                                                const TypePtr &t2, UntypedMode mode);
    friend TypePtr lubDistributeOr(const GlobalState &gs, const TypePtr &t1, const TypePtr &t2);
    friend TypePtr lubGround(const GlobalState &gs, const TypePtr &t1, const TypePtr &t2);
    friend TypePtr Types::lub(const GlobalState &gs, const TypePtr &t1, const TypePtr &t2);
    friend TypePtr Types::glb(const GlobalState &gs, const TypePtr &t1, const TypePtr &t2);
    friend TypePtr Types::dropSubtypesOf(const GlobalState &gs, const TypePtr &from, SymbolRef klass);
    friend TypePtr Types::unwrapSelfTypeParam(Context ctx, const TypePtr &t1);
    friend class Symbol; // the actual method is `recordSealedSubclass(Mutableconst GlobalState &gs, SymbolRef
                         // subclass)`, but refering to it introduces a cycle

    static TypePtr make_shared(const TypePtr &left, const TypePtr &right);
};
CheckSize(OrType, 40, 8);

class AndType final : public GroundType {
public:
    TypePtr left;
    TypePtr right;
    virtual int kind() final;

    virtual std::string toStringWithTabs(const GlobalState &gs, int tabs = 0) const final;
    virtual std::string show(const GlobalState &gs) const final;
    virtual std::string typeName() const final;
    virtual DispatchResult dispatchCall(const GlobalState &gs, DispatchArgs args) final;

    virtual TypePtr getCallArguments(const GlobalState &gs, NameRef name) final;
    virtual bool derivesFrom(const GlobalState &gs, SymbolRef klass) const final;
    void _sanityCheck(const GlobalState &gs) final;
    virtual bool isFullyDefined() final;
    virtual TypePtr _instantiate(const GlobalState &gs, const InlinedVector<SymbolRef, 4> &params,
                                 const std::vector<TypePtr> &targs) override;
    virtual TypePtr _replaceSelfType(const GlobalState &gs, const TypePtr &receiver) override;
    virtual bool hasUntyped() override;
    virtual TypePtr _approximate(const GlobalState &gs, const TypeConstraint &tc) override;
    virtual TypePtr _instantiate(const GlobalState &gs, const TypeConstraint &tc) override;

private:
    /*
     * You probably want Types::all() instead.
     */
    AndType(const TypePtr &left, const TypePtr &right);

    friend class GlobalSubstitution;
    friend class serialize::SerializerImpl;
    friend class TypeConstraint;

    friend bool Types::isSubTypeUnderConstraint(const GlobalState &gs, TypeConstraint &constr, const TypePtr &t1,
                                                const TypePtr &t2, UntypedMode mode);
    friend TypePtr lubGround(const GlobalState &gs, const TypePtr &t1, const TypePtr &t2);
    friend TypePtr glbDistributeAnd(const GlobalState &gs, const TypePtr &t1, const TypePtr &t2);
    friend TypePtr glbGround(const GlobalState &gs, const TypePtr &t1, const TypePtr &t2);
    friend TypePtr Types::lub(const GlobalState &gs, const TypePtr &t1, const TypePtr &t2);
    friend TypePtr Types::glb(const GlobalState &gs, const TypePtr &t1, const TypePtr &t2);
    friend TypePtr Types::unwrapSelfTypeParam(Context ctx, const TypePtr &t1);

    static TypePtr make_shared(const TypePtr &left, const TypePtr &right);
};
CheckSize(AndType, 40, 8);

class ShapeType final : public ProxyType {
public:
    std::vector<TypePtr> keys; // TODO: store sorted by whatever
    std::vector<TypePtr> values;
    const TypePtr underlying_;
    ShapeType();
    ShapeType(TypePtr underlying, std::vector<TypePtr> keys, std::vector<TypePtr> values);

    virtual std::string toStringWithTabs(const GlobalState &gs, int tabs = 0) const final;
    virtual std::string show(const GlobalState &gs) const final;
    virtual std::string typeName() const override;
    virtual DispatchResult dispatchCall(const GlobalState &gs, DispatchArgs args) final;
    void _sanityCheck(const GlobalState &gs) final;
    virtual bool isFullyDefined() final;

    virtual TypePtr _instantiate(const GlobalState &gs, const InlinedVector<SymbolRef, 4> &params,
                                 const std::vector<TypePtr> &targs) override;
    virtual int kind() final;
    virtual bool hasUntyped() override;
    virtual TypePtr _approximate(const GlobalState &gs, const TypeConstraint &tc) override;
    virtual TypePtr _instantiate(const GlobalState &gs, const TypeConstraint &tc) override;
    virtual TypePtr underlying() const override;
};
CheckSize(ShapeType, 72, 8);

class TupleType final : public ProxyType {
private:
    TupleType() = delete;

public:
    std::vector<TypePtr> elems;
    const TypePtr underlying_;

    TupleType(TypePtr underlying, std::vector<TypePtr> elements);
    static TypePtr build(const GlobalState &gs, std::vector<TypePtr> elements);

    virtual std::string toStringWithTabs(const GlobalState &gs, int tabs = 0) const final;
    virtual std::string show(const GlobalState &gs) const final;
    virtual std::string showWithMoreInfo(const GlobalState &gs) const final;
    virtual std::string typeName() const override;
    void _sanityCheck(const GlobalState &gs) final;
    virtual bool isFullyDefined() final;

    virtual TypePtr _instantiate(const GlobalState &gs, const InlinedVector<SymbolRef, 4> &params,
                                 const std::vector<TypePtr> &targs) override;
    virtual int kind() final;
    virtual DispatchResult dispatchCall(const GlobalState &gs, DispatchArgs args) final;
    virtual bool hasUntyped() override;
    virtual TypePtr _approximate(const GlobalState &gs, const TypeConstraint &tc) override;
    virtual TypePtr _instantiate(const GlobalState &gs, const TypeConstraint &tc) override;

    // Return the type of the underlying array that this tuple decays into
    TypePtr elementType() const;
    virtual TypePtr underlying() const override;
};
CheckSize(TupleType, 48, 8);

class AppliedType final : public Type {
public:
    SymbolRef klass;
    std::vector<TypePtr> targs;
    AppliedType(SymbolRef klass, std::vector<TypePtr> targs);

    virtual std::string toStringWithTabs(const GlobalState &gs, int tabs = 0) const final;
    virtual std::string show(const GlobalState &gs) const final;
    virtual std::string typeName() const final;
    virtual DispatchResult dispatchCall(const GlobalState &gs, DispatchArgs args) final;
    void _sanityCheck(const GlobalState &gs) final;
    virtual bool isFullyDefined() final;

    virtual TypePtr _instantiate(const GlobalState &gs, const InlinedVector<SymbolRef, 4> &params,
                                 const std::vector<TypePtr> &targs) override;
    virtual int kind() final;

    virtual TypePtr getCallArguments(const GlobalState &gs, NameRef name) final;

    virtual bool derivesFrom(const GlobalState &gs, SymbolRef klass) const final;
    virtual bool hasUntyped() override;
    virtual TypePtr _approximate(const GlobalState &gs, const TypeConstraint &tc) override;
    virtual TypePtr _instantiate(const GlobalState &gs, const TypeConstraint &tc) override;
};
CheckSize(AppliedType, 40, 8);

// MetaType is the type of a Type. You can think of it as generalization of
// Ruby's singleton classes to Types; Just as `A.singleton_class` is the *type*
// of the *value* `A`, MetaType[T] is the *type* of a *value* that holds the
// type T during execution. For instance, the type of `T.untyped` is
// `MetaType(Types::untyped())`.
//
// These are used within the inferencer in places where we need to track
// user-written types in the source code.
class MetaType final : public ProxyType {
public:
    TypePtr wrapped;

    MetaType(const TypePtr &wrapped);

    virtual std::string toStringWithTabs(const GlobalState &gs, int tabs = 0) const final;
    virtual std::string show(const GlobalState &gs) const final;
    virtual std::string typeName() const final;

    virtual bool derivesFrom(const GlobalState &gs, SymbolRef klass) const final;

    virtual DispatchResult dispatchCall(const GlobalState &gs, DispatchArgs args) final;
    void _sanityCheck(const GlobalState &gs) final;
    virtual bool isFullyDefined() final;

    virtual TypePtr _instantiate(const GlobalState &gs, const InlinedVector<SymbolRef, 4> &params,
                                 const std::vector<TypePtr> &targs) override;
    virtual int kind() final;
    virtual TypePtr _approximate(const GlobalState &gs, const TypeConstraint &tc) override;
    virtual TypePtr underlying() const override;
};
CheckSize(MetaType, 24, 8);

class SendAndBlockLink {
    SendAndBlockLink(const SendAndBlockLink &) = default;

public:
    SendAndBlockLink(SendAndBlockLink &&) = default;
    std::vector<ArgInfo::ArgFlags> argFlags;
    core::NameRef fun;
    int rubyBlockId;
    std::shared_ptr<DispatchResult> result;

    SendAndBlockLink(NameRef fun, std::vector<ArgInfo::ArgFlags> &&argFlags, int rubyBlockId);
    std::optional<int> fixedArity() const;
    std::shared_ptr<SendAndBlockLink> duplicate();
};

class TypeAndOrigins final {
public:
    TypePtr type;
    InlinedVector<Loc, 1> origins;
    std::vector<ErrorLine> origins2Explanations(const GlobalState &gs) const;
    ~TypeAndOrigins() noexcept;
    TypeAndOrigins() = default;
    TypeAndOrigins(const TypeAndOrigins &) = default;
    TypeAndOrigins(TypeAndOrigins &&) = default;
    TypeAndOrigins &operator=(const TypeAndOrigins &) = default;
    TypeAndOrigins &operator=(TypeAndOrigins &&) = default;
};
CheckSize(TypeAndOrigins, 40, 8);

struct CallLocs final {
    FileRef file;
    LocOffsets call;
    LocOffsets receiver;
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
    InlinedVector<const TypeAndOrigins *, 2> &args;
    const TypePtr &selfType;
    const TypePtr &fullType;
    const std::shared_ptr<const SendAndBlockLink> &block;

    DispatchArgs withSelfRef(const TypePtr &newSelfRef);
};

struct DispatchComponent {
    TypePtr receiver;
    SymbolRef method;
    std::vector<std::unique_ptr<Error>> errors;
    TypePtr sendTp;
    TypePtr blockReturnType;
    TypePtr blockPreType;
    ArgInfo blockSpec; // used only by LoadSelf to change type of self inside method.
    std::unique_ptr<TypeConstraint> constr;
};

struct DispatchResult {
    enum class Combinator { OR, AND };
    TypePtr returnType;
    DispatchComponent main;
    std::unique_ptr<DispatchResult> secondary;
    Combinator secondaryKind;

    DispatchResult() = default;
    DispatchResult(TypePtr returnType, TypePtr receiverType, core::SymbolRef method)
        : returnType(returnType),
          main(DispatchComponent{
              receiverType, method, {}, std::move(returnType), nullptr, nullptr, ArgInfo{}, nullptr}){};
    DispatchResult(TypePtr returnType, DispatchComponent comp)
        : returnType(std::move(returnType)), main(std::move(comp)){};
    DispatchResult(TypePtr returnType, DispatchComponent comp, std::unique_ptr<DispatchResult> secondary,
                   Combinator secondaryKind)
        : returnType(std::move(returnType)), main(std::move(comp)), secondary(std::move(secondary)),
          secondaryKind(secondaryKind){};
};

class BlamedUntyped final : public ClassType {
public:
    const core::SymbolRef blame;
    BlamedUntyped(SymbolRef whoToBlame) : ClassType(core::Symbols::untyped()), blame(whoToBlame){};
};

class UnresolvedClassType final : public ClassType {
public:
    const core::SymbolRef scope;
    const std::vector<core::NameRef> names;
    UnresolvedClassType(SymbolRef scope, std::vector<core::NameRef> names)
        : ClassType(core::Symbols::untyped()), scope(scope), names(names){};
    virtual std::string toStringWithTabs(const GlobalState &gs, int tabs = 0) const final;
    virtual std::string show(const GlobalState &gs) const final;
    virtual std::string typeName() const final;
};

class UnresolvedAppliedType final : public ClassType {
public:
    const core::SymbolRef klass;
    const std::vector<TypePtr> targs;
    UnresolvedAppliedType(SymbolRef klass, std::vector<TypePtr> targs)
        : ClassType(core::Symbols::untyped()), klass(klass), targs(std::move(targs)){};
    virtual std::string toStringWithTabs(const GlobalState &gs, int tabs = 0) const final;
    virtual std::string show(const GlobalState &gs) const final;
    virtual std::string typeName() const final;
};

} // namespace sorbet::core
#endif // SORBET_TYPES_H
