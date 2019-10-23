#ifndef SORBET_RESOLVER_CORRECT_TYPE_ALIAS_H
#define SORBET_RESOLVER_CORRECT_TYPE_ALIAS_H

#include "ast/ast.h"

namespace sorbet::resolver {

class CorrectTypeAlias final {
public:
    static void eagerToLazy(core::Context ctx, core::ErrorBuilder &e, ast::Send *send);
};

} // namespace sorbet::resolver

#endif
