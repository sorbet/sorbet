#ifndef SORBET_TYPE_DRIVEN_AUTOCORRECT_H
#define SORBET_TYPE_DRIVEN_AUTOCORRECT_H

#include "core/Error.h"
#include "core/GlobalState.h"
#include "core/TypeConstraint.h"
#include "core/Types.h"

namespace sorbet::core {

void typeDrivenAutocorrect(const core::GlobalState &gs, core::ErrorBuilder &e, core::Loc loc,
                           core::TypeConstraint &constr, core::TypePtr expectedType, core::TypePtr actualType);

}

#endif
