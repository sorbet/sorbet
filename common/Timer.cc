#include "common/Timer.h"
using namespace std;
namespace sorbet {

chrono::nanoseconds clock_gettime_coarse() {
    timespec tp;
    clock_gettime(CLOCK_MONOTONIC_COARSE, &tp);
    return chrono::nanoseconds(tp.tv_nsec);
}

Timer::Timer(spdlog::logger &log, ConstExprStr name, FlowId prev, initializer_list<pair<ConstExprStr, string>> args,
             chrono::nanoseconds start, initializer_list<int> histogramBuckets)
    : log(log), name(name), prev(prev), self{0}, args(args), start(start), histogramBuckets(histogramBuckets) {}

Timer::Timer(spdlog::logger &log, ConstExprStr name, FlowId prev, initializer_list<pair<ConstExprStr, string>> args,
             initializer_list<int> histogramBuckets)
    : Timer(log, name, prev, args, clock_gettime_coarse(), histogramBuckets){};

Timer::Timer(spdlog::logger &log, ConstExprStr name, initializer_list<pair<ConstExprStr, string>> args)
    : Timer(log, name, FlowId{0}, args, {}){};

Timer::Timer(spdlog::logger &log, ConstExprStr name, initializer_list<int> histogramBuckets)
    : Timer(log, name, FlowId{0}, {}, histogramBuckets) {}

Timer::Timer(const shared_ptr<spdlog::logger> &log, ConstExprStr name, FlowId prev,
             initializer_list<pair<ConstExprStr, string>> args)
    : Timer(*log, name, prev, args, {}){};

Timer::Timer(const shared_ptr<spdlog::logger> &log, ConstExprStr name,
             initializer_list<pair<ConstExprStr, string>> args)
    : Timer(*log, name, args){};

Timer::Timer(const shared_ptr<spdlog::logger> &log, ConstExprStr name)
    : Timer(*log, name, initializer_list<pair<ConstExprStr, string>>{}){};
Timer::Timer(const shared_ptr<spdlog::logger> &log, ConstExprStr name, FlowId prev) : Timer(*log, name, prev, {}, {}){};

Timer::Timer(spdlog::logger &log, ConstExprStr name)
    : Timer(log, name, initializer_list<pair<ConstExprStr, string>>{}){};
Timer::Timer(spdlog::logger &log, ConstExprStr name, FlowId prev) : Timer(log, name, prev, {}, {}){};

// Explicitly define to avoid reporting the timer twice.
Timer::Timer(Timer &&timer)
    : log(timer.log), name(timer.name), prev(timer.prev), self(timer.self), args(move(timer.args)),
      tags(move(timer.tags)), start(timer.start), histogramBuckets(move(timer.histogramBuckets)),
      canceled(timer.canceled) {
    // Don't report a latency metric for the moved timer.
    timer.cancel();
}

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

void Timer::cancel() {
    this->canceled = true;
}

Timer Timer::clone() const {
    return clone(name);
}

Timer Timer::clone(ConstExprStr name) const {
    Timer forked(log, name, prev, {}, start, {});
    forked.args = args;
    forked.tags = tags;
    forked.canceled = canceled;
    forked.histogramBuckets = histogramBuckets;
    return forked;
}

void Timer::setTag(ConstExprStr name, ConstExprStr value) {
    // Check if tag is already set; if so, update value.
    for (auto &tag : tags) {
        const auto tagName = tag.first;
        if (tagName.size == name.size && strncmp(tagName.str, name.str, tagName.size) == 0) {
            tag.second = value;
            return;
        }
    }
    // Add new tag.
    tags.push_back(make_pair(name, value));
}

Timer::~Timer() {
    auto clock = clock_gettime_coarse();
    auto dur = clock - start;
    if (!canceled && dur > chrono::nanoseconds(chrono::milliseconds(1))) {
        // the trick ^^^ is to skip double comparison in the common case and use the most efficient representation.
        auto dur = chrono::duration<double, milli>(clock - start);
        log.debug("{}: {}ms", this->name.str, dur.count());
        sorbet::timingAdd(this->name, start, clock, move(args), move(tags), self, prev, move(histogramBuckets));
    }
}

} // namespace sorbet
