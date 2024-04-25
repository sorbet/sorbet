#include "rewriter/Command.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/Context.h"
#include "core/Names.h"
#include "core/core.h"
#include "rewriter/rewriter.h"

using namespace std;

namespace sorbet::rewriter {

bool isCommand(core::MutableContext ctx, ast::ClassDef *klass) {
    if (klass->kind != ast::ClassDef::Kind::Class || klass->ancestors.empty()) {
        return false;
    }
    auto *cnst = ast::cast_tree<ast::UnresolvedConstantLit>(klass->ancestors.front());
    if (cnst == nullptr) {
        return false;
    }
    if (cnst->cnst != core::Names::Constants::Command()) {
        return false;
    }
    auto *scope = ast::cast_tree<ast::UnresolvedConstantLit>(cnst->scope);
    if (scope == nullptr) {
        return false;
    }
    if (scope->cnst != core::Names::Constants::Opus()) {
        return false;
    }
    return ast::MK::isRootScope(scope->scope);
}

void Command::run(core::MutableContext ctx, ast::ClassDef *klass) {
    if (ctx.state.runningUnderAutogen) {
        return;
    }

    if (!isCommand(ctx, klass)) {
        return;
    }

    int i = 0;
    ast::MethodDef *call = nullptr;
    ast::ExpressionPtr *callptr = nullptr;

    for (auto &stat : klass->rhs) {
        auto *mdef = ast::cast_tree<ast::MethodDef>(stat);
        if (mdef == nullptr) {
            continue;
        }
        if (mdef->name != core::Names::call()) {
            continue;
        }

        i = &stat - &klass->rhs.front();
        call = mdef;
        callptr = &stat;
        break;
    }
    // If we didn't find a `call` method, or if it was the first statement (and
    // thus couldn't have a `sig`)
    if (call == nullptr || i == 0) {
        return;
    }

    // Heuristic: Does the previous node look like a `sig`? Check that it's a
    // Send node and so is its receiver.
    //
    // This could in principle be `resolver::TypeSyntax::isSig`, but we don't
    // want to depend on the internals of the resolver, or accidentally rely on
    // passes that happen between here and the resolver.
    auto *sig = ast::cast_tree<ast::Send>(klass->rhs[i - 1]);
    if (sig == nullptr || sig->fun != core::Names::sig()) {
        return;
    }

    ast::MethodDef::ARGS_store newArgs;
    newArgs.reserve(call->args.size());
    for (auto &arg : call->args) {
        newArgs.emplace_back(arg.deepCopy());
    }

    // This method is only for type checking. It doesn't actually exist at runtime, and instead all
    // Opus::Command subclasses inherit the `self.call` method from `Opus::Command` directly.
    ast::MethodDef::Flags flags;
    flags.isSelfMethod = true;
    flags.discardDef = true;
    auto selfCall = ast::MK::SyntheticMethod(call->loc, call->declLoc, call->name, std::move(newArgs),
                                             ast::MK::RaiseTypedUnimplemented(call->declLoc), flags);

    // We are now in the weird situation where we have an actual method that
    // the user has written, but we have a synthetic method that lives at the
    // same location.  If we try to jump-to-def from the actual method, there
    // are no calls to it, which will frustrate the user.  Erase the location(s)
    // on the non-synthetic method so that LSP only sees the synthetic method.
    auto hiddenCall = ast::MK::Method(call->loc.copyWithZeroLength(),
                                      call->declLoc.copyWithZeroLength(),
                                      call->name,
                                      std::move(call->args), std::move(call->rhs),
                                      call->flags);

    // We need to make sure we assign into `callptr` prior to inserting into
    // `klass->rhs`, otherwise our pointer might not be live anymore.
    *callptr = std::move(hiddenCall);

    klass->rhs.insert(klass->rhs.begin() + i + 1, sig->deepCopy());
    klass->rhs.insert(klass->rhs.begin() + i + 2, std::move(selfCall));
}

}; // namespace sorbet::rewriter
