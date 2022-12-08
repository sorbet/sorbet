#include "rewriter/ConstantTLet.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/Names.h"
#include "core/core.h"

using namespace std;

namespace sorbet::rewriter {

void ConstantTLet::run(core::MutableContext ctx, ast::Assign *asgn) {
    if (ctx.file.data(ctx).strictLevel <= core::StrictLevel::False) {
        // Only do this transformation in files that are typed: true or higher, so that we know that
        // if this assumption about the type is wrong, that it will get checked down the line.
        return;
    }
    auto lhs = ast::cast_tree<ast::UnresolvedConstantLit>(asgn->lhs);
    if (lhs == nullptr) {
        return;
    }

    auto send = ast::cast_tree<ast::Send>(asgn->rhs);
    if (send == nullptr) {
        return;
    }

    auto recv = ast::cast_tree<ast::UnresolvedConstantLit>(send->recv);
    if (recv == nullptr) {
        // TODO(jez) No real reason to preclude ConstantLit here except laziness
        return;
    }

    if (send->fun != core::Names::new_()) {
        return;
    }

    auto type = recv->deepCopy();
    asgn->rhs = ast::MK::Let(asgn->rhs.loc(), move(asgn->rhs), move(type));
}

}; // namespace sorbet::rewriter
