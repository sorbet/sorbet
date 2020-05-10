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
// and manipulate. It aims to be a middle ground between the parser output
// (very verbose and fine grained) and the CFG data structure (very easy to
// typecheck but very hard to do ad-hoc transformations on).
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

class Expression;

class TreePtr {
private:
    std::unique_ptr<Expression> ptr;

public:
    constexpr TreePtr() noexcept : ptr(nullptr) {}

    explicit TreePtr(Expression *ptr) noexcept : ptr(ptr) {}

    TreePtr(std::nullptr_t) noexcept : TreePtr() {}

    TreePtr(const TreePtr &) = delete;
    TreePtr &operator=(const TreePtr &) = delete;

    TreePtr(TreePtr &&other) noexcept : ptr(std::move(other.ptr)) {}

    TreePtr &operator=(TreePtr &&other) noexcept {
        this->ptr = std::move(other.ptr);
        return *this;
    }

    Expression *release() noexcept {
        return ptr.release();
    }

    void reset(Expression *expr = nullptr) noexcept {
        ptr.reset(expr);
    }

    Expression *get() const noexcept {
        return ptr.get();
    }

    Expression *operator->() const noexcept {
        return ptr.get();
    }

    Expression *operator*() const noexcept {
        return ptr.get();
    }

    explicit operator bool() const noexcept {
        return get() != nullptr;
    }

    bool operator==(const TreePtr &other) const noexcept {
        return get() == other.get();
    }

    bool operator!=(const TreePtr &other) const noexcept {
        return get() != other.get();
    }
};

template <class E, typename... Args> TreePtr make_tree(Args &&... args) {
    return TreePtr(new E(std::forward<Args>(args)...));
}

class Expression {
public:
    Expression(core::LocOffsets loc);
    virtual ~Expression() = default;
    virtual std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const = 0;
    std::string toString(const core::GlobalState &gs) const {
        return toStringWithTabs(gs);
    }
    virtual std::string nodeName() = 0;
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0) = 0;
    TreePtr deepCopy() const;
    virtual void _sanityCheck() = 0;
    const core::LocOffsets loc;

    class DeepCopyError {};

    // This function should be private but it makes it hard to access from template methods in TreeCopy.cc
    virtual TreePtr _deepCopy(const Expression *avoid, bool root = false) const = 0;

    bool isSelfReference() const;
};
CheckSize(Expression, 16, 8);

struct ParsedFile {
    TreePtr tree;
    core::FileRef file;
};

/**
 * Stores a vector of `ParsedFile`s. May be empty if pass was canceled or encountered an error.
 * TODO: Modify to store reason if we ever have multiple reasons for a pass to stop. Currently, it's only empty if the
 * pass is canceled in LSP mode.
 */
class ParsedFilesOrCancelled final {
private:
    std::optional<std::vector<ParsedFile>> trees;

public:
    ParsedFilesOrCancelled();
    ParsedFilesOrCancelled(std::vector<ParsedFile> &&trees);

    bool hasResult() const;
    std::vector<ParsedFile> &result();
};

template <class To> To *cast_tree(TreePtr &what) {
    return fast_cast<Expression, To>(what.get());
}

// A variant of cast_tree that preserves the const-ness (if const in, then const out)
template <class To> To const *cast_tree_const(const TreePtr &what) {
    return fast_cast<Expression, To>(const_cast<Expression *>(what.get()));
}

template <class To> bool isa_tree(const TreePtr &what) {
    return cast_tree_const<To>(what) != nullptr;
}

template <class To> To &ref_tree(TreePtr &what) {
    ENFORCE(what != nullptr);
    ENFORCE(isa_tree<To>(what), "ref_tree failed!");
    return *reinterpret_cast<To *>(what.get());
}

template <class To> const To &ref_tree_const(const TreePtr &what) {
    ENFORCE(isa_tree<To>(what), "ref_tree failed!");
    return *reinterpret_cast<To *>(what.get());
}

