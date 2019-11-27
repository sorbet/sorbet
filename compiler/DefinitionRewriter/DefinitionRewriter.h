#ifndef SORBET_COMPILER_DEFINITON_REWRITER_H
#define SORBET_COMPILER_DEFINITON_REWRITER_H

#include "compiler/Core/ForwardDeclarations.h"

namespace sorbet::compiler {
class DefinitionRewriter {
public:
    static void run(core::MutableContext &ctx, ast::ClassDef *klass);
};
} // namespace sorbet::compiler
#endif
