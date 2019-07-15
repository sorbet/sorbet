#ifndef SORBET_VARIANCE_CHECKS_H
#define SORBET_VARIANCE_CHECKS_H

#include "core/core.h"

namespace sorbet::definition_validator::variance {

void validateMethodVariance(const core::Context ctx, const core::SymbolRef method);

};

#endif