class Reference : public Expression {
public:
    Reference(core::LocOffsets loc);
};
CheckSize(Reference, 16, 8);

class Declaration : public Expression {
public:
    core::Loc declLoc;
    core::SymbolRef symbol;

    Declaration(core::LocOffsets loc, core::Loc declLoc, core::SymbolRef symbol);
};
CheckSize(Declaration, 32, 8);

class ClassDef final : public Declaration {
public:
    enum class Kind : u1 {
        Module,
        Class,
    };
    Kind kind;
    static constexpr int EXPECTED_RHS_COUNT = 4;
    using RHS_store = InlinedVector<TreePtr, EXPECTED_RHS_COUNT>;

    RHS_store rhs;
    TreePtr name;
    // For unresolved names. Once they are typeAlias to Symbols they go into the Symbol

    static constexpr int EXPECTED_ANCESTORS_COUNT = 2;
    using ANCESTORS_store = InlinedVector<TreePtr, EXPECTED_ANCESTORS_COUNT>;

    ANCESTORS_store ancestors;
    ANCESTORS_store singletonAncestors;

    ClassDef(core::LocOffsets loc, core::Loc declLoc, core::SymbolRef symbol, TreePtr name, ANCESTORS_store ancestors,
             RHS_store rhs, ClassDef::Kind kind);

    virtual std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual TreePtr _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};
CheckSize(ClassDef, 136, 8);

class MethodDef final : public Declaration {
public:
    TreePtr rhs;

    using ARGS_store = InlinedVector<TreePtr, core::SymbolRef::EXPECTED_METHOD_ARGS_COUNT>;
    ARGS_store args;

    core::NameRef name;

    struct Flags {
        bool isSelfMethod : 1;
        bool isRewriterSynthesized : 1;

        // In C++20 we can replace this with bit field initialzers
        Flags() : isSelfMethod(false), isRewriterSynthesized(false) {}
    };
    CheckSize(Flags, 1, 1);

    Flags flags;

    MethodDef(core::LocOffsets loc, core::Loc declLoc, core::SymbolRef symbol, core::NameRef name, ARGS_store args,
              TreePtr rhs, Flags flags);
    virtual std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual TreePtr _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};
CheckSize(MethodDef, 72, 8);

class If final : public Expression {
public:
    TreePtr cond;
    TreePtr thenp;
    TreePtr elsep;

    If(core::LocOffsets loc, TreePtr cond, TreePtr thenp, TreePtr elsep);
    virtual std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual TreePtr _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};
CheckSize(If, 40, 8);

class While final : public Expression {
public:
    TreePtr cond;
    TreePtr body;

    While(core::LocOffsets loc, TreePtr cond, TreePtr body);
    virtual std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual TreePtr _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};
CheckSize(While, 32, 8);

class Break final : public Expression {
public:
    TreePtr expr;

    Break(core::LocOffsets loc, TreePtr expr);
    virtual std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual TreePtr _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};
CheckSize(Break, 24, 8);

class Retry final : public Expression {
public:
    Retry(core::LocOffsets loc);
    virtual std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual TreePtr _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};
CheckSize(Retry, 16, 8);

class Next final : public Expression {
public:
    TreePtr expr;

    Next(core::LocOffsets loc, TreePtr expr);
    virtual std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual TreePtr _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};
CheckSize(Next, 24, 8);

class Return final : public Expression {
public:
    TreePtr expr;

    Return(core::LocOffsets loc, TreePtr expr);
    virtual std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual TreePtr _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};
CheckSize(Return, 24, 8);

class RescueCase final : public Expression {
public:
    static constexpr int EXPECTED_EXCEPTION_COUNT = 2;
    using EXCEPTION_store = InlinedVector<TreePtr, EXPECTED_EXCEPTION_COUNT>;

    EXCEPTION_store exceptions;

    // If present, var is always an UnresolvedIdent[kind=Local] up until the
    // namer, at which point it is a Local.
    TreePtr var;
    TreePtr body;

