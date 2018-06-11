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
    static std::shared_ptr<Type> all(core::Context ctx, std::shared_ptr<Type> t1, std::shared_ptr<Type> t2);

    /** Lower upper bound: the narrowest type that is supper type of both t1 and t2 */
    static std::shared_ptr<Type> any(core::Context ctx, std::shared_ptr<Type> t1, std::shared_ptr<Type> t2);

    /** is every instance of  t1 an  instance of t2? */
    static bool isSubTypeUnderConstraint(core::Context ctx, TypeConstraint &constr, std::shared_ptr<Type> t1,
                                         std::shared_ptr<Type> t2);

    /** is every instance of  t1 an  instance of t2 when not allowed to modify constraint */
    static bool isSubType(core::Context ctx, std::shared_ptr<Type> t1, std::shared_ptr<Type> t2);
    static bool equiv(core::Context ctx, std::shared_ptr<Type> t1, std::shared_ptr<Type> t2);

    static std::shared_ptr<Type> top();
    static std::shared_ptr<Type> bottom();
    static std::shared_ptr<Type> nilClass();
    static std::shared_ptr<Type> dynamic();
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

    static std::shared_ptr<Type> dropSubtypesOf(core::Context ctx, std::shared_ptr<Type> from, core::SymbolRef klass);
    static std::shared_ptr<Type> approximateSubtract(core::Context ctx, std::shared_ptr<Type> from,
                                                     std::shared_ptr<Type> what);
    static bool canBeTruthy(core::Context ctx, std::shared_ptr<Type> what);
    static bool canBeFalsy(core::Context ctx, std::shared_ptr<Type> what);
    enum Combinator { OR, AND };

    static std::shared_ptr<Type> resultTypeAsSeenFrom(core::Context ctx, core::SymbolRef what, core::SymbolRef inWhat,
                                                      const std::vector<std::shared_ptr<Type>> &targs);

    static std::vector<core::SymbolRef> alignBaseTypeArgs(core::Context ctx, core::SymbolRef what,
                                                          const std::vector<std::shared_ptr<Type>> &targs,
                                                          core::SymbolRef asIf);
    // Extract the return value type from a proc.
    static std::shared_ptr<core::Type> getProcReturnType(core::Context ctx, std::shared_ptr<core::Type> procType);
    static std::shared_ptr<Type> instantiate(core::Context ctx, std::shared_ptr<core::Type> what,
                                             std::vector<SymbolRef> params,
                                             const std::vector<std::shared_ptr<Type>> &targs);
    /** Replace all type variables in `what` with their instantiations.
     * Requires that `tc` has already been solved.
     */
    static std::shared_ptr<Type> instantiate(core::Context ctx, std::shared_ptr<core::Type> what,
                                             const TypeConstraint &tc);
    /** Get rid of type variables in `what` and return a type that we deem close enough to continue
     * typechecking. We should be careful to only used this type when we are trying to guess a type.
     * We should do proper instatiation and subtype test after we have guessed type variables with
     * tc.solve(). If the constraint has already been solved, use `instantiate` instead.
     */
    static std::shared_ptr<Type> approximate(core::Context ctx, std::shared_ptr<core::Type> what,
                                             const TypeConstraint &tc);

    /** Internal implementation. You should probably use all(). */
    static std::shared_ptr<Type> glb(core::Context ctx, std::shared_ptr<Type> t1, std::shared_ptr<Type> t2);
    /** Internal implementation. You should probably use any(). */
    static std::shared_ptr<Type> lub(core::Context ctx, std::shared_ptr<Type> t1, std::shared_ptr<Type> t2);
};

