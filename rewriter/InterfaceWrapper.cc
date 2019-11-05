#include "rewriter/InterfaceWrapper.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/Context.h"
#include "core/Names.h"
#include "core/core.h"
#include "core/errors/rewriter.h"
#include "rewriter/rewriter.h"
#include "rewriter/util.h"

using namespace std;

namespace sorbet::rewriter {
unique_ptr<ast::Expression> InterfaceWrapper::run(core::MutableContext ctx, unique_ptr<ast::Send> send) {
    if (ctx.state.runningUnderAutogen) {
        return send;
    }

    if (send->fun != core::Names::wrapInstance()) {
        return send;
    }

    if (!ast::isa_tree<ast::UnresolvedConstantLit>(send->recv.get())) {
        if (auto e = ctx.state.beginError(send->recv->loc, core::errors::Rewriter::BadWrapInstance)) {
            e.setHeader("Unsupported wrap_instance() on a non-constant-literal");
        }
        return send;
    }

    if (send->args.size() != 1) {
        if (auto e = ctx.state.beginError(send->loc, core::errors::Rewriter::BadWrapInstance)) {
            e.setHeader("Wrong number of arguments to `{}`. Expected: `{}`, got: `{}`", "wrap_instance", 0,
                        send->args.size());
        }
        return send;
    }

    return ast::MK::Let(send->loc, move(send->args.front()), move(send->recv));
}
} // namespace sorbet::rewriter
