#include "rewriter/Initializer.h"
#include "absl/strings/str_split.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/core.h"
#include "core/errors/rewriter.h"
#include "rewriter/Util.h"

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

// if expr is of the form `@var = x`, and `x` is a literal or a typed local,
// then replace it with with `@var = T.let(x, type_of_x)`
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

    if (auto lit = ast::cast_tree<ast::Literal>(assn->rhs)) {
        auto underlying = core::isa_type<core::LiteralType>(lit->value) ? lit->value.underlying(ctx) : lit->value;
        auto klass = core::cast_type_nonnull<core::ClassType>(underlying);
        auto loc = lit->loc;
        auto newLet = ast::MK::Let(loc, move(assn->rhs), ast::MK::Constant(loc, klass.symbol));
        assn->rhs = move(newLet);
    } else if (auto ident = ast::cast_tree<ast::UnresolvedIdent>(assn->rhs)) {
        if (ident->kind != ast::UnresolvedIdent::Kind::Local) {
            return;
        }

        auto typeExpr = argTypeMap.find(ident->name);
        if (typeExpr != argTypeMap.end() && isCopyableType(*typeExpr->second)) {
            auto loc = ident->loc;
            auto newLet = ast::MK::Let(loc, move(assn->rhs), (*typeExpr->second).deepCopy());
            assn->rhs = move(newLet);
        }
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
    string statementAfterReturns = "";

    // try to find the invocation to returns. Save the source code of the invocation
    // immediately after returns() so that we can have the exact length it occupies
    while (send && send->fun != core::Names::returns()) {
        statementAfterReturns = send->fun.toString(ctx);
        send = ast::cast_tree<ast::Send>(send->recv);
    }

    // if the returns exists, then add an error an suggest the auto-correct. We need to account for things
    // being invoked after returns too. E.g.: sig { returns(Foo).on_failure(...) }
    if (send != nullptr) {
        if (auto e = ctx.beginError(originalSend.loc(), core::errors::Rewriter::InitializeReturnType)) {
            e.setHeader("The {} method should always return {}", "initialize", "void");

            auto loc = core::Loc(ctx.file, originalSend.loc());
            auto original = string(loc.source(ctx).value());
            unsigned long returnsStart = original.find("returns");
            unsigned long returnsLength, afterReturnsPosition;
            string replacement;

            // If there are no statements after returns(), we can use the length of the block to find the length
            // we need to replace. If there are statements after it, we need to find the exact length using the next
            // statement and remember to add a dot or else it will produce invalid code
            returnsLength = original.length() - returnsStart + 1;

            if (statementAfterReturns.empty()) {
                replacement = original.replace(returnsStart, returnsLength, "void");
            } else {
                afterReturnsPosition = original.find(statementAfterReturns, returnsStart);

                // If there is a line break between returns() and the next statement, change the returns() entry and
                // re-join the string with the line breaks. Otherwise, everything is on the same line and we can replace
                // directly without worrying about line breaks
                vector<string> lines = absl::StrSplit(original.substr(returnsStart, afterReturnsPosition), "\n");

                if (lines.size() > 1) {
                    lines[0] = "void";
                    replacement = original.replace(returnsStart, returnsLength,
                                                   fmt::format("{}", fmt::join(lines.begin(), lines.end(), "\n")));
                } else {
                    returnsLength = original.find(statementAfterReturns, returnsStart) - returnsStart;
                    replacement = original.replace(returnsStart, returnsLength, "void.");
                }
            }

            e.addAutocorrect(core::AutocorrectSuggestion{fmt::format("Replace `{}` with `{}`", original, replacement),
                                                         {core::AutocorrectSuggestion::Edit{loc, replacement}}});
        }
    }
}

} // namespace

void Initializer::run(core::MutableContext ctx, ast::MethodDef *methodDef, ast::ExpressionPtr *prevStat) {
    // this should only run in an `initialize` that has a sig
    if (methodDef->name != core::Names::initialize() || methodDef->flags.isSelfMethod) {
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
    // build a lookup table that maps from names to the types they have
    UnorderedMap<core::NameRef, const ast::ExpressionPtr *> argTypeMap;
    auto *params = findParams(bodyBlock);
    if (params != nullptr) {
        auto [kwStart, kwEnd] = params->kwArgsRange();
        for (int i = kwStart; i < kwEnd; i += 2) {
            auto *argName = ast::cast_tree<ast::Literal>(params->args[i]);
            auto *argVal = &params->args[i + 1];
            if (argName->isSymbol(ctx)) {
                argTypeMap[argName->asSymbol(ctx)] = argVal;
            }
        }
    }

    // look through the rhs to find statements of the form `@var = local`
    // or `@var = 0` (or any other literal)
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
