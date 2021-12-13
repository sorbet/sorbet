#ifndef SORBET_SUBSTITUTE_H
#define SORBET_SUBSTITUTE_H

#include "ast/ast.h"
#include "core/Context.h"

namespace sorbet::core {
class LazyNameSubstitution;
};

namespace sorbet::ast {
class Substitute {
public:
    static ExpressionPtr run(core::MutableContext ctx, const core::NameSubstitution &subst, ExpressionPtr what);
    static ExpressionPtr run(core::MutableContext ctx, core::LazyNameSubstitution &subst, ExpressionPtr what);
};
} // namespace sorbet::ast
#endif // SORBET_SUBSTITUTE_H
