#include "common/Counters.h"
#include "absl/strings/str_cat.h"
#include "common/Counters_impl.h"
#include <algorithm>
#include <chrono>
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
    this->histograms[histogram][key] += value;
}

void CounterImpl::categoryCounterAdd(const char *category, const char *counter, unsigned long value) {
    if (!enable_counters) {
        return;
    }
    prodCategoryCounterAdd(category, counter, value);
}

void CounterImpl::prodCategoryCounterAdd(const char *category, const char *counter, unsigned long value) {
    this->countersByCategory[category][counter] += value;
}

void CounterImpl::counterAdd(const char *counter, unsigned long value) {
    if (!enable_counters) {
        return;
    }
    prodCounterAdd(counter, value);
}

void CounterImpl::prodCounterAdd(const char *counter, unsigned long value) {
    this->counters[counter] += value;
}

void CounterImpl::timingAdd(CounterImpl::Timing timing) {
    this->timings.emplace_back(timing);
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

UnorderedMap<long, long> getAndClearHistogram(ConstExprStr histogram) {
    counterState.canonicalize();
    UnorderedMap<long, long> ret;
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
        counterState.timingAdd(e);
    }
}

void counterAdd(ConstExprStr counter, unsigned long value) {
    counterState.counterAdd(counter.str, value);
}

void counterInc(ConstExprStr counter) {
    counterAdd(counter, 1);
}

void prodCounterAdd(ConstExprStr counter, unsigned long value) {
    counterState.prodCounterAdd(counter.str, value);
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
    counterState.categoryCounterAdd(category.str, counter.str, value);
}

int getGlobalTimingId() {
    static atomic<int> counter = 0;
    return ++counter;
}

int genThreadId() {
    static atomic<int> counter{0};
    return ++counter;
}

int getThreadId() {
    thread_local static int counter = genThreadId();
    return counter;
}

vector<pair<const char *, string>> givenArgs2StoredArgs(initializer_list<pair<ConstExprStr, string_view>> given) {
    vector<pair<const char *, string>> stored;
    for (auto &e : given) {
        stored.emplace_back(e.first.str, (string)e.second);
    }
    return stored;
}

void timingAdd(ConstExprStr measure, unsigned long start, unsigned long end,
               initializer_list<pair<ConstExprStr, string_view>> args) {
    CounterImpl::Timing tim{
        0, measure.str, start, end - start, getThreadId(), givenArgs2StoredArgs(args), CounterImpl::Timing::Duration};
    counterState.timingAdd(tim);
}

FlowId timingAddFlowStart(ConstExprStr measure) {
    int id = getGlobalTimingId();
    CounterImpl::Timing tim{
        id,
        measure.str,
        static_cast<CounterImpl::CounterType>(
            chrono::duration_cast<chrono::nanoseconds>(chrono::steady_clock::now().time_since_epoch())
                .count()), // cast signed to unsigned. Will break if run with clock <1970
        nullopt,
        getThreadId(),
        {},
        CounterImpl::Timing::FlowStart};
    counterState.timingAdd(tim);
    FlowId ret{measure.str, id};
    return ret;
}

void timingAddFlowEnd(FlowId flowId) {
    CounterImpl::Timing tim{
        flowId.id,
        flowId.measure,
        static_cast<CounterImpl::CounterType>(
            chrono::duration_cast<chrono::nanoseconds>(chrono::steady_clock::now().time_since_epoch())
                .count()), // cast signed to unsigned. Will break if run with clock <1970
        nullopt,
        getThreadId(),
        {},
        CounterImpl::Timing::FlowEnd};
    counterState.timingAdd(tim);
}

void prodCategoryCounterAdd(ConstExprStr category, ConstExprStr counter, unsigned long value) {
    counterState.prodCategoryCounterAdd(category.str, counter.str, value);
}

void histogramInc(ConstExprStr histogram, int key) {
    histogramAdd(histogram, key, 1);
}

void histogramAdd(ConstExprStr histogram, int key, unsigned long value) {
    counterState.histogramAdd(histogram.str, key, value);
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
        out.timingAdd(e);
    }

    this->countersByCategory = std::move(out.countersByCategory);
    this->histograms = std::move(out.histograms);
    this->counters = std::move(out.counters);
    this->timings = std::move(out.timings);
}

const vector<string> Counters::ALL_COUNTERS = {"<all>"};
bool shouldShow(vector<string> &wantNames, string_view name) {
    if (wantNames == Counters::ALL_COUNTERS) {
        return true;
    }
    return absl::c_linear_search(wantNames, name);
}

constexpr double TIMING_TO_MSEC_MULTIPLIER = 0.001;

