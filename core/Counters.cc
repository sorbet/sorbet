#include "core/Counters.h"
#include "core/Names.h"
extern "C" {
#include "statsd-client.h"
}
#include "absl/strings/str_cat.h"
#include "common/Random.h"
#include "proto/SourceMetrics.pb.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <google/protobuf/util/json_util.h>
#include <iomanip> // setw
#include <map>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

namespace ruby_typer {
namespace core {
static constexpr bool enable_counters = debug_mode;

CounterState::CounterState(unique_ptr<CounterImpl> counters) : counters(move(counters)) {}

CounterState::CounterState() = default;
CounterState::~CounterState() = default;
CounterState::CounterState(CounterState &&rhs) = default;
CounterState &CounterState::operator=(CounterState &&rhs) = default;

struct CounterImpl {
    CounterImpl() = default;
    CounterImpl(CounterImpl const &) = delete;
    CounterImpl(CounterImpl &&) = default;
    CounterImpl &operator=(CounterImpl &&) = delete;
    typedef unsigned long CounterType;

    CounterImpl canonicalize();

    void clear();

    const char *internKey(const char *str) {
        auto it1 = this->strings_by_ptr.find(str);
        if (it1 != this->strings_by_ptr.end()) {
            return it1->second;
        }

        absl::string_view view(str);
        auto it2 = this->strings_by_value.find(view);
        if (it2 != this->strings_by_value.end()) {
            this->strings_by_ptr[str] = it2->second;
            return it2->second;
        }

        this->strings_by_value[view] = str;
        this->strings_by_ptr[str] = str;
        return str;
    }

    void histogramAdd(const char *histogram, int key, unsigned int value) {
        if (!enable_counters) {
            return;
        }
        this->histograms[histogram][key] += value;
    }

    void categoryCounterAdd(const char *category, const char *counter, unsigned int value) {
        if (!enable_counters) {
            return;
        }

        this->counters_by_category[category][counter] += value;
    }

    void counterAdd(const char *counter, unsigned int value) {
        if (!enable_counters) {
            return;
        }
        this->counters[counter] += value;
    }

    // absl::string_view isn't hashable, so we use an unordered map. We could
    // implement hash ourselves, but this is the slowpath anyways.
    std::map<absl::string_view, const char *> strings_by_value;
    std::map<const char *, const char *> strings_by_ptr;

