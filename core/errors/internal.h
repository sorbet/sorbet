#include "core/Errors.h"

namespace sorbet {
namespace core {
namespace errors {
namespace Internal {
constexpr ErrorClass InternalError = {1001, StrictLevel::Stripe};
constexpr ErrorClass WrongSigil = {1002, StrictLevel::Stripe};
} // namespace Internal
} // namespace errors
} // namespace core
} // namespace sorbet
