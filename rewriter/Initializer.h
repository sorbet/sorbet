#ifndef SORBET_REWRITER_INITIALIZER_H
#define SORBET_REWRITER_INITIALIZER_H
#include "ast/ast.h"

namespace sorbet::rewriter {

/**
 * This class adds T.lets to certain instance variable definitions. Specifically, when we see an initialize method that
 * has a sig and assigns a parameter from the sig to a local variable, like this:
 *
 *   sig {params(x: Integer).void}
 *   def initialize(x)
 *     @y = x
 *
 * we add a `T.let` with the type of the parameter into the body
 *
 *   sig {params(x: Integer).void}
 *   def initialize(x)
 *     @y = T.let(x, Integer)
 *
 * which allows us to get some instance variable types 'for free' even before we have started inference. This only
 * applies to methods named `initialize` with `sig`s of the appropriate shape, and to types that we can currently copy
 * (i.e. we skip `T.type_parameter` types), and can be easily bypassed by having something other than just a single
 * local paramter on its on on the right-hand side. e.g. even something like this
 *
 *   sig {params(x: Integer).void}
 *   def initialize(x)
 *     @y = x + 0
 *
 * is enough to prevent this change from happening.
 */

class Initializer final {
public:
    static void run(core::MutableContext ctx, ast::MethodDef *methodDef, ast::TreePtr *prevStat);

    Initializer() = delete;
};

} // namespace sorbet::rewriter

#endif
