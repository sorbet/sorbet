#ifndef SORBET_CORE_ERRORS_INTERNAL_H
#define SORBET_CORE_ERRORS_INTERNAL_H
#include "core/Error.h"

namespace sorbet::core::errors::Internal {
constexpr ErrorClass InternalError{1001, StrictLevel::Internal};
constexpr ErrorClass WrongSigil{1002, StrictLevel::Internal};
constexpr ErrorClass CyclicReferenceError{1003, StrictLevel::Internal};
} // namespace sorbet::core::errors::Internal

#endif
