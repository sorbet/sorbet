#ifndef SORBET_TREES_H
#define SORBET_TREES_H

#include "common/common.h"
#include "core/Context.h"
#include "core/Symbols.h"
#include "core/Types.h"
#include <memory>
#include <vector>

namespace sorbet {
namespace ast {

class Expression {
public:
    Expression(core::Loc loc);
    virtual ~Expression() = default;
    virtual std::string toString(const core::GlobalState &gs, int tabs = 0) = 0;
    virtual std::string nodeName() = 0;
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0) = 0;
    std::unique_ptr<Expression> deepCopy() const;
    virtual void _sanityCheck() = 0;
    core::Loc loc;

    class DeepCopyError {};

    // This function should be private but it makes it hard to access from template methods in TreeCopy.cc
    virtual std::unique_ptr<Expression> _deepCopy(const Expression *avoid, bool root = false) const = 0;
};

template <class To> To *cast_tree(Expression *what) {
    static_assert(!std::is_pointer<To>::value, "To has to be a pointer");
    static_assert(std::is_assignable<Expression *&, To *>::value, "Ill Formed To, has to be a subclass of Expression");
    return fast_cast<Expression, To>(what);
}

template <class To> bool isa_tree(Expression *what) {
    return cast_tree<To>(what) != nullptr;
}

class Reference : public Expression {
public:
    Reference(core::Loc loc);
};

class Declaration : public Expression {
public:
    core::SymbolRef symbol;

    Declaration(core::Loc loc, core::SymbolRef symbol);
};

enum ClassDefKind : u1 { Module, Class };

class ClassDef final : public Declaration {
public:
    inline core::SymbolRef parent(core::MutableContext ctx) {
        return symbol.data(ctx).parent(ctx);
    }

    static constexpr int EXPECTED_RHS_COUNT = 4;
    typedef InlinedVector<std::unique_ptr<Expression>, EXPECTED_RHS_COUNT> RHS_store;

    RHS_store rhs;
    std::unique_ptr<Expression> name;
    // For unresolved names. Once they are typeAlias to Symbols they go into the
    // Symbol

    static constexpr int EXPECTED_ANCESTORS_COUNT = 2;
    typedef InlinedVector<std::unique_ptr<Expression>, EXPECTED_ANCESTORS_COUNT> ANCESTORS_store;

    ANCESTORS_store ancestors;
    ANCESTORS_store singleton_ancestors;
    ClassDefKind kind;

    ClassDef(core::Loc loc, core::SymbolRef symbol, std::unique_ptr<Expression> name, ANCESTORS_store ancestors,
             RHS_store rhs, ClassDefKind kind);

    virtual std::string toString(const core::GlobalState &gs, int tabs = 0);
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual std::unique_ptr<Expression> _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};

class MethodDef final : public Declaration {
public:
    std::unique_ptr<Expression> rhs;

    static constexpr int EXPECTED_ARGS_COUNT = 2;
    typedef InlinedVector<std::unique_ptr<Expression>, EXPECTED_ARGS_COUNT> ARGS_store;
    ARGS_store args;

    core::NameRef name;
    u4 flags;

    enum Flags {
        SelfMethod = 1,
        DSLSynthesized = 2,
    };

    MethodDef(core::Loc loc, core::SymbolRef symbol, core::NameRef name, ARGS_store args,
              std::unique_ptr<Expression> rhs, u4 flags);
    virtual std::string toString(const core::GlobalState &gs, int tabs = 0);
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual std::unique_ptr<Expression> _deepCopy(const Expression *avoid, bool root = false) const;

    bool isSelf() {
        return (flags & SelfMethod) != 0;
    }

    bool isDSLSynthesized() {
        return (flags & DSLSynthesized) != 0;
    }

private:
    virtual void _sanityCheck();
};

class If final : public Expression {
public:
    std::unique_ptr<Expression> cond;
    std::unique_ptr<Expression> thenp;
    std::unique_ptr<Expression> elsep;

    If(core::Loc loc, std::unique_ptr<Expression> cond, std::unique_ptr<Expression> thenp,
       std::unique_ptr<Expression> elsep);
    virtual std::string toString(const core::GlobalState &gs, int tabs = 0);
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual std::unique_ptr<Expression> _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};

class While final : public Expression {
public:
    std::unique_ptr<Expression> cond;
    std::unique_ptr<Expression> body;

