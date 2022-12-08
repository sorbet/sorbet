#ifndef SORBET_REWRITER_CONSTANT_T_LET_H
#define SORBET_REWRITER_CONSTANT_T_LET_H
#include "ast/ast.h"

namespace sorbet::rewriter {

/**
 * Rewrites things like this:
 *
 *     X = A::B::C.new
 *
 *  to this:
 *
 *     X = T.let(A::B::C.new, A::B::C)
 *
 * but only in `# typed: true` files, so that we can be sure that the type annotation will be
 * checked for correctness.
 *
 */
class ConstantAssumeType final {
public:
    static void run(core::MutableContext ctx, ast::Assign *asgn);

    ConstantAssumeType() = delete;
};

} // namespace sorbet::rewriter

#endif
