#ifndef SORBET_TYPES_H
#define SORBET_TYPES_H

#include "Context.h"
#include "Counters.h"
#include "Symbols.h"
#include <memory>
#include <string>

namespace sorbet {
namespace core {
/** Dmitry: unlike in Dotty, those types are always dealiased. For now */
class Type;
class TypeConstraint;
class TypeVar;
class SendAndBlockLink;
class Types final {
public:
    /** Greater lower bound: the widest type that is subtype of both t1 and t2 */
    static std::shared_ptr<Type> all(Context ctx, const std::shared_ptr<Type> &t1, const std::shared_ptr<Type> &t2);

    /** Lower upper bound: the narrowest type that is supper type of both t1 and t2 */
    static std::shared_ptr<Type> any(Context ctx, const std::shared_ptr<Type> &t1, const std::shared_ptr<Type> &t2);

    /** is every instance of  t1 an  instance of t2? */
    static bool isSubTypeUnderConstraint(Context ctx, TypeConstraint &constr, const std::shared_ptr<Type> &t1,
                                         const std::shared_ptr<Type> &t2);

    /** is every instance of  t1 an  instance of t2 when not allowed to modify constraint */
    static bool isSubType(Context ctx, const std::shared_ptr<Type> &t1, const std::shared_ptr<Type> &t2);
    static bool equiv(Context ctx, const std::shared_ptr<Type> &t1, const std::shared_ptr<Type> &t2);

    static std::shared_ptr<Type> top();
    static std::shared_ptr<Type> bottom();
    static std::shared_ptr<Type> nilClass();
    static std::shared_ptr<Type> untyped();
    static std::shared_ptr<Type> void_();
    static std::shared_ptr<Type> trueClass();
    static std::shared_ptr<Type> falseClass();
    static std::shared_ptr<Type> Integer();
    static std::shared_ptr<Type> String();
    static std::shared_ptr<Type> Symbol();
    static std::shared_ptr<Type> Float();
    static std::shared_ptr<Type> Boolean();
    static std::shared_ptr<Type> Object();
    static std::shared_ptr<Type> arrayClass();
    static std::shared_ptr<Type> hashClass();
    static std::shared_ptr<Type> arrayOfUntyped();
    static std::shared_ptr<Type> hashOfUntyped();
    static std::shared_ptr<Type> procClass();
    static std::shared_ptr<Type> classClass();
    static std::shared_ptr<Type> falsyTypes();

    static std::shared_ptr<Type> dropSubtypesOf(Context ctx, const std::shared_ptr<Type> &from, SymbolRef klass);
    static std::shared_ptr<Type> approximateSubtract(Context ctx, const std::shared_ptr<Type> &from,
                                                     const std::shared_ptr<Type> &what);
    static bool canBeTruthy(Context ctx, const std::shared_ptr<Type> &what);
    static bool canBeFalsy(Context ctx, const std::shared_ptr<Type> &what);
    enum Combinator { OR, AND };

    static std::shared_ptr<Type> resultTypeAsSeenFrom(Context ctx, SymbolRef what, SymbolRef inWhat,
                                                      const std::vector<std::shared_ptr<Type>> &targs);

    static std::vector<SymbolRef> alignBaseTypeArgs(Context ctx, SymbolRef what,
                                                    const std::vector<std::shared_ptr<Type>> &targs, SymbolRef asIf);
    // Extract the return value type from a proc.
    static std::shared_ptr<Type> getProcReturnType(Context ctx, const std::shared_ptr<Type> &procType);
    static std::shared_ptr<Type> instantiate(Context ctx, const std::shared_ptr<Type> &what,
                                             std::vector<SymbolRef> params,
                                             const std::vector<std::shared_ptr<Type>> &targs);
    /** Replace all type variables in `what` with their instantiations.
     * Requires that `tc` has already been solved.
     */
    static std::shared_ptr<Type> instantiate(Context ctx, const std::shared_ptr<Type> &what, const TypeConstraint &tc);

