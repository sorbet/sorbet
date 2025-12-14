#ifndef SORBET_SIG_REWRITER_H
#define SORBET_SIG_REWRITER_H

#include "ast/ast.h"

namespace sorbet::rewriter {
class SigRewriter {
public:
    static bool run(core::MutableContext ctx, ast::Send *send);
};
} // namespace sorbet::rewriter
#endif
