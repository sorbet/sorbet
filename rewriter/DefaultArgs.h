#ifndef SORBET_REWRITER_DEFAULT_ARGS_H
#define SORBET_REWRITER_DEFAULT_ARGS_H
#include "ast/ast.h"

namespace sorbet::rewriter {

/**
 * This class desugars things of the form
 *
 *   sig {params(arg0: String, arg1: Integer).void}
 *   def foo(arg0, arg1 = my_expr)
 *   end
 *
 * into
 *
 *   sig {params(arg0: String, arg1: Integer).returns(Integer)}
 *   def foo<defaultArg>1(arg0, arg1)
 *       my_expr
 *   end
 *   sig {params(arg0: String, arg1: Integer).void}
 *   def foo(arg0, arg1 = foo<defaultArg>1(arg0, arg1))
 *   end
 */
class DefaultArgs final {
public:
    static ast::TreePtr run(core::MutableContext ctx, ast::TreePtr klass);

    DefaultArgs() = delete;
};

} // namespace sorbet::rewriter

#endif