    static std::shared_ptr<Type> replaceSelfType(Context ctx, const std::shared_ptr<Type> &what,
                                                 const std::shared_ptr<Type> &receiver);
    /** Get rid of type variables in `what` and return a type that we deem close enough to continue
     * typechecking. We should be careful to only used this type when we are trying to guess a type.
     * We should do proper instatiation and subtype test after we have guessed type variables with
     * tc.solve(). If the constraint has already been solved, use `instantiate` instead.
     */
    static std::shared_ptr<Type> approximate(Context ctx, const std::shared_ptr<Type> &what, const TypeConstraint &tc);

    static std::shared_ptr<Type> dropLiteral(const std::shared_ptr<Type> &type);

    /** Internal implementation. You should probably use all(). */
    static std::shared_ptr<Type> glb(Context ctx, const std::shared_ptr<Type> &t1, const std::shared_ptr<Type> &t2);
    /** Internal implementation. You should probably use any(). */
    static std::shared_ptr<Type> lub(Context ctx, const std::shared_ptr<Type> &t1, const std::shared_ptr<Type> &t2);

    static std::shared_ptr<Type> lubAll(Context ctx, std::vector<std::shared_ptr<Type>> &elements);
    static std::shared_ptr<Type> arrayOf(Context ctx, const std::shared_ptr<Type> &elem);
};

struct Intrinsic {
    const SymbolRef symbol;
    const bool singleton;
    const NameRef method;
    const Symbol::IntrinsicMethod *impl;
};
extern const std::vector<Intrinsic> intrinsicMethods;

class TypeAndOrigins final {
public:
    std::shared_ptr<Type> type;
    InlinedVector<Loc, 2> origins;
    std::vector<ErrorLine> origins2Explanations(Context ctx);
    ~TypeAndOrigins() {
        histogramInc("TypeAndOrigins.origins.size", origins.size());
    }
};

struct DispatchComponent {
    std::shared_ptr<Type> receiver;
    SymbolRef method;
    std::vector<std::unique_ptr<BasicError>> errors;
};

struct DispatchResult {
    using ComponentVec = absl::InlinedVector<DispatchComponent, 1>;

    std::shared_ptr<Type> returnType;
    ComponentVec components;

    DispatchResult() = default;
    DispatchResult(const std::shared_ptr<Type> &returnType, ComponentVec components)
        : returnType(move(returnType)), components(move(components)){};

    DispatchResult(const std::shared_ptr<Type> &returnType, const std::shared_ptr<Type> &receiver, SymbolRef method)
        : returnType(move(returnType)) {
        components.emplace_back(DispatchComponent{move(receiver), method, std::vector<std::unique_ptr<BasicError>>()});
    }
};

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
    virtual std::shared_ptr<Type> _instantiate(Context ctx, std::vector<SymbolRef> params,
                                               const std::vector<std::shared_ptr<Type>> &targs) = 0;
    virtual std::shared_ptr<Type> _instantiate(Context ctx, const TypeConstraint &tc);

    virtual std::shared_ptr<Type> _replaceSelfType(Context ctx, std::shared_ptr<Type> receiver);
    virtual DispatchResult dispatchCall(Context ctx, NameRef name, Loc callLoc, std::vector<TypeAndOrigins> &args,
                                        std::shared_ptr<Type> selfRef, std::shared_ptr<Type> fullType,
                                        std::shared_ptr<SendAndBlockLink> linkType) = 0;
    virtual std::shared_ptr<Type> getCallArgumentType(Context ctx, NameRef name, int i) = 0;
    virtual bool derivesFrom(const GlobalState &gs, SymbolRef klass) = 0;
    virtual void _sanityCheck(Context ctx) = 0;
    void sanityCheck(Context ctx) {
        if (!debug_mode)
            return;
        _sanityCheck(ctx);
    }

    bool isUntyped();
    bool isBottom();
    bool isTop();
    virtual bool hasUntyped();
    virtual bool isFullyDefined() = 0;
    virtual int kind() = 0;
    virtual std::shared_ptr<Type> _approximate(Context ctx, const TypeConstraint &tc);
    unsigned int hash(const GlobalState &gs) const;
};

template <class To> To *cast_type(Type *what) {
    static_assert(!std::is_pointer<To>::value, "To has to be a pointer");
    static_assert(std::is_assignable<Type *&, To *>::value, "Ill Formed To, has to be a subclass of Type");
    return fast_cast<Type, To>(what);
}

template <class To> bool isa_type(Type *what) {
    return cast_type<To>(what) != nullptr;
}

class SendAndBlockLink {
public:
    SymbolRef block;
    std::shared_ptr<Type> receiver;
    NameRef fun;
    std::shared_ptr<TypeConstraint> constr;
    std::shared_ptr<Type> returnTp;
    std::shared_ptr<Type> blockPreType;
    std::shared_ptr<Type> sendTp;
    SendAndBlockLink(const SendAndBlockLink &) = delete;
    SendAndBlockLink(SymbolRef block, NameRef fun);
};

class GroundType : public Type {};

class ProxyType : public Type {
public:
    // TODO: use shared pointers that use inline counter
    std::shared_ptr<Type> underlying;
    ProxyType(std::shared_ptr<Type> underlying);

