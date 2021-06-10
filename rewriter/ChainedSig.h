#ifndef SORBET_CHAINED_SIG_H
#define SORBET_CHAINED_SIG_H

#include "ast/ast.h"

namespace sorbet::rewriter {
/**
 * This class desugars the new chained sig syntax into
 * the original sig syntax.
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
