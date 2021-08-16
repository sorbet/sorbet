#ifndef SORBET_COMPILER_ERRORS_H
#define SORBET_COMPILER_ERRORS_H
#include "core/Error.h"

namespace sorbet::core::errors::Compiler {
constexpr ErrorClass Untyped{10001, StrictLevel::True};
constexpr ErrorClass Unanalyzable{10002, StrictLevel::True};
constexpr ErrorClass OptimizerFailure{10003, StrictLevel::True};
} // namespace sorbet::core::errors::Compiler
#endif
