#include "rewriter/ConstantAssumeType.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/Names.h"
#include "core/core.h"

using namespace std;

namespace sorbet::rewriter {

void ConstantAssumeType::run(core::MutableContext ctx, ast::Assign *asgn, bool isRoot) {
    if (ctx.state.cacheSensitiveOptions.runningUnderAutogen) {
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

    // Allow `A.new.freeze` to be treated the same as `A.new`
    if (send->fun == core::Names::freeze() && !send->hasNonBlockArgs() && !send->hasBlock()) {
        send = ast::cast_tree<ast::Send>(send->recv);
        if (send == nullptr) {
            return;
        }
    }

    if (send->fun != core::Names::new_()) {
        return;
    }

    ast::ExpressionPtr type;
    if (send->recv.isSelfReference()) {
        if (isRoot) {
            return; // Don't try to cast top level `X = new` to `<root>`
        }

        // For `X = new` (or `X = self.new`), the inferred type is the self type. The resolver will
        // replace this with the (then-resolved) enclosing class.
        type = ast::MK::Self(send->recv.loc());
    } else if (ast::isa_tree<ast::UnresolvedConstantLit>(send->recv) || ast::isa_tree<ast::ConstantLit>(send->recv)) {
        type = send->recv.deepCopy();
    } else {
        return;
    }

    asgn->rhs = ast::MK::AssumeType(asgn->rhs.loc(), move(asgn->rhs), move(type));
}

}; // namespace sorbet::rewriter
