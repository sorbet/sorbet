#ifndef SORBET_TYPES_H
#define SORBET_TYPES_H

#include "common/Counters.h"
#include "core/Context.h"
#include "core/Error.h"
#include "core/SymbolRef.h"
#include <memory>
#include <string>

namespace sorbet::core {
/** Dmitry: unlike in Dotty, those types are always dealiased. For now */
class Type;
class IntrinsicMethod;
class TypeConstraint;
struct DispatchArgs;
struct DispatchComponent;
struct DispatchResult;
struct CallLocs;
class TypeVar;
class SendAndBlockLink;
class TypeAndOrigins;
// using TypePtr = std::shared_ptr<Type>;
class TypePtr {
    std::shared_ptr<Type> store;

public:
    TypePtr() = default;
    TypePtr(TypePtr &&other) = default;
    TypePtr(const TypePtr &other) = default;
    TypePtr &operator=(TypePtr &&other) = default;
    TypePtr &operator=(const TypePtr &other) = default;
    TypePtr(Type *ptr) : store(ptr) {}
    operator bool() const {
        return (bool)store;
    }
    Type *get() const {
        return store.get();
    }
    Type *operator->() const {
        return get();
    }
    Type &operator*() const {
        return *get();
    }
    bool operator!=(const TypePtr &other) const {
        return store != other.store;
    }
    bool operator==(const TypePtr &other) const {
        return store == other.store;
    }
    bool operator!=(std::nullptr_t n) const {
        return store != nullptr;
    }
    bool operator==(std::nullptr_t n) const {
        return store == nullptr;
    }
};

template <class T, class... Args> TypePtr make_type(Args &&... args) {
    return TypePtr(new T(std::forward<Args>(args)...));
}

class Types final {
public:
    /** Greater lower bound: the widest type that is subtype of both t1 and t2 */
    static TypePtr all(Context ctx, const TypePtr &t1, const TypePtr &t2);

    /** Lower upper bound: the narrowest type that is supper type of both t1 and t2 */
    static TypePtr any(Context ctx, const TypePtr &t1, const TypePtr &t2);

    /** is every instance of  t1 an  instance of t2? */
    static bool isSubTypeUnderConstraint(Context ctx, TypeConstraint &constr, const TypePtr &t1, const TypePtr &t2);

    /** is every instance of  t1 an  instance of t2 when not allowed to modify constraint */
    static bool isSubType(Context ctx, const TypePtr &t1, const TypePtr &t2);
    static bool equiv(Context ctx, const TypePtr &t1, const TypePtr &t2);

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
    static TypePtr arrayClass();
    static TypePtr hashClass();
    static TypePtr arrayOfUntyped();
    static TypePtr hashOfUntyped();
    static TypePtr procClass();
    static TypePtr classClass();
    static TypePtr falsyTypes();

    static TypePtr dropSubtypesOf(Context ctx, const TypePtr &from, SymbolRef klass);
    static TypePtr approximateSubtract(Context ctx, const TypePtr &from, const TypePtr &what);
    static bool canBeTruthy(Context ctx, const TypePtr &what);
    static bool canBeFalsy(Context ctx, const TypePtr &what);
    enum Combinator { OR, AND };

    static TypePtr resultTypeAsSeenFrom(Context ctx, SymbolRef what, SymbolRef inWhat,
                                        const std::vector<TypePtr> &targs);

    static InlinedVector<SymbolRef, 4> alignBaseTypeArgs(Context ctx, SymbolRef what, const std::vector<TypePtr> &targs,
                                                         SymbolRef asIf);
    // Extract the return value type from a proc.
    static TypePtr getProcReturnType(Context ctx, const TypePtr &procType);
    static TypePtr instantiate(Context ctx, const TypePtr &what, const InlinedVector<SymbolRef, 4> &params,
                               const std::vector<TypePtr> &targs);
    /** Replace all type variables in `what` with their instantiations.
     * Requires that `tc` has already been solved.
     */
    static TypePtr instantiate(Context ctx, const TypePtr &what, const TypeConstraint &tc);

    static TypePtr replaceSelfType(Context ctx, const TypePtr &what, const TypePtr &receiver);
    /** Get rid of type variables in `what` and return a type that we deem close enough to continue
     * typechecking. We should be careful to only used this type when we are trying to guess a type.
     * We should do proper instatiation and subtype test after we have guessed type variables with
     * tc.solve(). If the constraint has already been solved, use `instantiate` instead.
     */
    static TypePtr approximate(Context ctx, const TypePtr &what, const TypeConstraint &tc);

    static TypePtr dropLiteral(const TypePtr &type);

