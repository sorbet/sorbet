#include "spdlog/spdlog.h"
#include <ctime>
#include <memory>
#include <string>

class Timer {
public:
    Timer(const std::shared_ptr<spdlog::logger> &log, std::string msg);
    ~Timer();

private:
    std::shared_ptr<spdlog::logger> log;
    const std::string msg;
    struct timespec begin;
};
