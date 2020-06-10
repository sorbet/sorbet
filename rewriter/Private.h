#ifndef SORBET_REWRITER_PRIVATE_H
#define SORBET_REWRITER_PRIVATE_H
#include "ast/ast.h"

namespace sorbet::rewriter {

/**
 * This class processes nodes like
 *
 *   private def foo; end
 *   private_class_method def foo; end
 *   private def self.foo; end
 *   private_class_method def self.foo; end
 *
 * And **only** emits errors when there's a mismatch.
 */
class Private final {
public:
    static std::vector<ast::TreePtr> run(core::MutableContext ctx, ast::Send *send);

    Private() = delete;
};

} // namespace sorbet::rewriter

#endif
