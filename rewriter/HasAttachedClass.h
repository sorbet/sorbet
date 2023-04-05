#ifndef SORBET_REWRITER_HAS_ATTACHED_CLASS_H
#define SORBET_REWRITER_HAS_ATTACHED_CLASS_H
#include "ast/ast.h"

namespace sorbet::rewriter {

/**
 * Converts things like this
 *
 *     module A
 *       has_attached_class!
 *     end
 *
 * into this:
 *
 *     module A
 *       <AttachedClass> = type_member
 *       has_attached_class!
 *     end
 */

class HasAttachedClass final {
public:
    static std::vector<ast::ExpressionPtr> run(core::MutableContext ctx, bool isClass, ast::Send *send);

    HasAttachedClass() = delete;
};

} // namespace sorbet::rewriter

#endif
