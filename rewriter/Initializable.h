#ifndef SORBET_REWRITER_INITIALIZABLE_H
#define SORBET_REWRITER_INITIALIZABLE_H
#include "ast/ast.h"

namespace sorbet::rewriter {

/**
 * Converts things like this
 *
 *     module A
 *       initializable!
 *     end
 *
 * into this:
 *
 *     module A
 *       <AttachedClass> = type_member
 *       initializable!
 *     end
 */

class Initializable final {
public:
    static std::vector<ast::ExpressionPtr> run(core::MutableContext ctx, bool isClass, ast::Send *send);

    Initializable() = delete;
};

} // namespace sorbet::rewriter

#endif
