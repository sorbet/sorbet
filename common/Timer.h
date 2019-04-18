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
    Timer(spdlog::logger &log, std::string_view name);
    Timer(const std::shared_ptr<spdlog::logger> &log, std::string_view name);
    ~Timer();

private:
    spdlog::logger &log;
    std::string name;
    const std::chrono::time_point<std::chrono::steady_clock> begin;
};
} // namespace sorbet

#endif
