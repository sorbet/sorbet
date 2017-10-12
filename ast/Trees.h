#ifndef SRUBY_TREES_H
#define SRUBY_TREES_H

#include "../common/common.h"
#include "Context.h"
#include "Symbols.h"
#include <memory>
#include <vector>

namespace ruby_typer {
namespace ast {

class Stat {
public:
    Stat() = default;
    virtual ~Stat() = default;
    virtual std::string toString(ContextBase &ctx, int tabs = 0) = 0;
};

class Expr : public Stat {};

class ControlFlow : public Expr {};

class Decl : public Expr {
public:
    SymbolRef symbol;

    Decl(SymbolRef symbol);
};

class ClassDef : public Decl {
public:
    inline SymbolRef parent(Context ctx) {
        return symbol.info(ctx).parent(ctx);
    }

    inline std::vector<SymbolRef> &mixins(Context ctx) {
        return symbol.info(ctx).mixins(ctx);
    }

    std::vector<std::unique_ptr<Stat>> rhs;
    std::unique_ptr<Expr> name;
    bool isModule;

    ClassDef(SymbolRef symbol, std::unique_ptr<Expr> name, std::vector<std::unique_ptr<Stat>> &rhs, bool isModule);

    virtual std::string toString(ContextBase &ctx, int tabs = 0);
};

class MethodDef : public Decl {
public:
    std::unique_ptr<Expr> rhs;
    std::vector<std::unique_ptr<Expr>> args;
    NameRef name;
    bool isSelf;

    MethodDef(SymbolRef symbol, NameRef name, std::vector<std::unique_ptr<Expr>> &args, std::unique_ptr<Expr> rhs,
              bool isSelf);
    virtual std::string toString(ContextBase &ctx, int tabs = 0);
};

class ConstDef : public Decl {
public:
    std::unique_ptr<Expr> rhs;

    ConstDef(SymbolRef symbol, std::unique_ptr<Expr> rhs);
    virtual std::string toString(ContextBase &ctx, int tabs = 0);
};

class If : public ControlFlow {
public:
    std::unique_ptr<Expr> cond;
    std::unique_ptr<Expr> thenp;
    std::unique_ptr<Expr> elsep;
    If(std::unique_ptr<Expr> cond, std::unique_ptr<Expr> thenp, std::unique_ptr<Expr> elsep);
    virtual std::string toString(ContextBase &ctx, int tabs = 0);
};

class Breakable : public ControlFlow {
public:
    u1 break_tag;

    Breakable(u1 break_tag);
};

class While : public Breakable {
public:
    std::unique_ptr<Expr> cond;
    std::unique_ptr<Stat> body;

    While(u1 break_tag, std::unique_ptr<Expr> cond, std::unique_ptr<Stat> body);
    virtual std::string toString(ContextBase &ctx, int tabs = 0);
};

class For : public Breakable {
    // TODO
};

class Break : public ControlFlow {
public:
    u1 break_tag;

    Break(u1 break_tag);
    virtual std::string toString(ContextBase &ctx, int tabs = 0);
};

class Next : public ControlFlow {
public:
    u1 break_tag;

    Next(u1 break_tag);
    virtual std::string toString(ContextBase &ctx, int tabs = 0);
};

class Return : public ControlFlow {
public:
    std::unique_ptr<Expr> expr;

    Return(std::unique_ptr<Expr> expr);
    virtual std::string toString(ContextBase &ctx, int tabs = 0);
};

class Rescue : public ControlFlow {
public:
    std::unique_ptr<Expr> body;
    SymbolRef binder;
    SymbolRef binder_type;
    std::unique_ptr<Expr> handler;

    virtual std::string toString(ContextBase &ctx, int tabs = 0);
};

class Ident : public Expr {
public:
    SymbolRef symbol;

    Ident(SymbolRef symbol);
    virtual std::string toString(ContextBase &ctx, int tabs = 0);
};

class Symbol : public Expr {
public:
    NameRef name;

    Symbol(NameRef name);

