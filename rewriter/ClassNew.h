#ifndef SORBET_REWRITER_CLASS_NEW_H
#define SORBET_REWRITER_CLASS_NEW_H
#include "ast/ast.h"

namespace sorbet::rewriter {

/**
 * This class desugars things of the form
 *
 *   A = B::Class.new(Parent) do
 *     ...
 *   end
 *
 * into
 *
 *   class A < Parent
 *     ...
 *   end
 */
class ClassNew final {
public:
    static std::vector<ast::TreePtr> run(core::MutableContext ctx, ast::Assign *asgn);

    ClassNew() = delete;
};

} // namespace sorbet::rewriter

#endif
