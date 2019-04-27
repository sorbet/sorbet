#include "common/Timer.h"
using namespace std;
namespace sorbet {
Timer::Timer(spdlog::logger &log, ConstExprStr name, initializer_list<pair<ConstExprStr, string_view>> args)
    : log(log), name(name), args(move(args)), begin(chrono::steady_clock::now()){};

Timer::Timer(const shared_ptr<spdlog::logger> &log, ConstExprStr name,
             initializer_list<pair<ConstExprStr, string_view>> args)
    : Timer(*log, name, args){};
Timer::Timer(const shared_ptr<spdlog::logger> &log, ConstExprStr name) : Timer(*log, name, {}){};
Timer::Timer(spdlog::logger &log, ConstExprStr name) : Timer(log, name, {}){};

Timer::~Timer() {
    auto clock = chrono::steady_clock::now();
    auto beginTs = std::chrono::duration_cast<std::chrono::nanoseconds>(begin.time_since_epoch()).count();
    auto endTs = std::chrono::duration_cast<std::chrono::nanoseconds>(clock.time_since_epoch()).count();
    log.debug("{}: {}ms", this->name.str, (endTs - beginTs) * 0.001);
    sorbet::timingAdd(this->name, beginTs, endTs, move(args));
}

} // namespace sorbet