    /** Internal implementation. You should probably use all(). */
    static TypePtr glb(Context ctx, const TypePtr &t1, const TypePtr &t2);
    /** Internal implementation. You should probably use any(). */
    static TypePtr lub(Context ctx, const TypePtr &t1, const TypePtr &t2);

    static TypePtr lubAll(Context ctx, std::vector<TypePtr> &elements);
    static TypePtr arrayOf(Context ctx, const TypePtr &elem);
    static TypePtr hashOf(Context ctx, const TypePtr &elem);

    /** Recursively replaces proxies with their underlying types */
    static TypePtr widen(Context ctx, const TypePtr &type);
};

struct Intrinsic {
    const SymbolRef symbol;
    const bool singleton;
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
    virtual std::string toString(const GlobalState &gs, int tabs = 0) const = 0;
    // User visible type. Should exactly match what the user can write.
    virtual std::string show(const GlobalState &gs) const = 0;
    virtual std::string typeName() const = 0;
    virtual TypePtr _instantiate(Context ctx, const InlinedVector<SymbolRef, 4> &params,
                                 const std::vector<TypePtr> &targs) = 0;
    virtual TypePtr _instantiate(Context ctx, const TypeConstraint &tc);

    virtual TypePtr _replaceSelfType(Context ctx, const TypePtr &receiver);

    virtual DispatchResult dispatchCall(Context ctx, DispatchArgs args) = 0;
    virtual TypePtr getCallArguments(Context ctx, NameRef name) = 0;
    virtual bool derivesFrom(const GlobalState &gs, SymbolRef klass) = 0;
    virtual void _sanityCheck(Context ctx) = 0;
    void sanityCheck(Context ctx) {
        if (!debug_mode)
            return;
        _sanityCheck(ctx);
    }

    bool isUntyped() const;
    core::SymbolRef untypedBlame() const;
    bool isBottom() const;
    bool isTop() const;
    virtual bool hasUntyped();
    virtual bool isFullyDefined() = 0;
    virtual int kind() = 0;
    virtual TypePtr _approximate(Context ctx, const TypeConstraint &tc);
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

    virtual DispatchResult dispatchCall(Context ctx, DispatchArgs args) override;
    virtual TypePtr getCallArguments(Context ctx, NameRef name) override;
    virtual bool derivesFrom(const GlobalState &gs, SymbolRef klass) override;

    void _sanityCheck(Context ctx) override;
};
CheckSize(ProxyType, 8, 8);

class ClassType : public GroundType {
public:
    SymbolRef symbol;
    ClassType(SymbolRef symbol);
    virtual int kind() final;

    virtual std::string toString(const GlobalState &gs, int tabs = 0) const final;
    virtual std::string show(const GlobalState &gs) const final;
    virtual std::string typeName() const final;
    virtual DispatchResult dispatchCall(Context ctx, DispatchArgs args) override;

    virtual TypePtr getCallArguments(Context ctx, NameRef name) final;
    virtual bool derivesFrom(const GlobalState &gs, SymbolRef klass) final;
    void _sanityCheck(Context ctx) final;
    virtual TypePtr _instantiate(Context ctx, const InlinedVector<SymbolRef, 4> &params,
                                 const std::vector<TypePtr> &targs) override;
    virtual bool isFullyDefined() final;
    virtual bool hasUntyped() override;
};
CheckSize(ClassType, 16, 8);

class LambdaParam final : public Type {
public:
    SymbolRef definition;

    LambdaParam(const SymbolRef definition);
    virtual std::string toString(const GlobalState &gs, int tabs = 0) const final;
    virtual std::string show(const GlobalState &gs) const final;
    virtual std::string typeName() const final;

    virtual bool derivesFrom(const GlobalState &gs, SymbolRef klass) final;

    virtual DispatchResult dispatchCall(Context ctx, DispatchArgs args) final;
    virtual TypePtr getCallArguments(Context ctx, NameRef name) final;
    void _sanityCheck(Context ctx) final;
    virtual bool isFullyDefined() final;

    virtual TypePtr _instantiate(Context ctx, const InlinedVector<SymbolRef, 4> &params,
                                 const std::vector<TypePtr> &targs) override;
    virtual int kind() final;
};
CheckSize(LambdaParam, 16, 8);

class SelfTypeParam final : public Type {
public:
    SymbolRef definition;

    SelfTypeParam(const SymbolRef definition);
    virtual std::string toString(const GlobalState &gs, int tabs = 0) const final;
    virtual std::string show(const GlobalState &gs) const final;
    virtual std::string typeName() const final;

