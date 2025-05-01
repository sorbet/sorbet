#include "ast/packager/packager.h"
#include "ast/Helpers.h"
#include "common/common.h"

namespace sorbet::ast::packager {

ExpressionPtr prependRegistry(ExpressionPtr scope) {
    auto lastConstLit = ast::cast_tree<ast::UnresolvedConstantLit>(scope);
    ENFORCE(lastConstLit != nullptr);
    while (auto constLit = ast::cast_tree<ast::UnresolvedConstantLit>(lastConstLit->scope)) {
        lastConstLit = constLit;
    }
    lastConstLit->scope =
        ast::MK::Constant(lastConstLit->scope.loc().copyWithZeroLength(), core::Symbols::PackageSpecRegistry());
    return scope;
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