class TypeAndOrigins final {
public:
    std::shared_ptr<core::Type> type;
    InlinedVector<core::Loc, 2> origins;
    std::vector<ErrorLine> origins2Explanations(core::Context ctx);
    ~TypeAndOrigins() {
        core::histogramInc("TypeAndOrigins.origins.size", origins.size());
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
    virtual std::shared_ptr<Type> _instantiate(core::Context ctx, std::vector<SymbolRef> params,
                                               const std::vector<std::shared_ptr<Type>> &targs) = 0;
    virtual std::shared_ptr<Type> _instantiate(core::Context ctx, const TypeConstraint &tc);

    // blockType is both an in and and out param; If nullptr, it indicates that
    // the caller is not passing a block; If populated, `dispatchCall` will set
    // to the type of the block argument to the called method, if any.
    virtual std::shared_ptr<Type> dispatchCall(core::Context ctx, core::NameRef name, core::Loc callLoc,
                                               std::vector<TypeAndOrigins> &args, std::shared_ptr<Type> selfRef,
                                               std::shared_ptr<Type> fullType,
                                               std::shared_ptr<SendAndBlockLink> linkType) = 0;
    virtual std::shared_ptr<Type> getCallArgumentType(core::Context ctx, core::NameRef name, int i) = 0;
    virtual bool derivesFrom(const core::GlobalState &gs, core::SymbolRef klass) = 0;
    virtual void _sanityCheck(core::Context ctx) = 0;
    void sanityCheck(core::Context ctx) {
        if (!debug_mode)
            return;
        _sanityCheck(ctx);
    }

    bool isDynamic();
    bool isBottom();
    bool isTop();
    virtual bool hasUntyped();
    virtual bool isFullyDefined() = 0;
    virtual int kind() = 0;
    virtual std::shared_ptr<Type> _approximate(core::Context ctx, const TypeConstraint &tc);
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
    core::SymbolRef block;
    std::shared_ptr<TypeConstraint> constr;
    std::shared_ptr<Type> returnTp;
    std::shared_ptr<Type> blockPreType;
    std::shared_ptr<Type> sendTp;
    bool isDynamic = false;
    SendAndBlockLink(const SendAndBlockLink &) = delete;
    SendAndBlockLink(SymbolRef block);
};

class GroundType : public Type {};

class ProxyType : public Type {
public:
    // TODO: use shared pointers that use inline counter
    std::shared_ptr<Type> underlying;
    ProxyType(std::shared_ptr<Type> underlying);

    virtual std::shared_ptr<Type> dispatchCall(core::Context ctx, core::NameRef name, core::Loc callLoc,
                                               std::vector<TypeAndOrigins> &args, std::shared_ptr<Type> selfRef,
                                               std::shared_ptr<Type> fullType,
                                               std::shared_ptr<SendAndBlockLink> block) override;
    virtual std::shared_ptr<Type> getCallArgumentType(core::Context ctx, core::NameRef name, int i) override;
    virtual bool derivesFrom(const core::GlobalState &gs, core::SymbolRef klass) override;

    void _sanityCheck(core::Context ctx) override;
};

class ClassType final : public GroundType {
public:
    core::SymbolRef symbol;
    ClassType(core::SymbolRef symbol);
    virtual int kind() final;

    virtual std::string toString(const GlobalState &gs, int tabs = 0) const final;
    virtual std::string show(const GlobalState &gs) const final;
    virtual std::string typeName() const final;
    virtual std::shared_ptr<Type> dispatchCall(core::Context ctx, core::NameRef fun, core::Loc callLoc,
                                               std::vector<TypeAndOrigins> &args, std::shared_ptr<Type> selfRef,
                                               std::shared_ptr<Type> fullType,
                                               std::shared_ptr<SendAndBlockLink> block) final;
    std::shared_ptr<Type> dispatchCallWithTargs(core::Context ctx, core::NameRef fun, core::Loc callLoc,
                                                std::vector<TypeAndOrigins> &args, std::shared_ptr<Type> fullType,
                                                std::vector<std::shared_ptr<Type>> &targs,
                                                std::shared_ptr<SendAndBlockLink> block);
    std::shared_ptr<Type> dispatchCallIntrinsic(core::Context ctx, core::NameRef name, core::Loc callLoc,
                                                std::vector<TypeAndOrigins> &args, std::shared_ptr<Type> fullType,
                                                std::vector<std::shared_ptr<Type>> &targs,
                                                std::shared_ptr<SendAndBlockLink> block);

