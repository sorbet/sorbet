#ifndef SORBET_CORE_ERRORS_CFG_H
#define SORBET_CORE_ERRORS_CFG_H
#include "core/Errors.h"

namespace sorbet {
namespace core {
namespace errors {
namespace CFG {
constexpr ErrorClass NoNextScope{6001, StrictLevel::Stripe};
}
} // namespace errors
} // namespace core
} // namespace sorbet
#endif