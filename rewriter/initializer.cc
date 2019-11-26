#include "rewriter/initializer.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/core.h"

#include <utility>

using namespace std;
namespace sorbet::rewriter {

bool isSig(const ast::Send *send) {
    if (send->fun != core::Names::sig()) {
        return false;
    }
    if (send->block.get() == nullptr) {
        return false;
    }
    auto nargs = send->args.size();
    if (nargs != 0 && nargs != 1) {
        return false;
    }
    auto block = ast::cast_tree<ast::Block>(send->block.get());
    ENFORCE(block);
    auto body = ast::cast_tree<ast::Send>(block->body.get());
    if (!body) {
        return false;
    }
    if (body->fun != core::Names::void_()) {
        return false;
    }

    return true;
}

void maybeAddLet(core::MutableContext ctx, ast::Expression *expr,
                 UnorderedMap<core::NameRef, ast::Expression *> &argTypeMap) {
    auto assn = ast::cast_tree<ast::Assign>(expr);
    if (!assn) {
        return;
    }

    auto lhs = ast::cast_tree<ast::UnresolvedIdent>(assn->lhs.get());
    if (!lhs || lhs->kind != ast::UnresolvedIdent::Instance) {
        return;
    }

    auto rhs = ast::cast_tree<ast::UnresolvedIdent>(assn->rhs.get());
    if (!rhs || rhs->kind != ast::UnresolvedIdent::Local) {
        return;
    }

    auto typeExpr = argTypeMap[rhs->name];
    if (typeExpr) {
        auto loc = rhs->loc;
        auto newLet = ast::MK::Let(loc, move(assn->rhs), typeExpr->deepCopy());
        assn->rhs = move(newLet);
    }
}

void Initializer::run(core::MutableContext ctx, ast::MethodDef *methodDef, const ast::Expression *prevStat) {
    if (methodDef->name != core::Names::initialize()) {
        return;
    }
    if (prevStat == nullptr) {
        return;
    }
    auto sig = ast::cast_tree_const<ast::Send>(prevStat);
    if (!sig || !isSig(sig)) {
        return;
    }
    auto block = ast::cast_tree<ast::Block>(sig->block.get());
    if (!block) {
        return;
    }

    auto body = ast::cast_tree<ast::Send>(block->body.get());
    if (!body) {
        return;
    }

    auto params = ast::cast_tree<ast::Send>(body->recv.get());
    if (!params || params->fun != core::Names::params() || params->args.size() != 1) {
        return;
    }
    auto argHash = ast::cast_tree<ast::Hash>(params->args.front().get());
    if (!argHash) {
        return;
    }

    UnorderedMap<core::NameRef, ast::Expression *> argTypeMap;
    for (int i = 0; i < argHash->keys.size(); i++) {
        auto argName = ast::cast_tree<ast::Literal>(argHash->keys[i].get());
        auto argVal = argHash->values[i].get();
        if (argName->isSymbol(ctx)) {
            argTypeMap[argName->asSymbol(ctx)] = argVal;
        }
    }

    if (auto stmts = ast::cast_tree<ast::InsSeq>(methodDef->rhs.get())) {
        for (auto &s : stmts->stats) {
            maybeAddLet(ctx, s.get(), argTypeMap);
        }
        maybeAddLet(ctx, stmts->expr.get(), argTypeMap);
    } else {
        maybeAddLet(ctx, methodDef->rhs.get(), argTypeMap);
    }
}

} // namespace sorbet::rewriter
