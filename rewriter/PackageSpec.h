#ifndef SORBET_REWRITER_PACKAGE_SPEC_H
#define SORBET_REWRITER_PACKAGE_SPEC_H
#include "ast/ast.h"

namespace sorbet::rewriter {

/**
 * Does some syntactic rewrites of `__package.rb` files.
 *
 * This rewriter only has an effect if `--sorbet-packages` has been passed.
 *
 * Rewrites:
 *
 *     class Foo::Bar < PackageSpec
 *     end
 *
 * to
 *
 *     class <PackageSpecRegistry>::Foo::Bar < ::Sorbet::Private::Static::PackageSpec
 *     end
 *
 * TODO(jez) When we move packages into the symbol table, we can drop the `<PackageSpecRegistry>`
 * bit, which is only required because there could be a class or module symbol with the `Foo::Bar`
 * name that collides with the package definition.
 */
class PackageSpec final {
public:
    static void run(core::MutableContext ctx, ast::ClassDef *klass);

    PackageSpec() = delete;
};

} // namespace sorbet::rewriter

#endif
