#ifndef SORBET_REWRITER_CLASS_NEW_H
#define SORBET_REWRITER_CLASS_NEW_H
#include "ast/ast.h"

namespace sorbet::rewriter {

/**
 * This class actually contains three different rewriters depending on the kind of `Class.new` we encounter:
 *
 * Assignments to a constant literal such as
 *
 *   A = Class.new(Parent) do
 *     ...
 *   end
 *
 * Are rewritten into
 *
 *   class A < Parent
 *     ...
 *   end
 *
 * Assignments to a non-constant literal such as
 *
 *   c = Class.new(Parent) do
 *     ...
 *   end
 *
 * Are rewritten into
 *
 *   c = Class.new(Parent)
 *     T.bind(self, T.class_of(Parent))
 *     ...
 *   end
 *
 * And sends such as
 *
 *   Class.new(Parent) do
 *     ...
 *   end
 *
 * Are rewritten into
 *
 *   Class.new(Parent)
 *     T.bind(self, T.class_of(Parent))
 *     ...
 *   end
 */
class ClassNew final {
public:
    static std::vector<ast::ExpressionPtr> run(core::MutableContext ctx, ast::Assign *asgn);
    static bool run(core::MutableContext ctx, ast::Send *send);

    ClassNew() = delete;
};

} // namespace sorbet::rewriter

#endif
