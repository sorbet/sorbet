// have to be included first as they violate our poisons
#include "core/proto/proto.h"

#include "absl/strings/str_cat.h"
#include "common/Random.h"
#include "common/counters/Counters_impl.h"

using namespace std;

namespace sorbet::core {

com::stripe::payserver::events::cibot::SourceMetrics Proto::toProto(const CounterState &counters, string_view prefix) {
    com::stripe::payserver::events::cibot::SourceMetrics metrics;
    auto unix_timestamp = chrono::seconds(time(nullptr));
    metrics.set_timestamp(unix_timestamp.count());

    // UUID version 1 as specified in RFC 4122
    string uuid = fmt::format(
        "{:#08x}-{:#04x}-{:#04x}-{:#04x}-{:#08x}{:#04x}",
        (unsigned long long)Random::uniformU8(), // Generates a 64-bit Hex number
        Random::uniformU4(),                     // Generates a 32-bit Hex number
        Random::uniformU4(0, 0x0fff) |
            0x4000, // Generates a 32-bit Hex number of the form 4xxx (4 indicates the UUID version)
        Random::uniformU4(0, 0x3fff) | 0x8000,            // Generates a 32-bit Hex number in the range [0x8000, 0xbfff]
        (unsigned long long)Random::uniformU8(), rand()); // Generates a 96-bit Hex number

    metrics.set_uuid(uuid);

    counters.counters->canonicalize();
    for (auto &cat : counters.counters->countersByCategory) {
        CounterImpl::CounterType sum = 0;
        for (auto &e : cat.second) {
            sum += e.second;
            com::stripe::payserver::events::cibot::SourceMetrics_SourceMetricEntry *metric = metrics.add_metrics();
            metric->set_name(absl::StrCat(prefix, ".", cat.first, ".", e.first));
            metric->set_value(e.second);
        }

        com::stripe::payserver::events::cibot::SourceMetrics_SourceMetricEntry *metric = metrics.add_metrics();
        metric->set_name(absl::StrCat(prefix, ".", cat.first, ".total"));
        metric->set_value(sum);
    }

    for (auto &hist : counters.counters->histograms) {
        CounterImpl::CounterType sum = 0;
        int histMin = hist.second.begin()->first;
        int histMax = hist.second.begin()->first;
        for (auto &e : hist.second) {
            sum += e.second;
            histMin = min(histMin, e.first);
            histMax = max(histMin, e.first);
        }
        CounterImpl::CounterType running = 0;
        vector<pair<int, bool>> percentiles = {{25, false}, {50, false}, {75, false}, {90, false}};
        for (auto &e : hist.second) {
            running += e.second;
            for (auto &pct : percentiles) {
                if (pct.second) {
                    continue;
                }
                if (running >= sum * pct.first / 100) {
                    pct.second = true;
                    com::stripe::payserver::events::cibot::SourceMetrics_SourceMetricEntry *metric =
                        metrics.add_metrics();
                    metric->set_name(absl::StrCat(prefix, ".", hist.first, ".p", pct.first));
                    metric->set_value(e.first);
                }
            }
        }
        com::stripe::payserver::events::cibot::SourceMetrics_SourceMetricEntry *metric = metrics.add_metrics();
        metric->set_name(absl::StrCat(prefix, ".", hist.first, ".min"));
        metric->set_value(histMin);

        metric = metrics.add_metrics();
        metric->set_name(absl::StrCat(prefix, ".", hist.first, ".max"));
        metric->set_value(histMax);

        metric = metrics.add_metrics();
        metric->set_name(absl::StrCat(prefix, ".", hist.first, ".total"));
        metric->set_value(sum);
    }

    for (auto &e : counters.counters->counters) {
        com::stripe::payserver::events::cibot::SourceMetrics_SourceMetricEntry *metric = metrics.add_metrics();
        metric->set_name(absl::StrCat(prefix, ".", e.first));
        metric->set_value(e.second);
    }

    return metrics;
}

string Proto::toJSON(const google::protobuf::Message &message) {
    string jsonString;
    google::protobuf::json::PrintOptions options;
    options.add_whitespace = true;
    options.preserve_proto_field_names = true;
    auto status = google::protobuf::json::MessageToJsonString(message, &jsonString, options);
    if (!status.ok()) {
        cerr << "error converting to proto json: " << status.message() << '\n';
        abort();
    }
    return jsonString;
}

} // namespace sorbet::core