    While(core::Loc loc, std::unique_ptr<Expression> cond, std::unique_ptr<Expression> body);
    virtual std::string toString(const core::GlobalState &gs, int tabs = 0);
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual std::unique_ptr<Expression> _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};

class Break final : public Expression {
public:
    std::unique_ptr<Expression> expr;

    Break(core::Loc loc, std::unique_ptr<Expression> expr);
    virtual std::string toString(const core::GlobalState &gs, int tabs = 0);
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual std::unique_ptr<Expression> _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};

class Retry final : public Expression {
public:
    Retry(core::Loc loc);
    virtual std::string toString(const core::GlobalState &gs, int tabs = 0);
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual std::unique_ptr<Expression> _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};

class Next final : public Expression {
public:
    std::unique_ptr<Expression> expr;

    Next(core::Loc loc, std::unique_ptr<Expression> expr);
    virtual std::string toString(const core::GlobalState &gs, int tabs = 0);
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual std::unique_ptr<Expression> _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};

class Return final : public Expression {
public:
    std::unique_ptr<Expression> expr;

    Return(core::Loc loc, std::unique_ptr<Expression> expr);
    virtual std::string toString(const core::GlobalState &gs, int tabs = 0);
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual std::unique_ptr<Expression> _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};

class Yield final : public Expression {
public:
    std::unique_ptr<Expression> expr;

    Yield(core::Loc loc, std::unique_ptr<Expression> expr);
    virtual std::string toString(const core::GlobalState &gs, int tabs = 0);
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual std::unique_ptr<Expression> _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};

class RescueCase final : public Expression {
public:
    static constexpr int EXPECTED_EXCEPTION_COUNT = 1;
    typedef InlinedVector<std::unique_ptr<Expression>, EXPECTED_EXCEPTION_COUNT> EXCEPTION_store;

    EXCEPTION_store exceptions;

    // If present, var is always an UnresolvedIdent[kind=Local] up until the
    // namer, at which point it is a Local.
    std::unique_ptr<Expression> var;
    std::unique_ptr<Expression> body;

    RescueCase(core::Loc loc, EXCEPTION_store exceptions, std::unique_ptr<Expression> var,
               std::unique_ptr<Expression> body);
    virtual std::string toString(const core::GlobalState &gs, int tabs = 0);
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual std::unique_ptr<Expression> _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};

class Rescue final : public Expression {
public:
    static constexpr int EXPECTED_RESCUE_CASE_COUNT = 1;
    typedef InlinedVector<std::unique_ptr<RescueCase>, EXPECTED_RESCUE_CASE_COUNT> RESCUE_CASE_store;

    std::unique_ptr<Expression> body;
    RESCUE_CASE_store rescueCases;
    std::unique_ptr<Expression> else_;
    std::unique_ptr<Expression> ensure;

    Rescue(core::Loc loc, std::unique_ptr<Expression> body, RESCUE_CASE_store rescueCases,
           std::unique_ptr<Expression> else_, std::unique_ptr<Expression> ensure);
    virtual std::string toString(const core::GlobalState &gs, int tabs = 0);
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual std::unique_ptr<Expression> _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};

class Field final : public Reference {
public:
    core::SymbolRef symbol;

    Field(core::Loc loc, core::SymbolRef symbol);
    virtual std::string toString(const core::GlobalState &gs, int tabs = 0);
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual std::unique_ptr<Expression> _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};

class Local final : public Expression {
public:
    core::LocalVariable localVariable;

    Local(core::Loc loc, core::LocalVariable localVariable1);
    virtual std::string toString(const core::GlobalState &gs, int tabs = 0);
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual std::unique_ptr<Expression> _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};

class UnresolvedIdent final : public Reference {
public:
    enum VarKind {
        Local,
        Instance,
        Class,
        Global,
    };
    VarKind kind;
    core::NameRef name;

    UnresolvedIdent(core::Loc loc, VarKind kind, core::NameRef name);
    virtual std::string toString(const core::GlobalState &gs, int tabs = 0);
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual std::unique_ptr<Expression> _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};

class RestArg final : public Reference {
public:
    std::unique_ptr<Reference> expr;

