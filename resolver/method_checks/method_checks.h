#ifndef SORBET_METHOD_CHECKS_H
#define SORBET_METHOD_CHECKS_H

#include "ast/ast.h"
#include "core/core.h"

#include <vector>

namespace sorbet::method_checks {

void validateOverriding(core::GlobalState &gs, core::SymbolRef method);
void validateAbstract(core::GlobalState &gs, UnorderedMap<core::SymbolRef, std::vector<core::SymbolRef>> &abstractCache,
                      core::SymbolRef sym);

} // namespace sorbet::method_checks

#endif
