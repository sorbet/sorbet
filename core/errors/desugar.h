#ifndef SORBET_CORE_ERRORS_DESUGAR_H
#define SORBET_CORE_ERRORS_DESUGAR_H
#include "core/Errors.h"

namespace sorbet {
namespace core {
namespace errors {
namespace Desugar {
constexpr ErrorClass InvalidSingletonDef{3001, StrictLevel::Typed};
constexpr ErrorClass IntegerOutOfRange{3002, StrictLevel::Typed};
constexpr ErrorClass UnsupportedNode{3003, StrictLevel::Stripe};
constexpr ErrorClass FloatOutOfRange{3004, StrictLevel::Typed};
constexpr ErrorClass NoConstantReassignment{3005, StrictLevel::Typed};
} // namespace Desugar
} // namespace errors
} // namespace core
} // namespace sorbet

#endif