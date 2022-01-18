#ifndef SORBET_CHAINED_SIG_H
#define SORBET_CHAINED_SIG_H

#include "ast/ast.h"

namespace sorbet::rewriter {
/**
 * This class desugars the new chained sig syntax into the original sig syntax. We've chosen this approach for backwards
 * compatibility: while we're transitioning to the new syntax, we have to be able to parse both, and pigeonholing the
 * new syntax into the old syntax lets us reuse more code.
 *
 * Eventually it will make sense to permanently remove the old syntax from Sorbet after a suitable migration period, at
 * which point this pass may no longer be required.
 *
 * sig.abstract { void }
 * sig.final { void }
 *
 * into
 *
 * sig { abstract.void }
 * sig { final.void }
 *
 */
class ChainedSig {
public:
    static ast::ExpressionPtr run(core::MutableContext &ctx, ast::ExpressionPtr tree);
};
} // namespace sorbet::rewriter
#endif
