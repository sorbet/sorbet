#ifndef SORBET_COUNTERS_H
#define SORBET_COUNTERS_H
#include "absl/container/flat_hash_map.h"
#include "common/strings/ConstExprStr.h"
#include "sorbet_version/sorbet_version.h"
#include "spdlog/spdlog.h"
#include <string>

namespace sorbet {

constexpr bool enable_counters = debug_mode;

// A comment on the use of strings in this API
//
// We have the following goals for our counters API:
//
// (1) It should be very lightweight to add new counters; You should not need to
//     predeclare counters.
// (2) Counters should be cheap to use (fast).
//
// Goal (1) implies an API that takes strings directly, but comparing strings is
// slow, and goal (2) suggests we don't want to compare strings to store a
// counter. In order to meet these conflicting goals, we store strings as `const
// char*`, and compare by pointer value.
//
// The ConstExprStr class below can only be constructed from a string literal,
// which enforces safety -- Using string literals prevents use-after-free and
// various other memory errors normally associated with raw C strings. The
// compiler will typically deduplicate string literals within a translation
// units (but not necessarily between translation units), but this can't be
// relied upon, so we canonicalize strings when retrieving statistics.

struct CounterImpl;

// forward declarations for classes that need private access to the counter
// implementation
class StatsD;
namespace core {
class Proto;
}
namespace web_tracer_framework {
class Tracing;
}

namespace test::lsp {
class CounterStateDatabase;
}

// We are explicitly not using <chrono> in this file, because we profiled it and realized that
// using its abstractions for computing on and gathering times were a substantial overhead.
struct microseconds {
    int64_t usec;
    microseconds(int64_t usec) : usec(usec) {}
};

struct CounterState {
    CounterState();
    ~CounterState();
    CounterState(CounterState &&rhs);
    CounterState &operator=(CounterState &&rhs);

    CounterState(const CounterState &rhs) = delete;

    /** If `true`, then `counters` is null. */
    bool hasNullCounters() const;

private:
    friend CounterState getAndClearThreadCounters();
    friend void counterConsume(CounterState cs);
    friend class core::Proto;
    friend class StatsD;
    friend class sorbet::web_tracer_framework::Tracing;
    friend class sorbet::test::lsp::CounterStateDatabase;

    CounterState(std::unique_ptr<CounterImpl> counters);
    std::unique_ptr<CounterImpl> counters;
};

CounterState getAndClearThreadCounters();
void counterConsume(CounterState cs);

void prodCounterInc(ConstExprStr counter);
void prodCounterAdd(ConstExprStr counter, unsigned long value);
void prodCounterSet(ConstExprStr counter, unsigned long value);
void counterInc(ConstExprStr counter);
void counterAdd(ConstExprStr counter, unsigned long value);
void categoryCounterInc(ConstExprStr category, ConstExprStr counter);
void categoryCounterAdd(ConstExprStr category, ConstExprStr counter, unsigned long value);
void prodCategoryCounterInc(ConstExprStr category, ConstExprStr counter);
void prodCategoryCounterAdd(ConstExprStr category, ConstExprStr counter, unsigned long value);
void histogramInc(ConstExprStr histogram, int key);
void histogramAdd(ConstExprStr histogram, int key, unsigned long value);
void prodHistogramInc(ConstExprStr histogram, int key);
void prodHistogramAdd(ConstExprStr histogram, int key, unsigned long value);
/* Does not aggregate over measures, instead, reports them separately.
 * Use with care, as it can make us report a LOT of data. */
struct FlowId {
    int id;
};

void timingAdd(ConstExprStr measure, microseconds start, microseconds end,
               std::unique_ptr<std::vector<std::pair<ConstExprStr, std::string>>> args,
               std::unique_ptr<std::vector<std::pair<ConstExprStr, ConstExprStr>>> tags, FlowId self, FlowId previous,
               std::unique_ptr<std::vector<int>> histogramBuckets);

absl::flat_hash_map<long, long> getAndClearHistogram(ConstExprStr histogram);
std::string getCounterStatistics();

} // namespace sorbet
#endif // SORBET_COUNTERS_H
