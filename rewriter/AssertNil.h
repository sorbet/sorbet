#ifndef SORBET_REWRITER_ASSERT_NIL_H
#define SORBET_REWRITER_ASSERT_NIL_H

#include "ast/ast.h"

namespace sorbet::rewriter {

/**
 * This class converts (assert|assert_not|refute)_nil calls to appropriate T.(cast|must) usage:
 *
 * For assignable expressions:
 *    assert_nil(x)  =>  x = T.cast(x, NilClass)
 *    assert_not_nil(x)  =>  x = T.must(x)
 *    refute_nil(x)  =>  x = T.must(x)
 *
 * For assignable expressions with a message argument:
 *    assert_nil(x, "message")  =>  x = T.cast(x, NilClass)
 *    assert_not_nil(x, "message")  =>  x = T.must(x)
 *    refute_nil(x, "message")  =>  x = T.must(x)
 *
 * For non-assignable expressions it will skip the rewrite.
 *     assert_nil(x.some_method) # stays as is
 *
 */
class AssertNil final {
public:
    static ast::ExpressionPtr run(core::MutableContext ctx, ast::Send *send);
    AssertNil() = delete;
};

} // namespace sorbet::rewriter

#endif /* SORBET_REWRITER_ASSERT_NIL_H */
