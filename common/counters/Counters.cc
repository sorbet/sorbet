#include "common/counters/Counters.h"
#include "absl/strings/str_cat.h"
#include "common/counters/Counters_impl.h"
#include "common/exception/Exception.h"
#include "common/sort/sort.h"
#include "common/strings/formatting.h"
#include <algorithm>
#include <cmath>
#include <iomanip> // set
#include <sstream>
#include <string>
#include <vector>

using namespace std;

namespace sorbet {
CounterState::CounterState(unique_ptr<CounterImpl> counters) : counters(move(counters)) {}

CounterState::CounterState() = default;
CounterState::~CounterState() = default;
CounterState::CounterState(CounterState &&rhs) = default;
CounterState &CounterState::operator=(CounterState &&rhs) = default;

bool CounterState::hasNullCounters() const {
    return counters.get() == nullptr;
}

const char *CounterImpl::internKey(const char *str) {
    auto it1 = this->stringsByPtr.find(str);
    if (it1 != this->stringsByPtr.end()) {
        return it1->second;
    }

    string_view view(str);
    auto it2 = this->strings_by_value.find(view);
    if (it2 != this->strings_by_value.end()) {
        this->stringsByPtr[str] = it2->second;
        return it2->second;
    }

    this->strings_by_value[view] = str;
    this->stringsByPtr[str] = str;
    return str;
}

void CounterImpl::histogramAdd(const char *histogram, int key, unsigned long value) {
    if (!enable_counters) {
        return;
    }
    prodHistogramAdd(histogram, key, value);
}

void CounterImpl::prodHistogramAdd(const char *histogram, int key, unsigned long value) {
    if (fuzz_mode) {
        return;
    }
    this->histograms[histogram][key] += value;
}

void CounterImpl::categoryCounterAdd(const char *category, const char *counter, unsigned long value) {
    if (!enable_counters) {
        return;
    }
    prodCategoryCounterAdd(category, counter, value);
}

void CounterImpl::prodCategoryCounterAdd(const char *category, const char *counter, unsigned long value) {
    if (fuzz_mode) {
        return;
    }
    this->countersByCategory[category][counter] += value;
}

void CounterImpl::counterAdd(const char *counter, unsigned long value) {
    if (!enable_counters) {
        return;
    }
    prodCounterAdd(counter, value);
}

void CounterImpl::prodCounterAdd(const char *counter, unsigned long value) {
    if (fuzz_mode) {
        return;
    }
    this->counters[counter] += value;
}

void CounterImpl::prodCounterSet(const char *counter, unsigned long value) {
    if (fuzz_mode) {
        return;
    }
    this->counters[counter] = value;
}

void CounterImpl::timingAdd(CounterImpl::Timing timing) {
    if (fuzz_mode) {
        return;
    }
    this->timings.emplace_back(move(timing));
}

thread_local CounterImpl counterState;

CounterState getAndClearThreadCounters() {
    auto state = make_unique<CounterImpl>(move(counterState));
    counterState.clear();
    return CounterState(move(state));
}

void CounterImpl::clear() {
    this->strings_by_value.clear();
    this->stringsByPtr.clear();
    this->histograms.clear();
    this->counters.clear();
    this->countersByCategory.clear();
}

absl::flat_hash_map<long, long> getAndClearHistogram(ConstExprStr histogram) {
    counterState.canonicalize();
    absl::flat_hash_map<long, long> ret;
    auto fnd = counterState.histograms.find(counterState.internKey(histogram.str));
    if (fnd != counterState.histograms.end()) {
        for (auto e : fnd->second) {
            ret[e.first] = e.second;
        }

        counterState.histograms.erase(fnd);
        return ret;
    }
    return ret;
}

void counterConsume(CounterState cs) {
    for (auto &cat : cs.counters->countersByCategory) {
        for (auto &e : cat.second) {
            counterState.prodCategoryCounterAdd(cat.first, e.first, e.second);
        }
    }

    for (auto &hist : cs.counters->histograms) {
        for (auto &e : hist.second) {
            counterState.prodHistogramAdd(hist.first, e.first, e.second);
        }
    }

    for (auto &e : cs.counters->counters) {
        counterState.prodCounterAdd(e.first, e.second);
    }
    for (auto &e : cs.counters->timings) {
        counterState.timingAdd(move(e));
    }
}

CounterState mergeCounters(CounterState counters) {
    if (!counters.hasNullCounters()) {
        counterConsume(move(counters));
    }
    return getAndClearThreadCounters();
}

void counterAdd(ConstExprStr counter, unsigned long value) {
    if constexpr (enable_counters) {
        counterState.counterAdd(counter.str, value);
    }
}

void counterInc(ConstExprStr counter) {
    counterAdd(counter, 1);
}

void prodCounterAdd(ConstExprStr counter, unsigned long value) {
    counterState.prodCounterAdd(counter.str, value);
}

void prodCounterSet(ConstExprStr counter, unsigned long value) {
    counterState.prodCounterSet(counter.str, value);
}

void prodCounterInc(ConstExprStr counter) {
    prodCounterAdd(counter, 1);
}

void categoryCounterInc(ConstExprStr category, ConstExprStr counter) {
    categoryCounterAdd(category, counter, 1);
}

void prodCategoryCounterInc(ConstExprStr category, ConstExprStr counter) {
    prodCategoryCounterAdd(category, counter, 1);
}

void categoryCounterAdd(ConstExprStr category, ConstExprStr counter, unsigned long value) {
    if constexpr (enable_counters) {
        counterState.categoryCounterAdd(category.str, counter.str, value);
    }
}

int genThreadId() {
    static atomic<int> counter{0};
    return ++counter;
}

int getThreadId() {
    thread_local static int counter = genThreadId();
    return counter;
}

unique_ptr<vector<pair<const char *, string>>>
givenArgs2StoredArgs(unique_ptr<vector<pair<ConstExprStr, string>>> given) {
    if (given == nullptr) {
        return nullptr;
    }

    auto stored = make_unique<vector<pair<const char *, string>>>();
    for (auto &e : *given) {
        stored->emplace_back(e.first.str, move(e.second));
    }
    return stored;
}

unique_ptr<vector<pair<const char *, const char *>>>
givenTags2StoredTags(unique_ptr<vector<pair<ConstExprStr, ConstExprStr>>> given) {
    if (given == nullptr) {
        return nullptr;
    }

    auto stored = make_unique<vector<pair<const char *, const char *>>>();
    for (auto &e : *given) {
        stored->emplace_back(e.first.str, e.second.str);
    }
    return stored;
}

void timingAdd(ConstExprStr measure, microseconds start, microseconds end,
               unique_ptr<vector<pair<ConstExprStr, string>>> args,
               unique_ptr<vector<pair<ConstExprStr, ConstExprStr>>> tags, FlowId self, FlowId previous,
               unique_ptr<vector<int>> histogramBuckets) {
    // Can't use ENFORCE, because each ENFORCE creates a new Timer.
    if (!(self.id == 0 || previous.id == 0)) {
        // see "case 1" in https://docs.google.com/document/d/1La_0PPfsTqHJihazYhff96thhjPtvq1KjAUOJu0dvEg/edit?pli=1#
        // for workaround
        Exception::raise("format doesn't support chaining");
    }

    CounterImpl::Timing tim{0,
                            measure.str,
                            start,
                            end,
                            getThreadId(),
                            givenArgs2StoredArgs(move(args)),
                            givenTags2StoredTags(move(tags)),
                            self,
                            previous};
    counterState.timingAdd(move(tim));

    if (histogramBuckets != nullptr && !histogramBuckets->empty()) {
        histogramBuckets->push_back(INT_MAX);
        fast_sort(*histogramBuckets);

        auto msCount = (end.usec - start.usec) / 1'000;
        // Find the bucket for this value. The last bucket is INT_MAX, so it's guaranteed to pick one if a histogram
        // is set. If histogramBuckets is empty (which is the common case), then nothing happens.
        for (const auto bucket : *histogramBuckets) {
            if (msCount < bucket) {
                prodHistogramInc(measure, bucket);
                break;
            }
        }
    }
}

void prodCategoryCounterAdd(ConstExprStr category, ConstExprStr counter, unsigned long value) {
    counterState.prodCategoryCounterAdd(category.str, counter.str, value);
}

void histogramInc(ConstExprStr histogram, int key) {
    histogramAdd(histogram, key, 1);
}

void histogramAdd(ConstExprStr histogram, int key, unsigned long value) {
    if constexpr (enable_counters) {
        counterState.histogramAdd(histogram.str, key, value);
    }
}

void prodHistogramInc(ConstExprStr histogram, int key) {
    prodHistogramAdd(histogram, key, 1);
}

void prodHistogramAdd(ConstExprStr histogram, int key, unsigned long value) {
    counterState.prodHistogramAdd(histogram.str, key, value);
}

const int MAX_WIDTH = 100;
const int PAD_LIMIT = 30;
const int MAX_STAT_WIDTH = MAX_WIDTH - PAD_LIMIT;
const float HIST_CUTOFF = 0.1;

void CounterImpl::canonicalize() {
    CounterImpl out;

    for (auto &cat : this->countersByCategory) {
        for (auto &e : cat.second) {
            out.prodCategoryCounterAdd(internKey(cat.first), internKey(e.first), e.second);
        }
    }

    for (auto &hist : this->histograms) {
        for (auto &e : hist.second) {
            out.prodHistogramAdd(internKey(hist.first), e.first, e.second);
        }
    }

    for (auto &e : this->counters) {
        out.prodCounterAdd(internKey(e.first), e.second);
    }

    for (auto &e : this->timings) {
        out.timingAdd(move(e));
    }

    this->countersByCategory = std::move(out.countersByCategory);
    this->histograms = std::move(out.histograms);
    this->counters = std::move(out.counters);
    this->timings = std::move(out.timings);
}

string getCounterStatistics() {
    counterState.canonicalize();

    fmt::memory_buffer buf;

    fmt::format_to(std::back_inserter(buf), "Counters and Histograms: \n");

    for (auto &cat : counterState.countersByCategory) {
        CounterImpl::CounterType sum = 0;
        vector<pair<CounterImpl::CounterType, string>> sorted;
        for (auto &e : cat.second) {
            sum += e.second;
            sorted.emplace_back(e.second, e.first);
        }
        fast_sort(sorted, [](const auto &e1, const auto &e2) -> bool { return e1.first > e2.first; });

        fmt::format_to(std::back_inserter(buf), " {}\n{:<36.36} Total :{:15}\n", cat.first, "", sum);

        if (sum == 0) {
            sum = 1;
        }
        for (auto &e : sorted) {
            auto perc = e.first * 100.0 / sum;
            string hashes((int)(MAX_STAT_WIDTH * 1.0 * e.first / sum), '#');
            fmt::format_to(std::back_inserter(buf), "  {:>40.40} :{:15}, {:3.1f}% {}\n", e.second, e.first, perc,
                           hashes);
        }
        fmt::format_to(std::back_inserter(buf), "\n");
    }

    for (auto &hist : counterState.histograms) {
        CounterImpl::CounterType sum = 0;
        vector<pair<int, CounterImpl::CounterType>> sorted;
        for (auto &e : hist.second) {
            sum += e.second;
            sorted.emplace_back(e);
        }
        fast_sort(sorted, [](const auto &e1, const auto &e2) -> bool { return e1.first < e2.first; });

        fmt::format_to(std::back_inserter(buf), " {}\n{:<36.36} Total :{:15}\n", hist.first, "", sum);

        CounterImpl::CounterType header = 0;
        auto it = sorted.begin();
        auto cutoff = sorted.size() > 30 ? HIST_CUTOFF : 0.0;
        while (it != sorted.end() && (header + it->second) * 1.0 / sum < cutoff) {
            header += it->second;
            it++;
        }
        if (it != sorted.begin()) {
            auto perc = header * 100.0 / sum;
            string hashes((int)(MAX_STAT_WIDTH * 1.0 * header / sum), '#');
            fmt::format_to(std::back_inserter(buf), "  <{:>39.39} :{:15}, {:5.1f}% {}\n", to_string(it->first), header,
                           perc, hashes);
        }

        while (it != sorted.end() && (sum - header) * 1.0 / sum > cutoff) {
            header += it->second;
            auto perc = it->second * 100.0 / sum;
            string hashes((int)(MAX_STAT_WIDTH * 1.0 * it->second / sum), '#');
            fmt::format_to(std::back_inserter(buf), "  {:>40.40} :{:15}, {:5.1f}% {}\n", to_string(it->first),
                           it->second, perc, hashes);
            it++;
        }
        if (it != sorted.end()) {
            auto perc = (sum - header) * 100.0 / sum;
            string hashes((int)(MAX_STAT_WIDTH * 1.0 * (sum - header) / sum), '#');
            fmt::format_to(std::back_inserter(buf), "  >={:>38.38} :{:15}, {:5.1f}% {}\n", to_string(it->first),
                           sum - header, perc, hashes);
        }
        fmt::format_to(std::back_inserter(buf), "\n");
    }

    {
        fmt::format_to(std::back_inserter(buf), "Timings: \n");
        vector<pair<string, string>> sortedTimings;
        absl::flat_hash_map<string, vector<double>> timings;
        for (const auto &e : counterState.timings) {
            int64_t durationMs = (e.end.usec - e.start.usec) / 1'000;
            timings[e.measure].emplace_back(durationMs);
        }
        for (const auto &e : timings) {
            if (e.second.size() == 1) {
                string line = fmt::format("  {:>34.34}.value :{:15.4} ms\n", e.first, e.second[0]);
                sortedTimings.emplace_back(e.first, line);
                continue;
            }

            // string line = fmt::format("  {:>26.26}.min :{:10.4} ms\n  {:>26.26}.max :{:10.4} ms\n\n", e.first,
            //                           *absl::c_min_element(e.second), e.first, *absl::c_max_element(e.second));

            string line = fmt::format(
                "  {:>36.36}.min :{:15.2f} ms\n  {:>36.36}.max :{:15.2f} ms\n  {:>36.36}.avg :{:15.2f} ms\n", e.first,
                *absl::c_min_element(e.second), e.first, *absl::c_max_element(e.second), e.first,
                absl::c_accumulate(e.second, 0.0) / e.second.size());
            sortedTimings.emplace_back(e.first, line);
        }
        fast_sort(sortedTimings, [](const auto &e1, const auto &e2) -> bool { return e1.first < e2.first; });

        fmt::format_to(std::back_inserter(buf), "{}",
                       fmt::map_join(
                           sortedTimings, "", [](const auto &el) -> auto{ return el.second; }));
    }

    {
        fmt::format_to(std::back_inserter(buf), "Counters: \n");

        vector<pair<string, string>> sortedOther;
        for (auto &e : counterState.counters) {
            string line = fmt::format("  {:>40.40} :{:15}\n", e.first, e.second);
            sortedOther.emplace_back(e.first, line);
        }

        fast_sort(sortedOther, [](const auto &e1, const auto &e2) -> bool { return e1.first < e2.first; });

        fmt::format_to(std::back_inserter(buf), "{}",
                       fmt::map_join(
                           sortedOther, "", [](const auto &el) -> auto{ return el.second; }));
    }
    return to_string(buf);
}

}; // namespace sorbet
