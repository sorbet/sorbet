#ifndef SORBET_SUBSTITUTE_H
#define SORBET_SUBSTITUTE_H

#include "ast/ast.h"
#include "core/Context.h"

namespace sorbet::ast {
class Substitute {
public:
    static std::unique_ptr<Expression> run(core::MutableContext ctx, const core::GlobalSubstitution &subst,
                                           std::unique_ptr<Expression> what);
};
} // namespace sorbet::ast
#endif // SORBET_SUBSTITUTE_H
