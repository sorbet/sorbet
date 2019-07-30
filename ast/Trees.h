#ifndef SORBET_TREES_H
#define SORBET_TREES_H

#include "common/common.h"
#include "core/Context.h"
#include "core/LocalVariable.h"
#include "core/SymbolRef.h"
#include "core/Types.h"
#include <memory>
#include <vector>

//
// This file defines the IR that most of the middle phases of Sorbet operate on
// and manipulate It aims to be a middle ground between the parser output (very
// verbose and fine grained) and the CFG data structure (very easy to typecheck
// but very hard to do ad-hoc transformations on).
//
// This IR is best learned by example. Try using the `--print` option to sorbet
// on a handful of test/testdata files. Since there are multiple phases that
// return this IR, there are multiple valid print options which will show you
// an ast::Expression.
//
// Another good way to discover things is to grep for the class name in the
// various *-raw.exp snapshot tests to fine a test file that uses that node.
// Keep in mind that this IR is meant to be somewhat coarse grained, so one
// node type can likely have been created from multiple Ruby constructs.
//

namespace sorbet::ast {

class Expression {
public:
    Expression(core::Loc loc);
    virtual ~Expression() = default;
    virtual std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const = 0;
    std::string toString(const core::GlobalState &gs) const {
        return toStringWithTabs(gs);
    }
    virtual std::string nodeName() = 0;
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0) = 0;
    std::unique_ptr<Expression> deepCopy() const;
    virtual void _sanityCheck() = 0;
    const core::Loc loc;

    class DeepCopyError {};

    // This function should be private but it makes it hard to access from template methods in TreeCopy.cc
    virtual std::unique_ptr<Expression> _deepCopy(const Expression *avoid, bool root = false) const = 0;

    bool isSelfReference() const;
};
// CheckSize(Expression, 16, 8);

struct ParsedFile {
    std::unique_ptr<ast::Expression> tree;
    core::FileRef file;
};

template <class To> To *cast_tree(Expression *what) {
    static_assert(!std::is_pointer<To>::value, "To has to be a pointer");
    static_assert(std::is_assignable<Expression *&, To *>::value, "Ill Formed To, has to be a subclass of Expression");
    return fast_cast<Expression, To>(what);
}

// A variant of cast_tree that preserves the const-ness (if const in, then const out)
template <class To> const To *cast_tree_const(const Expression *what) {
    static_assert(!std::is_pointer<To>::value, "To has to be a pointer");
    static_assert(std::is_assignable<Expression *&, To *>::value, "Ill Formed To, has to be a subclass of Expression");
    return fast_cast<Expression, To>(const_cast<Expression *>(what));
}

template <class To> bool isa_tree(Expression *what) {
    return cast_tree<To>(what) != nullptr;
}

class Reference : public Expression {
public:
    Reference(core::Loc loc);
};
// CheckSize(Reference, 16, 8);

class Declaration : public Expression {
public:
    core::Loc declLoc;
    core::SymbolRef symbol;

    Declaration(core::Loc loc, core::Loc declLoc, core::SymbolRef symbol);
};
// CheckSize(Declaration, 24, 8);

enum ClassDefKind : u1 { Module, Class };

class ClassDef final : public Declaration {
public:
    ClassDefKind kind;
    static constexpr int EXPECTED_RHS_COUNT = 4;
    typedef InlinedVector<std::unique_ptr<Expression>, EXPECTED_RHS_COUNT> RHS_store;

    RHS_store rhs;
    std::unique_ptr<Expression> name;
    // For unresolved names. Once they are typeAlias to Symbols they go into the Symbol

    static constexpr int EXPECTED_ANCESTORS_COUNT = 2;
    typedef InlinedVector<std::unique_ptr<Expression>, EXPECTED_ANCESTORS_COUNT> ANCESTORS_store;

    ANCESTORS_store ancestors;
    ANCESTORS_store singletonAncestors;

    ClassDef(core::Loc loc, core::Loc declLoc, core::SymbolRef symbol, std::unique_ptr<Expression> name,
             ANCESTORS_store ancestors, RHS_store rhs, ClassDefKind kind);

    virtual std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual std::unique_ptr<Expression> _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};