    virtual bool derivesFrom(const GlobalState &gs, SymbolRef klass) final;

    virtual DispatchResult dispatchCall(Context ctx, DispatchArgs args) final;
    virtual TypePtr getCallArguments(Context ctx, NameRef name) final;
    void _sanityCheck(Context ctx) final;
    virtual bool isFullyDefined() final;

    virtual TypePtr _instantiate(Context ctx, const InlinedVector<SymbolRef, 4> &params,
                                 const std::vector<TypePtr> &targs) override;
    virtual int kind() final;
};
CheckSize(SelfTypeParam, 16, 8);

class AliasType final : public Type {
public:
    AliasType(SymbolRef other);
    virtual std::string toString(const GlobalState &gs, int tabs = 0) const final;
    virtual std::string show(const GlobalState &gs) const final;
    virtual std::string typeName() const final;
    virtual DispatchResult dispatchCall(Context ctx, DispatchArgs args) final;
    virtual TypePtr getCallArguments(Context ctx, NameRef name) final;
    virtual bool derivesFrom(const GlobalState &gs, SymbolRef klass) final;

    SymbolRef symbol;
    void _sanityCheck(Context ctx) final;
    virtual bool isFullyDefined() final;

    virtual TypePtr _instantiate(Context ctx, const InlinedVector<SymbolRef, 4> &params,
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
    virtual std::string toString(const GlobalState &gs, int tabs = 0) const final;
    virtual std::string show(const GlobalState &gs) const final;
    virtual std::string showValue(const GlobalState &gs) const final;
    virtual std::string typeName() const override;
    virtual bool isFullyDefined() final;

    virtual TypePtr _instantiate(Context ctx, const InlinedVector<SymbolRef, 4> &params,
                                 const std::vector<TypePtr> &targs) override;
    virtual int kind() final;
    virtual TypePtr _replaceSelfType(Context ctx, const TypePtr &receiver) override;

    virtual DispatchResult dispatchCall(Context ctx, DispatchArgs args) final;
    void _sanityCheck(Context ctx) final;
    virtual TypePtr getCallArguments(Context ctx, NameRef name) final;
    virtual bool derivesFrom(const GlobalState &gs, SymbolRef klass) final;
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

    virtual std::string toString(const GlobalState &gs, int tabs = 0) const final;
    virtual std::string show(const GlobalState &gs) const final;
    virtual std::string showValue(const GlobalState &gs) const final;
    virtual std::string typeName() const override;
    virtual bool isFullyDefined() final;

    bool equals(const LiteralType &rhs) const;

    virtual TypePtr _instantiate(Context ctx, const InlinedVector<SymbolRef, 4> &params,
                                 const std::vector<TypePtr> &targs) override;
    virtual int kind() final;
};
CheckSize(LiteralType, 24, 8);

class TypeVar final : public Type {
public:
    SymbolRef sym;
    TypeVar(SymbolRef sym);
    virtual std::string toString(const GlobalState &gs, int tabs = 0) const final;
    virtual std::string show(const GlobalState &gs) const final;
    virtual std::string typeName() const final;
    virtual DispatchResult dispatchCall(Context ctx, DispatchArgs args) final;
    void _sanityCheck(Context ctx) final;
    virtual bool isFullyDefined() final;
    virtual TypePtr getCallArguments(Context ctx, NameRef name) final;
    virtual bool derivesFrom(const GlobalState &gs, SymbolRef klass) final;

    virtual TypePtr _instantiate(Context ctx, const InlinedVector<SymbolRef, 4> &params,
                                 const std::vector<TypePtr> &targs) override;
    virtual int kind() final;
    virtual TypePtr _approximate(Context ctx, const TypeConstraint &tc) override;
    virtual TypePtr _instantiate(Context ctx, const TypeConstraint &tc) override;
};
CheckSize(TypeVar, 16, 8);

class OrType final : public GroundType {
public:
    TypePtr left;
    TypePtr right;
    virtual int kind() final;

