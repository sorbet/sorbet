#include "rewriter/InterfaceWrapper.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/Context.h"
#include "core/Names.h"
#include "core/core.h"
#include "core/errors/rewriter.h"
#include "rewriter/Util.h"
#include "rewriter/rewriter.h"

using namespace std;

namespace sorbet::rewriter {

namespace {
ast::ExpressionPtr rewriteWrapInstance(core::MutableContext ctx, ast::Send *send) {
    if (!ast::isa_tree<ast::UnresolvedConstantLit>(send->recv)) {
        if (auto e = ctx.beginError(send->recv.loc(), core::errors::Rewriter::BadWrapInstance)) {
            e.setHeader("Unsupported wrap_instance() on a non-constant-literal");
        }
        return nullptr;
    }

    if (send->args.size() != 1) {
        if (auto e = ctx.beginError(send->loc, core::errors::Rewriter::BadWrapInstance)) {
            e.setHeader("Wrong number of arguments to `{}`. Expected: `{}`, got: `{}`", "wrap_instance", 0,
                        send->args.size());
        }
        return nullptr;
    }

    return ast::MK::Let(send->loc, move(send->args.front()), move(send->recv));
}

ast::ExpressionPtr rewriteDynamicCast(core::MutableContext ctx, ast::Send *send, core::NameRef name) {
    auto *cnst = ast::cast_tree<ast::UnresolvedConstantLit>(send->recv);
    if (cnst == nullptr) {
        return nullptr;
    }
    if (cnst->cnst != core::Names::Constants::InterfaceWrapper()) {
        return nullptr;
    }
    auto *scope = ast::cast_tree<ast::UnresolvedConstantLit>(cnst->scope);
    if (scope == nullptr) {
        return nullptr;
    }
    if (scope->cnst != core::Names::Constants::T()) {
        return nullptr;
    }
    if (!ast::MK::isRootScope(scope->scope)) {
        return nullptr;
    }

    if (send->args.size() != 2) {
        return nullptr;
    }

    auto type =
        name == core::Names::dynamicCast() ? ast::MK::Nilable(send->loc, move(send->args[1])) : move(send->args[1]);
    return ast::MK::Cast(send->loc, move(send->args[0]), move(type));
}
} // namespace

ast::ExpressionPtr InterfaceWrapper::run(core::MutableContext ctx, ast::Send *send) {
    if (ctx.state.runningUnderAutogen) {
        return nullptr;
    }

    if (send->fun == core::Names::wrapInstance()) {
        return rewriteWrapInstance(ctx, send);
    }

    if (send->fun == core::Names::dynamicCast() || send->fun == core::Names::nonNilDynamicCast()) {
        return rewriteDynamicCast(ctx, send, send->fun);
    }

    return nullptr;
}
} // namespace sorbet::rewriter
