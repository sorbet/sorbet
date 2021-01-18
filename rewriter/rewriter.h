#ifndef SORBET_REWRITER_DSL_H
#define SORBET_REWRITER_DSL_H
#include "ast/ast.h"
#include <memory>

namespace sorbet::rewriter {

class Rewriter final {
public:
    static ast::ExpressionPtr run(core::MutableContext ctx, ast::ExpressionPtr tree);

    Rewriter() = delete;
};

} // namespace sorbet::rewriter

#endif
