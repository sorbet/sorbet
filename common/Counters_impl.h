#ifndef SORBET_COUNTERS_IMPL_H
#define SORBET_COUNTERS_IMPL_H

#include "common/common.h"
#include <string_view>

namespace sorbet {
struct CounterImpl {
    CounterImpl() = default;
    CounterImpl(CounterImpl const &) = delete;
    CounterImpl(CounterImpl &&) = default;
    CounterImpl &operator=(CounterImpl &&) = delete;
    using CounterType = unsigned long;

    void canonicalize();
    void clear();

    const char *internKey(const char *str);

    void histogramAdd(const char *histogram, int key, unsigned long value);
    void prodHistogramAdd(const char *histogram, int key, unsigned long value);

    void categoryCounterAdd(const char *category, const char *counter, unsigned long value);
    void prodCategoryCounterAdd(const char *category, const char *counter, unsigned long value);

    void counterAdd(const char *counter, unsigned long value);
    void prodCounterAdd(const char *counter, unsigned long value);
    void timingAdd(const char *metring, unsigned long nanos);

    // std::string_view isn't hashable, so we use an unordered map. We could
    // implement hash ourselves, but this is the slowpath anyways.
    UnorderedMap<std::string_view, const char *> strings_by_value;
    UnorderedMap<const char *, const char *> stringsByPtr;

    UnorderedMap<const char *, UnorderedMap<int, CounterType>> histograms;
    UnorderedMap<const char *, CounterType> counters;
    UnorderedMap<const char *, std::vector<CounterType>> timings;
    UnorderedMap<const char *, UnorderedMap<const char *, CounterType>> countersByCategory;
};
} // namespace sorbet

#endif
