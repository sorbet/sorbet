#ifndef SORBET_REWRITER_TYPE_ASSERTION_H
#define SORBET_REWRITER_TYPE_ASSERTION_H

#include "ast/ast.h"

namespace sorbet::rewriter {

/**
 * This class converts syntactic type assertions to ast::Cast nodes:
 *
 *    T.let(...)
 *    T.cast(...)
 *    T.bind(...)
 *    T.uncheckedLet(...)
 *    T.assertType(...)
 *    T.cast(...)
 *
 * =>
 *
 */
class TypeAssertion final {
public:
    static ast::ExpressionPtr run(core::MutableContext ctx, ast::Send *send);
    TypeAssertion() = delete;
};

} // namespace sorbet::rewriter

#endif /* SORBET_REWRITER_TYPE_ASSERTION_H */
