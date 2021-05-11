#ifndef SORBET_REWRITER_DEF_DELEGATOR_H
#define SORBET_REWRITER_DEF_DELEGATOR_H
#include "ast/ast.h"

namespace sorbet::rewriter {

/**
 * This class implements Forwardable#def_delegator and Forwardable#def_delegators
 * by desugaring things of the form
 *
 *    class MyClass
 *      def_delegator :thing, :foo
 *      def_delegator :thing, :bar, :aliased_bar
 *      def_delegators :thing, :baz, :qux
 *    end
 *
 * into
 *
 *    class MyClass
 *      sig {params(arg0: T.untyped, blk: T.nilable(Proc)).returns(T.untyped)}
 *      def foo(*arg0, &blk); end
 *
 *      sig {params(arg0: T.untyped, blk: T.nilable(Proc)).returns(T.untyped)}
 *      def aliased_bar(*arg0, &blk); end
 *
 *      sig {params(arg0: T.untyped, blk: T.nilable(Proc)).returns(T.untyped)}
 *      def baz(*arg0, &blk); end
 *
 *      sig {params(arg0: T.untyped, blk: T.nilable(Proc)).returns(T.untyped)}
 *      def qux(*arg0, &blk); end
 *    end
 */
class DefDelegator final {
public:
    static std::vector<ast::ExpressionPtr> run(core::MutableContext ctx, const ast::Send *send);

    DefDelegator() = delete;
};

} // namespace sorbet::rewriter

#endif
