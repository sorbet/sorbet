#include "doctest/doctest.h"
// ^ Violates linting rules, so include first.
#include "test/helpers/CounterStateDatabase.h"

namespace sorbet::test::lsp {
using namespace std;
CounterStateDatabase::CounterStateDatabase(CounterState counters) : counters(move(counters)) {
    CHECK_FALSE(this->counters.hasNullCounters());
    // Combines counters with the same name but different char* pointers.
    this->counters.counters->canonicalize();
}

// Get counter value or 0.
CounterImpl::CounterType CounterStateDatabase::getCounter(ConstExprStr counter) const {
    auto internedCounter = counters.counters->internKey(counter.str);
    const auto &it = counters.counters->counters.find(internedCounter);
    if (it != counters.counters->counters.end()) {
        return it->second;
    }
    return 0;
}

CounterImpl::CounterType CounterStateDatabase::getCategoryCounter(ConstExprStr counter, ConstExprStr category) const {
    auto internedCounter = counters.counters->internKey(counter.str);
    const auto &it = counters.counters->countersByCategory.find(internedCounter);
    if (it == counters.counters->countersByCategory.end()) {
        return 0;
    }
    auto internedCategory = counters.counters->internKey(category.str);
    const auto &catIt = it->second.find(internedCategory);
    if (catIt == it->second.end()) {
        return 0;
    }
    return catIt->second;
}

CounterImpl::CounterType CounterStateDatabase::getCategoryCounterSum(ConstExprStr counter) const {
    auto internedCounter = counters.counters->internKey(counter.str);
    const auto &it = counters.counters->countersByCategory.find(internedCounter);
    CounterImpl::CounterType total = 0;
    if (it != counters.counters->countersByCategory.end()) {
        for (const auto &catIt : it->second) {
            total += catIt.second;
        }
    }
    return total;
}

CounterImpl::CounterType CounterStateDatabase::getHistogramCount(ConstExprStr histogram) const {
    auto internedHistogram = counters.counters->internKey(histogram.str);
    CounterImpl::CounterType rv = 0;
    const auto &it = counters.counters->histograms.find(internedHistogram);
    if (it != counters.counters->histograms.end()) {
        const auto &map = it->second;
        for (const auto &entry : map) {
            rv += entry.second;
        }
    }
    return rv;
}

vector<unique_ptr<CounterImpl::Timing>>
CounterStateDatabase::getTimings(ConstExprStr counter, vector<pair<ConstExprStr, ConstExprStr>> tags) const {
    // Note: Timers don't have interned names.
    vector<unique_ptr<CounterImpl::Timing>> rv;
    for (const auto &timing : counters.counters->timings) {
        auto timing_tags_size = timing.tags == nullptr ? 0 : timing.tags->size();
        if (strncmp(timing.measure, counter.str, counter.size + 1) == 0 && timing_tags_size >= tags.size()) {
            absl::flat_hash_map<std::string, const char *> timingTags;
            if (timing.tags != nullptr) {
                for (const auto &tag : *timing.tags) {
                    timingTags[tag.first] = tag.second;
                }
            }

            int tagsMatched = 0;
            for (const auto &tag : tags) {
                auto it = timingTags.find(tag.first.str);
                if (it == timingTags.end()) {
                    break;
                }
                if (strncmp(it->second, tag.second.str, tag.second.size + 1) != 0) {
                    break;
                }
                tagsMatched++;
            }

            if (tagsMatched == tags.size()) {
                unique_ptr<vector<pair<char const *, string>>> copiedArgs;
                if (timing.args != nullptr) {
                    copiedArgs = make_unique<vector<pair<char const *, string>>>(*timing.args);
                }

                unique_ptr<vector<pair<char const *, char const *>>> copiedTags;
                if (timing.tags != nullptr) {
                    copiedTags = make_unique<vector<pair<char const *, char const *>>>(*timing.tags);
                }

                CounterImpl::Timing copied{timing.id,        timing.measure,  timing.start,
                                           timing.end,       timing.threadId, move(copiedArgs),
                                           move(copiedTags), timing.self,     timing.prev};
                rv.emplace_back(make_unique<CounterImpl::Timing>(move(copied)));
            }
        }
    }
    return rv;
}
} // namespace sorbet::test::lsp