    RescueCase(core::LocOffsets loc, EXCEPTION_store exceptions, TreePtr var, TreePtr body);
    virtual std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual TreePtr _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};
CheckSize(RescueCase, 56, 8);

class Rescue final : public Expression {
public:
    static constexpr int EXPECTED_RESCUE_CASE_COUNT = 2;
    using RESCUE_CASE_store = InlinedVector<TreePtr, EXPECTED_RESCUE_CASE_COUNT>;

    TreePtr body;
    RESCUE_CASE_store rescueCases;
    TreePtr else_;
    TreePtr ensure;

    Rescue(core::LocOffsets loc, TreePtr body, RESCUE_CASE_store rescueCases, TreePtr else_, TreePtr ensure);
    virtual std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual TreePtr _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};
CheckSize(Rescue, 64, 8);

class Local final : public Reference {
public:
    core::LocalVariable localVariable;

    Local(core::LocOffsets loc, core::LocalVariable localVariable1);
    virtual std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual TreePtr _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};
CheckSize(Local, 24, 8);

class UnresolvedIdent final : public Reference {
public:
    enum class Kind : u1 {
        Local,
        Instance,
        Class,
        Global,
    };
    core::NameRef name;
    Kind kind;

    UnresolvedIdent(core::LocOffsets loc, Kind kind, core::NameRef name);
    virtual std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual TreePtr _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};
CheckSize(UnresolvedIdent, 24, 8);

class RestArg final : public Reference {
public:
    TreePtr expr;

    RestArg(core::LocOffsets loc, TreePtr arg);
    virtual std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual TreePtr _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};
CheckSize(RestArg, 24, 8);

class KeywordArg final : public Reference {
public:
    TreePtr expr;

    KeywordArg(core::LocOffsets loc, TreePtr expr);
    virtual std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual TreePtr _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};
CheckSize(KeywordArg, 24, 8);

class OptionalArg final : public Reference {
public:
    TreePtr expr;
    TreePtr default_;

    OptionalArg(core::LocOffsets loc, TreePtr expr, TreePtr default_);
    virtual std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual TreePtr _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};
CheckSize(OptionalArg, 32, 8);

class BlockArg final : public Reference {
public:
    TreePtr expr;

    BlockArg(core::LocOffsets loc, TreePtr expr);
    virtual std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual TreePtr _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};
CheckSize(BlockArg, 24, 8);

class ShadowArg final : public Reference {
public:
    TreePtr expr;

    ShadowArg(core::LocOffsets loc, TreePtr expr);
    virtual std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual TreePtr _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};
CheckSize(ShadowArg, 24, 8);

class Assign final : public Expression {
public:
    TreePtr lhs;
    TreePtr rhs;

    Assign(core::LocOffsets loc, TreePtr lhs, TreePtr rhs);
    virtual std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual TreePtr _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};
CheckSize(Assign, 32, 8);

class Block;

class Send final : public Expression {
public:
    core::NameRef fun;

    struct Flags {
        bool isPrivateOk : 1;
        bool isRewriterSynthesized : 1;

        // In C++20 we can replace this with bit field initialzers
        Flags() : isPrivateOk(false), isRewriterSynthesized(false) {}
    };
    CheckSize(Flags, 1, 1);

    Flags flags;

    TreePtr recv;

    static constexpr int EXPECTED_ARGS_COUNT = 2;
    using ARGS_store = InlinedVector<TreePtr, EXPECTED_ARGS_COUNT>;
    ARGS_store args;
    TreePtr block; // null if no block passed

    Send(core::LocOffsets loc, TreePtr recv, core::NameRef fun, ARGS_store args, TreePtr block = nullptr,
         Flags flags = {});
    virtual std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual TreePtr _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};
CheckSize(Send, 64, 8);

class Cast final : public Expression {
public:
    // The name of the cast operator.
    core::NameRef cast;

    core::TypePtr type;
    TreePtr arg;

    Cast(core::LocOffsets loc, core::TypePtr ty, TreePtr arg, core::NameRef cast);
    virtual std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual TreePtr _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};