    RestArg(core::Loc loc, std::unique_ptr<Reference> arg);
    virtual std::string toString(const core::GlobalState &gs, int tabs = 0);
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual std::unique_ptr<Expression> _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};

class KeywordArg final : public Reference {
public:
    std::unique_ptr<Reference> expr;

    KeywordArg(core::Loc loc, std::unique_ptr<Reference> expr);
    virtual std::string toString(const core::GlobalState &gs, int tabs = 0);
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual std::unique_ptr<Expression> _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};

class OptionalArg final : public Reference {
public:
    std::unique_ptr<Reference> expr;
    std::unique_ptr<Expression> default_;

    OptionalArg(core::Loc loc, std::unique_ptr<Reference> expr, std::unique_ptr<Expression> default_);
    virtual std::string toString(const core::GlobalState &gs, int tabs = 0);
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual std::unique_ptr<Expression> _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};

class BlockArg final : public Reference {
public:
    std::unique_ptr<Reference> expr;

    BlockArg(core::Loc loc, std::unique_ptr<Reference> expr);
    virtual std::string toString(const core::GlobalState &gs, int tabs = 0);
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual std::unique_ptr<Expression> _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};

class ShadowArg final : public Reference {
public:
    std::unique_ptr<Reference> expr;

    ShadowArg(core::Loc loc, std::unique_ptr<Reference> expr);
    virtual std::string toString(const core::GlobalState &gs, int tabs = 0);
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual std::unique_ptr<Expression> _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};

class Assign final : public Expression {
public:
    std::unique_ptr<Expression> lhs;
    std::unique_ptr<Expression> rhs;

    Assign(core::Loc loc, std::unique_ptr<Expression> lhs, std::unique_ptr<Expression> rhs);
    virtual std::string toString(const core::GlobalState &gs, int tabs = 0);
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual std::unique_ptr<Expression> _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};

class Block;

class Send final : public Expression {
public:
    std::unique_ptr<Expression> recv;
    core::NameRef fun;

    static constexpr int EXPECTED_ARGS_COUNT = 2;
    typedef InlinedVector<std::unique_ptr<Expression>, EXPECTED_ARGS_COUNT> ARGS_store;
    ARGS_store args;
    u4 flags = 0;

    static const int PRIVATE_OK = 1 << 0;

    // null if no block passed
    std::unique_ptr<Block> block;

    Send(core::Loc loc, std::unique_ptr<Expression> recv, core::NameRef fun, ARGS_store args,
         std::unique_ptr<Block> block = nullptr);
    virtual std::string toString(const core::GlobalState &gs, int tabs = 0);
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual std::unique_ptr<Expression> _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};

class Cast final : public Expression {
public:
    std::shared_ptr<core::Type> type;
    std::unique_ptr<Expression> arg;

    // The name of the cast operator.
    core::NameRef cast;

    Cast(core::Loc loc, std::shared_ptr<core::Type> ty, std::unique_ptr<Expression> arg, core::NameRef cast);
    virtual std::string toString(const core::GlobalState &gs, int tabs = 0);
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual std::unique_ptr<Expression> _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};

class Hash final : public Expression {
public:
    static constexpr int EXPECTED_ENTRY_COUNT = 2;
    typedef InlinedVector<std::unique_ptr<Expression>, EXPECTED_ENTRY_COUNT> ENTRY_store;

    ENTRY_store keys;
    ENTRY_store values;

    Hash(core::Loc loc, ENTRY_store keys, ENTRY_store values);

    virtual std::string toString(const core::GlobalState &gs, int tabs);
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual std::unique_ptr<Expression> _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};

class Array final : public Expression {
public:
    static constexpr int EXPECTED_ENTRY_COUNT = 4;
    typedef InlinedVector<std::unique_ptr<Expression>, EXPECTED_ENTRY_COUNT> ENTRY_store;

    ENTRY_store elems;

    Array(core::Loc loc, ENTRY_store elems);

    virtual std::string toString(const core::GlobalState &gs, int tabs);
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual std::unique_ptr<Expression> _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};

class Literal final : public Expression {
public:
    std::shared_ptr<core::Type> value;

