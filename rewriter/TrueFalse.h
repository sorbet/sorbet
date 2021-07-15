#ifndef SORBET_REWRITER_TRUE_FALSE_H
#define SORBET_REWRITER_TRUE_FALSE_H

#include "ast/ast.h"

namespace sorbet::rewriter {

/**
 * Converts code like this:
 *
 *     x = true
 *     y = false
 *
 * into this:
 *
 *     x = <Magic>.<booleanTrue>
 *     y = <Magic>.<booleanFalse>
 *
 * This allows updating the value of `x` and `y` inside a loop, because the type is the same
 * (`T::Boolean`) inside and outside the loop.
 */
class TrueFalse final {
public:
    static ast::ExpressionPtr run(core::MutableContext ctx, ast::Assign *asgn);
    TrueFalse() = delete;
};

} // namespace sorbet::rewriter

#endif /* SORBET_REWRITER_TRUE_FALSE_H */
