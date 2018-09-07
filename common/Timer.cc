#include "common/Timer.h"
using namespace std;

Timer::Timer(const shared_ptr<spdlog::logger> &log, string msg)
    : log(log), msg(move(msg)), begin(chrono::steady_clock::now()) {}

Timer::~Timer() {
    log->debug("{}: {}ms", this->msg, chrono::duration<double, milli>(chrono::steady_clock::now() - begin).count());
}
