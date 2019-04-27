#include "common/statsd/statsd.h"
#include "common/Counters_impl.h"

extern "C" {
#include "statsd-client.h"
}

#include "absl/strings/str_cat.h"
#include "absl/strings/str_replace.h"

#include <string>
#include <sys/resource.h> // getrusage

using namespace std;

namespace sorbet {

class StatsdClientWrapper {
    constexpr static int PKT_LEN = 512; // conservative bound for MTU
    statsd_link *link;
    string packet;

    string cleanMetricName(string_view name) {
        return absl::StrReplaceAll(name, {{":", "_"}, {"|", "_"}, {"@", "_"}});
    }

    void addMetric(string_view name, size_t value, string_view type) {
        // spec: https://github.com/etsy/statsd/blob/master/docs/metric_types.md#multi-metric-packets
        auto newLine = fmt::format("{}{}:{}|{}", link->ns ? link->ns : "", cleanMetricName(name), value, type);
        if (packet.size() + newLine.size() + 1 < PKT_LEN) {
            packet = packet + '\n' + newLine;
        } else {
            if (packet.size() > 0) {
                statsd_send(link, packet.c_str());
                packet = move(newLine);
            } else {
                // the packet itself might be bigger than MTU
                statsd_send(link, newLine.c_str());
            }
        }
    }

public:
    StatsdClientWrapper(string host, int port, string prefix)
        : link(statsd_init_with_namespace(host.c_str(), port, cleanMetricName(prefix).c_str())) {}

    ~StatsdClientWrapper() {
        if (packet.size() > 0) {
            statsd_send(link, packet.c_str());
        }
        statsd_finalize(link);
    }

    void gauge(string_view name, size_t value) { // type : g
        addMetric(name, value, "g");
    }
    void timing(string_view name, size_t microseconds) { // type: ms
        addMetric(absl::StrCat(name, ".duration_ns"), microseconds * 1000,
                  "ms"); // format suggested by #observability (@sjung and @an)
        // we started by storing nanoseconds but later switched to microseconds. We'll continue emmiting nanoseconds to
        // statsd to keep old graphs
    }
};

bool StatsD::submitCounters(const CounterState &counters, string_view host, int port, string_view prefix) {
    StatsdClientWrapper statsd(string(host), port, string(prefix));

    counters.counters->canonicalize();
    for (auto &cat : counters.counters->countersByCategory) {
        CounterImpl::CounterType sum = 0;
        for (auto &e : cat.second) {
            sum += e.second;
            statsd.gauge(absl::StrCat(cat.first, ".", e.first), e.second);
        }

        statsd.gauge(absl::StrCat(cat.first, ".total"), sum);
    }

    for (auto &hist : counters.counters->histograms) {
        CounterImpl::CounterType sum = 0;
        for (auto &e : hist.second) {
            sum += e.second;
            statsd.gauge(absl::StrCat(hist.first, ".", e.first), e.second);
        }

        statsd.gauge(absl::StrCat(hist.first, ".total"), sum);
    }

    for (auto &e : counters.counters->counters) {
        statsd.gauge(e.first, e.second);
    }

    UnorderedMap<int, CounterImpl::Timing> flowStarts;
    for (const auto &e : counters.counters->timings) {
        if (e.duration) {
            statsd.timing(e.measure, e.duration.value());
        } else if (e.kind == CounterImpl::Timing::FlowStart) {
            flowStarts.emplace(e.id, e);
        } else if (e.kind == CounterImpl::Timing::FlowEnd) {
            auto fnd = flowStarts.find(e.id);
            if (fnd != flowStarts.end()) {
                auto &start = *fnd;
                statsd.timing(e.measure, e.ts - start.second.ts);
            }
        }
    }

    return true;
}

void StatsD::addRusageStats() {
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
        prodCounterAdd("run.utilization.user_time.us", usage.ru_utime.tv_sec * 1000'000 + usage.ru_utime.tv_usec);
        prodCounterAdd("run.utilization.system_time.us", usage.ru_stime.tv_sec * 1000'000 + usage.ru_stime.tv_usec);
        prodCounterAdd("run.utilization.max_rss", usage.ru_maxrss);
        prodCounterAdd("run.utilization.minor_faults", usage.ru_minflt);
        prodCounterAdd("run.utilization.major_faults", usage.ru_majflt);
        prodCounterAdd("run.utilization.inblock", usage.ru_inblock);
        prodCounterAdd("run.utilization.oublock", usage.ru_oublock);
        prodCounterAdd("run.utilization.context_switch.voluntary", usage.ru_nvcsw);
        prodCounterAdd("run.utilization.context_switch.involuntary", usage.ru_nivcsw);
    }
}

} // namespace sorbet
