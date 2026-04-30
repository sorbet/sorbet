#include "ast/packager/packager.h"
#include "ast/Helpers.h"

using namespace std;

namespace sorbet::ast::packager {

ExpressionPtr prependRegistry(ExpressionPtr scope) {
    auto constLit = ast::cast_tree<ast::UnresolvedConstantLit>(scope);
    ENFORCE(constLit != nullptr);
    auto &root = constLit->scope;
    root = ast::MK::Constant(root.loc().copyWithZeroLength(), core::Symbols::PackageSpecRegistry());
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
