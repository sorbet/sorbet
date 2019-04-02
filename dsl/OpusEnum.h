#ifndef SRUBY_DSL_OPUSENUM_H
#define SRUBY_DSL_OPUSENUM_H
#include "ast/ast.h"

namespace sorbet::dsl {

/**
 * This class makes it easier to use Opus::Enum in `typed: strict` files:
 *
 *   class MyEnum < Opus::Enum
 *     X = new
 *     Y = new('y')
 *   end
 *
 * becomes
 *
 *   class MyEnum < Opus::Enum
 *     X = T.let(new, self)
 *     Y = T.let(new('y'), self)
 *   end
 */
class OpusEnum final {
public:
    static void patchDSL(core::MutableContext ctx, ast::ClassDef *klass);

    OpusEnum() = delete;
};

} // namespace sorbet::dsl

#endif
