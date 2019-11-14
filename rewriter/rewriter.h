#ifndef SORBET_REWRITER_DSL_H
#define SORBET_REWRITER_DSL_H
#include "ast/ast.h"
#include <memory>

namespace sorbet::rewriter {

class Rewriter final {
public:
    static std::unique_ptr<ast::Expression> run(core::MutableContext ctx, std::unique_ptr<ast::Expression> tree);

    Rewriter() = delete;
};

} // namespace sorbet::rewriter

#endif