    virtual DispatchResult dispatchCall(Context ctx, NameRef name, Loc callLoc, std::vector<TypeAndOrigins> &args,
                                        std::shared_ptr<Type> selfRef, std::shared_ptr<Type> fullType,
                                        std::shared_ptr<SendAndBlockLink> block) override;
    virtual std::shared_ptr<Type> getCallArgumentType(Context ctx, NameRef name, int i) override;
    virtual bool derivesFrom(const GlobalState &gs, SymbolRef klass) override;

    void _sanityCheck(Context ctx) override;
};

class ClassType final : public GroundType {
public:
    SymbolRef symbol;
    ClassType(SymbolRef symbol);
    virtual int kind() final;

    virtual std::string toString(const GlobalState &gs, int tabs = 0) const final;
    virtual std::string show(const GlobalState &gs) const final;
    virtual std::string typeName() const final;
    virtual DispatchResult dispatchCall(Context ctx, NameRef fun, Loc callLoc, std::vector<TypeAndOrigins> &args,
                                        std::shared_ptr<Type> selfRef, std::shared_ptr<Type> fullType,
                                        std::shared_ptr<SendAndBlockLink> block) final;
    DispatchResult dispatchCallWithTargs(Context ctx, NameRef fun, Loc callLoc, std::vector<TypeAndOrigins> &args,
                                         std::shared_ptr<Type> selfRef, std::shared_ptr<Type> fullType,
                                         std::vector<std::shared_ptr<Type>> &targs,
                                         std::shared_ptr<SendAndBlockLink> block);
    DispatchResult dispatchCallIntrinsic(Context ctx, NameRef name, Loc callLoc, std::vector<TypeAndOrigins> &args,
                                         std::shared_ptr<Type> fullType, std::vector<std::shared_ptr<Type>> &targs,
                                         std::shared_ptr<SendAndBlockLink> block);

    virtual std::shared_ptr<Type> getCallArgumentType(Context ctx, NameRef name, int i) final;
    virtual bool derivesFrom(const GlobalState &gs, SymbolRef klass) final;
    void _sanityCheck(Context ctx) final;
    virtual std::shared_ptr<Type> _instantiate(Context ctx, std::vector<SymbolRef> params,
                                               const std::vector<std::shared_ptr<Type>> &targs) override;
    virtual bool isFullyDefined() final;
    virtual bool hasUntyped() override;
};

class OrType final : public GroundType {
public:
    std::shared_ptr<Type> left;
    std::shared_ptr<Type> right;
    virtual int kind() final;

    virtual std::string toString(const GlobalState &gs, int tabs = 0) const final;
    virtual std::string show(const GlobalState &gs) const final;
    virtual std::string typeName() const final;
    virtual DispatchResult dispatchCall(Context ctx, NameRef name, Loc callLoc, std::vector<TypeAndOrigins> &args,
                                        std::shared_ptr<Type> selfRef, std::shared_ptr<Type> fullType,
                                        std::shared_ptr<SendAndBlockLink> block) final;
    virtual std::shared_ptr<Type> getCallArgumentType(Context ctx, NameRef name, int i) final;
    virtual bool derivesFrom(const GlobalState &gs, SymbolRef klass) final;
    void _sanityCheck(Context ctx) final;
    virtual bool isFullyDefined() final;
    virtual std::shared_ptr<Type> _instantiate(Context ctx, std::vector<SymbolRef> params,
                                               const std::vector<std::shared_ptr<Type>> &targs) override;
    virtual bool hasUntyped() override;
    virtual std::shared_ptr<Type> _approximate(Context ctx, const TypeConstraint &tc) override;
    virtual std::shared_ptr<Type> _instantiate(Context ctx, const TypeConstraint &tc) override;
    virtual std::shared_ptr<Type> _replaceSelfType(Context ctx, std::shared_ptr<Type> receiver) override;

private:
    /*
     * You probably want Types::any() instead.
     */
    OrType(std::shared_ptr<Type> left, std::shared_ptr<Type> right);

