#include "common/Counters.h"

#include "absl/strings/str_cat.h"
#include "absl/strings/str_replace.h"
#include "common/Counters_impl.h"
#include "common/web_tracer_framework/tracing.h"
#include "version/version.h"
#include <chrono>
#include <string>
#include <unistd.h>
using namespace std;
namespace sorbet::web_tracer_framework {
bool Tracing::storeTraces(const CounterState &counters, string_view fileName) {
    fmt::memory_buffer result;

    if (!FileOps::exists(fileName)) {
        fmt::format_to(result, "[\n");
    }
    auto now =
        chrono::duration_cast<chrono::nanoseconds>(chrono::steady_clock::now().time_since_epoch()).count() / 1000.0;

    auto pid = getpid();
    fmt::format_to(result,
                   "{{\"name\":\"process_name\",\"ph\":\"M\",\"pid\":{},\"args\":{{\"name\":\"Sorbet v{}\"}}}},\n", pid,
                   Version::full_version_string);
    counters.counters->canonicalize();

    for (auto &cat : counters.counters->countersByCategory) {
        fmt::format_to(result,
                       "{{\"name\":\"{}\",\"ph\":\"C\",\"ts\":{},\"pid\":{},\"cat\":\"CC\",\"args\":{{{}}}}},\n",
                       cat.first, now, pid, fmt::map_join(cat.second, ",", [](const auto &e) -> string {
                           return fmt::format("\"{}\":{}", e.first, e.second);
                       }));
    }

    for (auto &hist : counters.counters->histograms) {
        fmt::format_to(result, "{{\"name\":\"{}\",\"ph\":\"C\",\"ts\":{},\"pid\":{},\"cat\":\"H\",\"args\":{{{}}}}},\n",
                       hist.first, now, pid, fmt::map_join(hist.second, ",", [](const auto &e) -> string {
                           return fmt::format("\"{}\":{}", e.first, e.second);
                       }));
    }

    for (auto &e : counters.counters->counters) {
        fmt::format_to(result,
                       "{{\"name\":\"{}\",\"ph\":\"C\",\"ts\":{},\"pid\":{},\"cat\":\"C\",\"args\":"
                       "{{\"value\":{}}}}},\n",
                       e.first, now, pid, e.second);
    }

    // When drawing the trace, chrome does not take advantage of order for "X" traces.
    // If we have traces with the same time stamp for either begin or end, chrome will not draw them well.
    // For events with same timestamp that are stored in array "earlier" events are stored with
    // int count = 0;
    // int reverseCount = counters.counters->timings.size();
    // double incorrectionScale = 0.001 / reverseCount;
    for (const auto &e : counters.counters->timings) {
        string maybeArgs;
        // count++;
        // reverseCount--;
        if (!e.args.empty()) {
            maybeArgs = fmt::format(",\"args\":{{{}}}", fmt::map_join(e.args, ",", [](const auto &nameValue) -> string {
                                        return fmt::format("\"{}\":\"{}\"", nameValue.first, nameValue.second);
                                    }));
        }
        if (e.kind == CounterImpl::Timing::Duration) {
            fmt::format_to(result,
                           "{{\"name\":\"{}\",\"cat\":\"T,D\",\"ph\":\"X\",\"ts\":{:.3f},\"dur\":"
                           "{:.3f},\"pid\":{},\"tid\":{}{}}},\n",
                           e.measure, e.ts / 1000.0, e.duration.value() / 1000.0, pid, e.threadId.value(), maybeArgs);
        } else if (e.kind == CounterImpl::Timing::FlowStart) {
            fmt::format_to(result,
                           "{{\"name\":\"{}\",\"cat\":\"T,F\",\"ph\":\"s\",\"ts\":{:.3f},\"pid\":{},"
                           "\"tid\":{},\"id\":{}{}}},\n",
                           e.measure, e.ts / 1000.0, pid, e.threadId.value(), e.id, maybeArgs);
        } else if (e.kind == CounterImpl::Timing::FlowEnd) {
            fmt::format_to(result,
                           "{{\"name\":\"{}\",\"cat\":\"T,F\",\"ph\":\"f\",\"bp\":\"e\",\"ts\":"
                           "{:.3f},\"pid\":{},\"tid\":{},\"id\":{}{}}},\n",
                           e.measure, e.ts / 1000.0, pid, e.threadId.value(), e.id, maybeArgs);
        }
    }

    fmt::format_to(result, "\n");
    FileOps::append(fileName, to_string(result));
    return true;
}
} // namespace sorbet::web_tracer_framework
