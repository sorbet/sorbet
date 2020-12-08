#ifndef SORBET_REWRITER_VISIBILITY_H
#define SORBET_REWRITER_VISIBILITY_H
#include "ast/ast.h"

namespace sorbet::rewriter {

/**
 * This class is meant to support expressions of visibility
 *
 * It desugars things of the form
 *
 *   private
 *   def foo; end
 *   def bar; end
 *
 * into
 *
 *   def foo; end
 *   def bar; end
 *   private :foo
 *   private :bar
 *
 */
class Visibility final {
public:
    static void run(core::MutableContext ctx, ast::ClassDef *classDef);

    Visibility() = delete;
};

} // namespace sorbet::rewriter

#endif