string getCounterStatistics(vector<string> names) {
    counterState.canonicalize();

    fmt::memory_buffer buf;

    fmt::format_to(buf, "Counters and Histograms: \n");

    for (auto &cat : counterState.countersByCategory) {
        if (!shouldShow(names, cat.first)) {
            continue;
        }
        CounterImpl::CounterType sum = 0;
        vector<pair<CounterImpl::CounterType, string>> sorted;
        for (auto &e : cat.second) {
            sum += e.second;
            sorted.emplace_back(e.second, e.first);
        }
        fast_sort(sorted, [](const auto &e1, const auto &e2) -> bool { return e1.first > e2.first; });

        fmt::format_to(buf, " {}\n{:<26.26} Total :{:10.10}\n", cat.first, "", (double)sum);

        if (sum == 0) {
            sum = 1;
        }
        for (auto &e : sorted) {
            auto perc = e.first * 100.0 / sum;
            string hashes((int)(MAX_STAT_WIDTH * 1.0 * e.first / sum), '#');
            fmt::format_to(buf, "  {:>30.30} :{:10.10}, {:3.1f}% {}\n", e.second, (double)e.first, perc, hashes);
        }
        fmt::format_to(buf, "\n");
    }

    for (auto &hist : counterState.histograms) {
        if (!shouldShow(names, hist.first)) {
            continue;
        }
        CounterImpl::CounterType sum = 0;
        vector<pair<int, CounterImpl::CounterType>> sorted;
        for (auto &e : hist.second) {
            sum += e.second;
            sorted.emplace_back(e);
        }
        fast_sort(sorted, [](const auto &e1, const auto &e2) -> bool { return e1.first < e2.first; });

        fmt::format_to(buf, " {}\n{:<26.26} Total :{:10.10}\n", hist.first, "", (double)sum);

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
            fmt::format_to(buf, "  <{:>29.29} :{:10.10}, {:5.1f}% {}\n", to_string(it->first), (double)header, perc,
                           hashes);
        }

        while (it != sorted.end() && (sum - header) * 1.0 / sum > cutoff) {
            header += it->second;
            auto perc = it->second * 100.0 / sum;
            string hashes((int)(MAX_STAT_WIDTH * 1.0 * it->second / sum), '#');
            fmt::format_to(buf, "  {:>30.30} :{:10.10}, {:5.1f}% {}\n", to_string(it->first), (double)it->second, perc,
                           hashes);
            it++;
        }
        if (it != sorted.end()) {
            auto perc = (sum - header) * 100.0 / sum;
            string hashes((int)(MAX_STAT_WIDTH * 1.0 * (sum - header) / sum), '#');
            fmt::format_to(buf, "  >={:>28.28} :{:10.10}, {:5.1f}% {}\n", to_string(it->first), (double)(sum - header),
                           perc, hashes);
        }
        fmt::format_to(buf, "\n");
    }

    {
        fmt::format_to(buf, "Timings: \n");
        vector<pair<string, string>> sortedTimings;
        UnorderedMap<string, vector<CounterImpl::CounterType>> timings;
        for (const auto &e : counterState.timings) {
            if (e.duration) {
                timings[e.measure].emplace_back(e.duration.value());
            }
        }
        for (const auto &e : timings) {
            if (!shouldShow(names, e.first)) {
                continue;
            }
            if (e.second.size() == 1) {
                string line =
                    fmt::format("  {:>24.24}.value :{:10.4} ms\n", e.first, e.second[0] * TIMING_TO_MSEC_MULTIPLIER);
                sortedTimings.emplace_back(e.first, line);
                continue;
            }

            string line =
                fmt::format("  {:>26.26}.min :{:10.4} ms\n  {:>26.26}.max :{:10.4} ms\n  {:>26.26}.avg :{:10.4} ms\n",
                            e.first, *absl::c_min_element(e.second) * TIMING_TO_MSEC_MULTIPLIER, e.first,
                            *absl::c_max_element(e.second) * TIMING_TO_MSEC_MULTIPLIER, e.first,
                            absl::c_accumulate(e.second, 0) * TIMING_TO_MSEC_MULTIPLIER / e.second.size());
            sortedTimings.emplace_back(e.first, line);
        }
        fast_sort(sortedTimings, [](const auto &e1, const auto &e2) -> bool { return e1.first < e2.first; });

        fmt::format_to(buf, "{}",
                       fmt::map_join(
                           sortedTimings, "", [](const auto &el) -> auto { return el.second; }));
    }

    {
        fmt::format_to(buf, "Counters: \n");

        vector<pair<string, string>> sortedOther;
        for (auto &e : counterState.counters) {
            if (!shouldShow(names, e.first)) {
                continue;
            }

            string line = fmt::format("  {:>30.30} :{:10.10}\n", e.first, (double)e.second);
            sortedOther.emplace_back(e.first, line);
        }

        fast_sort(sortedOther, [](const auto &e1, const auto &e2) -> bool { return e1.first < e2.first; });

        fmt::format_to(buf, "{}",
                       fmt::map_join(
                           sortedOther, "", [](const auto &el) -> auto { return el.second; }));
    }
    return to_string(buf);
}

}; // namespace sorbet
