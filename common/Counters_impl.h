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
    void prodCounterSet(const char *counter, unsigned long value);

    // std::string_view isn't hashable, so we use an unordered map. We could
    // implement hash ourselves, but this is the slowpath anyways.
    UnorderedMap<std::string_view, const char *> strings_by_value;
    UnorderedMap<const char *, const char *> stringsByPtr;
    struct Timing {
        // see https://docs.google.com/document/d/1CvAClvFfyA5R-PhYUmn5OOQtYMH4h6I0nSsKchNAySU/edit
        // and https://docs.google.com/document/d/1La_0PPfsTqHJihazYhff96thhjPtvq1KjAUOJu0dvEg/edit
        int id;
        char const *measure;
        microseconds start, end;
        int threadId;
        std::unique_ptr<std::vector<std::pair<char const *, std::string>>> args;
        std::unique_ptr<std::vector<std::pair<char const *, char const *>>> tags;
        FlowId self;
        FlowId prev;
    };
    void timingAdd(Timing timing);
    UnorderedMap<const char *, UnorderedMap<int, CounterType>> histograms;
    UnorderedMap<const char *, CounterType> counters;
    std::vector<Timing> timings;
    UnorderedMap<const char *, UnorderedMap<const char *, CounterType>> countersByCategory;
};
} // namespace sorbet

#endif