// CheckSize(ClassDef, 120, 8);

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

    MethodDef(core::Loc loc, core::Loc declLoc, core::SymbolRef symbol, core::NameRef name, ARGS_store args,
              std::unique_ptr<Expression> rhs, u4 flags);
    virtual std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual std::unique_ptr<Expression> _deepCopy(const Expression *avoid, bool root = false) const;

    bool isSelf() const {
        return (flags & SelfMethod) != 0;
    }

    bool isDSLSynthesized() const {
        return (flags & DSLSynthesized) != 0;
    }

private:
    virtual void _sanityCheck();
};
// CheckSize(MethodDef, 64, 8);

class If final : public Expression {
public:
    std::unique_ptr<Expression> cond;
    std::unique_ptr<Expression> thenp;
    std::unique_ptr<Expression> elsep;

    If(core::Loc loc, std::unique_ptr<Expression> cond, std::unique_ptr<Expression> thenp,
       std::unique_ptr<Expression> elsep);
    virtual std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual std::unique_ptr<Expression> _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};
// CheckSize(If, 40, 8);

class While final : public Expression {
public:
    std::unique_ptr<Expression> cond;
    std::unique_ptr<Expression> body;

    While(core::Loc loc, std::unique_ptr<Expression> cond, std::unique_ptr<Expression> body);
    virtual std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual std::unique_ptr<Expression> _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};
// CheckSize(While, 32, 8);

class Break final : public Expression {
public:
    std::unique_ptr<Expression> expr;

    Break(core::Loc loc, std::unique_ptr<Expression> expr);
    virtual std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual std::unique_ptr<Expression> _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};
// CheckSize(Break, 24, 8);

class Retry final : public Expression {
public:
    Retry(core::Loc loc);
    virtual std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual std::unique_ptr<Expression> _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};
// CheckSize(Retry, 16, 8);

class Next final : public Expression {
public:
    std::unique_ptr<Expression> expr;

    Next(core::Loc loc, std::unique_ptr<Expression> expr);
    virtual std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual std::unique_ptr<Expression> _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};
// CheckSize(Next, 24, 8);

class Return final : public Expression {
public:
    std::unique_ptr<Expression> expr;

    Return(core::Loc loc, std::unique_ptr<Expression> expr);
    virtual std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual std::unique_ptr<Expression> _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};
// CheckSize(Return, 24, 8);

class RescueCase final : public Expression {
public:
    static constexpr int EXPECTED_EXCEPTION_COUNT = 2;
    typedef InlinedVector<std::unique_ptr<Expression>, EXPECTED_EXCEPTION_COUNT> EXCEPTION_store;

    EXCEPTION_store exceptions;

    // If present, var is always an UnresolvedIdent[kind=Local] up until the
    // namer, at which point it is a Local.
    std::unique_ptr<Expression> var;
    std::unique_ptr<Expression> body;

    RescueCase(core::Loc loc, EXCEPTION_store exceptions, std::unique_ptr<Expression> var,
               std::unique_ptr<Expression> body);
    virtual std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual std::unique_ptr<Expression> _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};
// CheckSize(RescueCase, 56, 8);

class Rescue final : public Expression {
public:
    static constexpr int EXPECTED_RESCUE_CASE_COUNT = 2;
    typedef InlinedVector<std::unique_ptr<RescueCase>, EXPECTED_RESCUE_CASE_COUNT> RESCUE_CASE_store;

    std::unique_ptr<Expression> body;
    RESCUE_CASE_store rescueCases;
    std::unique_ptr<Expression> else_;
    std::unique_ptr<Expression> ensure;

    Rescue(core::Loc loc, std::unique_ptr<Expression> body, RESCUE_CASE_store rescueCases,
           std::unique_ptr<Expression> else_, std::unique_ptr<Expression> ensure);
    virtual std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual std::unique_ptr<Expression> _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};
// CheckSize(Rescue, 64, 8);

class Field final : public Reference {
public:
    core::SymbolRef symbol;

    Field(core::Loc loc, core::SymbolRef symbol);
    virtual std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual std::unique_ptr<Expression> _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};
// CheckSize(Field, 24, 8);

class Local final : public Reference {
public:
    core::LocalVariable localVariable;

