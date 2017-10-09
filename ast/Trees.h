#ifndef SRUBY_TREES_H
#define SRUBY_TREES_H

#include "Context.h"
#include "Symbols.h"
#include <memory>
#include <vector>

namespace sruby {
namespace ast {
class Stat {
public:
    Stat() = default;
    virtual ~Stat() = default;
};
class Expr : public Stat {};
class ControlFlow : public Expr {};
class Decl {
public:
    Decl() = default;

    virtual ~Decl() = default;

    SymbolRef symbol;
};

class ClassDef : public Decl {
public:
    inline SymbolRef parent(Context ctx) {
        return symbol.info(ctx).parent(ctx);
    }

    inline std::vector<SymbolRef> &mixins(Context ctx) {
        return symbol.info(ctx).mixins(ctx);
    }

    std::unique_ptr<Expr> rhs;
};

class MethodDef : public Decl {
public:
    std::unique_ptr<Expr> rhs;
    std::vector<SymbolRef> args;
};

class SelfMethodDef : public Decl {
public:
    std::unique_ptr<Expr> rhs;
    std::vector<SymbolRef> args;
};

class ConstDef : public Decl {
public:
    std::unique_ptr<Expr> rhs;
};

class If : public ControlFlow {
    std::unique_ptr<Expr> cond;
    std::unique_ptr<Expr> thenp;
    std::unique_ptr<Expr> elsep;
};

class Breakable : public ControlFlow {
    u1 break_tag;
};

class While : public Breakable {
    std::unique_ptr<Expr> cond;
    std::unique_ptr<Expr> body;
};

class For : public Breakable {
    // TODO
};

class Break : public ControlFlow {
    u1 break_tag;
};

class Next : public ControlFlow {
    u1 break_tag;
};

class Return : public ControlFlow {
    std::unique_ptr<Expr> expr;
};

class Rescue : public ControlFlow {
    std::unique_ptr<Expr> body;
    SymbolRef binder;
    SymbolRef binder_type;
    std::unique_ptr<Expr> handler;
};

class Ident : public Expr {
    SymbolRef symbol;
};

class Assign : public Expr {
    std::unique_ptr<Expr> lhs;
    std::unique_ptr<Expr> rhs;
};

class Send : public Expr {
    std::unique_ptr<Expr> recv;
    NameRef fun;
    std::vector<std::unique_ptr<Expr>> args;
};

class New : public Expr {
    SymbolRef claz;
    std::vector<std::unique_ptr<Expr>> args;
};

class NamedArg : public Expr {
    NameRef name;
    std::unique_ptr<Expr> arg;
};

class Hash : public Expr {
    // TODO
};

class Array : public Expr {
    // TODO
};

class FloatLit : public Expr {
    float value;
};

class IntLit : public Expr {
    int value;
};

class StringLit : public Expr {
    NameRef value;
};

class ConstantLit : public Expr {
    NameRef cnst;
};

class ArraySplat : public Expr {
    std::unique_ptr<Expr> arg;
};

class HashSplat : public Expr {
    std::unique_ptr<Expr> arg;
};

class Self : public Expr {
    SymbolRef claz;
};

class Closure : public Expr {
    SymbolRef method;
};

class InsSeq : public Expr {
    std::vector<std::unique_ptr<Expr>> stats;
    std::unique_ptr<Expr> expr;
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
} // namespace sruby

#endif // SRUBY_TREES_H
