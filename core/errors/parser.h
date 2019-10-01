#ifndef SORBET_CORE_ERRORS_PARSER_H
#define SORBET_CORE_ERRORS_PARSER_H
#include "core/Error.h"

namespace sorbet::core::errors::Parser {
constexpr ErrorClass ParserError{2001, StrictLevel::False};

// These errors should all be "recovered" parser errors.
// Sorbet will take the fast path for all these errors.
constexpr ErrorClass MethodWithoutSelector{2002, StrictLevel::False};
} // namespace sorbet::core::errors::Parser
#endif
