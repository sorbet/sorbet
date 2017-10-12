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

std::unique_ptr<Statement> mkAssign(SymbolRef symbol, std::unique_ptr<Statement> &rhs) {
    auto lhsChecked = stat2Expr(mkIdent(symbol));
    auto rhsChecked = stat2Expr(rhs);
    return std::make_unique<Assign>(std::move(lhsChecked), std::move(rhsChecked));
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

std::unique_ptr<Statement> node2TreeImpl(Context ctx, std::unique_ptr<parser::Node> &what) {
    if (what.get() == nullptr)
        return std::make_unique<EmptyTree>();
    std::unique_ptr<Statement> result;
    typecase(what.get(),
             [&](parser::And *a) {
                 auto send = mkSend1(node2TreeImpl(ctx, a->left), Names::andAnd(), node2TreeImpl(ctx, a->right));
                 result.swap(send);
             },
             [&](parser::AndAsgn *a) {
                 auto recv = node2TreeImpl(ctx, a->left);
                 auto arg = node2TreeImpl(ctx, a->right);
                 auto argChecked = stat2Expr(arg);
                 if (auto s = dynamic_cast<Send *>(recv.get())) {
                     Error::check(s->args.empty());
                     auto tempSym = ctx.state.newTemporary(UniqueNameKind::Desugar, s->fun, ctx.owner);
                     auto temp = mkAssign(tempSym, std::move(s->recv));
                     recv.release();
                     auto cond = mkSend0(mkIdent(tempSym), s->fun);
                     auto body = mkSend1(mkIdent(tempSym), s->fun.addEq(), arg);
                     auto elsep = mkEmptyTree();
                     auto iff = mkIf(cond, body, elsep);
                     result.swap(iff);
                 } else if (auto i = dynamic_cast<Ident *>(recv.get())) {
                     auto cond = mkIdent(i->symbol);
                     auto body = mkAssign(i->symbol, arg);
                     auto elsep = mkEmptyTree();
                     auto iff = mkIf(cond, body, elsep);
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
                 auto res = std::unique_ptr<Statement>(
                     new ClassDef(ctx.state.defn_todo(), stat2Expr(node2TreeImpl(ctx, module->name)), body, true));
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
                 auto res = std::unique_ptr<Statement>(
                     new ClassDef(ctx.state.defn_todo(), stat2Expr(node2TreeImpl(ctx, claz->name)), body, false));
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