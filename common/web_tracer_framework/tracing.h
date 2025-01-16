#ifndef SORBET_CORE_WEB_TRACER_FRAMEWORK_TRACING_H
#define SORBET_CORE_WEB_TRACER_FRAMEWORK_TRACING_H

#include "core/core.h"

namespace sorbet::web_tracer_framework {
class Tracing {
    friend class TracingTestHelper;

    static std::string stateToJSONL(const CounterState &counters, pid_t pid, microseconds now);

public:
    Tracing() = delete;

    static bool storeTraces(const CounterState &counters, const std::string &fileName, bool strict = false);
};
} // namespace sorbet::web_tracer_framework

#endif
