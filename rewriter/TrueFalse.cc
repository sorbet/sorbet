#include "rewriter/TrueFalse.h"

#include "ast/Helpers.h"

namespace sorbet::rewriter {

ast::ExpressionPtr TrueFalse::run(core::MutableContext ctx, ast::Assign *asgn) {
    auto *rhs = ast::cast_tree<ast::Literal>(asgn->rhs);
    if (rhs == nullptr || !(rhs->isTrue(ctx) || rhs->isFalse(ctx))) {
        return nullptr;
    }

    return ast::MK::Assign(asgn->loc, std::move(asgn->lhs), ast::MK::Boolean(rhs->loc, rhs->isTrue(ctx)));
    return nullptr;
}

} // namespace sorbet::rewriter
