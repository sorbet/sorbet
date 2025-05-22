#ifndef SORBET_REWRITER_PACKAGE_SPEC_H
#define SORBET_REWRITER_PACKAGE_SPEC_H
#include "ast/ast.h"

namespace sorbet::rewriter {

/**
 * Does some syntactic rewrites of `__package.rb` files.
 *
 * This rewriter only has an effect if `--stripe-packages` has been passed.
 *
 * Rewrites:
 *
 *     class Foo::Bar < PackageSpec
 *     end
 *
 * to
 *
 *     class Foo::Bar::<PackageSpec> < ::Sorbet::Private::Static::PackageSpec
 *     end
 */
class PackageSpec final {
public:
    static void run(core::MutableContext ctx, ast::ClassDef *klass);

    PackageSpec() = delete;
};

} // namespace sorbet::rewriter

#endif
