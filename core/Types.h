#ifndef SRUBY_TYPES_H
#define SRUBY_TYPES_H

#include "Context.h"
#include "Symbols.h"
#include <memory>
#include <string>

namespace ruby_typer {
namespace core {
/** Dmitry: unlike in Dotty, those types are always dealiased. For now */
class Type;
class Types final {
public:
    /** Greater lower bound: the widest type that is subtype of both t1 and t2 */
    static std::shared_ptr<Type> glb(core::Context ctx, std::shared_ptr<Type> &t1, std::shared_ptr<Type> &t2);
    inline static std::shared_ptr<Type> glb(core::Context ctx, std::shared_ptr<Type> &t1, std::shared_ptr<Type> &&t2) {
        return glb(ctx, t1, t2);
    }
    inline static std::shared_ptr<Type> glb(core::Context ctx, std::shared_ptr<Type> &&t1, std::shared_ptr<Type> &t2) {
        return glb(ctx, t1, t2);
    }
    inline static std::shared_ptr<Type> glb(core::Context ctx, std::shared_ptr<Type> &&t1, std::shared_ptr<Type> &&t2) {
        return glb(ctx, t1, t2);
    }
    static std::shared_ptr<Type> _glb(core::Context ctx, std::shared_ptr<Type> &t1, std::shared_ptr<Type> &t2);

    /** Lower upper bound: the narrowest type that is supper type of both t1 and t2 */
    static std::shared_ptr<Type> lub(core::Context ctx, std::shared_ptr<Type> &t1, std::shared_ptr<Type> &t2);
    inline static std::shared_ptr<Type> lub(core::Context ctx, std::shared_ptr<Type> &t1, std::shared_ptr<Type> &&t2) {
        return lub(ctx, t1, t2);
    }
    inline static std::shared_ptr<Type> lub(core::Context ctx, std::shared_ptr<Type> &&t1, std::shared_ptr<Type> &t2) {
        return lub(ctx, t1, t2);
    }
    inline static std::shared_ptr<Type> lub(core::Context ctx, std::shared_ptr<Type> &&t1, std::shared_ptr<Type> &&t2) {
        return lub(ctx, t1, t2);
    }
    static std::shared_ptr<Type> _lub(core::Context ctx, std::shared_ptr<Type> &t1, std::shared_ptr<Type> &t2);

    /** is every instance of  t1 an  instance of t2? */
    static bool isSubType(core::Context ctx, std::shared_ptr<Type> &t1, std::shared_ptr<Type> &t2);
    inline static bool isSubType(core::Context ctx, std::shared_ptr<Type> &t1, std::shared_ptr<Type> &&t2) {
        return isSubType(ctx, t1, t2);
    }
    inline static bool isSubType(core::Context ctx, std::shared_ptr<Type> &&t1, std::shared_ptr<Type> &t2) {
        return isSubType(ctx, t1, t2);
    }
    inline static bool isSubType(core::Context ctx, std::shared_ptr<Type> &&t1, std::shared_ptr<Type> &&t2) {
        return isSubType(ctx, t1, t2);
    }

    static bool equiv(core::Context ctx, std::shared_ptr<Type> &t1, std::shared_ptr<Type> &t2);
    inline static bool equiv(core::Context ctx, std::shared_ptr<Type> &t1, std::shared_ptr<Type> &&t2) {
        return equiv(ctx, t1, t2);
    }
    inline static bool equiv(core::Context ctx, std::shared_ptr<Type> &&t1, std::shared_ptr<Type> &t2) {
        return equiv(ctx, t1, t2);
    }
    inline static bool equiv(core::Context ctx, std::shared_ptr<Type> &&t1, std::shared_ptr<Type> &&t2) {
        return equiv(ctx, t1, t2);
    }

    static inline std::shared_ptr<Type> buildOr(core::Context ctx, std::shared_ptr<Type> t1, std::shared_ptr<Type> t2) {
        return lub(ctx, t1, t2);
    }
    static inline std::shared_ptr<Type> buildAnd(core::Context ctx, std::shared_ptr<Type> t1,
                                                 std::shared_ptr<Type> t2) {
        return glb(ctx, t1, t2);
    }