    Local(core::Loc loc, core::LocalVariable localVariable1);
    virtual std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual std::unique_ptr<Expression> _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};
// CheckSize(Local, 24, 8);

class UnresolvedIdent final : public Reference {
public:
    enum VarKind {
        Local,
        Instance,
        Class,
        Global,
    };
    core::NameRef name;
    VarKind kind;

    UnresolvedIdent(core::Loc loc, VarKind kind, core::NameRef name);
    virtual std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual std::unique_ptr<Expression> _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};
// CheckSize(UnresolvedIdent, 24, 8);

class RestArg final : public Reference {
public:
    std::unique_ptr<Reference> expr;

    RestArg(core::Loc loc, std::unique_ptr<Reference> arg);
    virtual std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual std::unique_ptr<Expression> _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};
// CheckSize(RestArg, 24, 8);

class KeywordArg final : public Reference {
public:
    std::unique_ptr<Reference> expr;

    KeywordArg(core::Loc loc, std::unique_ptr<Reference> expr);
    virtual std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual std::unique_ptr<Expression> _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};
// CheckSize(KeywordArg, 24, 8);

class OptionalArg final : public Reference {
public:
    std::unique_ptr<Reference> expr;
    std::unique_ptr<Expression> default_;

    OptionalArg(core::Loc loc, std::unique_ptr<Reference> expr, std::unique_ptr<Expression> default_);
    virtual std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual std::unique_ptr<Expression> _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};
// CheckSize(OptionalArg, 32, 8);

class BlockArg final : public Reference {
public:
    std::unique_ptr<Reference> expr;

    BlockArg(core::Loc loc, std::unique_ptr<Reference> expr);
    virtual std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual std::unique_ptr<Expression> _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};
// CheckSize(BlockArg, 24, 8);

class ShadowArg final : public Reference {
public:
    std::unique_ptr<Reference> expr;

    ShadowArg(core::Loc loc, std::unique_ptr<Reference> expr);
    virtual std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual std::unique_ptr<Expression> _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};
// CheckSize(ShadowArg, 24, 8);

class Assign final : public Expression {
public:
    std::unique_ptr<Expression> lhs;
    std::unique_ptr<Expression> rhs;

    Assign(core::Loc loc, std::unique_ptr<Expression> lhs, std::unique_ptr<Expression> rhs);
    virtual std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual std::unique_ptr<Expression> _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};
// CheckSize(Assign, 32, 8);

class Block;

class Send final : public Expression {
public:
    core::NameRef fun;

    static const int PRIVATE_OK = 1 << 0;
    static const int DSL_SYNTHESIZED = 1 << 1;
    u4 flags;

    std::unique_ptr<Expression> recv;

    static constexpr int EXPECTED_ARGS_COUNT = 2;
    typedef InlinedVector<std::unique_ptr<Expression>, EXPECTED_ARGS_COUNT> ARGS_store;
    ARGS_store args;
    std::unique_ptr<Block> block; // null if no block passed

    Send(core::Loc loc, std::unique_ptr<Expression> recv, core::NameRef fun, ARGS_store args,
         std::unique_ptr<Block> block = nullptr, u4 flags = 0);
    virtual std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual std::unique_ptr<Expression> _deepCopy(const Expression *avoid, bool root = false) const;

    bool isDSLSynthesized() const {
        return (flags & DSL_SYNTHESIZED) != 0;
    }

private:
    virtual void _sanityCheck();
};
// CheckSize(Send, 64, 8);

class Cast final : public Expression {
public:
    // The name of the cast operator.
    core::NameRef cast;

    core::TypePtr type;
    std::unique_ptr<Expression> arg;

    Cast(core::Loc loc, core::TypePtr ty, std::unique_ptr<Expression> arg, core::NameRef cast);
    virtual std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual std::unique_ptr<Expression> _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};
// CheckSize(Cast, 48, 8);

class Hash final : public Expression {
public:
    static constexpr int EXPECTED_ENTRY_COUNT = 2;
    typedef InlinedVector<std::unique_ptr<Expression>, EXPECTED_ENTRY_COUNT> ENTRY_store;

    ENTRY_store keys;
    ENTRY_store values;

    Hash(core::Loc loc, ENTRY_store keys, ENTRY_store values);

