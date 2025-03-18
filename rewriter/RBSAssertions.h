#ifndef SORBET_REWRITER_RBS_ASSERTIONS_H
#define SORBET_REWRITER_RBS_ASSERTIONS_H
#include "ast/ast.h"

namespace sorbet::rewriter {

/**
 * This class rewrites RBS type assertions into Sorbet `T.let` calls.
 *
 * So this:
 *
 *     x = foo #: Integer
 *
 * Will be rewritten to:
 *
 *     x = T.let(foo, Integer)
 */
class RBSAssertions final {
public:
    static void run(core::MutableContext ctx, ast::ClassDef *classDef);
};

} // namespace sorbet::rewriter

#endif
