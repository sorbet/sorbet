#ifndef SORBET_METHOD_CHECKS_H
#define SORBET_METHOD_CHECKS_H

#include "ast/ast.h"
#include "core/core.h"

#include <vector>

namespace sorbet::method_checks {

void validateSymbols(core::GlobalState &gs);

} // namespace sorbet::method_checks

#endif
