#ifndef TEST_LSP_COUNTERSTATEDATABASE_H
#define TEST_LSP_COUNTERSTATEDATABASE_H

#include "common/counters/Counters.h"
#include "common/counters/Counters_impl.h"

namespace sorbet::test::lsp {
class CounterStateDatabase final {
    const CounterState counters;

public:
    CounterStateDatabase(CounterState counters);

    // Get counter value or 0.
    CounterImpl::CounterType getCounter(ConstExprStr counter) const;

    CounterImpl::CounterType getCategoryCounter(ConstExprStr counter, ConstExprStr category) const;

    CounterImpl::CounterType getCategoryCounterSum(ConstExprStr counter) const;

    CounterImpl::CounterType getHistogramCount(ConstExprStr histogram) const;

    std::vector<std::unique_ptr<CounterImpl::Timing>>
    getTimings(ConstExprStr counter, std::vector<std::pair<ConstExprStr, ConstExprStr>> tags = {}) const;
};
} // namespace sorbet::test::lsp
#endif // TEST_LSP_COUNTERSTATEDATABASE_H
