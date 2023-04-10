#ifndef SORBET_REWRITER_DATA_H
#define SORBET_REWRITER_DATA_H
#include "ast/ast.h"

namespace sorbet::rewriter {

/**
 * This class desugars things of the form
 *
 *   A = Data.define(:foo, :bar)
 *
 * into
 *
 *   class A < Data
 *       def foo; end
 *       def bar; end
 *       sig {params(foo: BasicObject, bar: BasicObject).returns(A)}
 *       def self.new(foo=nil, bar=nil)
 *           T.cast(nil, A)
 *       end
 *   end
 */
class Data final {
public:
    static std::vector<ast::ExpressionPtr> run(core::MutableContext ctx, ast::Assign *asgn);

    Data() = delete;
};

} // namespace sorbet::rewriter

#endif