    /*
     * These implementation methods are allowed to directly instantiate
     * `OrType`. Because `make_shared<OrType>` is not compatible with private
     * constructors + friend declarations (it's `shared_ptr` calling the
     * constructor, so the friend declaration doesn't work), we provide the
     * `make_shared` helper here.
     */
    friend std::shared_ptr<Type> Types::falsyTypes();
    friend std::shared_ptr<Type> Types::Boolean();
    friend class GlobalSubstitution;
    friend class serialize::SerializerImpl;
    friend std::shared_ptr<Type> lubDistributeOr(Context ctx, const std::shared_ptr<Type> &t1,
                                                 const std::shared_ptr<Type> &t2);
    friend std::shared_ptr<Type> lubGround(Context ctx, const std::shared_ptr<Type> &t1,
                                           const std::shared_ptr<Type> &t2);
    friend std::shared_ptr<Type> Types::lub(Context ctx, const std::shared_ptr<Type> &t1,
                                            const std::shared_ptr<Type> &t2);
    friend std::shared_ptr<Type> Types::glb(Context ctx, const std::shared_ptr<Type> &t1,
                                            const std::shared_ptr<Type> &t2);

    static inline std::shared_ptr<Type> make_shared(std::shared_ptr<Type> left, std::shared_ptr<Type> right) {
        std::shared_ptr<Type> res(new OrType(left, right));
        return res;
    }
};

class AndType final : public GroundType {
public:
    std::shared_ptr<Type> left;
    std::shared_ptr<Type> right;
    virtual int kind() final;

    virtual std::string toString(const GlobalState &gs, int tabs = 0) const final;
    virtual std::string show(const GlobalState &gs) const final;
    virtual std::string typeName() const final;
    virtual DispatchResult dispatchCall(Context ctx, NameRef name, Loc callLoc, std::vector<TypeAndOrigins> &args,
                                        std::shared_ptr<Type> selfRef, std::shared_ptr<Type> fullType,
                                        std::shared_ptr<SendAndBlockLink> block) final;

    virtual std::shared_ptr<Type> getCallArgumentType(Context ctx, NameRef name, int i) final;
    virtual bool derivesFrom(const GlobalState &gs, SymbolRef klass) final;
    void _sanityCheck(Context ctx) final;
    virtual bool isFullyDefined() final;
    virtual std::shared_ptr<Type> _instantiate(Context ctx, std::vector<SymbolRef> params,
                                               const std::vector<std::shared_ptr<Type>> &targs) override;
    virtual std::shared_ptr<Type> _replaceSelfType(Context ctx, std::shared_ptr<Type> receiver) override;
    virtual bool hasUntyped() override;
    virtual std::shared_ptr<Type> _approximate(Context ctx, const TypeConstraint &tc) override;
    virtual std::shared_ptr<Type> _instantiate(Context ctx, const TypeConstraint &tc) override;

private:
    /*
     * You probably want Types::all() instead.
     */
    AndType(std::shared_ptr<Type> left, std::shared_ptr<Type> right);

    friend class GlobalSubstitution;
    friend class serialize::SerializerImpl;
    friend class TypeConstraint;

    friend std::shared_ptr<Type> lubGround(Context ctx, const std::shared_ptr<Type> &t1,
                                           const std::shared_ptr<Type> &t2);
    friend std::shared_ptr<Type> glbDistributeAnd(Context ctx, const std::shared_ptr<Type> &t1,
                                                  const std::shared_ptr<Type> &t2);
    friend std::shared_ptr<Type> glbGround(Context ctx, const std::shared_ptr<Type> &t1,
                                           const std::shared_ptr<Type> &t2);
    friend std::shared_ptr<Type> Types::lub(Context ctx, const std::shared_ptr<Type> &t1,
                                            const std::shared_ptr<Type> &t2);
    friend std::shared_ptr<Type> Types::glb(Context ctx, const std::shared_ptr<Type> &t1,
                                            const std::shared_ptr<Type> &t2);

