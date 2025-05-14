#include "doctest/doctest.h"
// has to go first as it violates our requirements
#include "absl/algorithm/container.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_split.h"
#include "common/counters/Counters_impl.h"
#include "common/sort/sort.h"
#include "common/web_tracer_framework/tracing.h"

using namespace std;

namespace sorbet::web_tracer_framework {

class TracingTestHelper {
public:
    static unique_ptr<CounterImpl> generateCounterStateImpl() {
        auto impl = make_unique<CounterImpl>();

        // Deliberately output these in non-alphabetical order to maybe encourage the
        // linker to output the strings in such a way as to motivate the sorting below.
        impl->counterAdd("other", 6);
        impl->counterAdd("example", 5);

        // Don't test categoryCounterAdd because the generation of `args` is
        // non-deterministic, based on wherever the linker decided to output the strings
        // for the counter names.

        // Don't test timingAdd; there's a lot to setup.  If you are motivated to add
        // testing for timingAdd, you will probably have to forego testing `args` and
        // `tags` for the same reason as categoryCounterAdd.

        return impl;
    }

    static CounterState generateCounterState() {
        return CounterState{generateCounterStateImpl()};
    }

    static string generateJSON(pid_t pid, microseconds now, bool strict) {
        CounterState state(TracingTestHelper::generateCounterState());

        string jsonl = Tracing::stateToJSONL(state, pid, now);

        // The first line of `jsonl` is going to contain information about the Sorbet
        // version.  We don't really care about that, and handling that would make the
        // test harness significantly more complicated, so just strip it off.
        {
            auto it = absl::c_find(jsonl, '\n');
            CHECK_NE(it, jsonl.end());
            CHECK_NE(it + 1, jsonl.end());

            jsonl.erase(jsonl.begin(), it + 1);
            CHECK_NE(jsonl[0], '\n');
        }

        // The remaining lines will be counters, but we need to sort them; the implementation
        // stores them based on the hashes of the string pointers, and that in turn depends
        // on the vagaries of where the linker decided to put the string constants for the
        // counter names.
        vector<string> lines = absl::StrSplit(jsonl, '\n');
        {
            // Don't sort empty lines.
            auto it = absl::c_find_if(lines, [](const auto &line) { return line.empty(); });
            CHECK_NE(it, lines.end());
            CHECK_EQ(it + 1, lines.end());
            fast_sort_range(lines.begin(), it);
            jsonl = absl::StrJoin(lines, "\n");
        }

        const bool needsOpeningBracket = true;
        return Tracing::jsonlToJSON(jsonl, needsOpeningBracket, strict);
    }
};

TEST_SUITE("Tracing") {
    TEST_CASE("storeTraces non-strict") {
        pid_t pid = 1729;
        microseconds now{20200928};

        const bool strict = false;
        string json = TracingTestHelper::generateJSON(pid, now, strict);

        // clang-format off
        CHECK_EQ("[\n"
                 "{\"name\":\"example\",\"ph\":\"C\",\"ts\":20200928.000,\"pid\":1729,\"args\":{\"value\":\"5\"}},\n"
                 "{\"name\":\"other\",\"ph\":\"C\",\"ts\":20200928.000,\"pid\":1729,\"args\":{\"value\":\"6\"}},\n"
                 "\n"
                 , json);
        // clang-format on
    }

    TEST_CASE("storeTraces strict") {
        pid_t pid = 1729;
        microseconds now{20200928};

        const bool strict = true;
        string json = TracingTestHelper::generateJSON(pid, now, strict);

        // clang-format off
        CHECK_EQ("[\n"
                 "{\"name\":\"example\",\"ph\":\"C\",\"ts\":20200928.000,\"pid\":1729,\"args\":{\"value\":\"5\"}},\n"
                 "{\"name\":\"other\",\"ph\":\"C\",\"ts\":20200928.000,\"pid\":1729,\"args\":{\"value\":\"6\"}}\n"
                 "]\n"
                 , json);
        // clang-format on
    }
}

} // namespace sorbet::web_tracer_framework
