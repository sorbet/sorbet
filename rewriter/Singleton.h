#ifndef SORBET_REWRITER_SINGLETON_H
#define SORBET_REWRITER_SINGLETON_H

#include "ast/ast.h"

namespace sorbet::rewriter {

/**
 * Rewrite uses of the `Singleton` module to include the `self.instance` method, with a type signature that gives sorbet
 * more information.
 *
 * ```
 * class Foo
 *   include Singleton
 * end
 * ```
 *
 * Is rewritten to
 *
 * ```
 * class Foo
 *
 *   sig {returns(T.attached_class)}
 *   def self.instance
 *     raise "Not implemented"
 *   end
 *
 *   include Singleton
 * end
 * ```
 */
class Singleton final {
public:
    static void run(core::MutableContext ctx, ast::ClassDef *cdef);

    Singleton() = delete;
};

} // namespace sorbet::rewriter

#endif