    virtual std::shared_ptr<Type> getCallArgumentType(core::Context ctx, core::NameRef name, int i) final;
    virtual bool derivesFrom(const core::GlobalState &gs, core::SymbolRef klass) final;
    void _sanityCheck(core::Context ctx) final;
    virtual std::shared_ptr<Type> _instantiate(core::Context ctx, std::vector<SymbolRef> params,
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
    virtual std::shared_ptr<Type> dispatchCall(core::Context ctx, core::NameRef name, core::Loc callLoc,
                                               std::vector<TypeAndOrigins> &args, std::shared_ptr<Type> selfRef,
                                               std::shared_ptr<Type> fullType,
                                               std::shared_ptr<SendAndBlockLink> block) final;
    virtual std::shared_ptr<Type> getCallArgumentType(core::Context ctx, core::NameRef name, int i) final;
    virtual bool derivesFrom(const core::GlobalState &gs, core::SymbolRef klass) final;
    void _sanityCheck(core::Context ctx) final;
    virtual bool isFullyDefined() final;
    virtual std::shared_ptr<Type> _instantiate(core::Context ctx, std::vector<SymbolRef> params,
                                               const std::vector<std::shared_ptr<Type>> &targs) override;
    virtual bool hasUntyped() override;
    virtual std::shared_ptr<Type> _approximate(core::Context ctx, const TypeConstraint &tc) override;
    virtual std::shared_ptr<Type> _instantiate(core::Context ctx, const TypeConstraint &tc) override;

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
    friend class core::GlobalSubstitution;
    friend class core::serialize::Serializer;
    friend std::shared_ptr<Type> lubDistributeOr(core::Context ctx, std::shared_ptr<Type> t1, std::shared_ptr<Type> t2);
    friend std::shared_ptr<Type> lubGround(core::Context ctx, std::shared_ptr<Type> t1, std::shared_ptr<Type> t2);
    friend std::shared_ptr<Type> Types::lub(core::Context ctx, std::shared_ptr<Type> t1, std::shared_ptr<Type> t2);
    friend std::shared_ptr<Type> Types::glb(core::Context ctx, std::shared_ptr<Type> t1, std::shared_ptr<Type> t2);

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
    virtual std::shared_ptr<Type> dispatchCall(Context ctx, core::NameRef name, core::Loc callLoc,
                                               std::vector<TypeAndOrigins> &args, std::shared_ptr<Type> selfRef,
                                               std::shared_ptr<Type> fullType,
                                               std::shared_ptr<SendAndBlockLink> block) final;

    virtual std::shared_ptr<Type> getCallArgumentType(Context ctx, core::NameRef name, int i) final;
    virtual bool derivesFrom(const core::GlobalState &gs, core::SymbolRef klass) final;
    void _sanityCheck(core::Context ctx) final;
    virtual bool isFullyDefined() final;
    virtual std::shared_ptr<Type> _instantiate(core::Context ctx, std::vector<SymbolRef> params,
                                               const std::vector<std::shared_ptr<Type>> &targs) override;
    virtual bool hasUntyped() override;
    virtual std::shared_ptr<Type> _approximate(core::Context ctx, const TypeConstraint &tc) override;
    virtual std::shared_ptr<Type> _instantiate(core::Context ctx, const TypeConstraint &tc) override;

private:
    /*
     * You probably want Types::all() instead.
     */
    AndType(std::shared_ptr<Type> left, std::shared_ptr<Type> right);

    friend class core::GlobalSubstitution;
    friend class core::serialize::Serializer;
    friend class core::TypeConstraint;

    friend std::shared_ptr<Type> lubGround(Context ctx, std::shared_ptr<Type> t1, std::shared_ptr<Type> t2);
    friend std::shared_ptr<Type> glbDistributeAnd(Context ctx, std::shared_ptr<Type> t1, std::shared_ptr<Type> t2);
    friend std::shared_ptr<Type> glbGround(Context ctx, std::shared_ptr<Type> t1, std::shared_ptr<Type> t2);
    friend std::shared_ptr<Type> Types::lub(core::Context ctx, std::shared_ptr<Type> t1, std::shared_ptr<Type> t2);
    friend std::shared_ptr<Type> Types::glb(core::Context ctx, std::shared_ptr<Type> t1, std::shared_ptr<Type> t2);

    static inline std::shared_ptr<Type> make_shared(std::shared_ptr<Type> left, std::shared_ptr<Type> right) {
        std::shared_ptr<Type> res(new AndType(left, right));
        return res;
    }
};

class LiteralType final : public ProxyType {
public:
    union {
        int64_t value;
        double floatval;
    };
    LiteralType(int64_t val);
    LiteralType(double val);
    LiteralType(core::SymbolRef klass, core::NameRef val);
    LiteralType(bool val);

