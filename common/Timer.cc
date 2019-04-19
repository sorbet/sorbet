#include "common/Timer.h"
using namespace std;
namespace sorbet {
Timer::Timer(spdlog::logger &log, string_view name) : log(log), name(name), begin(chrono::steady_clock::now()) {}

Timer::Timer(const shared_ptr<spdlog::logger> &log, string_view name)
    : log(*log), name(name), begin(chrono::steady_clock::now()) {}

Timer::~Timer() {
    auto clock = chrono::steady_clock::now();
    auto beginTs = std::chrono::duration_cast<std::chrono::microseconds>(begin.time_since_epoch()).count();
    auto endTs = std::chrono::duration_cast<std::chrono::microseconds>(clock.time_since_epoch()).count();
    log.debug("{}: {}ms", this->name, (endTs - beginTs) * 0.001);
    sorbet::timingAdd(this->name, beginTs, endTs);
}

} // namespace sorbet
