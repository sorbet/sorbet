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
    auto now = std::chrono::duration<double, std::micro>(chrono::steady_clock::now().time_since_epoch()).count();

    auto pid = getpid();
    fmt::format_to(result,
                   "{{\"name\":\"process_name\",\"ph\":\"M\",\"pid\":{},\"args\":{{\"name\":\"Sorbet v{}\"}}}},\n", pid,
                   Version::full_version_string);
    counters.counters->canonicalize();

    for (auto &cat : counters.counters->countersByCategory) {
        fmt::format_to(result, "{{\"name\":\"{}\",\"ph\":\"C\",\"ts\":{:.3f},\"pid\":{},\"args\":{{{}}}}},\n",
                       cat.first, now, pid, fmt::map_join(cat.second, ",", [](const auto &e) -> string {
                           return fmt::format("\"{}\":{}", e.first, e.second);
                       }));
    }

    for (auto &e : counters.counters->counters) {
        fmt::format_to(result, "{{\"name\":\"{}\",\"ph\":\"C\",\"ts\":{:.3f},\"pid\":{},\"args\":{{\"value\":{}}}}},\n",
                       e.first, now, pid, e.second);
    }

    // // for some reason, emmiting all of our counters breaks flow even visualitaion. @dmitry decided to not emmit
    // // histograms
    //
    // for (auto &hist : counters.counters->histograms) {
    //     fmt::format_to(result,
    //     "{{\"name\":\"{}\",\"ph\":\"C\",\"ts\":{},\"pid\":{},\"args\":{{{}}}}},\n",
    //                    hist.first, now, pid, fmt::map_join(hist.second, ",", [](const auto &e) -> string {
    //                        return fmt::format("\"{}\":{}", e.first, e.second);
    //                    }));
    // }

    for (const auto &e : counters.counters->timings) {
        string maybeArgs;
        if (!e.args.empty()) {
            maybeArgs = fmt::format(",\"args\":{{{}}}", fmt::map_join(e.args, ",", [](const auto &nameValue) -> string {
                                        return fmt::format("\"{}\":\"{}\"", nameValue.first, nameValue.second);
                                    }));
        }

        string maybeFlow;
        if (e.self.id != 0) {
            ENFORCE(e.prev.id == 0);
            maybeFlow = fmt::format(",\"bind_id\":{},\"flow_out\":true", e.self.id);
        } else if (e.prev.id != 0) {
            maybeFlow = fmt::format(",\"bind_id\":{},\"flow_in\":true", e.prev.id);
        }

        fmt::format_to(result,
                       "{{\"name\":\"{}\",\"ph\":\"X\",\"ts\":{:.3f},\"dur\":{:.3f},\"pid\":{},\"tid\":{}{}{}}},\n",
                       e.measure, (std::chrono::duration<double, std::micro>(e.start.time_since_epoch())).count(),
                       (std::chrono::duration<double, std::micro>(e.end - e.start)).count(), pid, e.threadId, maybeArgs,
                       maybeFlow);
    }

    fmt::format_to(result, "\n");
    FileOps::append(fileName, to_string(result));
    return true;
}
} // namespace sorbet::web_tracer_framework
