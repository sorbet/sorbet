#ifndef SORBET_COUNTERS_IMPL_H
#define SORBET_COUNTERS_IMPL_H

#include "absl/strings/string_view.h"

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

    void categoryCounterAdd(const char *category, const char *counter, unsigned long value);
    void prodCategoryCounterAdd(const char *category, const char *counter, unsigned long value);

    void counterAdd(const char *counter, unsigned long value);
    void prodCounterAdd(const char *counter, unsigned long value);
    void timingAdd(const char *metring, unsigned long nanos);

    // absl::string_view isn't hashable, so we use an unordered map. We could
    // implement hash ourselves, but this is the slowpath anyways.
    std::map<absl::string_view, const char *> strings_by_value;
    std::map<const char *, const char *> strings_by_ptr;

    std::map<const char *, std::map<int, CounterType>> histograms;
    std::map<const char *, CounterType> counters;
    std::map<const char *, std::vector<CounterType>> timings;
    std::map<const char *, std::map<const char *, CounterType>> counters_by_category;
};
} // namespace sorbet

#endif
