#ifndef SORBET_CORE_ERRORS_CFG_H
#define SORBET_CORE_ERRORS_CFG_H
#include "core/Error.h"

namespace sorbet::core::errors::CFG {
constexpr ErrorClass NoNextScope{6001, StrictLevel::Stripe};
} // namespace sorbet::core::errors::CFG
#endif