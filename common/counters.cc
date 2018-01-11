#include "counters.h"
extern "C" {
#include "statsd-client.h"
}
#include <algorithm>
#include <cmath>
#include <iomanip> // setw
#include <map>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

static constexpr bool enable_counters = ruby_typer::debug_mode;

thread_local ruby_typer::CounterState counterState;

ruby_typer::CounterState ruby_typer::getAndClearThreadCounters() {
    if (!enable_counters) {
        ruby_typer::CounterState empty;
        return empty;
    }
    ruby_typer::CounterState result{move(counterState.histograms), move(counterState.counters),
                                    move(counterState.counters_by_category)};
    counterState.histograms.clear();
    counterState.counters.clear();
    counterState.counters_by_category.clear();
    return result;
}

void ruby_typer::counterConsume(ruby_typer::CounterState cs) {
    if (!enable_counters) {
        return;
    }
    for (auto &cat : counterState.counters_by_category) {
        for (auto &e : cat.second) {
            ruby_typer::categoryCounterAdd(cat.first, e.first, e.second);
        }
    }

    for (auto &hist : counterState.histograms) {
        for (auto &e : hist.second) {
            ruby_typer::histogramAdd(hist.first, e.first, e.second);
        }
    }

    for (auto &e : counterState.counters) {
        ruby_typer::counterAdd(e.first, e.second);
    }
}

void ruby_typer::counterAdd(ConstExprStr counter, unsigned int value) {
    if (!enable_counters) {
        return;
    }
    string key(counter.str, counter.size);
    counterState.counters[key] += value;
}

void ruby_typer::counterInc(ConstExprStr counter) {
    counterAdd(counter, 1);
}

void ruby_typer::categoryCounterInc(ConstExprStr category, ConstExprStr counter) {
    categoryCounterAdd(category, counter, 1);
}

void ruby_typer::categoryCounterAdd(ConstExprStr category, ConstExprStr counter, unsigned int value) {
    if (!enable_counters) {
        return;
    }

    string categoryKey(category.str, category.size);
    string key(counter.str, counter.size);
    counterState.counters_by_category[categoryKey][key] += value;
}

void ruby_typer::histogramInc(ConstExprStr histogram, int key) {
    histogramAdd(histogram, key, 1);
}

void ruby_typer::histogramAdd(ConstExprStr histogram, int key, unsigned int value) {
    if (!enable_counters) {
        return;
    }
    string skey(histogram.str, histogram.size);
    counterState.histograms[skey][key] += value;
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
    if (!enable_counters) {
        return "Statistics collection is not available in production build. Use debug build.";
    }
    stringstream buf;

    buf << "Counters: " << endl;
    for (auto &cat : counterState.counters_by_category) {
        CounterState::CounterType sum = 0;
        std::vector<pair<CounterState::CounterType, string>> sorted;
        for (auto &e : cat.second) {
            sum += e.second;
            sorted.emplace_back(e.second, e.first);
        }
        sort(sorted.begin(), sorted.end(), [](const auto &e1, const auto &e2) -> bool { return e1.first > e2.first; });

        buf << " " << cat.first << "    Total: " << sum << endl;

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
    for (auto &hist : counterState.histograms) {
        CounterState::CounterType sum = 0;
        for (auto &e : hist.second) {
            sum += e.second;
        }
        buf << " " << hist.first << "    Total: " << sum << endl;

        CounterState::CounterType header = 0;
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
    {
        vector<pair<string, string>> sortedOther;
        for (auto &e : counterState.counters) {
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

bool ruby_typer::submitCountersToStatsd(std::string host, int port, std::string prefix) {
    if (!enable_counters)
        return false;

    StatsdClientWrapper statsd(host, port, prefix);

    for (auto &cat : counterState.counters_by_category) {
        CounterState::CounterType sum = 0;
        for (auto &e : cat.second) {
            sum += e.second;
            statsd.gauge(cat.first + "." + e.first, e.second);
        }

        statsd.gauge(cat.first + ".total", sum);
    }

    for (auto &hist : counterState.histograms) {
        CounterState::CounterType sum = 0;
        for (auto &e : hist.second) {
            sum += e.second;
            statsd.gauge(hist.first + "." + to_string(e.first), e.second);
        }

        statsd.gauge(hist.first + ".total", sum);
    }

    for (auto &e : counterState.counters) {
        statsd.gauge(e.first, e.second);
    }

    return true;
}
