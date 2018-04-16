#include "dsl/InterfaceWrapper.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/Context.h"
#include "core/Names/dsl.h"
#include "core/core.h"
#include "core/errors/dsl.h"
#include "dsl/dsl.h"
#include "dsl/util.h"

using namespace std;

namespace ruby_typer {
namespace dsl {
std::unique_ptr<ast::Expression> InterfaceWrapper::replaceDSL(core::MutableContext ctx,
                                                              std::unique_ptr<ast::Send> send) {
    if (send->fun != core::Names::wrapInstance()) {
        return send;
    }

    if (!ast::isa_tree<ast::ConstantLit>(send->recv.get())) {
        if (auto e = ctx.state.beginError(send->recv->loc, core::errors::DSL::BadWrapInstance)) {
            e.setHeader("Unsupported wrap_instance() on a non-constant-literal");
        }
        return send;
    }

    if (send->args.size() != 1) {
        if (auto e = ctx.state.beginError(send->loc, core::errors::DSL::BadWrapInstance)) {
            e.setHeader("Wrong number of arguments to wrap_instance(): expected 0, got {}", send->args.size());
        }
        return send;
    }

    return ast::MK::Let(send->loc, move(send->args.front()), move(send->recv));
}
} // namespace dsl
} // namespace ruby_typer
