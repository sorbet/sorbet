#include "ast/packager/packager.h"
#include "ast/Helpers.h"

using namespace std;

namespace sorbet::ast::packager {

ExpressionPtr appendRegistry(ExpressionPtr scope) {
    // We use the real loc (not a zero-width one) so that this constant does show up in LSP queries.
    // Various other places will hide <PackageSpec> results from LSP queries, as needed.
    auto loc = scope.loc();
    return ast::MK::UnresolvedConstant(loc, move(scope), core::Names::Constants::PackageSpec_Storage());
}

const ast::ClassDef *asPackageSpecClass(const ast::ExpressionPtr &expr) {
    auto packageSpecClass = ast::cast_tree<ast::ClassDef>(expr);
    if (packageSpecClass == nullptr || packageSpecClass->ancestors.empty()) {
        return nullptr;
    }

    auto superClassLit = ast::cast_tree<ast::ConstantLit>(packageSpecClass->ancestors[0]);
    if (superClassLit == nullptr || superClassLit->symbol() != core::Symbols::PackageSpec()) {
        return nullptr;
    }

    return packageSpecClass;
}

} // namespace sorbet::ast::packager
