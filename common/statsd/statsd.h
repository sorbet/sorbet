#ifndef SORBET_CORE_STATSD_H
#define SORBET_CORE_STATSD_H

#include "core/core.h"
#include <map>

namespace sorbet {
class StatsD {
public:
    StatsD() = delete;

    // these are histograms which we do not want to report to statsd/SignalFX under any
    // circumstances.
    static constexpr std::array<std::string_view, 2> ignoredHistograms{{"untyped.usages", "untyped.blames"}};

    /** Adds standard process and sorbet-related metrics (RSS, faults, Sorbet version, etc). */
    static void addStandardMetrics();
    static bool submitCounters(const CounterState &counters, std::string_view host, int port, std::string_view prefix);

    static void addExtraTags(const std::map<std::string, std::string> &tags);
};
} // namespace sorbet

#endif
