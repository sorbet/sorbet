#ifndef SORBET_CORE_STATSD_H
#define SORBET_CORE_STATSD_H

#include "core/core.h"

namespace sorbet {
class StatsD {
public:
    StatsD() = delete;

    // these are histograms which we do not want to report to statsd/SignalFX under any
    // circumstances.
    static constexpr std::array<std::string_view, 2> ignoredHistograms{{"untyped.usages", "untyped.blames"}};

    /** Adds standard process and sorbet-related metrics (RSS, faults, Sorbet version, etc). */
    static void addStandardMetrics();
    /**
     * If `latestVersionFile` is non-empty, it's re-read on every call, and every metric is tagged with
     * `is_latest_version` according to whether its contents match the revision Sorbet was built from.
     * Re-reading on every call means a long-running LSP process notices when the file changes.
     */
    static bool submitCounters(const CounterState &counters, std::string_view host, int port, std::string_view prefix,
                               const UnorderedMap<std::string, std::string> &extraTags,
                               std::string_view latestVersionFile);
};
} // namespace sorbet

#endif
