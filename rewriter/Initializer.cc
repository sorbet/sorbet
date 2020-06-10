#include "rewriter/Initializer.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/core.h"
#include "rewriter/Util.h"

using namespace std;
namespace sorbet::rewriter {

namespace {

// We can't actually use a T.type_parameter type in the body of a method, so this prevents us from copying those.
//
// TODO: remove once https://github.com/sorbet/sorbet/issues/1715 is fixed
bool isCopyableType(const ast::TreePtr &typeExpr) {
    auto send = ast::cast_tree_const<ast::Send>(typeExpr);
    if (send && send->fun == core::Names::typeParameter()) {
        return false;
    }
    return true;
}

// if expr is of the form `@var = local`, and `local` is typed, then replace it with with `@var = T.let(local,
// type_of_local)`
void maybeAddLet(core::MutableContext ctx, ast::TreePtr &expr,
                 const UnorderedMap<core::NameRef, const ast::TreePtr *> &argTypeMap) {
    auto assn = ast::cast_tree<ast::Assign>(expr);
    if (assn == nullptr) {
        return;
    }

    auto lhs = ast::cast_tree<ast::UnresolvedIdent>(assn->lhs);
    if (lhs == nullptr || lhs->kind != ast::UnresolvedIdent::Kind::Instance) {
        return;
    }

    auto rhs = ast::cast_tree<ast::UnresolvedIdent>(assn->rhs);
    if (rhs == nullptr || rhs->kind != ast::UnresolvedIdent::Kind::Local) {
        return;
    }

    auto typeExpr = argTypeMap.find(rhs->name);
    if (typeExpr != argTypeMap.end() && isCopyableType(*typeExpr->second)) {
        auto loc = rhs->loc;
        auto newLet = ast::MK::Let(loc, move(assn->rhs), (*typeExpr->second)->deepCopy());
        assn->rhs = move(newLet);
    }
}

// this walks through the chain of sends contained in the body of the `sig` block to find the `params` one, if it
// exists; and otherwise returns a null pointer
const ast::Hash *findParamHash(const ast::Send *send) {
    while (send && send->fun != core::Names::params()) {
        send = ast::cast_tree_const<ast::Send>(send->recv);
    }
    if (!send || send->args.size() != 1) {
        return nullptr;
    }
    return ast::cast_tree_const<ast::Hash>(send->args.front());
}

} // namespace

void Initializer::run(core::MutableContext ctx, ast::MethodDef *methodDef, ast::TreePtr *prevStat) {
    // this should only run in an `initialize` that has a sig
    if (methodDef->name != core::Names::initialize()) {
        return;
    }
    if (prevStat == nullptr) {
        return;
    }
    // make sure that the `sig` block looks like a valid sig block
    auto *sig = ASTUtil::castSig(*prevStat, core::Names::void_());
    if (sig == nullptr) {
        return;
    }
    auto *block = ast::cast_tree_const<ast::Block>(sig->block);
    if (block == nullptr) {
        return;
    }

    // walk through, find the `params()` invocation, and get its hash
    auto *argHash = findParamHash(ast::cast_tree_const<ast::Send>(block->body));
    if (argHash == nullptr) {
        return;
    }

    // build a lookup table that maps from names to the types they have
    UnorderedMap<core::NameRef, const ast::TreePtr *> argTypeMap;
    for (int i = 0; i < argHash->keys.size(); i++) {
        auto *argName = ast::cast_tree_const<ast::Literal>(argHash->keys[i]);
        auto *argVal = &argHash->values[i];
        if (argName->isSymbol(ctx)) {
            argTypeMap[argName->asSymbol(ctx)] = argVal;
        }
    }

    // look through the rhs to find statements of the form `@var = local`
    if (auto stmts = ast::cast_tree<ast::InsSeq>(methodDef->rhs)) {
        for (auto &s : stmts->stats) {
            maybeAddLet(ctx, s, argTypeMap);
        }
        maybeAddLet(ctx, stmts->expr, argTypeMap);
    } else {
        maybeAddLet(ctx, methodDef->rhs, argTypeMap);
    }
}

} // namespace sorbet::rewriter