    static inline std::shared_ptr<Type> make_shared(std::shared_ptr<Type> left, std::shared_ptr<Type> right) {
        std::shared_ptr<Type> res(new AndType(left, right));
        return res;
    }
};

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

    virtual std::shared_ptr<Type> _instantiate(Context ctx, std::vector<SymbolRef> params,
                                               const std::vector<std::shared_ptr<Type>> &targs) override;
    virtual int kind() final;
    virtual std::shared_ptr<Type> _replaceSelfType(Context ctx, std::shared_ptr<Type> receiver) override;

    virtual DispatchResult dispatchCall(Context ctx, NameRef name, Loc callLoc, std::vector<TypeAndOrigins> &args,
                                        std::shared_ptr<Type> selfRef, std::shared_ptr<Type> fullType,
                                        std::shared_ptr<SendAndBlockLink> link) final;
    void _sanityCheck(Context ctx) final;
    virtual std::shared_ptr<Type> getCallArgumentType(Context ctx, NameRef name, int i) final;
    virtual bool derivesFrom(const GlobalState &gs, SymbolRef klass) final;
};

class LiteralType final : public ProxyType {
public:
    union {
        int64_t value;
        double floatval;
    };
    LiteralType(int64_t val);
    LiteralType(double val);
    LiteralType(SymbolRef klass, NameRef val);
    LiteralType(bool val);

    virtual std::string toString(const GlobalState &gs, int tabs = 0) const final;
    virtual std::string show(const GlobalState &gs) const final;
    virtual std::string showValue(const GlobalState &gs) const final;
    virtual std::string typeName() const override;
    virtual bool isFullyDefined() final;

    bool equals(std::shared_ptr<LiteralType> rhs) const;

    virtual std::shared_ptr<Type> _instantiate(Context ctx, std::vector<SymbolRef> params,
                                               const std::vector<std::shared_ptr<Type>> &targs) override;
    virtual int kind() final;
};

class ShapeType final : public ProxyType {
public:
    std::vector<std::shared_ptr<LiteralType>> keys; // TODO: store sorted by whatever
    std::vector<std::shared_ptr<Type>> values;
    ShapeType();
    ShapeType(std::vector<std::shared_ptr<LiteralType>> keys, std::vector<std::shared_ptr<Type>> values);

    virtual std::string toString(const GlobalState &gs, int tabs = 0) const final;
    virtual std::string show(const GlobalState &gs) const final;
    virtual std::string typeName() const override;
    virtual DispatchResult dispatchCall(Context ctx, NameRef fun, Loc callLoc, std::vector<TypeAndOrigins> &args,
                                        std::shared_ptr<Type> selfRef, std::shared_ptr<Type> fullType,
                                        std::shared_ptr<SendAndBlockLink> block) final;
    void _sanityCheck(Context ctx) final;
    virtual bool isFullyDefined() final;

    virtual std::shared_ptr<Type> _instantiate(Context ctx, std::vector<SymbolRef> params,
                                               const std::vector<std::shared_ptr<Type>> &targs) override;
    virtual int kind() final;
    virtual bool hasUntyped() override;
    virtual std::shared_ptr<Type> _approximate(Context ctx, const TypeConstraint &tc) override;
    virtual std::shared_ptr<Type> _instantiate(Context ctx, const TypeConstraint &tc) override;
};

class TupleType final : public ProxyType {
private:
    TupleType() = delete;

public:
    std::vector<std::shared_ptr<Type>> elems;

    TupleType(std::shared_ptr<Type> underlying, std::vector<std::shared_ptr<Type>> elements);
    static std::shared_ptr<Type> build(Context ctx, std::vector<std::shared_ptr<Type>> elements);

    virtual std::string toString(const GlobalState &gs, int tabs = 0) const final;
    virtual std::string show(const GlobalState &gs) const final;
    virtual std::string typeName() const override;
    void _sanityCheck(Context ctx) final;
    virtual bool isFullyDefined() final;

