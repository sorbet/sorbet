#ifndef SORBET_SIG_REWRITER_H
#define SORBET_SIG_REWRITER_H

#include "ast/ast.h"

namespace sorbet::rewriter {
class SigRewriter {
public:
    static void run(core::MutableContext &ctx, ast::ClassDef *klass);
};
} // namespace sorbet::rewriter
#endif