    virtual std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual std::unique_ptr<Expression> _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};
// CheckSize(Hash, 64, 8);

class Array final : public Expression {
public:
    static constexpr int EXPECTED_ENTRY_COUNT = 4;
    typedef InlinedVector<std::unique_ptr<Expression>, EXPECTED_ENTRY_COUNT> ENTRY_store;

    ENTRY_store elems;

    Array(core::Loc loc, ENTRY_store elems);

    virtual std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual std::unique_ptr<Expression> _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};
// CheckSize(Array, 56, 8);

class Literal final : public Expression {
public:
    core::TypePtr value;

    Literal(core::Loc loc, const core::TypePtr &value);
    virtual std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    bool isString(const core::GlobalState &gs) const;
    bool isSymbol(const core::GlobalState &gs) const;
    bool isNil(const core::GlobalState &gs) const;
    core::NameRef asString(const core::GlobalState &gs) const;
    core::NameRef asSymbol(const core::GlobalState &gs) const;
    bool isTrue(const core::GlobalState &gs) const;
    bool isFalse(const core::GlobalState &gs) const;
    virtual std::unique_ptr<Expression> _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};
// CheckSize(Literal, 32, 8);

class UnresolvedConstantLit final : public Expression {
public:
    core::NameRef cnst;
    std::unique_ptr<Expression> scope;

    UnresolvedConstantLit(core::Loc loc, std::unique_ptr<Expression> scope, core::NameRef cnst);
    virtual std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual std::unique_ptr<Expression> _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};
// CheckSize(UnresolvedConstantLit, 32, 8);

class ConstantLit final : public Expression {
public:
    core::SymbolRef symbol; // If this is a normal constant. This symbol may be already dealiased.
    core::SymbolRef
        resolutionScope; // for constats that failed resolution, symbol will be set to StubModule and resolutionScope
                         // will be set to whatever symbol we estimate the constant should have been defined in.
    std::unique_ptr<UnresolvedConstantLit> original;

    ConstantLit(core::Loc loc, core::SymbolRef symbol, std::unique_ptr<UnresolvedConstantLit> original);
    virtual std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual std::unique_ptr<Expression> _deepCopy(const Expression *avoid, bool root = false) const;
    std::optional<std::pair<core::SymbolRef, std::vector<core::NameRef>>>
    fullUnresolvedPath(const core::GlobalState &gs) const;

private:
    virtual void _sanityCheck();
};
// CheckSize(ConstantLit, 40, 8);

class ZSuperArgs final : public Expression {
public:
    // null if no block passed
    ZSuperArgs(core::Loc loc);
    virtual std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual std::unique_ptr<Expression> _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};
// CheckSize(ZSuperArgs, 16, 8);

class Block final : public Expression {
public:
    MethodDef::ARGS_store args;
    std::unique_ptr<Expression> body;

    Block(core::Loc loc, MethodDef::ARGS_store args, std::unique_ptr<Expression> body);
    virtual std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual std::unique_ptr<Expression> _deepCopy(const Expression *avoid, bool root = false) const;
    virtual void _sanityCheck();
};
// CheckSize(Block, 56, 8);

class InsSeq final : public Expression {
public:
    static constexpr int EXPECTED_STATS_COUNT = 4;
    typedef InlinedVector<std::unique_ptr<Expression>, EXPECTED_STATS_COUNT> STATS_store;
    // Statements
    STATS_store stats;

    // The distinguished final expression (determines return value)
    std::unique_ptr<Expression> expr;

    InsSeq(core::Loc loc, STATS_store stats, std::unique_ptr<Expression> expr);
    virtual std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual std::unique_ptr<Expression> _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};
// CheckSize(InsSeq, 64, 8);

class EmptyTree final : public Expression {
public:
    EmptyTree();
    virtual std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual std::unique_ptr<Expression> _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};
// CheckSize(EmptyTree, 16, 8);

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

// TODO Find a better place for this
bool classDefinesBehavior(std::unique_ptr<ast::ClassDef> &);
bool ignoreChild(ast::Expression *expr);
bool definesBehavior(ast::Expression *expr);

} // namespace sorbet::ast

#endif // SORBET_TREES_H
