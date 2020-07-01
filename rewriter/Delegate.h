#ifndef SORBET_REWRITER_DELEGATE_H
#define SORBET_REWRITER_DELEGATE_H
#include "ast/ast.h"

namespace sorbet::rewriter {

/**
 * This class implements Module#delegate from ActiveSupport
 * by desugaring things of the form
 *
 *    class MyClass
 *      delegate :foo, :bar, to: 'thing'
 *    end
 *
 * into
 *
 *    class MyClass
 *      sig {params(arg0: T.untyped).returns(T.untyped)}
 *      def foo(*arg0); end
 *
 *      sig {params(arg0: T.untyped).returns(T.untyped)}
 *      def bar(*arg0); end
 *    end
 */
class Delegate final {
public:
    static std::vector<ast::TreePtr> run(core::MutableContext ctx, const ast::Send *send);

    Delegate() = delete;
};

} // namespace sorbet::rewriter

#endif
