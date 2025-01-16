#include "doctest/doctest.h"
#include "common/counters/Counters_impl.h"
#include "common/web_tracer_framework/tracing.h"

namespace sorbet::web_tracer_framework {

class TracingTestHelper {
public:
    static std::unique_ptr<CounterImpl> generateCounterStateImpl() {
        auto impl = std::make_unique<CounterImpl>();

        impl->counterAdd("example", 5);
        impl->counterAdd("other", 6);

        impl->categoryCounterAdd("adt", "state1", 5);
        impl->categoryCounterAdd("adt", "state2", 6);

        return impl;
    }

    static CounterState generateCounterState() {
        return CounterState{generateCounterStateImpl()};
    }

    static std::string generateJSONL(pid_t pid, microseconds now) {
        CounterState state(TracingTestHelper::generateCounterState());
        
        return Tracing::stateToJSONL(state, pid, now);
    }
};

TEST_SUITE("Tracing") {
    TEST_CASE("storeTraces non-strict") {
        pid_t pid = 1729;
        microseconds now{20200928};

        std::string jsonl = TracingTestHelper::generateJSONL(pid, now);

        CHECK_EQ("[]", json);
    }

    TEST_CASE("storeTraces strict") {
        pid_t pid = 1729;
        microseconds now{20200928};

        std::string jsonl = TracingTestHelper::generateJSONL(pid, now);

        CHECK_EQ("[]", json);
    }
}

}