CheckSize(Cast, 48, 8);

class Hash final : public Expression {
public:
    static constexpr int EXPECTED_ENTRY_COUNT = 2;
    using ENTRY_store = InlinedVector<TreePtr, EXPECTED_ENTRY_COUNT>;

    ENTRY_store keys;
    ENTRY_store values;

    Hash(core::LocOffsets loc, ENTRY_store keys, ENTRY_store values);

    virtual std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual TreePtr _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};
CheckSize(Hash, 64, 8);

class Array final : public Expression {
public:
    static constexpr int EXPECTED_ENTRY_COUNT = 4;
    using ENTRY_store = InlinedVector<TreePtr, EXPECTED_ENTRY_COUNT>;

    ENTRY_store elems;

    Array(core::LocOffsets loc, ENTRY_store elems);

    virtual std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual TreePtr _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};
CheckSize(Array, 56, 8);

class Literal final : public Expression {
public:
    core::TypePtr value;

    Literal(core::LocOffsets loc, const core::TypePtr &value);
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
    virtual TreePtr _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};
CheckSize(Literal, 32, 8);

class UnresolvedConstantLit final : public Expression {
public:
    core::NameRef cnst;
    TreePtr scope;

    UnresolvedConstantLit(core::LocOffsets loc, TreePtr scope, core::NameRef cnst);
    virtual std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual TreePtr _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};
CheckSize(UnresolvedConstantLit, 32, 8);

class ConstantLit final : public Expression {
public:
    core::SymbolRef symbol; // If this is a normal constant. This symbol may be already dealiased.
    // For constants that failed resolution, symbol will be set to StubModule and resolutionScopes
    // will be set to whatever nesting scope we estimate the constant could have been defined in.
    using ResolutionScopes = InlinedVector<core::SymbolRef, 1>;
    ResolutionScopes resolutionScopes;
    TreePtr original;

    ConstantLit(core::LocOffsets loc, core::SymbolRef symbol, TreePtr original);
    virtual std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual TreePtr _deepCopy(const Expression *avoid, bool root = false) const;
    std::optional<std::pair<core::SymbolRef, std::vector<core::NameRef>>>
    fullUnresolvedPath(const core::GlobalState &gs) const;

private:
    virtual void _sanityCheck();
};
CheckSize(ConstantLit, 56, 8);

class ZSuperArgs final : public Expression {
public:
    // null if no block passed
    ZSuperArgs(core::LocOffsets loc);
    virtual std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual TreePtr _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};
CheckSize(ZSuperArgs, 16, 8);

class Block final : public Expression {
public:
    MethodDef::ARGS_store args;
    TreePtr body;

    Block(core::LocOffsets loc, MethodDef::ARGS_store args, TreePtr body);
    virtual std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual TreePtr _deepCopy(const Expression *avoid, bool root = false) const;
    virtual void _sanityCheck();
};
CheckSize(Block, 48, 8);

class InsSeq final : public Expression {
public:
    static constexpr int EXPECTED_STATS_COUNT = 4;
    using STATS_store = InlinedVector<TreePtr, EXPECTED_STATS_COUNT>;
    // Statements
    STATS_store stats;

    // The distinguished final expression (determines return value)
    TreePtr expr;

    InsSeq(core::LocOffsets locOffsets, STATS_store stats, TreePtr expr);
    virtual std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual TreePtr _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};
CheckSize(InsSeq, 64, 8);

class EmptyTree final : public Expression {
public:
    EmptyTree();
    virtual std::string toStringWithTabs(const core::GlobalState &gs, int tabs = 0) const;
    virtual std::string showRaw(const core::GlobalState &gs, int tabs = 0);
    virtual std::string nodeName();
    virtual TreePtr _deepCopy(const Expression *avoid, bool root = false) const;

private:
    virtual void _sanityCheck();
};
CheckSize(EmptyTree, 16, 8);

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

} // namespace sorbet::ast

#endif // SORBET_TREES_H
