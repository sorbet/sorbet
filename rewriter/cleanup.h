#ifndef SORBET_REWRITER_CLEANUP_H
#define SORBET_REWRITER_CLEANUP_H
#include "ast/ast.h"

namespace sorbet::rewriter {

class Cleanup final {
public:
    static std::unique_ptr<ast::Expression> run(core::Context ctx, std::unique_ptr<ast::Expression> tree);

    Cleanup() = delete;
};

} // namespace sorbet::rewriter

#endif
