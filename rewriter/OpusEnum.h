#ifndef SRUBY_DSL_OPUSENUM_H
#define SRUBY_DSL_OPUSENUM_H
#include "ast/ast.h"

namespace sorbet::rewriter {

/**
 * This class provides the following features for Opus::Enum:
 *
 * - no need to manually annotate constants in `T.let` for use in strict mode
 * - support for exhaustiveness checks over enum values (by pretending that
 *   Opus::Enum values are actually singleton instances of synthetic classes)
 * - allows using Opus::Enum values in type signatures directly.
 *
 * Given either:
 *
 *   class MyEnum < Opus::Enum
 *     X = new
 *     Y = new('y')
 *     Z = T.let(new, Z)
 *   end
 *
 *   class MyEnum < Opus::Enum
 *     enums do
 *       X = new
 *       Y = new('y')
 *       Z = T.let(new, Z)
 *     end
 *   end
 *
 * Outputs:
 *
 *   class MyEnum < Opus::Enum
 *     extend T::Helpers
 *     sealed!
 *     abstract!
 *
 *     class X$1 < MyEnum; include Singleton; final!; end
 *     X = T.let(X$1.instance, X$1)
 *     new
 *
 *     class Y$1 < MyEnum; include Singleton; final!; end
 *     Y = T.let(Y$1.instance, Y$1)
 *     new('y')
 *
 *     class Z$1 < MyEnum; include Singleton; final!; end
 *     Z = T.let(Z$1.instance, Z$1)
 *     T.let(new, Z)
 *   end
 */
class OpusEnum final {
public:
    static void patchDSL(core::MutableContext ctx, ast::ClassDef *klass);

    OpusEnum() = delete;
};

} // namespace sorbet::dsl

#endif
