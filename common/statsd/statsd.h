#ifndef SORBET_CORE_STATSD_H
#define SORBET_CORE_STATSD_H

#include "core/core.h"

namespace sorbet {
class StatsD {
public:
    StatsD() = delete;

    static bool submitCounters(const CounterState &counters, std::string_view host, int port, std::string_view prefix);
};
} // namespace sorbet

#endif
