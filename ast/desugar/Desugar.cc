#include <algorithm>

#include "../ast.h"
#include "Desugar.h"
#include "ast/ast.h"
#include "common/common.h"

namespace ruby_typer {
namespace ast {
namespace desugar {
using namespace parser;

std::unique_ptr<Expression> stat2Expr(std::unique_ptr<Statement> &expr) {
    Error::check(dynamic_cast<Expression *>(expr.get()));
    return std::unique_ptr<Expression>(dynamic_cast<Expression *>(expr.release()));
}

std::unique_ptr<Expression> stat2Expr(std::unique_ptr<Statement> &&expr) {
    return stat2Expr(expr);
}

std::unique_ptr<Statement> mkSend(std::unique_ptr<Statement> &recv, NameRef fun,
                                  std::vector<std::unique_ptr<Statement>> &args) {
    Error::check(dynamic_cast<Expression *>(recv.get()));
    auto recvChecked = stat2Expr(recv);
    auto nargs = std::vector<std::unique_ptr<Expression>>();
    for (auto &a : args) {
        nargs.emplace_back(stat2Expr(a));
    }
    return std::make_unique<Send>(std::move(recvChecked), fun, std::move(nargs));
}

std::unique_ptr<Statement> mkSend1(std::unique_ptr<Statement> &recv, NameRef fun, std::unique_ptr<Statement> &arg1) {
    auto recvChecked = stat2Expr(recv);
    auto argChecked = stat2Expr(arg1);
    auto nargs = std::vector<std::unique_ptr<Expression>>();
    nargs.emplace_back(std::move(argChecked));
    return std::make_unique<Send>(std::move(recvChecked), fun, std::move(nargs));
}

std::unique_ptr<Statement> mkSend1(std::unique_ptr<Statement> &&recv, NameRef fun, std::unique_ptr<Statement> &&arg1) {
    return mkSend1(recv, fun, arg1);
}

std::unique_ptr<Statement> mkSend1(std::unique_ptr<Statement> &&recv, NameRef fun, std::unique_ptr<Statement> &arg1) {
    return mkSend1(recv, fun, arg1);
}

std::unique_ptr<Statement> mkSend1(std::unique_ptr<Statement> &recv, NameRef fun, std::unique_ptr<Statement> &&arg1) {
    return mkSend1(recv, fun, arg1);
}

std::unique_ptr<Statement> mkSend0(std::unique_ptr<Statement> &recv, NameRef fun) {
    auto recvChecked = stat2Expr(recv);
    auto nargs = std::vector<std::unique_ptr<Expression>>();
    return std::make_unique<Send>(std::move(recvChecked), fun, std::move(nargs));
}

std::unique_ptr<Statement> mkSend0(std::unique_ptr<Statement> &&recv, NameRef fun) {
    return mkSend0(recv, fun);
}

std::unique_ptr<Statement> mkIdent(SymbolRef symbol) {
    return std::unique_ptr<Statement>(new Ident(symbol));
}

std::unique_ptr<Statement> cpIdent(Ident &id) {
    if (id.symbol.isSynthetic()) {
        return std::unique_ptr<Statement>(new Ident(id.name, id.symbol));
    } else {
        return std::unique_ptr<Statement>(new Ident(id.symbol));
    }
}

std::unique_ptr<Statement> mkAssign(std::unique_ptr<Statement> &lhs, std::unique_ptr<Statement> &rhs) {
    auto lhsChecked = stat2Expr(lhs);
    auto rhsChecked = stat2Expr(rhs);
    return std::make_unique<Assign>(std::move(lhsChecked), std::move(rhsChecked));
}

std::unique_ptr<Statement> mkAssign(SymbolRef symbol, std::unique_ptr<Statement> &rhs) {
    auto id = mkIdent(symbol);
    return mkAssign(id, rhs);
}

std::unique_ptr<Statement> mkAssign(SymbolRef symbol, std::unique_ptr<Statement> &&rhs) {
    return mkAssign(symbol, rhs);
}

std::unique_ptr<Statement> mkIf(std::unique_ptr<Statement> &cond, std::unique_ptr<Statement> &thenp,
                                std::unique_ptr<Statement> &elsep) {
    return std::make_unique<If>(stat2Expr(cond), stat2Expr(thenp), stat2Expr(elsep));
}

std::unique_ptr<Statement> mkEmptyTree() {
    return std::make_unique<EmptyTree>();
}

std::unique_ptr<Statement> mkInsSeq(std::vector<std::unique_ptr<Statement>> &&stats,
                                    std::unique_ptr<Expression> &&expr) {
    return std::make_unique<InsSeq>(std::move(stats), std::move(expr));
}
std::unique_ptr<Statement> mkTrue() {
    return std::make_unique<BoolLit>(true);
}

std::unique_ptr<Statement> mkFalse() {
    return std::make_unique<BoolLit>(false);
}

std::unique_ptr<Statement> node2TreeImpl(Context ctx, std::unique_ptr<parser::Node> &what) {
    if (what.get() == nullptr)
        return std::make_unique<EmptyTree>();
    std::unique_ptr<Statement> result;
    typecase(what.get(),
             [&](parser::And *a) {
                 auto iff = mkIf(node2TreeImpl(ctx, a->left), node2TreeImpl(ctx, a->right), mkFalse());
                 result.swap(iff);
             },
             [&](parser::Or *a) {
                 auto iff = mkIf(node2TreeImpl(ctx, a->left), mkTrue(), node2TreeImpl(ctx, a->right));
                 result.swap(iff);
             },
             [&](parser::AndAsgn *a) {
                 auto recv = node2TreeImpl(ctx, a->left);
                 auto arg = node2TreeImpl(ctx, a->right);
                 if (auto s = dynamic_cast<Send *>(recv.get())) {
                     Error::check(s->args.empty());
                     auto tempSym = ctx.state.newTemporary(UniqueNameKind::Desugar, s->fun, ctx.owner);
                     auto temp = mkAssign(tempSym, std::move(s->recv));
                     recv.reset();
                     auto cond = mkSend0(mkIdent(tempSym), s->fun);
                     auto body = mkSend1(mkIdent(tempSym), s->fun.addEq(), arg);
                     auto elsep = mkIdent(tempSym);
                     auto iff = mkIf(cond, body, elsep);
                     result.swap(iff);
                 } else if (auto i = dynamic_cast<Ident *>(recv.get())) {
                     auto cond = cpIdent(*i);
                     auto body = mkAssign(recv, arg);
                     auto elsep = cpIdent(*i);
                     auto iff = mkIf(cond, body, elsep);
                     result.swap(iff);
                 } else {
                     Error::notImplemented();
                 }
             },
             [&](parser::OrAsgn *a) {
                 auto recv = node2TreeImpl(ctx, a->left);
                 auto arg = node2TreeImpl(ctx, a->right);
                 if (auto s = dynamic_cast<Send *>(recv.get())) {
                     Error::check(s->args.empty());
                     auto tempSym = ctx.state.newTemporary(UniqueNameKind::Desugar, s->fun, ctx.owner);
                     auto temp = mkAssign(tempSym, std::move(s->recv));
                     recv.reset();
                     auto cond = mkSend0(mkIdent(tempSym), s->fun);
                     auto body = mkSend1(mkIdent(tempSym), s->fun.addEq(), arg);
                     auto elsep = mkIdent(tempSym);
                     auto iff = mkIf(cond, elsep, body);
                     result.swap(iff);
                 } else if (auto i = dynamic_cast<Ident *>(recv.get())) {
                     auto cond = cpIdent(*i);
                     auto body = mkAssign(recv, arg);
                     auto elsep = cpIdent(*i);
                     auto iff = mkIf(cond, elsep, body);
                     result.swap(iff);

                 } else {
                     Error::notImplemented();
                 }
             },
             [&](parser::Send *a) {
                 auto rec = node2TreeImpl(ctx, a->receiver);
                 std::vector<std::unique_ptr<Statement>> args;
                 for (auto &stat : a->args) {
                     args.emplace_back(stat2Expr(node2TreeImpl(ctx, stat)));
                 };

                 auto send = mkSend(rec, a->method, args);
                 result.swap(send);
             },
             [&](parser::DString *a) {
                 auto it = a->nodes.begin();
                 auto end = a->nodes.end();
                 Error::check(it != end);
                 auto res = mkSend0(node2TreeImpl(ctx, *it), Names::to_s());
                 ++it;
                 for (; it != end; ++it) {
                     auto &stat = *it;
                     auto narg = node2TreeImpl(ctx, stat);
                     auto n = mkSend1(res, Names::concat(), mkSend0(narg, Names::to_s()));
                     res.reset(n.release());
                 };

                 result.swap(res);
             },
             [&](parser::Symbol *a) {
                 auto res = std::unique_ptr<Statement>(new ast::Symbol(ctx.state.enterNameUTF8(a->val)));
                 result.swap(res);
             },
             [&](parser::String *a) {
                 auto res = std::unique_ptr<Statement>(new StringLit(ctx.state.enterNameUTF8(a->val)));
                 result.swap(res);
             },
             [&](parser::Const *a) {
                 auto scope = node2TreeImpl(ctx, a->scope);
                 auto res = std::unique_ptr<Statement>(new ConstantLit(stat2Expr(scope), a->name));
                 result.swap(res);
             },
             [&](parser::Begin *a) {
                 std::vector<std::unique_ptr<Statement>> stats;
                 auto end = --a->stmts.end();
                 for (auto it = a->stmts.begin(); it != end; ++it) {
                     auto &stat = *it;
                     stats.emplace_back(node2TreeImpl(ctx, stat));
                 };
                 auto &last = a->stmts.back();
                 auto expr = node2TreeImpl(ctx, last);
                 if (auto *epx = dynamic_cast<Expression *>(expr.get())) {
                     auto exp = stat2Expr(expr);
                     auto block = mkInsSeq(std::move(stats), std::move(exp));
                     result.swap(block);
                 } else {
                     stats.emplace_back(std::move(expr));
                     auto block = mkInsSeq(std::move(stats), stat2Expr(mkEmptyTree()));
                     result.swap(block);
                 }
             },
             [&](parser::Module *module) {
                 std::vector<std::unique_ptr<Statement>> body;
                 if (auto *a = dynamic_cast<parser::Begin *>(module->body.get())) {
                     for (auto &stat : a->stmts) {
                         body.emplace_back(node2TreeImpl(ctx, stat));
                     };
                 } else {
                     body.emplace_back(node2TreeImpl(ctx, module->body));
                 }
                 auto res = std::unique_ptr<Statement>(new ClassDef(
                     ctx.state.defn_todo(), stat2Expr(node2TreeImpl(ctx, module->name)), body, ClassDefKind::Module));
                 result.swap(res);
             },
             [&](parser::Class *claz) {
                 std::vector<std::unique_ptr<Statement>> body;
                 if (auto *a = dynamic_cast<parser::Begin *>(claz->body.get())) {
                     for (auto &stat : a->stmts) {
                         body.emplace_back(node2TreeImpl(ctx, stat));
                     };
                 } else {
                     body.emplace_back(node2TreeImpl(ctx, claz->body));
                 }
                 auto res = std::unique_ptr<Statement>(new ClassDef(
                     ctx.state.defn_todo(), stat2Expr(node2TreeImpl(ctx, claz->name)), body, ClassDefKind::Class));
                 result.swap(res);
             },
             [&](parser::Arg *arg) {
                 auto res = std::unique_ptr<Statement>(new Ident(arg->name, ContextBase::defn_lvar_todo()));
                 result.swap(res);
             },
             [&](parser::DefMethod *method) {
                 std::vector<std::unique_ptr<Expression>> args;
                 if (auto *oargs = dynamic_cast<parser::Args *>(method->args.get())) {
                     for (auto &arg : oargs->args) {
                         args.emplace_back(stat2Expr(node2TreeImpl(ctx, arg)));
                     }
                 } else if (auto *arg = dynamic_cast<parser::Arg *>(method->args.get())) {
                     args.emplace_back(stat2Expr(node2TreeImpl(ctx, method->args)));
                 } else if (method->args.get() == nullptr) {
                     // do nothing
                 } else {
                     Error::notImplemented();
                 }
                 auto res = std::unique_ptr<Statement>(new MethodDef(
                     ctx.state.defn_todo(), method->name, args, stat2Expr(node2TreeImpl(ctx, method->body)), false));
                 result.swap(res);
             },
             [&](parser::Block *block) {
                 std::vector<std::unique_ptr<Expression>> args;
                 auto recv = node2TreeImpl(ctx, block->send);
                 Error::check(dynamic_cast<Send *>(recv.get()));
                 std::unique_ptr<Send> send(dynamic_cast<Send *>(recv.release()));
                 if (auto *oargs = dynamic_cast<parser::Args *>(block->args.get())) {
                     for (auto &arg : oargs->args) {
                         args.emplace_back(stat2Expr(node2TreeImpl(ctx, arg)));
                     }
                 } else if (auto *arg = dynamic_cast<parser::Arg *>(block->args.get())) {
                     args.emplace_back(stat2Expr(node2TreeImpl(ctx, block->args)));
                 } else if (block->args.get() == nullptr) {
                     // do nothing
                 } else {
                     Error::notImplemented();
                 }
                 auto res = std::unique_ptr<Statement>(
                     new Block(std::move(send), args, stat2Expr(node2TreeImpl(ctx, block->body))));
                 result.swap(res);
             },

             [&](parser::IVar *var) {
                 auto res = std::unique_ptr<Statement>(new Ident(var->name, ContextBase::defn_ivar_todo()));
                 result.swap(res);
             },
             [&](parser::LVar *var) {
                 auto res = std::unique_ptr<Statement>(new Ident(var->name, ContextBase::defn_lvar_todo()));
                 result.swap(res);
             },
             [&](parser::GVar *var) {
                 auto res = std::unique_ptr<Statement>(new Ident(var->name, ContextBase::defn_gvar_todo()));
                 result.swap(res);
             },
             [&](parser::CVar *var) {
                 auto res = std::unique_ptr<Statement>(new Ident(var->name, ContextBase::defn_cvar_todo()));
                 result.swap(res);
             },
             [&](parser::IVar *var) {
                 auto res = std::unique_ptr<Statement>(new Ident(var->name, ContextBase::defn_ivar_todo()));
                 result.swap(res);
             },
             [&](parser::LVarLhs *var) {
                 auto res = std::unique_ptr<Statement>(new Ident(var->name, ContextBase::defn_lvar_todo()));
                 result.swap(res);
             },
             [&](parser::GVarLhs *var) {
                 auto res = std::unique_ptr<Statement>(new Ident(var->name, ContextBase::defn_gvar_todo()));
                 result.swap(res);
             },
             [&](parser::CVarLhs *var) {
                 auto res = std::unique_ptr<Statement>(new Ident(var->name, ContextBase::defn_cvar_todo()));
                 result.swap(res);
             },
             [&](parser::IVarAsgn *asgn) {
                 auto lhs = std::unique_ptr<Statement>(new Ident(asgn->name, ContextBase::defn_ivar_todo()));
                 auto rhs = node2TreeImpl(ctx, asgn->expr);
                 auto res = mkAssign(lhs, rhs);
                 result.swap(res);
             },
             [&](parser::LVarAsgn *asgn) {
                 auto lhs = std::unique_ptr<Statement>(new Ident(asgn->name, ContextBase::defn_lvar_todo()));
                 auto rhs = node2TreeImpl(ctx, asgn->expr);
                 auto res = mkAssign(lhs, rhs);
                 result.swap(res);
             },
             [&](parser::GVarAsgn *asgn) {
                 auto lhs = std::unique_ptr<Statement>(new Ident(asgn->name, ContextBase::defn_gvar_todo()));
                 auto rhs = node2TreeImpl(ctx, asgn->expr);
                 auto res = mkAssign(lhs, rhs);
                 result.swap(res);
             },
             [&](parser::CVarAsgn *asgn) {
                 auto lhs = std::unique_ptr<Statement>(new Ident(asgn->name, ContextBase::defn_cvar_todo()));
                 auto rhs = node2TreeImpl(ctx, asgn->expr);
                 auto res = mkAssign(lhs, rhs);
                 result.swap(res);
             },
             [&](parser::ConstAsgn *constAsgn) {
                 auto scope = node2TreeImpl(ctx, constAsgn->scope);
                 auto lhs = std::unique_ptr<Statement>(new ConstantLit(stat2Expr(scope), constAsgn->name));
                 auto rhs = node2TreeImpl(ctx, constAsgn->expr);
                 auto res = mkAssign(lhs, rhs);
                 result.swap(res);
             },
             [&](parser::Super *super) {
                 std::vector<std::unique_ptr<Expression>> args;
                 for (auto &stat : super->args) {
                     args.emplace_back(stat2Expr(node2TreeImpl(ctx, stat)));
                 };

                 auto res = std::unique_ptr<Statement>(new Super(std::move(args)));
                 result.swap(res);
             },
             [&](parser::Integer *integer) {
                 auto res = std::unique_ptr<Statement>(new IntLit(std::stoi(integer->val)));
                 result.swap(res);
             },
             [&](parser::Array *array) {
                 std::vector<std::unique_ptr<Expression>> elems;
                 for (auto &stat : array->elts) {
                     elems.emplace_back(stat2Expr(node2TreeImpl(ctx, stat)));
                 };

                 auto res = std::unique_ptr<Statement>(new Array(elems));
                 result.swap(res);
             },

             [&](parser::Return *ret) {
                 if (ret->exprs.size() > 1) {
                     std::vector<std::unique_ptr<Expression>> elems;
                     for (auto &stat : ret->exprs) {
                         elems.emplace_back(stat2Expr(node2TreeImpl(ctx, stat)));
                     };
                     auto arr = std::unique_ptr<Expression>(new Array(elems));
                     auto res = std::unique_ptr<Statement>(new Return(std::move(arr)));
                     result.swap(res);
                 } else if (ret->exprs.size() == 1) {
                     auto res = std::unique_ptr<Statement>(new Return(stat2Expr(node2TreeImpl(ctx, ret->exprs[0]))));
                     result.swap(res);
                 } else {
                     auto res = std::unique_ptr<Statement>(new Return(stat2Expr(mkEmptyTree())));
                     result.swap(res);
                 }
             },
             [&](parser::If *a) {
                 auto cond = node2TreeImpl(ctx, a->condition);
                 auto thenp = node2TreeImpl(ctx, a->then_);
                 auto elsep = node2TreeImpl(ctx, a->else_);
                 auto iff = mkIf(cond, thenp, elsep);
                 result.swap(iff);
             },

             [&](parser::Node *a) { result.reset(new NotSupported(a->nodeName())); });
    Error::check(result.get());
    return result;
}

std::unique_ptr<Statement> node2Tree(Context ctx, parser::Node *what) {
    auto adapter = std::unique_ptr<Node>(what);
    auto result = node2TreeImpl(ctx, adapter);
    adapter.release();
    return result;
}
} // namespace desugar
} // namespace ast
} // namespace ruby_typer