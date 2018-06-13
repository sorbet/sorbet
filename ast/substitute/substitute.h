#ifndef SORBET_SUBSTITUTE_H
#define SORBET_SUBSTITUTE_H

#include "ast/ast.h"
#include "core/Context.h"

namespace sorbet {
namespace ast {
class Substitute {
public:
    static std::unique_ptr<Expression> run(core::MutableContext ctx, const core::GlobalSubstitution &subst,
                                           std::unique_ptr<Expression> what);
};
} // namespace ast
} // namespace sorbet
#endif // SORBET_SUBSTITUTE_H
