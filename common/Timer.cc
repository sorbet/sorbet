#include "common/Timer.h"
using namespace std;
namespace sorbet {

Timer::Timer(spdlog::logger &log, ConstExprStr name, FlowId prev, initializer_list<pair<ConstExprStr, string>> args)
    : log(log), name(name), prev(prev), self{0}, args(args), start(chrono::steady_clock::now()){};

Timer::Timer(spdlog::logger &log, ConstExprStr name, initializer_list<pair<ConstExprStr, string>> args)
    : Timer(log, name, FlowId{0}, args){};

Timer::Timer(const shared_ptr<spdlog::logger> &log, ConstExprStr name, FlowId prev,
             initializer_list<pair<ConstExprStr, string>> args)
    : Timer(*log, name, prev, args){};

Timer::Timer(const shared_ptr<spdlog::logger> &log, ConstExprStr name,
             initializer_list<pair<ConstExprStr, string>> args)
    : Timer(*log, name, args){};

Timer::Timer(const shared_ptr<spdlog::logger> &log, ConstExprStr name) : Timer(*log, name, {}){};
Timer::Timer(const shared_ptr<spdlog::logger> &log, ConstExprStr name, FlowId prev) : Timer(*log, name, prev, {}){};

Timer::Timer(spdlog::logger &log, ConstExprStr name) : Timer(log, name, {}){};
Timer::Timer(spdlog::logger &log, ConstExprStr name, FlowId prev) : Timer(log, name, prev, {}){};

int getGlobalTimingId() {
    static atomic<int> counter = 1;
    return ++counter;
}

FlowId Timer::getFlowEdge() {
    if (this->self.id == 0) {
        this->self.id = getGlobalTimingId();
    }
    return this->self;
}

Timer::~Timer() {
    auto clock = chrono::steady_clock::now();
    auto dur = std::chrono::duration<double, std::milli>(clock - start);
    log.debug("{}: {}ms", this->name.str, dur.count());
    sorbet::timingAdd(this->name, start, clock, move(args), self, prev);
}

} // namespace sorbet