    virtual std::string toString(ContextBase &ctx, int tabs);
};

class Assign : public Expr {
public:
    std::unique_ptr<Expr> lhs;
    std::unique_ptr<Expr> rhs;

    Assign(std::unique_ptr<Expr> lhs, std::unique_ptr<Expr> rhs);
    virtual std::string toString(ContextBase &ctx, int tabs = 0);
};

class Send : public Expr {
public:
    std::unique_ptr<Expr> recv;
    NameRef fun;
    std::vector<std::unique_ptr<Expr>> args;

    Send(std::unique_ptr<Expr> recv, NameRef fun, std::vector<std::unique_ptr<Expr>> &&args);
    virtual std::string toString(ContextBase &ctx, int tabs = 0);
};

class New : public Expr {
public:
    SymbolRef claz;
    std::vector<std::unique_ptr<Expr>> args;

    New(SymbolRef claz, std::vector<std::unique_ptr<Expr>> &&args);
    virtual std::string toString(ContextBase &ctx, int tabs = 0);
};

class NamedArg : public Expr {
public:
    NameRef name;
    std::unique_ptr<Expr> arg;

    NamedArg(NameRef name, std::unique_ptr<Expr> arg);
    virtual std::string toString(ContextBase &ctx, int tabs = 0);
};

class Hash : public Expr {
    // TODO
};

class Array : public Expr {
    // TODO
};

class FloatLit : public Expr {
public:
    float value;

    FloatLit(float value);
    virtual std::string toString(ContextBase &ctx, int tabs = 0);
};

class IntLit : public Expr {
public:
    int value;

    IntLit(int value);
    virtual std::string toString(ContextBase &ctx, int tabs = 0);
};

class StringLit : public Expr {
public:
    NameRef value;

    StringLit(NameRef value);
    virtual std::string toString(ContextBase &ctx, int tabs = 0);
};

class ConstantLit : public Expr {
public:
    NameRef cnst;
    std::unique_ptr<Expr> scope;

    ConstantLit(std::unique_ptr<Expr> scope, NameRef cnst);
    virtual std::string toString(ContextBase &ctx, int tabs = 0);
};

class ArraySplat : public Expr {
public:
    std::unique_ptr<Expr> arg;

    ArraySplat(std::unique_ptr<Expr> arg);
    virtual std::string toString(ContextBase &ctx, int tabs = 0);
};

class HashSplat : public Expr {
public:
    std::unique_ptr<Expr> arg;

    HashSplat(std::unique_ptr<Expr> arg);
    virtual std::string toString(ContextBase &ctx, int tabs = 0);
};

class Self : public Expr {
public:
    SymbolRef claz;

    Self(SymbolRef claz);
    virtual std::string toString(ContextBase &ctx, int tabs = 0);
};

class Closure : public Expr {
public:
    SymbolRef method;

    Closure(SymbolRef method);
    virtual std::string toString(ContextBase &ctx, int tabs = 0);
};

class InsSeq : public Expr {
public:
    std::vector<std::unique_ptr<Stat>> stats;
    std::unique_ptr<Expr> expr;

    InsSeq(std::vector<std::unique_ptr<Stat>> &&stats, std::unique_ptr<Expr> expr);
    virtual std::string toString(ContextBase &ctx, int tabs = 0);
};

class EmptyTree : public Expr {

    virtual std::string toString(ContextBase &ctx, int tabs = 0);
};

class NotSupported : public Expr {
    std::string why;

public:
    NotSupported(const std::string &why);

    virtual std::string toString(ContextBase &ctx, int tabs);
};

/** https://git.corp.stripe.com/gist/nelhage/51564501674174da24822e60ad770f64
 *
 *  [] - prototype only
 *
 *                 / Control Flow <- while, if, for, break, next, return, rescue, case
 * Pre-CFG-Node <-
 *                 \ Instruction <- assign, send, [new], ident, named_arg, hash, array, literals(symbols, ints, floats,
 * strings, constants, nil), constants(resolver will desugar it into literals), array_splat(*), hash_splat(**), self,
 * insseq, closure)
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
} // namespace ruby_typer

#endif // SRUBY_TREES_H
