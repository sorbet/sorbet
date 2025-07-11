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
    static ParsedFile run(core::Context ctx, const core::NameSubstitution &subst, ParsedFile what);
    static ParsedFile run(core::Context ctx, core::LazyNameSubstitution &subst, ParsedFile what);
};
} // namespace sorbet::ast
#endif // SORBET_SUBSTITUTE_H
