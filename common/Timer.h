#ifndef SORBET_TIMER_H
#define SORBET_TIMER_H
#include "common/Counters.h"
#include <chrono>
#include <memory>
#include <string>
#include <string_view>

namespace sorbet {
class Timer {
public:
    Timer(spdlog::logger &log, ConstExprStr name);
    Timer(spdlog::logger &log, ConstExprStr name, FlowId prev);
    Timer(spdlog::logger &log, ConstExprStr name,
          std::initializer_list<std::pair<ConstExprStr, std::string_view>> args);
    Timer(spdlog::logger &log, ConstExprStr name, FlowId prev,
          std::initializer_list<std::pair<ConstExprStr, std::string_view>> args);
    Timer(const std::shared_ptr<spdlog::logger> &log, ConstExprStr name, FlowId prev);
    Timer(const std::shared_ptr<spdlog::logger> &log, ConstExprStr name);
    Timer(const std::shared_ptr<spdlog::logger> &log, ConstExprStr name, FlowId prev,
          std::initializer_list<std::pair<ConstExprStr, std::string_view>> args);
    Timer(const std::shared_ptr<spdlog::logger> &log, ConstExprStr name,
          std::initializer_list<std::pair<ConstExprStr, std::string_view>> args);
    ~Timer();
    FlowId getFlowEdge();

private:
    spdlog::logger &log;
    ConstExprStr name;
    FlowId prev;
    FlowId self;
    std::initializer_list<std::pair<ConstExprStr, std::string_view>> args;
    const std::chrono::time_point<std::chrono::steady_clock> begin;
};
} // namespace sorbet

#endif
