#include "core/Counters.h"
#include "absl/strings/str_cat.h"
#include "core/Counters_impl.h"
#include "core/Names.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <iomanip> // setw
#include <map>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

namespace ruby_typer {
namespace core {
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

void CounterImpl::histogramAdd(const char *histogram, int key, unsigned int value) {
    if (!enable_counters) {
        return;
    }
    this->histograms[histogram][key] += value;
}

void CounterImpl::categoryCounterAdd(const char *category, const char *counter, unsigned int value) {
    if (!enable_counters) {
        return;
    }

    this->counters_by_category[category][counter] += value;
}

void CounterImpl::counterAdd(const char *counter, unsigned int value, bool isProdCounter) {
    if (!enable_counters && !isProdCounter) {
        return;
    }
    this->counters[counter] += value;
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

void counterConsume(CounterState cs) {
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
        counterState.counterAdd(e.first, e.second, true);
    }
}

void counterAdd(ConstExprStr counter, unsigned int value) {
    counterState.counterAdd(counter.str, value, false);
}

void counterInc(ConstExprStr counter) {
    counterAdd(counter, 1);
}

void prodCounterAdd(ConstExprStr counter, unsigned int value) {
    counterState.counterAdd(counter.str, value, true);
}

void prodCounterInc(ConstExprStr counter) {
    prodCounterAdd(counter, 1);
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
        out.counterAdd(internKey(e.first), e.second, true);
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
        if (sum == 0) {
            sum = 1;
        }
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
        auto cutoff = hist.second.size() > 30 ? HIST_CUTOFF : 0.0;
        while (it != hist.second.end() && (header + it->second) * 1.0 / sum < cutoff) {
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

        while (it != hist.second.end() && (sum - header) * 1.0 / sum > cutoff) {
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

} // namespace core
}; // namespace ruby_typer