    virtual std::shared_ptr<Type> _instantiate(Context ctx, std::vector<SymbolRef> params,
                                               const std::vector<std::shared_ptr<Type>> &targs) override;
    virtual int kind() final;
    virtual DispatchResult dispatchCall(Context ctx, NameRef fun, Loc callLoc, std::vector<TypeAndOrigins> &args,
                                        std::shared_ptr<Type> selfRef, std::shared_ptr<Type> fullType,
                                        std::shared_ptr<SendAndBlockLink> block) override;
    virtual bool hasUntyped() override;
    virtual std::shared_ptr<Type> _approximate(Context ctx, const TypeConstraint &tc) override;
    virtual std::shared_ptr<Type> _instantiate(Context ctx, const TypeConstraint &tc) override;

    // Return the type of the underlying array that this tuple decays into
    std::shared_ptr<Type> elementType() const;
};

class TypeVar final : public Type {
public:
    SymbolRef sym;
    TypeVar(SymbolRef sym);
    virtual std::string toString(const GlobalState &gs, int tabs = 0) const final;
    virtual std::string show(const GlobalState &gs) const final;
    virtual std::string typeName() const final;
    virtual DispatchResult dispatchCall(Context ctx, NameRef name, Loc callLoc, std::vector<TypeAndOrigins> &args,
                                        std::shared_ptr<Type> selfRef, std::shared_ptr<Type> fullType,
                                        std::shared_ptr<SendAndBlockLink> link) final;
    void _sanityCheck(Context ctx) final;
    virtual bool isFullyDefined() final;
    virtual std::shared_ptr<Type> getCallArgumentType(Context ctx, NameRef name, int i) final;
    virtual bool derivesFrom(const GlobalState &gs, SymbolRef klass) final;

    virtual std::shared_ptr<Type> _instantiate(Context ctx, std::vector<SymbolRef> params,
                                               const std::vector<std::shared_ptr<Type>> &targs) override;
    virtual int kind() final;
    virtual std::shared_ptr<Type> _approximate(Context ctx, const TypeConstraint &tc) override;
    virtual std::shared_ptr<Type> _instantiate(Context ctx, const TypeConstraint &tc) override;
};

class AppliedType final : public Type {
public:
    // .underlying is always a ClassType
    SymbolRef klass;
    std::vector<std::shared_ptr<Type>> targs;
    AppliedType(SymbolRef klass, std::vector<std::shared_ptr<Type>> targs) : klass(klass), targs(targs){};

    virtual std::string toString(const GlobalState &gs, int tabs = 0) const final;
    virtual std::string show(const GlobalState &gs) const final;
    virtual std::string typeName() const final;
    virtual DispatchResult dispatchCall(Context ctx, NameRef name, Loc callLoc, std::vector<TypeAndOrigins> &args,
                                        std::shared_ptr<Type> selfRef, std::shared_ptr<Type> fullType,
                                        std::shared_ptr<SendAndBlockLink> block) final;
    void _sanityCheck(Context ctx) final;
    virtual bool isFullyDefined() final;

    virtual std::shared_ptr<Type> _instantiate(Context ctx, std::vector<SymbolRef> params,
                                               const std::vector<std::shared_ptr<Type>> &targs) override;
    virtual int kind() final;

    virtual std::shared_ptr<Type> getCallArgumentType(Context ctx, NameRef name, int i) final;

    virtual bool derivesFrom(const GlobalState &gs, SymbolRef klass) final;
    virtual bool hasUntyped() override;
    virtual std::shared_ptr<Type> _approximate(Context ctx, const TypeConstraint &tc) override;
    virtual std::shared_ptr<Type> _instantiate(Context ctx, const TypeConstraint &tc) override;
};

// MetaType is the type of a Type. You can think of it as generalization of
// Ruby's singleton classes to Types; Just as `A.singleton_class` is the *type*
// of the *value* `A`, MetaType[T] is the *type* of a *value* that holds the
// type T during execution. For instance, the type of `T.untyped` is
// `MetaType(Types::untyped())`.
//
// These are used within the inferencer in places where we need to track
// user-written types in the source code.
class MetaType final : public Type {
public:
    std::shared_ptr<Type> wrapped;

    MetaType(std::shared_ptr<Type> wrapped);

    virtual std::string toString(const GlobalState &gs, int tabs = 0) const final;
    virtual std::string show(const GlobalState &gs) const final;
    virtual std::string typeName() const final;

