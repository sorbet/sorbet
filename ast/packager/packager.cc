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

} // namespace sorbet::ast::packager
