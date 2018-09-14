#include "common/Counters.h"
#include "spdlog/spdlog.h"
#include <chrono>
#include <memory>
#include <string>

namespace sorbet {
class Timer {
public:
    Timer(const std::shared_ptr<spdlog::logger> &log, ConstExprStr name);
    ~Timer();

private:
    std::shared_ptr<spdlog::logger> log;
    ConstExprStr name;
    const std::chrono::time_point<std::chrono::steady_clock> begin;
};
} // namespace sorbet