    virtual std::string toString(const GlobalState &gs, int tabs = 0) const final;
    virtual std::string show(const GlobalState &gs) const final;
    virtual std::string typeName() const final;
    virtual DispatchResult dispatchCall(Context ctx, DispatchArgs args) final;
    virtual TypePtr getCallArguments(Context ctx, NameRef name) final;
    virtual bool derivesFrom(const GlobalState &gs, SymbolRef klass) final;
    void _sanityCheck(Context ctx) final;
    virtual bool isFullyDefined() final;
    virtual TypePtr _instantiate(Context ctx, const InlinedVector<SymbolRef, 4> &params,
                                 const std::vector<TypePtr> &targs) override;
    virtual bool hasUntyped() override;
    virtual TypePtr _approximate(Context ctx, const TypeConstraint &tc) override;
    virtual TypePtr _instantiate(Context ctx, const TypeConstraint &tc) override;
    virtual TypePtr _replaceSelfType(Context ctx, const TypePtr &receiver) override;

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
    friend TypePtr lubDistributeOr(Context ctx, const TypePtr &t1, const TypePtr &t2);
    friend TypePtr lubGround(Context ctx, const TypePtr &t1, const TypePtr &t2);
    friend TypePtr Types::lub(Context ctx, const TypePtr &t1, const TypePtr &t2);
    friend TypePtr Types::glb(Context ctx, const TypePtr &t1, const TypePtr &t2);
    friend TypePtr Types::dropSubtypesOf(Context ctx, const TypePtr &from, SymbolRef klass);

    static TypePtr make_shared(const TypePtr &left, const TypePtr &right);
};
CheckSize(OrType, 40, 8);

class AndType final : public GroundType {
public:
    TypePtr left;
    TypePtr right;
    virtual int kind() final;

    virtual std::string toString(const GlobalState &gs, int tabs = 0) const final;
    virtual std::string show(const GlobalState &gs) const final;
    virtual std::string typeName() const final;
    virtual DispatchResult dispatchCall(Context ctx, DispatchArgs args) final;

    virtual TypePtr getCallArguments(Context ctx, NameRef name) final;
    virtual bool derivesFrom(const GlobalState &gs, SymbolRef klass) final;
    void _sanityCheck(Context ctx) final;
    virtual bool isFullyDefined() final;
    virtual TypePtr _instantiate(Context ctx, const InlinedVector<SymbolRef, 4> &params,
                                 const std::vector<TypePtr> &targs) override;
    virtual TypePtr _replaceSelfType(Context ctx, const TypePtr &receiver) override;
    virtual bool hasUntyped() override;
    virtual TypePtr _approximate(Context ctx, const TypeConstraint &tc) override;
    virtual TypePtr _instantiate(Context ctx, const TypeConstraint &tc) override;

private:
    /*
     * You probably want Types::all() instead.
     */
    AndType(const TypePtr &left, const TypePtr &right);

    friend class GlobalSubstitution;
    friend class serialize::SerializerImpl;
    friend class TypeConstraint;

    friend TypePtr lubGround(Context ctx, const TypePtr &t1, const TypePtr &t2);
    friend TypePtr glbDistributeAnd(Context ctx, const TypePtr &t1, const TypePtr &t2);
    friend TypePtr glbGround(Context ctx, const TypePtr &t1, const TypePtr &t2);
    friend TypePtr Types::lub(Context ctx, const TypePtr &t1, const TypePtr &t2);
    friend TypePtr Types::glb(Context ctx, const TypePtr &t1, const TypePtr &t2);

    static TypePtr make_shared(const TypePtr &left, const TypePtr &right);
};
CheckSize(AndType, 40, 8);

class ShapeType final : public ProxyType {
public:
    std::vector<TypePtr> keys; // TODO: store sorted by whatever
    std::vector<TypePtr> values;
    const TypePtr underlying_;
    ShapeType();
    ShapeType(const TypePtr &underlying, std::vector<TypePtr> keys, std::vector<TypePtr> values);

    virtual std::string toString(const GlobalState &gs, int tabs = 0) const final;
    virtual std::string show(const GlobalState &gs) const final;
    virtual std::string typeName() const override;
    virtual DispatchResult dispatchCall(Context ctx, DispatchArgs args) final;
    void _sanityCheck(Context ctx) final;
    virtual bool isFullyDefined() final;

    virtual TypePtr _instantiate(Context ctx, const InlinedVector<SymbolRef, 4> &params,
                                 const std::vector<TypePtr> &targs) override;
    virtual int kind() final;
    virtual bool hasUntyped() override;
    virtual TypePtr _approximate(Context ctx, const TypeConstraint &tc) override;
    virtual TypePtr _instantiate(Context ctx, const TypeConstraint &tc) override;
    virtual TypePtr underlying() const override;
};
CheckSize(ShapeType, 72, 8);

class TupleType final : public ProxyType {
private:
    TupleType() = delete;

public:
    std::vector<TypePtr> elems;
    const TypePtr underlying_;

    TupleType(const TypePtr &underlying, std::vector<TypePtr> elements);
    static TypePtr build(Context ctx, std::vector<TypePtr> elements);

