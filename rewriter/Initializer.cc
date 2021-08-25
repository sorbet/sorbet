#include "rewriter/Initializer.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/core.h"
#include "core/errors/rewriter.h"
#include "rewriter/Util.h"
#include <iostream>

using namespace std;
namespace sorbet::rewriter {

namespace {

// We can't actually use a T.type_parameter type in the body of a method, so this prevents us from copying those.
//
// TODO: remove once https://github.com/sorbet/sorbet/issues/1715 is fixed
bool isCopyableType(const ast::ExpressionPtr &typeExpr) {
    auto send = ast::cast_tree<ast::Send>(typeExpr);
    if (send && send->fun == core::Names::typeParameter()) {
        return false;
    }
    return true;
}

// if expr is of the form `@var = local`, and `local` is typed, then replace it with with `@var = T.let(local,
// type_of_local)`
void maybeAddLet(core::MutableContext ctx, ast::ExpressionPtr &expr,
                 const UnorderedMap<core::NameRef, const ast::ExpressionPtr *> &argTypeMap) {
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
        auto newLet = ast::MK::Let(loc, move(assn->rhs), (*typeExpr->second).deepCopy());
        assn->rhs = move(newLet);
    }
}

// this walks through the chain of sends contained in the body of the `sig` block to find the `params` one, if it
// exists; and otherwise returns a null pointer
const ast::Send *findParams(const ast::Send *send) {
    while (send && send->fun != core::Names::params()) {
        send = ast::cast_tree<ast::Send>(send->recv);
    }

    return send;
}

// this function checks if the signature of the initialize method is using returns(Something)
// instead of void and provides an auto-correct option
void checkSigReturnType(core::MutableContext ctx, const ast::Send *send) {
    auto originalSend = send->deepCopy();

    // try to find the invocation to returns
    while (send && send->fun != core::Names::returns()) {
        send = ast::cast_tree<ast::Send>(send->recv);
    }

    // if the returns exists, then add an error an suggest the auto-correct. We need to account for things
    // being invoked after returns too. E.g.: sig { returns(Foo).on_failure(...) }
    if (send != nullptr) {
        if (auto e = ctx.beginError(originalSend.loc(), core::errors::Rewriter::InitializeReturnType)) {
            e.setHeader("The {} method should always return {}", "initialize", "void");

            auto loc = core::Loc(ctx.file, originalSend.loc());
            string original = loc.source(ctx).value();
            auto returnsStart = original.find("returns");
            auto returnsLength = original.find(")", returnsStart) - returnsStart + 1;
            string replacement = original.replace(returnsStart, returnsLength, "void");

            e.addAutocorrect(core::AutocorrectSuggestion{fmt::format("Replace `{}` with `{}`", original, replacement),
                                                         {core::AutocorrectSuggestion::Edit{loc, replacement}}});
        }
    }
}

} // namespace

void Initializer::run(core::MutableContext ctx, ast::MethodDef *methodDef, ast::ExpressionPtr *prevStat) {
    // this should only run in an `initialize` that has a sig
    if (methodDef->name != core::Names::initialize()) {
        return;
    }
    if (prevStat == nullptr) {
        return;
    }
    // make sure that the `sig` block looks like a valid sig block
    auto *sig = ASTUtil::castSig(*prevStat);
    if (sig == nullptr) {
        return;
    }
    auto *block = ast::cast_tree<ast::Block>(sig->block);
    if (block == nullptr) {
        return;
    }

    auto *bodyBlock = ast::cast_tree<ast::Send>(block->body);
    checkSigReturnType(ctx, bodyBlock);

    // walk through, find the `params()` invocation, and get its hash
    auto *params = findParams(bodyBlock);
    if (params == nullptr) {
        return;
    }

    // build a lookup table that maps from names to the types they have
    auto [kwStart, kwEnd] = params->kwArgsRange();
    UnorderedMap<core::NameRef, const ast::ExpressionPtr *> argTypeMap;
    for (int i = kwStart; i < kwEnd; i += 2) {
        auto *argName = ast::cast_tree<ast::Literal>(params->args[i]);
        auto *argVal = &params->args[i + 1];
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
