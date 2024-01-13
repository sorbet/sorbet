#ifndef SORBET_REWRITER_MODULE_FUNCTION_H
#define SORBET_REWRITER_MODULE_FUNCTION_H
#include "ast/ast.h"

namespace sorbet::rewriter {

/**
 * This class desugars `module_function`, which has several ways it can be used. If it is used with a method definition
 * as its argument, then it will desugar
 *
 *   module_function def foo(x, y, ...); end
 *
 * into
 *
 *   private def foo(x, y, ...); end
 *   def self.foo(x, y, ...); end
 *
 * possibly replicating the sig for the method if possible. If it is used with a string or symbol, then it instead
 * desugars into an untyped empty method for the purposes of forwarding, so
 *
 *   module function def :foo
 *
 * becomes
 *
 *   private def foo(*args, **kwargs); end
 *   def self.foo(*args, **kwargs); end
 *
 * finally, you can use module_function on its own, in which case it will do the above-described rewrite to every
 * subsequent method definition in the class
 */
class ModuleFunction final {
public:
    static void run(core::MutableContext ctx, ast::ClassDef *cdef);
    ModuleFunction() = delete;

private:
    static std::vector<ast::ExpressionPtr> run(core::MutableContext ctx, ast::Send *send,
                                               const ast::ExpressionPtr *prevStat);

    static std::vector<ast::ExpressionPtr> rewriteDefn(core::MutableContext ctx, const ast::ExpressionPtr &expr,
                                                       const ast::ExpressionPtr *prevStat);
};

} // namespace sorbet::rewriter

#endif
