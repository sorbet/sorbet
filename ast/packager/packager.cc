#include "ast/packager/packager.h"
#include "ast/Helpers.h"

using namespace std;

namespace sorbet::ast::packager {

ExpressionPtr appendRegistry(ExpressionPtr scope) {
    return ast::MK::UnresolvedConstant(scope.loc().copyEndWithZeroLength(), move(scope),
                                       core::Names::Constants::PackageSpec_Storage());
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
