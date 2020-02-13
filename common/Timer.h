#ifndef SORBET_TIMER_H
#define SORBET_TIMER_H
#include "common/Counters.h"
#include <chrono>
#include <memory>
#include <string>
#include <thread>

namespace sorbet {
class Timer {
    Timer(spdlog::logger &log, ConstExprStr name, FlowId prev,
          std::initializer_list<std::pair<ConstExprStr, std::string>> args,
          std::chrono::time_point<std::chrono::steady_clock> start);

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

    // Don't report timer when it gets destructed.
    void cancel();

    // Creates a new timer with the same start time and args but a different name.
    Timer fork(ConstExprStr name);

    // TODO We could add more overloads for this if we need them (to create other kinds of Timers)
    // We could also make this more generic to allow more sleep duration types.
    static void timedSleep(const std::chrono::microseconds &sleep_duration, spdlog::logger &log, ConstExprStr name) {
        Timer timer(log, name);
        auto dur = std::chrono::duration<double, std::milli>(sleep_duration);
        log.debug("{}: sleeping for {}ms", name.str, dur.count());
        std::this_thread::sleep_for(sleep_duration);
    }

private:
    spdlog::logger &log;
    ConstExprStr name;
    FlowId prev;
    FlowId self;
    std::vector<std::pair<ConstExprStr, std::string>> args;
    const std::chrono::time_point<std::chrono::steady_clock> start;
    bool canceled = false;
};
} // namespace sorbet

#endif
