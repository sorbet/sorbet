#ifndef SORBET_CORE_ERRORS_PLUGIN_H
#define SORBET_CORE_ERRORS_PLUGIN_H
#include "core/Error.h"

namespace sorbet::core::errors::Plugin {
constexpr ErrorClass SubProcessError{3401, StrictLevel::Internal};
} // namespace sorbet::core::errors::Plugin
#endif
