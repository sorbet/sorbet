#ifndef SORBET_CORE_ERRORS_PARSER_H
#define SORBET_CORE_ERRORS_PARSER_H
#include "core/Errors.h"

namespace sorbet {
namespace core {
namespace errors {
namespace Parser {
constexpr ErrorClass ParserError{2001, StrictLevel::Stripe};
}
} // namespace errors
} // namespace core
} // namespace sorbet
#endif