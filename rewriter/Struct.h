#ifndef SORBET_REWRITER_STRUCT_H
#define SORBET_REWRITER_STRUCT_H
#include "ast/ast.h"

namespace sorbet::rewriter {

/**
 * This class desugars things of the form
 *
 *   A = Struct.new(:foo, :bar) do
 *       # ... body ...
 *   end
 *
 * into
 *
 *   class A$1 < Struct
 *       sig {returns(T.untyped)}
 *       def foo; end
 *
 *       sig {params(arg: T.untyped).returns(T.untyped)}
 *       def foo=(arg); arg; end
 *
 *       sig {returns(T.untyped)}
 *       def bar; end
 *
 *       sig {params(arg: T.untyped).returns(T.untyped)}
 *       def bar=(arg); arg; end
 *
 *       sig {params(foo: BasicObject, bar: BasicObject).returns(A)}
 *       def initialize(foo=nil, bar=nil); end
 *
 *       Elem = type_member {{fixed: T.untyped}}
 *   end
 *
 *   class A < A$1
 *       Elem = type_member {{fixed: T.untyped}}
 *
 *       # ... body ...
 *   end
 *
 *   The fake A$1 class mimics how `Struct.new` works at runtime, and allows doing things like
 *   overriding methods inside `# ... body ...` instead of redefining them (which means `super` will
 *   simply dispatch the the original method, rather than having to use something like
 *   `alias_method` to get a reference to the original method before redefinition.)
 */
class Struct final {
public:
    static std::vector<ast::ExpressionPtr> run(core::MutableContext ctx, ast::Assign *asgn);

    Struct() = delete;
};

} // namespace sorbet::rewriter

#endif
