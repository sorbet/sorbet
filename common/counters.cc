#include "counters.h"
#include <algorithm>
#include <cmath>
#include <iomanip> // setw
#include <map>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
using namespace std;

typedef unsigned long CounterType;

unordered_map<string, CounterType> counters;

void ruby_typer::counterInc(ConstExprStr counter) {
    if (!debug_mode) {
        return;
    }
    string key(counter.str, counter.size);
    counters[key]++;
}

unordered_map<string, unordered_map<string, CounterType>> counters_by_category;

void ruby_typer::categoryCounterInc(ConstExprStr category, ConstExprStr counter) {
    if (!debug_mode) {
        return;
    }

    string categoryKey(category.str, category.size);
    string key(counter.str, counter.size);
    counters_by_category[categoryKey][key]++;
}

unordered_map<string, map<int, CounterType>> histograms;

void ruby_typer::histogramInc(ConstExprStr histogram, int value) {
    if (!debug_mode) {
        return;
    }
    string key(histogram.str, histogram.size);
    histograms[key][value]++;
}

const int MAX_WIDTH = 100;
const int PAD_LIMIT = 20;
const int MAX_STAT_WIDTH = MAX_WIDTH - PAD_LIMIT;
const float HIST_CUTOFF = 0.05;

string padOrLimit(string s, int l) {
    if (s.size() < l) {
        string prefix(l - s.size(), ' ');
        s = prefix + s;
    } else {
        s = s.substr(0, l);
    }
    return s;
}

string ruby_typer::getCounterStatistics() {
    if (!debug_mode) {
        return "Statistics collection is not available in production build. Use debug build.";
    }
    stringstream buf;

    buf << "Counters: " << endl;
    for (auto &cat : counters_by_category) {
        buf << " " << cat.first << endl;
        CounterType sum = 0;
        std::vector<pair<CounterType, string>> sorted;
        for (auto &e : cat.second) {
            sum += e.second;
            sorted.emplace_back(e.second, e.first);
        }
        sort(sorted.begin(), sorted.end(), [](const auto &e1, const auto &e2) -> bool { return e1.first > e2.first; });

        for (auto &e : sorted) {
            string number = padOrLimit(to_string(e.first), 6);
            string perc = padOrLimit(to_string(round(e.first * 1000.0 / sum) / 10.0), 3);
            string hashes((int)(MAX_STAT_WIDTH * 1.0 * e.first / sum), '#');
            buf << "  " << padOrLimit(e.second, PAD_LIMIT - 4) << " :" << number << ", " << perc << "% " << hashes
                << endl;
        }
        buf << endl;
    }

    buf << "\nHistograms: " << endl;
    for (auto &hist : histograms) {
        buf << " " << hist.first << endl;
        CounterType sum = 0;
        for (auto &e : hist.second) {
            sum += e.second;
        }
        CounterType header = 0;
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
                << hashes << endl;
        }

        while (it != hist.second.end() && (sum - header) * 1.0 / sum > HIST_CUTOFF) {
            header += it->second;
            string number = padOrLimit(to_string(it->second), 6);
            string perc = padOrLimit(to_string(round(it->second * 1000.0 / sum) / 10.0), 3);
            string hashes((int)(MAX_STAT_WIDTH * 1.0 * it->second / sum), '#');
            buf << "  " << padOrLimit(to_string(it->first), PAD_LIMIT - 4) << " :" << number << ", " << perc << "% "
                << hashes << endl;
            it++;
        }
        if (it != hist.second.end()) {
            string number = padOrLimit(to_string(sum - header), 6);
            string perc = padOrLimit(to_string(round((sum - header) * 1000.0 / sum) / 10.0), 3);
            string hashes((int)(MAX_STAT_WIDTH * 1.0 * (sum - header) / sum), '#');
            buf << ">=" << padOrLimit(to_string(it->first), PAD_LIMIT - 4) << " :" << number << ", " << perc << "% "
                << hashes << endl;
        }
        buf << endl;
    }

    buf << "\nOther:\n";
    for (auto &e : counters) {
        string number = to_string(e.second);
        if (number.size() < 6) {
            number = padOrLimit(number, 6);
        }
        buf << "  " << padOrLimit(e.first, PAD_LIMIT - 4) << " :" << number << endl;
    }

    return buf.str();
}
