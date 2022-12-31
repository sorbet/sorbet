#include "rewriter/ConstantAssumeType.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/Names.h"
#include "core/core.h"

using namespace std;

namespace sorbet::rewriter {

void ConstantAssumeType::run(core::MutableContext ctx, ast::Assign *asgn) {
    if (ctx.state.runningUnderAutogen) {
        return;
    }

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

    if (send->fun != core::Names::new_()) {
        return;
    }

    if (!(ast::isa_tree<ast::UnresolvedConstantLit>(send->recv) || ast::isa_tree<ast::ConstantLit>(send->recv))) {
        return;
    }

    auto type = send->recv.deepCopy();
    asgn->rhs = ast::MK::AssumeType(asgn->rhs.loc(), move(asgn->rhs), move(type));
}

}; // namespace sorbet::rewriter