    virtual std::string toString(const GlobalState &gs, int tabs = 0) const final;
    virtual std::string show(const GlobalState &gs) const final;
    virtual std::string showValue(const GlobalState &gs) const final;
    virtual std::string typeName() const override;
    virtual bool isFullyDefined() final;

    virtual std::shared_ptr<Type> _instantiate(core::Context ctx, std::vector<SymbolRef> params,
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
    virtual std::shared_ptr<Type> dispatchCall(core::Context ctx, core::NameRef fun, core::Loc callLoc,
                                               std::vector<TypeAndOrigins> &args, std::shared_ptr<Type> selfRef,
                                               std::shared_ptr<Type> fullType,
                                               std::shared_ptr<SendAndBlockLink> block) final;
    void _sanityCheck(core::Context ctx) final;
    virtual bool isFullyDefined() final;

    virtual std::shared_ptr<Type> _instantiate(core::Context ctx, std::vector<SymbolRef> params,
                                               const std::vector<std::shared_ptr<Type>> &targs) override;
    virtual int kind() final;
    virtual bool hasUntyped() override;
    virtual std::shared_ptr<Type> _approximate(core::Context ctx, const TypeConstraint &tc) override;
    virtual std::shared_ptr<Type> _instantiate(core::Context ctx, const TypeConstraint &tc) override;
};

class TupleType final : public ProxyType {
public:
    std::vector<std::shared_ptr<Type>> elems;
    TupleType(std::vector<std::shared_ptr<Type>> elements);

    virtual std::string toString(const GlobalState &gs, int tabs = 0) const final;
    virtual std::string show(const GlobalState &gs) const final;
    virtual std::string typeName() const override;
    void _sanityCheck(core::Context ctx) final;
    virtual bool isFullyDefined() final;

    virtual std::shared_ptr<Type> _instantiate(core::Context ctx, std::vector<SymbolRef> params,
                                               const std::vector<std::shared_ptr<Type>> &targs) override;
    virtual int kind() final;
    virtual std::shared_ptr<Type> dispatchCall(core::Context ctx, core::NameRef fun, core::Loc callLoc,
                                               std::vector<TypeAndOrigins> &args, std::shared_ptr<Type> selfRef,
                                               std::shared_ptr<Type> fullType,
                                               std::shared_ptr<SendAndBlockLink> block) override;
    virtual bool hasUntyped() override;
    virtual std::shared_ptr<Type> _approximate(core::Context ctx, const TypeConstraint &tc) override;
    virtual std::shared_ptr<Type> _instantiate(core::Context ctx, const TypeConstraint &tc) override;
};

// MagicType is the type of the built-in core::Symbols::Magic()
// object. Its `dispatchCall` knows how to handle a number of special methods
// that are used when building CFGs to desugar features that can't be described
// purely within our existing type system and IR.
class MagicType final : public ProxyType {
public:
    MagicType();
    virtual std::string toString(const GlobalState &gs, int tabs = 0) const final;
    virtual std::string show(const GlobalState &gs) const final;
    virtual std::string typeName() const override;
    virtual std::shared_ptr<Type> dispatchCall(core::Context ctx, core::NameRef fun, core::Loc callLoc,
                                               std::vector<TypeAndOrigins> &args, std::shared_ptr<Type> selfRef,
                                               std::shared_ptr<Type> fullType,
                                               std::shared_ptr<SendAndBlockLink> block) final;
    void _sanityCheck(core::Context ctx) final;
    virtual bool isFullyDefined() final;