    Literal(core::Loc loc, std::shared_ptr<core::Type> value);
    virtual std::string toString(const core::GlobalState &gs, int tabs = 0);
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    bool isString(const core::GlobalState &gs) const;
    bool isSymbol(const core::GlobalState &gs) const;
    bool isNil(const core::GlobalState &gs) const;
    core::NameRef asString(const core::GlobalState &gs) const;
    core::NameRef asSymbol(const core::GlobalState &gs) const;
    bool isTrue(const core::GlobalState &gs);
    bool isFalse(const core::GlobalState &gs);
    virtual std::unique_ptr<Expression> _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};

class UnresolvedConstantLit final : public Expression {
public:
    core::NameRef cnst;
    std::unique_ptr<Expression> scope;

    UnresolvedConstantLit(core::Loc loc, std::unique_ptr<Expression> scope, core::NameRef cnst);
    virtual std::string toString(const core::GlobalState &gs, int tabs = 0);
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual std::unique_ptr<Expression> _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};

class ConstantLit final : public Expression {
public:
    core::SymbolRef symbol; // If this is a normal constant. This symbol may be already dealiased.
    std::unique_ptr<UnresolvedConstantLit> original;
    std::unique_ptr<Expression> typeAlias; // if this constant used to point to type alias

    ConstantLit(core::Loc loc, core::SymbolRef symbol, std::unique_ptr<UnresolvedConstantLit> original,
                std::unique_ptr<Expression> resolved);
    virtual std::string toString(const core::GlobalState &gs, int tabs = 0);
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual std::unique_ptr<Expression> _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};

class ZSuperArgs final : public Expression {
public:
    // null if no block passed
    ZSuperArgs(core::Loc loc);
    virtual std::string toString(const core::GlobalState &gs, int tabs = 0);
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual std::unique_ptr<Expression> _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};

class Self final : public Expression {
public:
    core::SymbolRef claz;

    Self(core::Loc loc, core::SymbolRef claz);
    virtual std::string toString(const core::GlobalState &gs, int tabs = 0);
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual std::unique_ptr<Expression> _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};

class Block final : public Expression {
public:
    MethodDef::ARGS_store args;
    std::unique_ptr<Expression> body;
    core::SymbolRef symbol;

    Block(core::Loc loc, MethodDef::ARGS_store args, std::unique_ptr<Expression> body);
    virtual std::string toString(const core::GlobalState &gs, int tabs = 0);
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual std::unique_ptr<Expression> _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};

class InsSeq final : public Expression {
public:
    static constexpr int EXPECTED_STATS_COUNT = 4;
    typedef InlinedVector<std::unique_ptr<Expression>, EXPECTED_STATS_COUNT> STATS_store;
    STATS_store stats;

    std::unique_ptr<Expression> expr;

    InsSeq(core::Loc loc, STATS_store stats, std::unique_ptr<Expression> expr);
    virtual std::string toString(const core::GlobalState &gs, int tabs = 0);
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual std::unique_ptr<Expression> _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};

class EmptyTree final : public Expression {
public:
    EmptyTree(core::Loc loc);
    virtual std::string toString(const core::GlobalState &gs, int tabs = 0);
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual std::unique_ptr<Expression> _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};

/** https://git.corp.stripe.com/gist/nelhage/51564501674174da24822e60ad770f64
 *
 *  [] - prototype only
 *
 *                 / Control Flow <- while, if, for, break, next, retry, return, rescue, case
 * Pre-CFG-Node <-
 *                 \ Instruction <- assign, send, [new], ident, named_arg, hash, array, literals(symbols, ints, floats,
 * strings, constants, nil), constants(resolver will desugar it into literals), array_splat(*), hash_splat(**), self,
 * insseq, Block)
 *
 *                  \ Definition  <-  class(name, parent, mixins, body)
 *                                    module
 *                                    def
 *                                    defself
 *                                    const_assign
 *
 *
 *
 * know id for: top, bottom, kernel?, basicobject, class, module [postponed], unit, Hash, Array, String, Symbol, float,
 * int, numeric, double, unknown
 *
 *
 *
 * Desugar string concatenation into series of .to_s calls and string concatenations
 */

} // namespace ast
} // namespace sorbet

#endif // SORBET_TREES_H
