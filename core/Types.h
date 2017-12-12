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
    static std::shared_ptr<Type> arrayClass();
    static std::shared_ptr<Type> hashClass();
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
    std::vector<core::Loc> origins; // todo: use tiny vector
    std::vector<core::Reporter::ErrorLine> origins2Explanations(core::Context ctx) {
        std::vector<core::Reporter::ErrorLine> result;
        for (auto o : origins) {
            result.emplace_back(o, "");
        }
        return result;
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
                                               std::vector<TypeAndOrigins> &args, std::shared_ptr<Type> fullType) = 0;
    virtual std::shared_ptr<Type> getCallArgumentType(core::Context ctx, core::NameRef name, int i) = 0;
    virtual bool derivesFrom(core::Context ctx, core::SymbolRef klass) = 0;
    bool isDynamic();
    bool isBottom();
};

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
                                               std::vector<TypeAndOrigins> &args, std::shared_ptr<Type> fullType);
    virtual std::shared_ptr<Type> getCallArgumentType(core::Context ctx, core::NameRef name, int i);
    virtual bool derivesFrom(core::Context ctx, core::SymbolRef klass);
};

class ClassType final : public GroundType {
public:
    core::SymbolRef symbol;
    ClassType(core::SymbolRef symbol);
    virtual int kind();

    virtual std::string toString(GlobalState &gs, int tabs = 0);
    virtual std::string typeName();
    virtual std::shared_ptr<Type> dispatchCall(core::Context ctx, core::NameRef name, core::Loc callLoc,
                                               std::vector<TypeAndOrigins> &args, std::shared_ptr<Type> fullType);
    virtual std::shared_ptr<Type> getCallArgumentType(core::Context ctx, core::NameRef name, int i);
    virtual bool derivesFrom(core::Context ctx, core::SymbolRef klass);
};

class OrType final : public GroundType {
public:
    std::shared_ptr<Type> left;
    std::shared_ptr<Type> right;
    virtual int kind();
    OrType(std::shared_ptr<Type> left, std::shared_ptr<Type> right);

    virtual std::string toString(GlobalState &gs, int tabs = 0);
    virtual std::string typeName();
    virtual std::shared_ptr<Type> dispatchCall(core::Context ctx, core::NameRef name, core::Loc callLoc,
                                               std::vector<TypeAndOrigins> &args, std::shared_ptr<Type> fullType);
    virtual std::shared_ptr<Type> getCallArgumentType(core::Context ctx, core::NameRef name, int i);
    virtual bool derivesFrom(core::Context ctx, core::SymbolRef klass);
};

class AndType final : public GroundType {
public:
    std::shared_ptr<Type> left;
    std::shared_ptr<Type> right;
    virtual int kind();
    AndType(std::shared_ptr<Type> left, std::shared_ptr<Type> right);

    virtual std::string toString(GlobalState &gs, int tabs = 0);
    virtual std::string typeName();
    virtual std::shared_ptr<Type> dispatchCall(core::Context ctx, core::NameRef name, core::Loc callLoc,
                                               std::vector<TypeAndOrigins> &args, std::shared_ptr<Type> fullType);

    virtual std::shared_ptr<Type> getCallArgumentType(core::Context ctx, core::NameRef name, int i);
    virtual bool derivesFrom(core::Context ctx, core::SymbolRef klass);
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
                                               std::vector<TypeAndOrigins> &args, std::shared_ptr<Type> fullType);
};

class TupleType final : public ProxyType {
public:
    std::vector<std::shared_ptr<Type>> elems;
    TupleType(std::vector<std::shared_ptr<Type>> &elems);

    virtual std::string toString(GlobalState &gs, int tabs = 0);
    virtual std::string typeName();
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
                                               std::vector<TypeAndOrigins> &args, std::shared_ptr<Type> fullType);
};

class AliasType final : public Type {
public:
    AliasType(SymbolRef other);
    virtual std::string toString(GlobalState &gs, int tabs = 0);
    virtual std::string typeName();
    virtual std::shared_ptr<Type> dispatchCall(core::Context ctx, core::NameRef name, core::Loc callLoc,
                                               std::vector<TypeAndOrigins> &args, std::shared_ptr<Type> fullType);
    virtual std::shared_ptr<Type> getCallArgumentType(core::Context ctx, core::NameRef name, int i);
    virtual bool derivesFrom(core::Context ctx, core::SymbolRef klass);

    SymbolRef symbol;
};

} // namespace core
} // namespace ruby_typer
#endif // SRUBY_TYPES_H
