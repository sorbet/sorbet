#include "Trees.h"

namespace sruby {
namespace ast {

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

ClassDef::ClassDef(SymbolRef symbol, std::unique_ptr<Stat> rhs) : Decl(symbol), rhs(std::move(rhs)) {}

MethodDef::MethodDef(SymbolRef symbol, std::vector<SymbolRef> args, std::unique_ptr<Expr> rhs)
    : Decl(symbol), rhs(std::move(rhs)), args(std::move(args)) {}

Decl::Decl(SymbolRef symbol) : symbol(symbol) {}

SelfMethodDef::SelfMethodDef(SymbolRef symbol, std::vector<SymbolRef> args, std::unique_ptr<Expr> rhs)
    : Decl(symbol), rhs(std::move(rhs)), args(std::move(args)) {}

ConstDef::ConstDef(SymbolRef symbol, std::unique_ptr<Expr> rhs) : Decl(symbol), rhs(std::move(rhs)) {}

If::If(std::unique_ptr<Expr> cond, std::unique_ptr<Expr> thenp, std::unique_ptr<Expr> elsep)
    : cond(std::move(cond)), thenp(std::move(thenp)), elsep(std::move(elsep)) {}

Breakable::Breakable(u1 break_tag) : break_tag(break_tag) {}

While::While(u1 break_tag, std::unique_ptr<Expr> cond, std::unique_ptr<Stat> body)
    : Breakable(break_tag), cond(std::move(cond)), body(std::move(body)) {}

Break::Break(u1 break_tag) : break_tag(break_tag) {}

Next::Next(u1 break_tag) : break_tag(break_tag) {}

Return::Return(std::unique_ptr<Expr> expr) : expr(std::move(expr)) {}

Ident::Ident(SymbolRef symbol) : symbol(symbol) {}

Assign::Assign(std::unique_ptr<Expr> lhs, std::unique_ptr<Expr> rhs) : lhs(std::move(lhs)), rhs(std::move(rhs)) {}

Send::Send(std::unique_ptr<Expr> recv, NameRef fun, std::vector<std::unique_ptr<Expr>> &&args)
    :

      recv(std::move(recv)), fun(std::move(fun)), args(std::move(args)) {}

New::New(SymbolRef claz, std::vector<std::unique_ptr<Expr>> &&args) : claz(claz), args(std::move(args)) {}

NamedArg::NamedArg(NameRef name, std::unique_ptr<Expr> arg) : name(name), arg(std::move(arg)) {}

FloatLit::FloatLit(float value) : value(value) {}

IntLit::IntLit(int value) : value(value) {}

StringLit::StringLit(NameRef value) : value(value) {}

ConstantLit::ConstantLit(NameRef cnst) : cnst(cnst) {}

ArraySplat::ArraySplat(std::unique_ptr<Expr> arg) : arg(std::move(arg)) {}

HashSplat::HashSplat(std::unique_ptr<Expr> arg) : arg(std::move(arg)) {}

Self::Self(SymbolRef claz) : claz(claz) {}

Closure::Closure(SymbolRef method) : method(method) {}

InsSeq::InsSeq(std::vector<std::unique_ptr<Stat>> &&stats, std::unique_ptr<Expr> expr)
    : stats(std::move(stats)), expr(std::move(expr)) {}

} // namespace ast
} // namespace sruby
