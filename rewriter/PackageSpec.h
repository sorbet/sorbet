#ifndef SORBET_REWRITER_PACKAGE_SPEC_H
#define SORBET_REWRITER_PACKAGE_SPEC_H
#include "ast/ast.h"

namespace sorbet::rewriter {

/**
 * TODO(jez) Document me
 */
class PackageSpec final {
public:
    static void run(core::MutableContext ctx, ast::ClassDef *klass);

    PackageSpec() = delete;
};

} // namespace sorbet::rewriter

#endif
