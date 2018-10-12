#include "common/Timer.h"
using namespace std;
namespace sorbet {
Timer::Timer(spdlog::logger &log, ConstExprStr name) : log(log), name(name), begin(chrono::steady_clock::now()) {}

Timer::Timer(const shared_ptr<spdlog::logger> &log, ConstExprStr name)
    : log(*log), name(name), begin(chrono::steady_clock::now()) {}

Timer::~Timer() {
    auto timeNanos = chrono::duration<long, nano>(chrono::steady_clock::now() - begin).count();
    log.debug("{}: {}ms", this->name.str, timeNanos * 1.0 / 1000000);
    sorbet::timingAdd(this->name, timeNanos);
}

} // namespace sorbet