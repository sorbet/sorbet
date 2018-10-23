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

const char *CounterImpl::internKey(const char *str) {
    auto it1 = this->strings_by_ptr.find(str);
    if (it1 != this->strings_by_ptr.end()) {
        return it1->second;
    }

    string_view view(str);
    auto it2 = this->strings_by_value.find(view);
    if (it2 != this->strings_by_value.end()) {
        this->strings_by_ptr[str] = it2->second;
        return it2->second;
    }

    this->strings_by_value[view] = str;
    this->strings_by_ptr[str] = str;
    return str;
}

void CounterImpl::histogramAdd(const char *histogram, int key, unsigned long value) {
    if (!enable_counters) {
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
    this->counters_by_category[category][counter] += value;
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

void CounterImpl::timingAdd(const char *metric, unsigned long nanos) {
    this->timings[metric].emplace_back(nanos);
}

thread_local CounterImpl counterState;

CounterState getAndClearThreadCounters() {
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
    for (auto &cat : cs.counters->counters_by_category) {
        for (auto &e : cat.second) {
            counterState.prodCategoryCounterAdd(cat.first, e.first, e.second);
        }
    }

    for (auto &hist : cs.counters->histograms) {
        for (auto &e : hist.second) {
            counterState.histogramAdd(hist.first, e.first, e.second);
        }
    }

    for (auto &e : cs.counters->counters) {
        counterState.prodCounterAdd(e.first, e.second);
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

void timingAdd(ConstExprStr measure, unsigned long nanos) {
    counterState.timingAdd(measure.str, nanos);
}

void prodCategoryCounterAdd(ConstExprStr category, ConstExprStr counter, unsigned long value) {
    counterState.prodCategoryCounterAdd(category.str, counter.str, value);
}

void histogramInc(ConstExprStr histogram, int key) {
    counterState.histogramAdd(histogram.str, key, 1);
}

void histogramAdd(ConstExprStr histogram, int key, unsigned long value) {
    counterState.histogramAdd(histogram.str, key, value);
}

const int MAX_WIDTH = 100;
const int PAD_LIMIT = 30;
const int MAX_STAT_WIDTH = MAX_WIDTH - PAD_LIMIT;
const float HIST_CUTOFF = 0.1;

void CounterImpl::canonicalize() {
    CounterImpl out;

    for (auto &cat : this->counters_by_category) {
        for (auto &e : cat.second) {
            out.prodCategoryCounterAdd(internKey(cat.first), internKey(e.first), e.second);
        }
    }

    for (auto &hist : this->histograms) {
        for (auto &e : hist.second) {
            out.histogramAdd(internKey(hist.first), e.first, e.second);
        }
    }

    for (auto &e : this->counters) {
        out.prodCounterAdd(internKey(e.first), e.second);
    }

    for (auto &e : this->timings) {
        for (auto v : e.second) {
            out.timingAdd(internKey(e.first), v);
        }
    }

    this->counters_by_category = move(out.counters_by_category);
    this->histograms = move(out.histograms);
    this->counters = move(out.counters);
    this->timings = move(out.timings);
}

const vector<string> Counters::ALL_COUNTERS = {"<all>"};
bool shouldShow(vector<string> &wantNames, string_view name) {
    if (wantNames == Counters::ALL_COUNTERS) {
        return true;
    }
    return absl::c_linear_search(wantNames, name);
}

constexpr double TIMING_TO_MSEC_MULTIPLIER = 0.000001;

string getCounterStatistics(vector<string> names) {
    counterState.canonicalize();

    fmt::memory_buffer buf;

    fmt::format_to(buf, "Counters and Histograms: \n");

    for (auto &cat : counterState.counters_by_category) {
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
        for (auto &e : hist.second) {
            sum += e.second;
        }
        fmt::format_to(buf, " {}\n{:<26.26} Total :{:10.10}\n", hist.first, "", (double)sum);

        CounterImpl::CounterType header = 0;
        auto it = hist.second.begin();
        auto cutoff = hist.second.size() > 30 ? HIST_CUTOFF : 0.0;
        while (it != hist.second.end() && (header + it->second) * 1.0 / sum < cutoff) {
            header += it->second;
            it++;
        }
        if (it != hist.second.begin()) {
            auto perc = header * 100.0 / sum;
            string hashes((int)(MAX_STAT_WIDTH * 1.0 * header / sum), '#');
            fmt::format_to(buf, "  <{:>29.29} :{:10.10}, {:5.1f}% {}\n", to_string(it->first), (double)header, perc,
                           hashes);
        }

        while (it != hist.second.end() && (sum - header) * 1.0 / sum > cutoff) {
            header += it->second;
            auto perc = it->second * 100.0 / sum;
            string hashes((int)(MAX_STAT_WIDTH * 1.0 * it->second / sum), '#');
            fmt::format_to(buf, "  {:>30.30} :{:10.10}, {:5.1f}% {}\n", to_string(it->first), (double)it->second, perc,
                           hashes);
            it++;
        }
        if (it != hist.second.end()) {
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
        for (auto &e : counterState.timings) {
            if (!shouldShow(names, e.first)) {
                continue;
            }
            if (e.second.size() == 1) {
                string line =
                    fmt::format("  {:>24.24}.value :{:10.10} ms\n", e.first, e.second[0] * TIMING_TO_MSEC_MULTIPLIER);
                sortedTimings.emplace_back(e.first, line);
                continue;
            }

            string line = fmt::format(
                "  {:>26.26}.min :{:10.10} ms\n  {:>26.26}.max :{:10.10} ms\n  {:>26.26}.avg :{:10.10} ms\n", e.first,
                *absl::c_min_element(e.second) * TIMING_TO_MSEC_MULTIPLIER, e.first,
                *absl::c_max_element(e.second) * TIMING_TO_MSEC_MULTIPLIER, e.first,
                absl::c_accumulate(e.second, 0) * TIMING_TO_MSEC_MULTIPLIER / e.second.size());
            sortedTimings.emplace_back(e.first, line);
        }
        fast_sort(sortedTimings, [](const auto &e1, const auto &e2) -> bool { return e1.first < e2.first; });

        fmt::format_to(buf, "{}", fmt::map_join(sortedTimings.begin(), sortedTimings.end(), "", [
                       ](const auto &el) -> auto { return el.second; }));
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

        fmt::format_to(buf, "{}", fmt::map_join(sortedOther.begin(), sortedOther.end(), "", [](const auto &el) -> auto {
                           return el.second;
                       }));
    }
    return to_string(buf);
}

}; // namespace sorbet
