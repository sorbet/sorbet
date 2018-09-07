#include "spdlog/spdlog.h"
#include <chrono>
#include <memory>
#include <string>

class Timer {
public:
    Timer(const std::shared_ptr<spdlog::logger> &log, std::string msg);
    ~Timer();

private:
    std::shared_ptr<spdlog::logger> log;
    const std::string msg;
    const std::chrono::time_point<std::chrono::steady_clock> begin;
};
