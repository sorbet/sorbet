#ifndef SORBET_REWRITER_CLEANUP_H
#define SORBET_REWRITER_CLEANUP_H
#include "ast/ast.h"

namespace sorbet::rewriter {

class Cleanup final {
public:
    static ast::TreePtr run(core::Context ctx, ast::TreePtr tree);

    Cleanup() = delete;
};

} // namespace sorbet::rewriter

#endif
