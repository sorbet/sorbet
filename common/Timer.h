#ifndef SORBET_TIMER_H
#define SORBET_TIMER_H
#include "common/Counters.h"
#include <chrono>
#include <memory>
#include <string>

namespace sorbet {
class Timer {
public:
    Timer(spdlog::logger &log, ConstExprStr name);
    Timer(spdlog::logger &log, ConstExprStr name, FlowId prev);
    Timer(spdlog::logger &log, ConstExprStr name, std::initializer_list<std::pair<ConstExprStr, std::string>> args);
    Timer(spdlog::logger &log, ConstExprStr name, FlowId prev,
          std::initializer_list<std::pair<ConstExprStr, std::string>> args);
    Timer(const std::shared_ptr<spdlog::logger> &log, ConstExprStr name, FlowId prev);
    Timer(const std::shared_ptr<spdlog::logger> &log, ConstExprStr name);
    Timer(const std::shared_ptr<spdlog::logger> &log, ConstExprStr name, FlowId prev,
          std::initializer_list<std::pair<ConstExprStr, std::string>> args);
    Timer(const std::shared_ptr<spdlog::logger> &log, ConstExprStr name,
          std::initializer_list<std::pair<ConstExprStr, std::string>> args);
    ~Timer();
    FlowId getFlowEdge();

private:
    spdlog::logger &log;
    ConstExprStr name;
    FlowId prev;
    FlowId self;
    std::initializer_list<std::pair<ConstExprStr, std::string>> args;
    const std::chrono::time_point<std::chrono::steady_clock> begin;
};
} // namespace sorbet

#endif