    virtual std::shared_ptr<Type> _instantiate(core::Context ctx, std::vector<SymbolRef> params,
                                               const std::vector<std::shared_ptr<Type>> &targs) override;
    virtual int kind() final;
};

class TypeVar final : public Type {
public:
    SymbolRef sym;
    TypeVar(SymbolRef sym);
    virtual std::string toString(const GlobalState &gs, int tabs = 0) const final;
    virtual std::string show(const GlobalState &gs) const final;
    virtual std::string typeName() const final;
    virtual std::shared_ptr<Type> dispatchCall(core::Context ctx, core::NameRef name, core::Loc callLoc,
                                               std::vector<TypeAndOrigins> &args, std::shared_ptr<Type> selfRef,
                                               std::shared_ptr<Type> fullType,
                                               std::shared_ptr<SendAndBlockLink> link) final;
    void _sanityCheck(core::Context ctx) final;
    virtual bool isFullyDefined() final;
    virtual std::shared_ptr<Type> getCallArgumentType(core::Context ctx, core::NameRef name, int i) final;
    virtual bool derivesFrom(const core::GlobalState &gs, core::SymbolRef klass) final;

    virtual std::shared_ptr<Type> _instantiate(core::Context ctx, std::vector<SymbolRef> params,
                                               const std::vector<std::shared_ptr<Type>> &targs) override;
    virtual int kind() final;
    virtual std::shared_ptr<Type> _approximate(core::Context ctx, const TypeConstraint &tc) override;
    virtual std::shared_ptr<Type> _instantiate(core::Context ctx, const TypeConstraint &tc) override;
};

class AppliedType final : public Type {
public:
    // .underlying is always a ClassType
    core::SymbolRef klass;
    std::vector<std::shared_ptr<Type>> targs;
    AppliedType(core::SymbolRef klass, std::vector<std::shared_ptr<Type>> targs) : klass(klass), targs(targs){};

    virtual std::string toString(const GlobalState &gs, int tabs = 0) const final;
    virtual std::string show(const GlobalState &gs) const final;
    virtual std::string typeName() const final;
    virtual std::shared_ptr<Type> dispatchCall(core::Context ctx, core::NameRef name, core::Loc callLoc,
                                               std::vector<TypeAndOrigins> &args, std::shared_ptr<Type> selfRef,
                                               std::shared_ptr<Type> fullType,
                                               std::shared_ptr<SendAndBlockLink> block) final;
    void _sanityCheck(core::Context ctx) final;
    virtual bool isFullyDefined() final;

    virtual std::shared_ptr<Type> _instantiate(core::Context ctx, std::vector<SymbolRef> params,
                                               const std::vector<std::shared_ptr<Type>> &targs) override;
    virtual int kind() final;

    virtual std::shared_ptr<Type> getCallArgumentType(core::Context ctx, core::NameRef name, int i) final;

    virtual bool derivesFrom(const core::GlobalState &gs, core::SymbolRef klass) final;
    virtual bool hasUntyped() override;
    virtual std::shared_ptr<Type> _approximate(core::Context ctx, const TypeConstraint &tc) override;
    virtual std::shared_ptr<Type> _instantiate(core::Context ctx, const TypeConstraint &tc) override;
};

// MetaType is the type of a Type. You can think of it as generalization of
// Ruby's singleton classes to Types; Just as `A.singleton_class` is the *type*
// of the *value* `A`, MetaType[T] is the *type* of a *value* that holds the
// type T during execution. For instance, the type of `T.untyped` is
// `MetaType(Types::dynamic())`.
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

    virtual bool derivesFrom(const core::GlobalState &gs, core::SymbolRef klass) final;

    virtual std::shared_ptr<Type> dispatchCall(core::Context ctx, core::NameRef name, core::Loc callLoc,
                                               std::vector<TypeAndOrigins> &args, std::shared_ptr<Type> selfRef,
                                               std::shared_ptr<Type> fullType,
                                               std::shared_ptr<SendAndBlockLink> block) final;
    virtual std::shared_ptr<Type> getCallArgumentType(core::Context ctx, core::NameRef name, int i) final;
    void _sanityCheck(core::Context ctx) final;
    virtual bool isFullyDefined() final;