    std::map<const char *, std::map<int, CounterType>> histograms;
    std::map<const char *, CounterType> counters;
    std::map<const char *, std::map<const char *, CounterType>> counters_by_category;
};

thread_local CounterImpl counterState;

CounterState getAndClearThreadCounters() {
    if (!enable_counters) {
        return CounterState(make_unique<CounterImpl>());
    }
    auto state = make_unique<CounterImpl>(move(counterState));
    counterState.clear();
    return CounterState(move(state));
}

void CounterImpl::clear() {
    this->strings_by_value.clear();
    this->strings_by_ptr.clear();
    this->histograms.clear();
    this->counters.clear();
    this->counters_by_category.clear();
}

void counterConsume(CounterState cs) {
    if (!enable_counters) {
        return;
    }
    for (auto &cat : cs.counters->counters_by_category) {
        for (auto &e : cat.second) {
            counterState.categoryCounterAdd(cat.first, e.first, e.second);
        }
    }

    for (auto &hist : cs.counters->histograms) {
        for (auto &e : hist.second) {
            counterState.histogramAdd(hist.first, e.first, e.second);
        }
    }

    for (auto &e : cs.counters->counters) {
        counterState.counterAdd(e.first, e.second);
    }
}

void counterAdd(ConstExprStr counter, unsigned int value) {
    counterState.counterAdd(counter.str, value);
}

void counterInc(ConstExprStr counter) {
    counterAdd(counter, 1);
}

void categoryCounterInc(ConstExprStr category, ConstExprStr counter) {
    categoryCounterAdd(category, counter, 1);
}

void categoryCounterAdd(ConstExprStr category, ConstExprStr counter, unsigned int value) {
    counterState.categoryCounterAdd(category.str, counter.str, value);
}

void histogramInc(ConstExprStr histogram, int key) {
    counterState.histogramAdd(histogram.str, key, 1);
}

void histogramAdd(ConstExprStr histogram, int key, unsigned int value) {
    counterState.histogramAdd(histogram.str, key, value);
}

const int MAX_WIDTH = 100;
const int PAD_LIMIT = 20;
const int MAX_STAT_WIDTH = MAX_WIDTH - PAD_LIMIT;
const float HIST_CUTOFF = 0.1;

string padOrLimit(string s, int l) {
    if (s.size() < l) {
        string prefix(l - s.size(), ' ');
        s = prefix + s;
    } else {
        s = s.substr(0, l);
    }
    return s;
}

CounterImpl CounterImpl::canonicalize() {
    CounterImpl out;

    for (auto &cat : this->counters_by_category) {
        for (auto &e : cat.second) {
            out.categoryCounterAdd(internKey(cat.first), internKey(e.first), e.second);
        }
    }

    for (auto &hist : this->histograms) {
        for (auto &e : hist.second) {
            out.histogramAdd(internKey(hist.first), e.first, e.second);
        }
    }

    for (auto &e : this->counters) {
        out.counterAdd(internKey(e.first), e.second);
    }
    return out;
}

const vector<string> Counters::ALL_COUNTERS = {"<all>"};
bool shouldShow(vector<string> &wantNames, string name) {
    if (wantNames == Counters::ALL_COUNTERS) {
        return true;
    }
    return find(wantNames.begin(), wantNames.end(), name) != wantNames.end();
}

string getCounterStatistics(vector<string> names) {
    if (!enable_counters) {
        return "Statistics collection is not available in production build. Use debug build.";
    }
    stringstream buf;

    buf << "Counters and Histograms: " << '\n';

    auto canon = counterState.canonicalize();

    for (auto &cat : canon.counters_by_category) {
        if (!shouldShow(names, cat.first)) {
            continue;
        }
        CounterImpl::CounterType sum = 0;
        std::vector<pair<CounterImpl::CounterType, string>> sorted;
        for (auto &e : cat.second) {
            sum += e.second;
            sorted.emplace_back(e.second, e.first);
        }
        sort(sorted.begin(), sorted.end(), [](const auto &e1, const auto &e2) -> bool { return e1.first > e2.first; });

        buf << " " << cat.first << "    Total: " << sum << '\n';

        for (auto &e : sorted) {
            string number = padOrLimit(to_string(e.first), 6);
            string perc = padOrLimit(to_string(round(e.first * 1000.0 / sum) / 10.0), 3);
            string hashes((int)(MAX_STAT_WIDTH * 1.0 * e.first / sum), '#');
            buf << "  " << padOrLimit(e.second, PAD_LIMIT - 4) << " :" << number << ", " << perc << "% " << hashes
                << '\n';
        }
        buf << '\n';
    }

    for (auto &hist : canon.histograms) {
        if (!shouldShow(names, hist.first)) {
            continue;
        }
        CounterImpl::CounterType sum = 0;
        for (auto &e : hist.second) {
            sum += e.second;
        }
        buf << " " << hist.first << "    Total: " << sum << '\n';

        CounterImpl::CounterType header = 0;
        auto it = hist.second.begin();
        while (it != hist.second.end() && (header + it->second) * 1.0 / sum < HIST_CUTOFF) {
            header += it->second;
            it++;
        }
        if (it != hist.second.begin()) {
            string number = padOrLimit(to_string(header), 6);
            string perc = padOrLimit(to_string(round(header * 1000.0 / sum) / 10.0), 3);
            string hashes((int)(MAX_STAT_WIDTH * 1.0 * header / sum), '#');
            buf << " <" << padOrLimit(to_string(it->first), PAD_LIMIT - 4) << " :" << number << ", " << perc << "% "
                << hashes << '\n';
        }

        while (it != hist.second.end() && (sum - header) * 1.0 / sum > HIST_CUTOFF) {
            header += it->second;
            string number = padOrLimit(to_string(it->second), 6);
            string perc = padOrLimit(to_string(round(it->second * 1000.0 / sum) / 10.0), 3);
            string hashes((int)(MAX_STAT_WIDTH * 1.0 * it->second / sum), '#');
            buf << "  " << padOrLimit(to_string(it->first), PAD_LIMIT - 4) << " :" << number << ", " << perc << "% "
                << hashes << '\n';
            it++;
        }
        if (it != hist.second.end()) {
            string number = padOrLimit(to_string(sum - header), 6);
            string perc = padOrLimit(to_string(round((sum - header) * 1000.0 / sum) / 10.0), 3);
            string hashes((int)(MAX_STAT_WIDTH * 1.0 * (sum - header) / sum), '#');
            buf << ">=" << padOrLimit(to_string(it->first), PAD_LIMIT - 4) << " :" << number << ", " << perc << "% "
                << hashes << '\n';
        }
        buf << '\n';
    }

    {
        vector<pair<string, string>> sortedOther;
        for (auto &e : canon.counters) {
            if (!shouldShow(names, e.first)) {
                continue;
            }
            string number = to_string(e.second);
            if (number.size() < 6) {
                number = padOrLimit(number, 6);
            }
            string line = "  " + padOrLimit(e.first, PAD_LIMIT - 4) + " :" + number + "\n";
            sortedOther.emplace_back(e.first, line);
        }
        sort(sortedOther.begin(), sortedOther.end(),
             [](const auto &e1, const auto &e2) -> bool { return e1.first < e2.first; });

        for (auto &e : sortedOther) {
            buf << e.second;
        }
    }
    return buf.str();
}

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

bool submitCountersToStatsd(std::string host, int port, std::string prefix) {
    if (!enable_counters) {
        return false;
    }

    StatsdClientWrapper statsd(host, port, prefix);

    auto canon = counterState.canonicalize();
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

bool storeCountersToProtoFile(const std::string &fileName, const std::string &prefix, const std::string &repo,
                              const std::string &branch, const std::string &sha, const std::string &status) {
    if (!enable_counters) {
        return false;
    }
    com::stripe::payserver::events::cibot::SourceMetrics metrics;
    metrics.set_repo(repo);
    metrics.set_branch(branch);
    metrics.set_sha(sha);
    metrics.set_status(status);
    auto unix_timestamp = std::chrono::seconds(std::time(nullptr));
    metrics.set_timestamp(unix_timestamp.count());

    // UUID version 1 as specified in RFC 4122
    string uuid =
        strprintf("%lx-%x-%x-%x-%lx%x",
                  Random::uniformU8(), // Generates a 64-bit Hex number
                  Random::uniformU4(), // Generates a 32-bit Hex number
                  Random::uniformU4(0, 0x0fff) |
                      0x4000, // Generates a 32-bit Hex number of the form 4xxx (4 indicates the UUID version)
                  Random::uniformU4(0, 0x3fff) | 0x8000, // Generates a 32-bit Hex number in the range [0x8000, 0xbfff]
                  Random::uniformU8(), rand());          // Generates a 96-bit Hex number

    metrics.set_uuid(uuid);

    auto canon = counterState.canonicalize();
    for (auto &cat : canon.counters_by_category) {
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

    for (auto &hist : canon.histograms) {
        CounterImpl::CounterType sum = 0;
        for (auto &e : hist.second) {
            sum += e.second;
            com::stripe::payserver::events::cibot::SourceMetrics_SourceMetricEntry *metric = metrics.add_metrics();
            metric->set_name(absl::StrCat(prefix, ".", hist.first, ".", e.first));
            metric->set_value(e.second);
        }
        com::stripe::payserver::events::cibot::SourceMetrics_SourceMetricEntry *metric = metrics.add_metrics();
        metric->set_name(absl::StrCat(prefix, ".", hist.first, ".total"));
        metric->set_value(sum);
    }

    for (auto &e : canon.counters) {
        com::stripe::payserver::events::cibot::SourceMetrics_SourceMetricEntry *metric = metrics.add_metrics();
        metric->set_name(absl::StrCat(prefix, ".", e.first));
        metric->set_value(e.second);
    }

    std::string json_string;

    // Create a json_string from sr.
    google::protobuf::util::JsonPrintOptions options;
    options.add_whitespace = true;
    options.always_print_primitive_fields = true;
    options.preserve_proto_field_names = true;
    google::protobuf::util::MessageToJsonString(metrics, &json_string, options);
    FileOps::write(fileName, json_string);
    return true;
}

} // namespace core
}; // namespace ruby_typer
