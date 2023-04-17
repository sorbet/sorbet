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

enum class ArgKind {
    Plain,
    RestArg,
    KeywordRestArg,
};

// Type parameters are only scoped to the body of a method.
//
// But an instance variable is scoped to the entire class, so it's not allowed to use type
// parameters as instance variable annotations.
bool isCopyableType(const ast::ExpressionPtr &typeExpr) {
    auto send = ast::cast_tree<ast::Send>(typeExpr);
    if (send && send->fun == core::Names::typeParameter()) {
        return false;
    }
    return true;
}

// Not checking for being T.proc, it's expected to be checked
void maybeRemoveBind(core::Context ctx, ast::Send *send) {
    auto lastSend = send;
    while (send != nullptr) {
        if (send->fun == core::Names::bind()) {
            // `bind` is the last send in the chain of sends.
            // This is incorrect syntax which is covered by the resolver.
            // Safe to ignore.
            if (send == lastSend) {
                return;
            }
            auto recvSend = ast::cast_tree<ast::Send>(send->recv);
            if (recvSend != nullptr) {
                lastSend->recv = move(send->recv);
                return;
            }
            // We handle only one `.bind` call in the whole chain,
            // no need to traverse the rest of the tree.
            return;
        }
        send = ast::cast_tree<ast::Send>(send->recv);
    }
}

// if expr is of the form `@var = local`, and `local` is typed, then replace it with with `@var = T.let(local,
// type_of_local)`
void maybeAddLet(core::MutableContext ctx, ast::ExpressionPtr &expr,
                 const UnorderedMap<core::NameRef, const ast::ExpressionPtr *> &argTypeMap,
                 const UnorderedMap<core::NameRef, ArgKind> &argKindMap) {
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
        auto type = (*typeExpr->second).deepCopy();
        auto it = argKindMap.find(rhs->name);
        if (it == argKindMap.end()) {
            return;
        }
        switch (it->second) {
            case ArgKind::Plain:
                break;
            case ArgKind::RestArg: {
                auto loc = type.loc();
                auto zloc = loc.copyWithZeroLength();
                type = ast::MK::Send1(loc, ast::MK::Constant(zloc, core::Symbols::T_Array()),
                                      core::Names::squareBrackets(), zloc, move(type));
                break;
            }
            case ArgKind::KeywordRestArg: {
                auto loc = type.loc();
                auto zloc = loc.copyWithZeroLength();
                type =
                    ast::MK::Send2(loc, ast::MK::Constant(zloc, core::Symbols::T_Hash()), core::Names::squareBrackets(),
                                   zloc, ast::MK::Constant(zloc, core::Symbols::Symbol()), move(type));
                break;
            }
        }

        auto send = ast::cast_tree<ast::Send>(type);
        if (send != nullptr) {
            maybeRemoveBind(ctx, send);
        }

        auto newLet = ast::MK::Let(loc, move(assn->rhs), move(type));
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
    auto originalSendLoc = send->loc;
    core::NameRef funAfterReturns;

    // try to find the invocation to returns. Save the source code of the invocation
    // immediately after returns() so that we can have the exact length it occupies
    while (send && send->fun != core::Names::returns()) {
        funAfterReturns = send->fun;
        send = ast::cast_tree<ast::Send>(send->recv);
    }

    // if the returns exists, then add an error an suggest the auto-correct. We need to account for things
    // being invoked after returns too. E.g.: sig { returns(Foo).on_failure(...) }
    if (send == nullptr) {
        return;
    }

    if (auto e = ctx.beginError(originalSendLoc, core::errors::Rewriter::InitializeReturnType)) {
        e.setHeader("The {} method should always return {}", "initialize", "void");

        auto loc = core::Loc(ctx.file, originalSendLoc);
        auto original = string(loc.source(ctx).value());
        unsigned long returnsStart = original.find("returns");
        unsigned long returnsLength, afterReturnsPosition;
        string replacement;
        string statementAfterReturns = "";
        if (funAfterReturns.exists()) {
            statementAfterReturns = funAfterReturns.toString(ctx);
        }

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

    auto *block = sig->block();
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
    auto numKwArgs = params->numKwArgs();
    UnorderedMap<core::NameRef, const ast::ExpressionPtr *> argTypeMap;
    for (int i = 0; i < numKwArgs; ++i) {
        auto *argName = ast::cast_tree<ast::Literal>(params->getKwKey(i));
        auto *argVal = &params->getKwValue(i);
        if (argName->isSymbol()) {
            argTypeMap[argName->asSymbol()] = argVal;
        }
    }

    UnorderedMap<core::NameRef, ArgKind> argKindMap;
    for (const auto &arg : methodDef->args) {
        const auto *restArg = ast::cast_tree<ast::RestArg>(arg);
        if (restArg == nullptr) {
            argKindMap[ast::MK::arg2Name(arg)] = ArgKind::Plain;
        } else if (const auto *kwRestArg = ast::cast_tree<ast::KeywordArg>(restArg->expr)) {
            argKindMap[ast::MK::arg2Name(arg)] = ArgKind::KeywordRestArg;
        } else {
            ENFORCE(ast::isa_tree<ast::UnresolvedIdent>(restArg->expr));
            argKindMap[ast::MK::arg2Name(arg)] = ArgKind::RestArg;
        }
    }

    // look through the rhs to find statements of the form `@var = local`
    if (auto stmts = ast::cast_tree<ast::InsSeq>(methodDef->rhs)) {
        for (auto &s : stmts->stats) {
            maybeAddLet(ctx, s, argTypeMap, argKindMap);
        }
        maybeAddLet(ctx, stmts->expr, argTypeMap, argKindMap);
    } else {
        maybeAddLet(ctx, methodDef->rhs, argTypeMap, argKindMap);
    }
}

} // namespace sorbet::rewriter
