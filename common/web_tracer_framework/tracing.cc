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
        fmt::format_to(result, "[");
    }
    auto now = chrono::duration_cast<chrono::microseconds>(chrono::steady_clock::now().time_since_epoch()).count();

    auto pid = getpid();
    fmt::format_to(result,
                   "{{\"name\":\"process_name\",\"ph\":\"M\",\"pid\":{},\"args\":{{\"name\":\"Sorbet v{}\"}}}},", pid,
                   Version::full_version_string);
    counters.counters->canonicalize();
    for (auto &cat : counters.counters->countersByCategory) {
        fmt::format_to(result, "{{\"name\":\"{}\",\"ph\":\"C\",\"ts\":{},\"pid\":{},\"cat\":\"CC\",\"args\":{{",
                       cat.first, now, pid);
        bool seenFirst = false;
        for (auto &e : cat.second) {
            if (seenFirst) {
                fmt::format_to(result, ",");
            }
            fmt::format_to(result, "\"{}\": {}", e.first, e.second);
            seenFirst = true;
        }
        fmt::format_to(result, "}}}},");
    }

    for (auto &hist : counters.counters->histograms) {
        fmt::format_to(result, "{{\"name\":\"{}\",\"ph\":\"C\",\"ts\":{},\"pid\":{},\"cat\":\"H\",\"args\":{{",
                       hist.first, now, pid);

        bool seenFirst = false;
        for (auto &e : hist.second) {
            if (seenFirst) {
                fmt::format_to(result, ",");
            }

            fmt::format_to(result, "\"{}\":{}", e.first, e.second);
            seenFirst = true;
        }
        fmt::format_to(result, "}}}},");
    }

    for (auto &e : counters.counters->counters) {
        fmt::format_to(result,
                       "{{\"name\":\"{}\",\"ph\":\"C\",\"ts\":{},\"pid\":{},\"cat\":\"C\",\"args\":"
                       "{{\"value\":{}}}}},",
                       e.first, now, pid, e.second);
    }

    for (const auto &e : counters.counters->timings) {
        if (e.kind == CounterImpl::Timing::Duration) {
            fmt::format_to(result,
                           "{{\"name\":\"{}\",\"cat\":\"T,D\",\"ph\":\"X\",\"ts\":{},\"dur\":"
                           "{},\"pid\":{},\"tid\":{}}},",
                           e.namePrefix, e.ts, e.duration.value(), pid, e.threadId.value());
        } else if (e.kind == CounterImpl::Timing::Async) {
            fmt::format_to(result,
                           "{{\"name\":\"{}\",\"cat\":\"T,A\",\"ph\":\"b\",\"ts\":{},\"pid\":{},"
                           "\"id\":{}}},",
                           e.namePrefix, e.ts, pid, e.id);
            fmt::format_to(result,
                           "{{\"name\":\"{}\",\"cat\":\"T,A\",\"ph\":\"e\",\"ts\":{},\"pid\":{}, "
                           "\"id\":{}}},",
                           e.namePrefix, e.ts + e.duration.value(), pid, e.id);
        } else if (e.kind == CounterImpl::Timing::FlowStart) {
            fmt::format_to(result,
                           "{{\"name\":\"{}\",\"cat\":\"T,F\",\"ph\":\"s\",\"ts\":{},\"pid\":{},"
                           "\"tid\":{},\"id\":{}}},",
                           e.namePrefix, e.ts, pid, e.threadId.value(), e.id);
        } else if (e.kind == CounterImpl::Timing::FlowEnd) {
            fmt::format_to(result,
                           "{{\"name\":\"{}\",\"cat\":\"T,F\",\"ph\":\"f\",\"bp\":\"e\",\"ts\":"
                           "{},\"pid\":{},\"tid\":{},\"id\":{}}},",
                           e.namePrefix, e.ts, pid, e.threadId.value(), e.id);
        }
    }
    fmt::format_to(result, "\n");
    FileOps::append(fileName, to_string(result));
    return true;
}
} // namespace sorbet::web_tracer_framework
