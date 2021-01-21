#ifndef SORBET_REWRITER_FLATTEN_H
#define SORBET_REWRITER_FLATTEN_H
#include "ast/ast.h"

namespace sorbet::rewriter {

class Flatten final {
public:
    static ast::ExpressionPtr run(core::Context ctx, ast::ExpressionPtr tree);

    Flatten() = delete;
};

} // namespace sorbet::rewriter

#endif
