#ifndef SORBET_REWRITER_CLASS_EVAL_H
#define SORBET_REWRITER_CLASS_EVAL_H
#include "ast/ast.h"

namespace sorbet::rewriter {

/**
 * This rewriter handles top-level `class_eval`/`class_exec` calls whose receiver is a
 * constant literal and whose block defines methods. The method-defining statements (defs,
 * visibility-wrapped defs like `private def`, their sigs, and bare visibility modifiers)
 * are hoisted out of the block into a synthesized reopening of the receiver, while every
 * other statement stays in the block untouched. For example
 *
 *   x = true
 *   Foo.class_eval do
 *     sig {void}
 *     def bar; end
 *     if x; raise; end
 *   end
 *
 * Is rewritten into
 *
 *   x = true
 *   Foo.class_eval do
 *     if x; raise; end
 *   end
 *   class Foo
 *     sig {void}
 *     def bar; end
 *   end
 *
 * so that the methods defined inside the block are owned by `Foo` (with normal method
 * visibility) instead of being hoisted to top-level `Object` and marked implicitly private,
 * while the rest of the block keeps its closure semantics (locals captured from the
 * enclosing scope keep working). Moving the defs out cannot sever any local variable
 * capture, because `def` is a scope gate even inside `class_eval` blocks in Ruby.
 * See https://github.com/sorbet/sorbet/issues/10452 and
 * https://github.com/sorbet/sorbet/issues/10436
 *
 * The rewrite is deliberately narrow. It only applies when all of these hold:
 *
 * - the call is a top-level statement (not nested inside a class, module, method, or block),
 *   so that constant lookup of the receiver and of `class Foo` agree
 * - the receiver is a constant literal (`Foo`, `A::B`); dynamic receivers
 *   (`klass.class_eval`) keep their current behavior
 * - a literal block is present (which rules out the string form of `class_eval`)
 * - the block body defines at least one method; blocks that don't define methods — the
 *   common incidental uses on receivers that may be modules, like
 *   `Enumerable.class_eval { |mod| puts mod }` — keep their current behavior
 *
 * Known limitations of the prototype:
 *
 * - `M.class_eval` where `M` is a module and the block defines methods synthesizes
 *   `class M`, which Sorbet reports as a class/module redefinition mismatch
 * - a misspelled receiver defines a new class instead of reporting an unresolved constant
 * - constants inside the hoisted defs resolve in the lexical scope of `class Foo`, not the
 *   scope at the call site (these differ once `Foo` itself defines constants with the same
 *   name)
 */
class ClassEval final {
public:
    static std::vector<ast::ExpressionPtr> run(core::MutableContext ctx, ast::Send *send);

    ClassEval() = delete;
};

} // namespace sorbet::rewriter

#endif
