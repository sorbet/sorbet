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
    ast::MethodDef *call;

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
    auto selfCall = ast::MK::SyntheticMethod(call->loc, call->loc, call->name, std::move(newArgs),
                                             ast::MK::RaiseTypedUnimplemented(call->declLoc), flags);

    klass->rhs.insert(klass->rhs.begin() + i + 1, sig->deepCopy());
    klass->rhs.insert(klass->rhs.begin() + i + 2, std::move(selfCall));
}

}; // namespace sorbet::rewriter
