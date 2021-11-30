#ifndef SORBET_RESOLVER_SUGGEST_PACKAGE_H
#define SORBET_RESOLVER_SUGGEST_PACKAGE_H

#include "ast/ast.h"

namespace sorbet::resolver {

class SuggestPackage final {
public:
    // Returns true iff package-specific error messages/corrections were found.
    static bool tryPackageCorrections(core::Context ctx, core::ErrorBuilder &e,
                                      const ast::ConstantLit::ResolutionScopes &scopes,
                                      ast::UnresolvedConstantLit &unresolved);

    SuggestPackage() = delete;
};

} // namespace sorbet::resolver

#endif
