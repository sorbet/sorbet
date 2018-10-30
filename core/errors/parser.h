#ifndef SORBET_CORE_ERRORS_PARSER_H
#define SORBET_CORE_ERRORS_PARSER_H
#include "core/Error.h"

namespace sorbet::core::errors::Parser {
constexpr ErrorClass ParserError{2001, StrictLevel::Stripe};
} // namespace sorbet::core::errors::Parser
#endif