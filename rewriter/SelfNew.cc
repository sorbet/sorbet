#include "rewriter/SelfNew.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/core.h"

namespace sorbet::rewriter {

std::unique_ptr<ast::Expression> SelfNew::run(core::MutableContext ctx, ast::Send *send) {
    if (send->fun != core::Names::new_()) {
        return nullptr;
    }

    auto *recv = ast::cast_tree<ast::Local>(send->recv.get());
    if (recv == nullptr || recv->localVariable._name != core::Names::selfLocal()) {
        return nullptr;
    }

    // This is unfortunate: the desugar pass adds a `self` node for the receiver
    // if there is an EmptyTree after parsing, and the only way we can tell it
    // wasn't there originally is to test if the Loc is zero-width.
    if (recv->loc.beginPos() == recv->loc.endPos()) {
        return nullptr;
    }

    return ast::MK::Send(send->loc, std::move(send->recv), core::Names::selfNew(),
                         std::move(send->args));
}

} // namespace sorbet::rewriter
