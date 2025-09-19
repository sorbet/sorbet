#ifndef SORBET_REWRITER_REQUIRES_ANCESTOR_KERNEL_H
#define SORBET_REWRITER_REQUIRES_ANCESTOR_KERNEL_H
#include "ast/ast.h"

namespace sorbet::rewriter {

/**
 * This class adds `requires_ancestor { Kernel }` to Module definitions that:
 * 1. Are Module definitions (not Class)
 * 2. Have requires_ancestor feature enabled
 * 3. Don't already have any `requires_ancestor` declarations
 */
class RequiresAncestorKernel final {
public:
    static void run(core::MutableContext ctx, ast::ClassDef *klass);

    RequiresAncestorKernel() = delete;
};

} // namespace sorbet::rewriter

#endif
