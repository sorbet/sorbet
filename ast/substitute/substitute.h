#ifndef SORBET_SUBSTITUTE_H
#define SORBET_SUBSTITUTE_H

#include "ast/ast.h"
#include "core/Context.h"

namespace sorbet::ast {
class Substitute {
public:
    static ExpressionPtr run(core::MutableContext ctx, const core::GlobalSubstitution &subst, ExpressionPtr what);
};
} // namespace sorbet::ast
#endif // SORBET_SUBSTITUTE_H
