#ifndef SORBET_CORE_STATSD_H
#define SORBET_CORE_STATSD_H

#include "core/core.h"

namespace sorbet {
namespace core {
class StatsD {
public:
    StatsD() = delete;

    static bool submitCounters(const core::CounterState &counters, std::string host, int port, std::string prefix);
};
} // namespace core
} // namespace sorbet

#endif
