#ifndef SORBET_COMMON_OPENTELEMETRY_H
#define SORBET_COMMON_OPENTELEMETRY_H

#include "common/Counters.h"
#include <string_view>

namespace sorbet {
class OpenTelemetry {
public:
    OpenTelemetry() = delete;

    // TODO(jez) StatsD also has `addStandardMetrics` and `addExtraTags`...?
    // TODO(jez) Submit non-timer stuff?
    static void submitTimers(const CounterState &counters);
};
} // namespace sorbet

#endif
