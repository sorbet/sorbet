#ifndef SORBET_AST_PACKAGER_PACKAGER_H
#define SORBET_AST_PACKAGER_PACKAGER_H

#include "ast/ast.h"

namespace sorbet::ast::packager {

ExpressionPtr appendRegistry(ExpressionPtr scope);

const ast::ClassDef *asPackageSpecClass(const ast::ExpressionPtr &expr);

} // namespace sorbet::ast::packager
  //
#endif