    virtual std::shared_ptr<Type> _instantiate(core::Context ctx, std::vector<SymbolRef> params,
                                               const std::vector<std::shared_ptr<Type>> &targs) override;
    virtual int kind() final;
    virtual std::shared_ptr<Type> _approximate(core::Context ctx, const TypeConstraint &tc) override;
};

class LambdaParam final : public Type {
public:
    core::SymbolRef definition;

    LambdaParam(const SymbolRef definition);
    virtual std::string toString(const GlobalState &gs, int tabs = 0) const final;
    virtual std::string show(const GlobalState &gs) const final;
    virtual std::string typeName() const final;

    virtual bool derivesFrom(const core::GlobalState &gs, core::SymbolRef klass) final;

    virtual std::shared_ptr<Type> dispatchCall(core::Context ctx, core::NameRef name, core::Loc callLoc,
                                               std::vector<TypeAndOrigins> &args, std::shared_ptr<Type> selfRef,
                                               std::shared_ptr<Type> fullType,
                                               std::shared_ptr<SendAndBlockLink> link) final;
    virtual std::shared_ptr<Type> getCallArgumentType(core::Context ctx, core::NameRef name, int i) final;
    void _sanityCheck(core::Context ctx) final;
    virtual bool isFullyDefined() final;

    virtual std::shared_ptr<Type> _instantiate(core::Context ctx, std::vector<SymbolRef> params,
                                               const std::vector<std::shared_ptr<Type>> &targs) override;
    virtual int kind() final;
};

class SelfTypeParam final : public Type {
public:
    core::SymbolRef definition;

    SelfTypeParam(const SymbolRef definition);
    virtual std::string toString(const GlobalState &gs, int tabs = 0) const final;
    virtual std::string show(const GlobalState &gs) const final;
    virtual std::string typeName() const final;

    virtual bool derivesFrom(const core::GlobalState &gs, core::SymbolRef klass) final;

    virtual std::shared_ptr<Type> dispatchCall(core::Context ctx, core::NameRef name, core::Loc callLoc,
                                               std::vector<TypeAndOrigins> &args, std::shared_ptr<Type> selfType,
                                               std::shared_ptr<Type> fullType,
                                               std::shared_ptr<SendAndBlockLink> block) final;
    virtual std::shared_ptr<Type> getCallArgumentType(core::Context ctx, core::NameRef name, int i) final;
    void _sanityCheck(core::Context ctx) final;
    virtual bool isFullyDefined() final;

    virtual std::shared_ptr<Type> _instantiate(core::Context ctx, std::vector<SymbolRef> params,
                                               const std::vector<std::shared_ptr<Type>> &targs) override;
    virtual int kind() final;
};

class AliasType final : public Type {
public:
    AliasType(SymbolRef other);
    virtual std::string toString(const GlobalState &gs, int tabs = 0) const final;
    virtual std::string show(const GlobalState &gs) const final;
    virtual std::string typeName() const final;
    virtual std::shared_ptr<Type> dispatchCall(core::Context ctx, core::NameRef name, core::Loc callLoc,
                                               std::vector<TypeAndOrigins> &args, std::shared_ptr<Type> selfRef,
                                               std::shared_ptr<Type> fullType,
                                               std::shared_ptr<SendAndBlockLink> link) final;
    virtual std::shared_ptr<Type> getCallArgumentType(core::Context ctx, core::NameRef name, int i) final;
    virtual bool derivesFrom(const core::GlobalState &gs, core::SymbolRef klass) final;

    SymbolRef symbol;
    void _sanityCheck(core::Context ctx) final;
    virtual bool isFullyDefined() final;

    virtual std::shared_ptr<Type> _instantiate(core::Context ctx, std::vector<SymbolRef> params,
                                               const std::vector<std::shared_ptr<Type>> &targs) override;
    virtual int kind() final;
};

} // namespace core
} // namespace sorbet
#endif // SORBET_TYPES_H
