#ifndef SORBET_CORE_WEB_TRACER_FRAMEWORK_TRACING_H
#define SORBET_CORE_WEB_TRACER_FRAMEWORK_TRACING_H

#include "core/core.h"

namespace sorbet::web_tracer_framework {
class Tracing {
public:
    Tracing() = delete;

    static bool storeTraces(const CounterState &counters, const std::string &fileName);
};
} // namespace sorbet::web_tracer_framework

#endif
