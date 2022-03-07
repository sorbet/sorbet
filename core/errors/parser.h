#ifndef SORBET_CORE_ERRORS_PARSER_H
#define SORBET_CORE_ERRORS_PARSER_H
#include "core/Error.h"

namespace sorbet::core::errors::Parser {
constexpr ErrorClass ParserError{2001, StrictLevel::False};
constexpr ErrorClass ReservedForNumparamError{2002, StrictLevel::False};
constexpr ErrorClass ErrorRecoveryHint{2003, StrictLevel::False};
constexpr ErrorClass AssignmentToNumparamError{2004, StrictLevel::False};
} // namespace sorbet::core::errors::Parser
#endif
