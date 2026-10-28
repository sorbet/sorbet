#include "common/statsd/statsd.h"
using namespace std;

namespace sorbet {

bool StatsD::submitCounters(const CounterState &counters, string_view host, int port, string_view prefix,
                            const UnorderedMap<string, string> &extraTags, string_view latestVersionFile) {
    return false;
}

void StatsD::addStandardMetrics() {}

} // namespace sorbet
