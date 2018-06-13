#ifndef SORBET_COUNTERS_IMPL_H
#define SORBET_COUNTERS_IMPL_H

#include "absl/strings/string_view.h"

namespace sorbet {
namespace core {
struct CounterImpl {
    CounterImpl() = default;
    CounterImpl(CounterImpl const &) = delete;
    CounterImpl(CounterImpl &&) = default;
    CounterImpl &operator=(CounterImpl &&) = delete;
    using CounterType = unsigned long;

    CounterImpl canonicalize();
    void clear();

    const char *internKey(const char *str);

    void histogramAdd(const char *histogram, int key, unsigned int value);

    void categoryCounterAdd(const char *category, const char *counter, unsigned int value);

    void counterAdd(const char *counter, unsigned int value, bool isProdCounter);

    // absl::string_view isn't hashable, so we use an unordered map. We could
    // implement hash ourselves, but this is the slowpath anyways.
    std::map<absl::string_view, const char *> strings_by_value;
    std::map<const char *, const char *> strings_by_ptr;

    std::map<const char *, std::map<int, CounterType>> histograms;
    std::map<const char *, CounterType> counters;
    std::map<const char *, std::map<const char *, CounterType>> counters_by_category;
};
} // namespace core
} // namespace sorbet

#endif
