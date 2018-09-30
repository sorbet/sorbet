#ifndef SORBET_CORE_ERRORS_DSL_H
#define SORBET_CORE_ERRORS_DSL_H
#include "core/Errors.h"

namespace sorbet::core::errors::DSL {
constexpr ErrorClass BadAttrArg{3501, StrictLevel::Typed};
constexpr ErrorClass BadWrapInstance{3502, StrictLevel::Typed};
} // namespace sorbet::core::errors::DSL
#endif