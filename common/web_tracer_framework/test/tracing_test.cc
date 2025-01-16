#include "doctest/doctest.h"
#include "absl/algorithm/container.h"
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

    static std::string generateJSON(pid_t pid, microseconds now, bool strict) {
        CounterState state(TracingTestHelper::generateCounterState());
        
        std::string jsonl = Tracing::stateToJSONL(state, pid, now);

        // The first line of `jsonl` is going to contain information about the Sorbet
        // version.  We don't really care about that, and handling that would make the
        // test harness significantly more complicated, so just strip it off.
        auto it = absl::c_find(jsonl, '\n');
        CHECK_NE(it, jsonl.end());
        CHECK_NE(it + 1, jsonl.end());

        jsonl.erase(jsonl.begin(), it + 1);

        const bool needsOpeningBracket = true;
        return Tracing::jsonlToJSON(jsonl, needsOpeningBracket, strict);
    }
};

TEST_SUITE("Tracing") {
    TEST_CASE("storeTraces non-strict") {
        pid_t pid = 1729;
        microseconds now{20200928};

        const bool strict = false;
        std::string json = TracingTestHelper::generateJSON(pid, now, strict);

        CHECK_EQ("[]", json);
    }

    TEST_CASE("storeTraces strict") {
        pid_t pid = 1729;
        microseconds now{20200928};

        const bool strict = true;
        std::string json = TracingTestHelper::generateJSON(pid, now, strict);

        CHECK_EQ("[]", json);
    }
}

}
