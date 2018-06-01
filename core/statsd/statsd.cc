#include "core/statsd/statsd.h"
#include "core/Counters_impl.h"

extern "C" {
#include "statsd-client.h"
}

#include "absl/strings/str_cat.h"

#include <string>

using namespace std;

namespace ruby_typer {
namespace core {

class StatsdClientWrapper {
    statsd_link *link;

public:
    StatsdClientWrapper(string host, int port, string prefix)
        : link(statsd_init_with_namespace(host.c_str(), port, prefix.c_str())) {}

    ~StatsdClientWrapper() {
        statsd_finalize(link);
    }

    void gauge(string name, size_t value) {
        statsd_gauge(link, const_cast<char *>(name.c_str()), value);
    }
    void timing(string name, size_t ms) {
        statsd_timing(link, const_cast<char *>(name.c_str()), ms);
    }
};

bool StatsD::submitCounters(const core::CounterState &counters, string host, int port, string prefix) {
    StatsdClientWrapper statsd(host, port, prefix);

    auto canon = counters.counters->canonicalize();
    for (auto &cat : canon.counters_by_category) {
        CounterImpl::CounterType sum = 0;
        for (auto &e : cat.second) {
            sum += e.second;
            statsd.gauge(absl::StrCat(cat.first, ".", e.first), e.second);
        }

        statsd.gauge(absl::StrCat(cat.first, ".total"), sum);
    }

    for (auto &hist : canon.histograms) {
        CounterImpl::CounterType sum = 0;
        for (auto &e : hist.second) {
            sum += e.second;
            statsd.gauge(absl::StrCat(hist.first, ".", e.first), e.second);
        }

        statsd.gauge(absl::StrCat(hist.first, ".total"), sum);
    }

    for (auto &e : canon.counters) {
        statsd.gauge(e.first, e.second);
    }

    return true;
}

} // namespace core
} // namespace ruby_typer
