#ifndef SORBET_DSL_MATTR_H
#define SORBET_DSL_MATTR_H
#include "ast/ast.h"

namespace sorbet::dsl {

/**
 * This class implements Module#mattr_accessor and similar APIs from ActiveSupport.
 * Things of the following form
 *
 *    class MyClass
 *      cattr_accessor :foo
 *    end
 *
 * are desugared into
 *
 *    class MyClass
 *      sig {returns(T.untyped)}
 *      def foo; end
 *
 *      sig {params(arg0: T.untyped).void}
 *      def foo=(arg0); end
 *
 *      sig {returns(T.untyped)}
 *      def self.foo; end
 *
 *      sig {params(arg0: T.untyped).void}
 *      def self.foo=(arg0); end
 *    end
 */
class Mattr final {
public:
    static std::vector<std::unique_ptr<ast::Expression>> replaceDSL(core::MutableContext ctx, const ast::Send *send,
                                                                    ast::ClassDefKind classDefKind);

    Mattr() = delete;
};

} // namespace sorbet::dsl

#endif
