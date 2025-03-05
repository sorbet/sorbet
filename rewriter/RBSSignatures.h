#ifndef SORBET_REWRITER_RBS_SIGNATURES_H
#define SORBET_REWRITER_RBS_SIGNATURES_H
#include "ast/ast.h"

namespace sorbet::rewriter {

/**
 * This class rewrites RBS signatures comments into Sorbet signatures.
 *
 * So this:
 *
 *     #: (Integer) -> String
 *     def foo(x); end
 *
 * Will be rewritten to:
 *
 *     sig { params(x: Integer).returns(String) }
 *     def foo(x); end
 */
class RBSSignatures final {
public:
    static ast::ExpressionPtr run(core::MutableContext ctx, ast::ExpressionPtr tree);

    RBSSignatures() = delete;
};

} // namespace sorbet::rewriter

#endif
