#ifndef SORBET_CORE_ERRORS_DSL_H
#define SORBET_CORE_ERRORS_DSL_H
#include "core/Error.h"

namespace sorbet::core::errors::DSL {
constexpr ErrorClass BadAttrArg{3501, StrictLevel::True};
constexpr ErrorClass BadWrapInstance{3502, StrictLevel::True};
constexpr ErrorClass PrivateMethodMismatch{3503, StrictLevel::False};
constexpr ErrorClass BadAttrType{3504, StrictLevel::True};
} // namespace sorbet::core::errors::DSL
#endif
