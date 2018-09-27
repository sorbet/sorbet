#include "common/Counters.h"
#include "absl/algorithm/container.h"
#include "absl/strings/str_cat.h"
#include "common/Counters_impl.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <iomanip> // set
#include <map>
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

void CounterImpl::timingAdd(const char *metric, unsigned long millis) {
    this->timings[metric].emplace_back(millis);
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

map<long, long> getAndClearHistogram(ConstExprStr histogram) {
    counterState.canonicalize();
    map<long, long> ret;
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

string padOrLimit(string s, int l) {
    if (s.size() < l) {
        string prefix(l - s.size(), ' ');
        s = prefix + s;
    } else {
        s = s.substr(0, l);
    }
    return s;
}

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
bool shouldShow(vector<string> &wantNames, string name) {
    if (wantNames == Counters::ALL_COUNTERS) {
        return true;
    }
    return absl::c_linear_search(wantNames, name);
}

string getCounterStatistics(vector<string> names) {
    stringstream buf;

    buf << "Counters and Histograms: " << '\n';

    counterState.canonicalize();

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
        absl::c_sort(sorted, [](const auto &e1, const auto &e2) -> bool { return e1.first > e2.first; });

        buf << " " << cat.first << "    Total: " << sum << '\n';
        if (sum == 0) {
            sum = 1;
        }
        for (auto &e : sorted) {
            string number = to_string(e.first);
            if (number.size() < 6) {
                number = padOrLimit(number, 6);
            }
            string perc = padOrLimit(to_string(round(e.first * 1000.0 / sum) / 10.0), 3);
            string hashes((int)(MAX_STAT_WIDTH * 1.0 * e.first / sum), '#');
            buf << "  " << padOrLimit(e.second, PAD_LIMIT - 4) << " :" << number << ", " << perc << "% " << hashes
                << '\n';
        }
        buf << '\n';
    }

    for (auto &hist : counterState.histograms) {
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
        auto cutoff = hist.second.size() > 30 ? HIST_CUTOFF : 0.0;
        while (it != hist.second.end() && (header + it->second) * 1.0 / sum < cutoff) {
            header += it->second;
            it++;
        }
        if (it != hist.second.begin()) {
            string number = to_string(header);
            if (number.size() < 6) {
                number = padOrLimit(number, 6);
            }
            string perc = padOrLimit(to_string(round(header * 1000.0 / sum) / 10.0), 3);
            string hashes((int)(MAX_STAT_WIDTH * 1.0 * header / sum), '#');
            buf << " <" << padOrLimit(to_string(it->first), PAD_LIMIT - 4) << " :" << number << ", " << perc << "% "
                << hashes << '\n';
        }

        while (it != hist.second.end() && (sum - header) * 1.0 / sum > cutoff) {
            header += it->second;
            string number = to_string(it->second);
            if (number.size() < 6) {
                number = padOrLimit(number, 6);
            }
            string perc = padOrLimit(to_string(round(it->second * 1000.0 / sum) / 10.0), 3);
            string hashes((int)(MAX_STAT_WIDTH * 1.0 * it->second / sum), '#');
            buf << "  " << padOrLimit(to_string(it->first), PAD_LIMIT - 4) << " :" << number << ", " << perc << "% "
                << hashes << '\n';
            it++;
        }
        if (it != hist.second.end()) {
            string number = to_string(sum - header);
            if (number.size() < 6) {
                number = padOrLimit(number, 6);
            }
            string perc = padOrLimit(to_string(round((sum - header) * 1000.0 / sum) / 10.0), 3);
            string hashes((int)(MAX_STAT_WIDTH * 1.0 * (sum - header) / sum), '#');
            buf << ">=" << padOrLimit(to_string(it->first), PAD_LIMIT - 4) << " :" << number << ", " << perc << "% "
                << hashes << '\n';
        }
        buf << '\n';
    }

    {
        buf << "Timings: " << '\n';
        vector<pair<string, string>> sortedTimings;
        for (auto &e : counterState.timings) {
            if (!shouldShow(names, e.first)) {
                continue;
            }
            if (e.second.size() == 1) {
                string value = to_string(e.second[0]);
                if (value.size() < 10) {
                    value = padOrLimit(value, 10);
                }
                string line = "  " + padOrLimit(e.first, PAD_LIMIT - 4) + ".value :" + value + "\n";
                sortedTimings.emplace_back(e.first, line);
                continue;
            }

            string min = to_string(*absl::c_min_element(e.second));
            string max = to_string(*absl::c_max_element(e.second));
            string avg = to_string(absl::c_accumulate(e.second, 0) / e.second.size());
            if (min.size() < 10) {
                min = padOrLimit(min, 10);
            }
            if (max.size() < 10) {
                max = padOrLimit(max, 10);
            }
            if (avg.size() < 10) {
                avg = padOrLimit(avg, 10);
            }
            string line = "  " + padOrLimit(e.first, PAD_LIMIT - 4) + ".min :" + min + "\n" + "  " +
                          padOrLimit(e.first, PAD_LIMIT - 4) + ".max :" + max + "\n" + "  " +
                          padOrLimit(e.first, PAD_LIMIT - 4) + ".avg :" + avg + "\n";
            sortedTimings.emplace_back(e.first, line);
        }
        absl::c_sort(sortedTimings, [](const auto &e1, const auto &e2) -> bool { return e1.first < e2.first; });

        for (auto &e : sortedTimings) {
            buf << e.second;
        }
    }

    {
        buf << "Counters: " << '\n';
        vector<pair<string, string>> sortedOther;
        for (auto &e : counterState.counters) {
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

        absl::c_sort(sortedOther, [](const auto &e1, const auto &e2) -> bool { return e1.first < e2.first; });

        for (auto &e : sortedOther) {
            buf << e.second;
        }
    }
    return buf.str();
}

}; // namespace sorbet
