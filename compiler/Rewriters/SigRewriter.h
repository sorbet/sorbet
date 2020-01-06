#ifndef SORBET_COMPILER_SIG_REWRITER_H
#define SORBET_COMPILER_SIG_REWRITER_H

#include "compiler/Core/ForwardDeclarations.h"

namespace sorbet::compiler {
class SigRewriter {
public:
    static void run(core::MutableContext &ctx, ast::ClassDef *klass);
};
} // namespace sorbet::compiler
#endif
