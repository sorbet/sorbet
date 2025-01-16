#ifndef SORBET_REWRITER_RBS_SIGNATURES_H
#define SORBET_REWRITER_RBS_SIGNATURES_H
#include "ast/ast.h"

namespace sorbet::rewriter {

/**
 * TODO
 */
class RBSSignatures final {
public:
    static ast::ExpressionPtr run(core::MutableContext ctx, ast::ExpressionPtr tree);

    RBSSignatures() = delete;
};

} // namespace sorbet::rewriter

#endif
