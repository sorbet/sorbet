#ifndef SORBET_AST_PACKAGER_PACKAGER_H
#define SORBET_AST_PACKAGER_PACKAGER_H

#include "ast/ast.h"

namespace sorbet::ast::packager {

ExpressionPtr prependRegistry(ExpressionPtr scope);

} // namespace sorbet::ast::packager
  //
#endif