    virtual std::string toString(const GlobalState &gs, int tabs = 0) const final;
    virtual std::string show(const GlobalState &gs) const final;
    virtual std::string typeName() const override;
    void _sanityCheck(Context ctx) final;
    virtual bool isFullyDefined() final;

    virtual TypePtr _instantiate(Context ctx, const InlinedVector<SymbolRef, 4> &params,
                                 const std::vector<TypePtr> &targs) override;
    virtual int kind() final;
    virtual DispatchResult dispatchCall(Context ctx, DispatchArgs args) final;
    virtual bool hasUntyped() override;
    virtual TypePtr _approximate(Context ctx, const TypeConstraint &tc) override;
    virtual TypePtr _instantiate(Context ctx, const TypeConstraint &tc) override;

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

    virtual std::string toString(const GlobalState &gs, int tabs = 0) const final;
    virtual std::string show(const GlobalState &gs) const final;
    virtual std::string typeName() const final;
    virtual DispatchResult dispatchCall(Context ctx, DispatchArgs args) final;
    void _sanityCheck(Context ctx) final;
    virtual bool isFullyDefined() final;

    virtual TypePtr _instantiate(Context ctx, const InlinedVector<SymbolRef, 4> &params,
                                 const std::vector<TypePtr> &targs) override;
    virtual int kind() final;

    virtual TypePtr getCallArguments(Context ctx, NameRef name) final;

    virtual bool derivesFrom(const GlobalState &gs, SymbolRef klass) final;
    virtual bool hasUntyped() override;
    virtual TypePtr _approximate(Context ctx, const TypeConstraint &tc) override;
    virtual TypePtr _instantiate(Context ctx, const TypeConstraint &tc) override;
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

    virtual std::string toString(const GlobalState &gs, int tabs = 0) const final;
    virtual std::string show(const GlobalState &gs) const final;
    virtual std::string typeName() const final;

    virtual bool derivesFrom(const GlobalState &gs, SymbolRef klass) final;

    virtual DispatchResult dispatchCall(Context ctx, DispatchArgs args) final;
    void _sanityCheck(Context ctx) final;
    virtual bool isFullyDefined() final;

    virtual TypePtr _instantiate(Context ctx, const InlinedVector<SymbolRef, 4> &params,
                                 const std::vector<TypePtr> &targs) override;
    virtual int kind() final;
    virtual TypePtr _approximate(Context ctx, const TypeConstraint &tc) override;
    virtual TypePtr underlying() const override;
};
CheckSize(MetaType, 24, 8);

class SendAndBlockLink {
    SendAndBlockLink(const SendAndBlockLink &) noexcept = default;

public:
    SendAndBlockLink(SendAndBlockLink &&) noexcept = default;
    SymbolRef block;
    TypePtr receiver;
    NameRef fun;
    std::shared_ptr<TypeConstraint> constr;
    TypePtr returnTp;
    TypePtr blockPreType;
    TypePtr sendTp;
    SendAndBlockLink(SymbolRef block, NameRef fun);
    std::shared_ptr<SendAndBlockLink> duplicate();
};

class TypeAndOrigins final {
public:
    TypePtr type;
    InlinedVector<Loc, 2> origins;
    std::vector<ErrorLine> origins2Explanations(Context ctx) const;
    ~TypeAndOrigins();
};
CheckSize(TypeAndOrigins, 40, 8);

struct CallLocs final {
    Loc call;
    Loc receiver;
    InlinedVector<Loc, 2> &args;
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
    const std::shared_ptr<SendAndBlockLink> &block;

    DispatchArgs withSelfRef(const TypePtr &newSelfRef);
    core::TypeConstraint &constraint();
};

struct DispatchComponent {
    TypePtr receiver;
    SymbolRef method;
    std::vector<std::unique_ptr<Error>> errors;
};

struct DispatchResult {
    using ComponentVec = InlinedVector<DispatchComponent, 1>;

    TypePtr returnType;
    ComponentVec components;

    DispatchResult() = default;
    DispatchResult(TypePtr returnType, ComponentVec components)
        : returnType(std::move(returnType)), components(std::move(components)){};

    DispatchResult(TypePtr returnType, TypePtr receiver, SymbolRef method) : returnType(std::move(returnType)) {
        components.emplace_back(DispatchComponent{std::move(receiver), method, std::vector<std::unique_ptr<Error>>()});
    }
};

class BlamedUntyped final : public ClassType {
public:
    const core::SymbolRef blame;
    BlamedUntyped(SymbolRef whoToBlame) : ClassType(core::Symbols::untyped()), blame(whoToBlame){};
};

} // namespace sorbet::core
#endif // SORBET_TYPES_H