    virtual bool derivesFrom(const GlobalState &gs, SymbolRef klass) final;

    virtual DispatchResult dispatchCall(Context ctx, NameRef name, Loc callLoc, std::vector<TypeAndOrigins> &args,
                                        std::shared_ptr<Type> selfRef, std::shared_ptr<Type> fullType,
                                        std::shared_ptr<SendAndBlockLink> block) final;
    virtual std::shared_ptr<Type> getCallArgumentType(Context ctx, NameRef name, int i) final;
    void _sanityCheck(Context ctx) final;
    virtual bool isFullyDefined() final;

    virtual std::shared_ptr<Type> _instantiate(Context ctx, std::vector<SymbolRef> params,
                                               const std::vector<std::shared_ptr<Type>> &targs) override;
    virtual int kind() final;
    virtual std::shared_ptr<Type> _approximate(Context ctx, const TypeConstraint &tc) override;
};

class LambdaParam final : public Type {
public:
    SymbolRef definition;

    LambdaParam(const SymbolRef definition);
    virtual std::string toString(const GlobalState &gs, int tabs = 0) const final;
    virtual std::string show(const GlobalState &gs) const final;
    virtual std::string typeName() const final;

    virtual bool derivesFrom(const GlobalState &gs, SymbolRef klass) final;

    virtual DispatchResult dispatchCall(Context ctx, NameRef name, Loc callLoc, std::vector<TypeAndOrigins> &args,
                                        std::shared_ptr<Type> selfRef, std::shared_ptr<Type> fullType,
                                        std::shared_ptr<SendAndBlockLink> link) final;
    virtual std::shared_ptr<Type> getCallArgumentType(Context ctx, NameRef name, int i) final;
    void _sanityCheck(Context ctx) final;
    virtual bool isFullyDefined() final;

    virtual std::shared_ptr<Type> _instantiate(Context ctx, std::vector<SymbolRef> params,
                                               const std::vector<std::shared_ptr<Type>> &targs) override;
    virtual int kind() final;
};

class SelfTypeParam final : public Type {
public:
    SymbolRef definition;

    SelfTypeParam(const SymbolRef definition);
    virtual std::string toString(const GlobalState &gs, int tabs = 0) const final;
    virtual std::string show(const GlobalState &gs) const final;
    virtual std::string typeName() const final;

    virtual bool derivesFrom(const GlobalState &gs, SymbolRef klass) final;

    virtual DispatchResult dispatchCall(Context ctx, NameRef name, Loc callLoc, std::vector<TypeAndOrigins> &args,
                                        std::shared_ptr<Type> selfType, std::shared_ptr<Type> fullType,
                                        std::shared_ptr<SendAndBlockLink> block) final;
    virtual std::shared_ptr<Type> getCallArgumentType(Context ctx, NameRef name, int i) final;
    void _sanityCheck(Context ctx) final;
    virtual bool isFullyDefined() final;

    virtual std::shared_ptr<Type> _instantiate(Context ctx, std::vector<SymbolRef> params,
                                               const std::vector<std::shared_ptr<Type>> &targs) override;
    virtual int kind() final;
};

class AliasType final : public Type {
public:
    AliasType(SymbolRef other);
    virtual std::string toString(const GlobalState &gs, int tabs = 0) const final;
    virtual std::string show(const GlobalState &gs) const final;
    virtual std::string typeName() const final;
    virtual DispatchResult dispatchCall(Context ctx, NameRef name, Loc callLoc, std::vector<TypeAndOrigins> &args,
                                        std::shared_ptr<Type> selfRef, std::shared_ptr<Type> fullType,
                                        std::shared_ptr<SendAndBlockLink> link) final;
    virtual std::shared_ptr<Type> getCallArgumentType(Context ctx, NameRef name, int i) final;
    virtual bool derivesFrom(const GlobalState &gs, SymbolRef klass) final;

    SymbolRef symbol;
    void _sanityCheck(Context ctx) final;
    virtual bool isFullyDefined() final;

    virtual std::shared_ptr<Type> _instantiate(Context ctx, std::vector<SymbolRef> params,
                                               const std::vector<std::shared_ptr<Type>> &targs) override;
    virtual int kind() final;
};

} // namespace core
} // namespace sorbet
#endif // SORBET_TYPES_H
