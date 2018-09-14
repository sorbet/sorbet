#ifndef SORBET_CORE_STATSD_H
#define SORBET_CORE_STATSD_H

#include "core/core.h"

namespace sorbet {
class StatsD {
public:
    StatsD() = delete;

    static bool submitCounters(const CounterState &counters, std::string host, int port, std::string prefix);
};
} // namespace sorbet

#endif