    static std::shared_ptr<Type> top();
    static std::shared_ptr<Type> bottom();
    static std::shared_ptr<Type> nil();
    static std::shared_ptr<Type> dynamic();
    static std::shared_ptr<Type> trueClass();
    static std::shared_ptr<Type> falseClass();
    static std::shared_ptr<Type> Integer();
    static std::shared_ptr<Type> String();
    static std::shared_ptr<Type> Symbol();
    static std::shared_ptr<Type> Float();
    static std::shared_ptr<Type> Object();
    static std::shared_ptr<Type> arrayClass();
    static std::shared_ptr<Type> hashClass();
    static std::shared_ptr<Type> procClass();
    static std::shared_ptr<Type> falsyTypes();
    static std::shared_ptr<Type> dropSubtypesOf(core::Context ctx, std::shared_ptr<Type> from, core::SymbolRef klass);
    static std::shared_ptr<Type> approximateSubtract(core::Context ctx, std::shared_ptr<Type> from,
                                                     std::shared_ptr<Type> what);
    static bool canBeTruthy(core::Context ctx, std::shared_ptr<Type> what);
    static bool canBeFalsy(core::Context ctx, std::shared_ptr<Type> what);
};

class TypeAndOrigins final {
public:
    std::shared_ptr<core::Type> type;
    InlinedVector<core::Loc, 2> origins;
    std::vector<core::Reporter::ErrorLine> origins2Explanations(core::Context ctx) {
        std::vector<core::Reporter::ErrorLine> result;
        for (auto o : origins) {
            result.emplace_back(o, "");
        }
        return result;
    }
    ~TypeAndOrigins() {
        histogramInc("TypeAndOrigins.origins.size", origins.size());
    }
};

class Type {
public:
    Type() = default;
    Type(const Type &obj) = delete;
    virtual ~Type() = default;
    virtual std::string toString(GlobalState &gs, int tabs = 0) = 0;
    virtual std::string typeName() = 0;
    virtual std::shared_ptr<Type> dispatchCall(core::Context ctx, core::NameRef name, core::Loc callLoc,
                                               std::vector<TypeAndOrigins> &args, std::shared_ptr<Type> fullType,
                                               bool hasBlock) = 0;
    virtual std::shared_ptr<Type> getCallArgumentType(core::Context ctx, core::NameRef name, int i) = 0;
    virtual bool derivesFrom(core::Context ctx, core::SymbolRef klass) = 0;
    virtual void _sanityCheck(core::Context ctx) = 0;
    void sanityCheck(core::Context ctx) {
        if (!debug_mode)
            return;
        _sanityCheck(ctx);
    }

    bool isDynamic();
    bool isBottom();
    virtual bool isFullyDefined() = 0;
};

template <class To> To *cast_type(Type *what) {
    static_assert(!std::is_pointer<To>::value, "To has to be a pointer");
    static_assert(std::is_assignable<Type *&, To *>::value, "Ill Formed To, has to be a subclass of Type");
    return fast_cast<Type, To>(what);
}

template <class To> bool isa_type(Type *what) {
    return cast_type<To>(what) != nullptr;
}

class GroundType : public Type {
public:
    virtual int kind() = 0;
};

class ProxyType : public Type {
public:
    // TODO: use shared pointers that use inline counter
    std::shared_ptr<Type> underlying;
    ProxyType(std::shared_ptr<Type> underlying);

    virtual std::shared_ptr<Type> dispatchCall(core::Context ctx, core::NameRef name, core::Loc callLoc,
                                               std::vector<TypeAndOrigins> &args, std::shared_ptr<Type> fullType,
                                               bool hasBlock) override;
    virtual std::shared_ptr<Type> getCallArgumentType(core::Context ctx, core::NameRef name, int i) override;
    virtual bool derivesFrom(core::Context ctx, core::SymbolRef klass) override;

    void _sanityCheck(core::Context ctx) override;
};

class ClassType final : public GroundType {
public:
    core::SymbolRef symbol;
    ClassType(core::SymbolRef symbol);
    virtual int kind() final;

    virtual std::string toString(GlobalState &gs, int tabs = 0);
    virtual std::string typeName();
    virtual std::shared_ptr<Type> dispatchCall(core::Context ctx, core::NameRef name, core::Loc callLoc,
                                               std::vector<TypeAndOrigins> &args, std::shared_ptr<Type> fullType,
                                               bool hasBlock) final;
    virtual std::shared_ptr<Type> getCallArgumentType(core::Context ctx, core::NameRef name, int i) final;
    virtual bool derivesFrom(core::Context ctx, core::SymbolRef klass) final;
    void _sanityCheck(core::Context ctx) final;

    virtual bool isFullyDefined() final;
};

class OrType final : public GroundType {
public:
    std::shared_ptr<Type> left;
    std::shared_ptr<Type> right;
    virtual int kind() final;

