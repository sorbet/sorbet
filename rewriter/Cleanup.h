#ifndef SORBET_REWRITER_CLEANUP_H
#define SORBET_REWRITER_CLEANUP_H
#include "ast/ast.h"

namespace sorbet::rewriter {

class Cleanup final {
public:
    static ast::ExpressionPtr run(core::Context ctx, ast::ExpressionPtr tree);

    Cleanup() = delete;
};

} // namespace sorbet::rewriter

#endif
