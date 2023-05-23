#include "common/statsd/statsd.h"
#include "common/counters/Counters_impl.h"
#include "common/strings/formatting.h"
#include "sorbet_version/sorbet_version.h"

extern "C" {
#include "statsd-client.h"
}

#include "absl/strings/str_cat.h"
#include "absl/strings/str_replace.h"

#include <string>
#include <sys/resource.h> // getrusage

using namespace std;

namespace sorbet {

namespace {
map<string, string> extraGlobalTags;
}
void StatsD::addExtraTags(const map<string, string> &tags) {
    if (!extraGlobalTags.empty()) {
        bool isSame =
            tags.size() == extraGlobalTags.size() && std::equal(tags.begin(), tags.end(), extraGlobalTags.begin(),
                                                                [](auto a, auto b) { return a.first == b.first; });
        if (!isSame) {
            Exception::raise("re setting statsD global tags not supported");
        }
    }
    extraGlobalTags = tags;
}

class StatsdClientWrapper {
    constexpr static int PKT_LEN = 512; // conservative bound for MTU
    statsd_link *link;
    string packet;

    string cleanMetricName(string_view name) {
        return absl::StrReplaceAll(name, {{":", "_"}, {"|", "_"}, {"@", "_"}});
    }

    string cleanTagNameOrValue(const char *tag) {
        // The keys and values must not be empty, and must not contain commas. Keys must not contain colons.
        // To keep things simple, we put the same restrictions on keys and values.
        ENFORCE(strnlen(tag, 1) != 0);
        return absl::StrReplaceAll(tag, {{":", "_"}, {"|", "_"}, {"@", "_"}, {",", "_"}});
    }

    void addMetric(string_view name, size_t value, string_view type, vector<pair<const char *, const char *>> tags) {
        for (const auto &[key, value] : extraGlobalTags) {
            tags.emplace_back(key.c_str(), value.c_str());
        }
        // spec: https://github.com/etsy/statsd/blob/master/docs/metric_types.md#multi-metric-packets
        auto newLine = fmt::format("{}{}:{}|{}{}{}", link->ns ? link->ns : "", cleanMetricName(name), value, type,
                                   tags.empty() ? "" : "|#", fmt::map_join(tags, ",", [&](const auto &tag) -> string {
                                       return fmt::format("{}:{}", cleanTagNameOrValue(tag.first),
                                                          cleanTagNameOrValue(tag.second));
                                   }));
        if (packet.size() + newLine.size() + 1 < PKT_LEN) {
            packet = absl::StrCat(packet, "\n", newLine);
        } else {
            if (!packet.empty()) {
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
        if (!packet.empty()) {
            statsd_send(link, packet.c_str());
        }
        statsd_finalize(link);
    }

    void gauge(string_view name, size_t value) { // type : g
        addMetric(name, value, "g", {});
    }
    void timing(const CounterImpl::Timing &tim) { // type: ms
        auto nanoseconds = (tim.end.usec - tim.start.usec) * 1'000;
        // format suggested by #observability (@sjung and @an)
        addMetric(absl::StrCat(tim.measure, ".duration_ns"), nanoseconds, "ms",
                  tim.tags == nullptr ? (vector<pair<const char *, const char *>>{}) : *tim.tags);
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
        if (std::find(ignoredHistograms.begin(), ignoredHistograms.end(), hist.first) != ignoredHistograms.end()) {
            continue;
        }
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

    for (const auto &e : counters.counters->timings) {
        statsd.timing(e);
    }

    return true;
}

void StatsD::addStandardMetrics() {
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
    prodCounterAdd("release.build_scm_commit_count", sorbet_build_scm_commit_count);
    prodCounterAdd("release.build_timestamp", sorbet_build_timestamp);
}

} // namespace sorbet