    virtual std::string toString(GlobalState &gs, int tabs = 0);
    virtual std::string typeName();
    virtual std::shared_ptr<Type> dispatchCall(core::Context ctx, core::NameRef name, core::Loc callLoc,
                                               std::vector<TypeAndOrigins> &args, std::shared_ptr<Type> fullType,
                                               bool hasBlock) final;
    virtual std::shared_ptr<Type> getCallArgumentType(core::Context ctx, core::NameRef name, int i) final;
    virtual bool derivesFrom(core::Context ctx, core::SymbolRef klass) final;
    void _sanityCheck(core::Context ctx) final;
    virtual bool isFullyDefined() final;

private:
    /*
     * We hide the constructor. Use Types::buildOr instead.
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
    friend class ruby_typer::core::GlobalSubstitution;
    friend class ruby_typer::core::serialize::GlobalStateSerializer;
    friend std::shared_ptr<Type> lubDistributeOr(core::Context ctx, std::shared_ptr<Type> t1, std::shared_ptr<Type> t2);
    friend std::shared_ptr<Type> lubGround(core::Context ctx, std::shared_ptr<Type> &t1, std::shared_ptr<Type> &t2);

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

    virtual std::string toString(GlobalState &gs, int tabs = 0);
    virtual std::string typeName();
    virtual std::shared_ptr<Type> dispatchCall(Context ctx, core::NameRef name, core::Loc callLoc,
                                               std::vector<TypeAndOrigins> &args, std::shared_ptr<Type> fullType,
                                               bool hasBlock) final;

    virtual std::shared_ptr<Type> getCallArgumentType(Context ctx, core::NameRef name, int i) final;
    virtual bool derivesFrom(core::Context ctx, core::SymbolRef klass) final;
    void _sanityCheck(core::Context ctx) final;
    virtual bool isFullyDefined() final;

private:
    // See the comments on OrType()
    AndType(std::shared_ptr<Type> left, std::shared_ptr<Type> right);

    friend class ruby_typer::core::GlobalSubstitution;
    friend class ruby_typer::core::serialize::GlobalStateSerializer;
    friend std::shared_ptr<Type> lubGround(Context ctx, std::shared_ptr<Type> &t1, std::shared_ptr<Type> &t2);
    friend std::shared_ptr<Type> glbDistributeAnd(Context ctx, std::shared_ptr<Type> t1, std::shared_ptr<Type> t2);
    friend std::shared_ptr<Type> glbGround(Context ctx, std::shared_ptr<Type> &t1, std::shared_ptr<Type> &t2);

    static inline std::shared_ptr<Type> make_shared(std::shared_ptr<Type> left, std::shared_ptr<Type> right) {
        std::shared_ptr<Type> res(new AndType(left, right));
        return res;
    }
};

class LiteralType final : public ProxyType {
public:
    int64_t value;
    LiteralType(int64_t val);
    LiteralType(double val);
    LiteralType(core::SymbolRef klass, core::NameRef val);
    LiteralType(bool val);

    virtual std::string toString(GlobalState &gs, int tabs = 0);
    virtual std::string typeName();
    virtual bool isFullyDefined() final;
};

class ShapeType final : public ProxyType {
public:
    std::vector<std::shared_ptr<LiteralType>> keys; // TODO: store sorted by whatever
    std::vector<std::shared_ptr<Type>> values;
    ShapeType();
    ShapeType(std::vector<std::shared_ptr<LiteralType>> &keys, std::vector<std::shared_ptr<Type>> &values);

    virtual std::string toString(GlobalState &gs, int tabs = 0);
    virtual std::string typeName();
    virtual std::shared_ptr<Type> dispatchCall(core::Context ctx, core::NameRef name, core::Loc callLoc,
                                               std::vector<TypeAndOrigins> &args, std::shared_ptr<Type> fullType,
                                               bool hasBlock) final;
    void _sanityCheck(core::Context ctx) final;
    virtual bool isFullyDefined() final;
};

class TupleType final : public ProxyType {
public:
    std::vector<std::shared_ptr<Type>> elems;
    TupleType(std::vector<std::shared_ptr<Type>> &elems);

    virtual std::string toString(GlobalState &gs, int tabs = 0);
    virtual std::string typeName();
    void _sanityCheck(core::Context ctx) final;
    virtual bool isFullyDefined() final;
};

// MagicType is the type of the built-in core::GlobalState::defn_Magic()
// object. Its `dispatchCall` knows how to handle a number of special methods
// that are used when building CFGs to desugar features that can't be described
// purely within our existing type system and IR.
class MagicType final : public ProxyType {
public:
    MagicType();
    virtual std::string toString(GlobalState &gs, int tabs = 0);
    virtual std::string typeName();
    virtual std::shared_ptr<Type> dispatchCall(core::Context ctx, core::NameRef name, core::Loc callLoc,
                                               std::vector<TypeAndOrigins> &args, std::shared_ptr<Type> fullType,
                                               bool hasBlock) final;
    void _sanityCheck(core::Context ctx) final;
    virtual bool isFullyDefined() final;
};

class AliasType final : public Type {
public:
    AliasType(SymbolRef other);
    virtual std::string toString(GlobalState &gs, int tabs = 0);
    virtual std::string typeName();
    virtual std::shared_ptr<Type> dispatchCall(core::Context ctx, core::NameRef name, core::Loc callLoc,
                                               std::vector<TypeAndOrigins> &args, std::shared_ptr<Type> fullType,
                                               bool hasBlock) final;
    virtual std::shared_ptr<Type> getCallArgumentType(core::Context ctx, core::NameRef name, int i) final;
    virtual bool derivesFrom(core::Context ctx, core::SymbolRef klass) final;

    SymbolRef symbol;
    void _sanityCheck(core::Context ctx) final;
    virtual bool isFullyDefined() final;
};

} // namespace core
} // namespace ruby_typer
#endif // SRUBY_TYPES_H
