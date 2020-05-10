#ifndef SORBET_REWRITER_STRUCT_H
#define SORBET_REWRITER_STRUCT_H
#include "ast/ast.h"

namespace sorbet::rewriter {

/**
 * This class desugars things of the form
 *
 *   A = Struct.new(:foo, :bar)
 *
 * into
 *
 *   class A < Struct
 *       def foo; end
 *       def foo=(arg); arg; end
 *       def bar; end
 *       def bar=(arg); arg; end
 *       sig {params(foo: BasicObject, bar: BasicObject).returns(A)}
 *       def self.new(foo=nil, bar=nil)
 *           T.cast(nil, A)
 *       end
 *   end
 */
class Struct final {
public:
    static std::vector<ast::TreePtr> run(core::MutableContext ctx, ast::Assign *asgn);

    Struct() = delete;
};

} // namespace sorbet::rewriter

#endif
