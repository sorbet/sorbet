#ifndef SORBET_CORE_ERRORS_DESUGAR_H
#define SORBET_CORE_ERRORS_DESUGAR_H
#include "core/Error.h"

namespace sorbet::core::errors::Desugar {
constexpr ErrorClass InvalidSingletonDef{3001, StrictLevel::Typed};
constexpr ErrorClass IntegerOutOfRange{3002, StrictLevel::Typed};
constexpr ErrorClass UnsupportedNode{3003, StrictLevel::Stripe};
constexpr ErrorClass FloatOutOfRange{3004, StrictLevel::Typed};
constexpr ErrorClass NoConstantReassignment{3005, StrictLevel::Typed};
// constexpr ErrorClass SimpleSuperclass{3006, StrictLevel::Typed};
} // namespace sorbet::core::errors::Desugar

#endif
