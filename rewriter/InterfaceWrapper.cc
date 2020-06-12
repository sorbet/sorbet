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
ast::TreePtr InterfaceWrapper::run(core::MutableContext ctx, ast::Send *send) {
    if (ctx.state.runningUnderAutogen) {
        return nullptr;
    }

    if (send->fun != core::Names::wrapInstance()) {
        return nullptr;
    }

    if (!ast::isa_tree<ast::UnresolvedConstantLit>(send->recv)) {
        if (auto e = ctx.beginError(send->recv->loc, core::errors::Rewriter::BadWrapInstance)) {
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
} // namespace sorbet::rewriter
