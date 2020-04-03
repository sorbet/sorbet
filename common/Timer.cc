#include "common/Timer.h"
using namespace std;
namespace sorbet {

// We are using clock_gettime instead of similar APIs from the C++ <chrono> library,
// because this was measured to make timers noticably faster.
//
// https://stackoverflow.com/questions/48609413/fastest-way-to-get-a-timestamp
microseconds clock_gettime_coarse() {
    timespec tp;
#ifdef __linux__
    // This is faster, as measured via the benchmark above, but is not portable.
    clock_gettime(CLOCK_MONOTONIC_COARSE, &tp);
#else
    clock_gettime(CLOCK_MONOTONIC, &tp);
#endif
    return {(tp.tv_sec * 1'000'000L) + (tp.tv_nsec / 1'000L)};
}

Timer::Timer(spdlog::logger &log, ConstExprStr name, FlowId prev, initializer_list<pair<ConstExprStr, string>> args,
             microseconds start, initializer_list<int> histogramBuckets)
    : log(log), name(name), prev(prev), self{0}, start(start) {
    if (args.size() != 0) {
        this->args = make_unique<vector<pair<ConstExprStr, string>>>(args);
    }

    if (histogramBuckets.size() != 0) {
        this->histogramBuckets = make_unique<vector<int>>(histogramBuckets);
    }
}

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
    if (this->args != nullptr) {
        forked.args = make_unique<vector<pair<ConstExprStr, string>>>(*args);
    }
    if (this->tags != nullptr) {
        forked.tags = make_unique<vector<pair<ConstExprStr, ConstExprStr>>>(*tags);
    }
    forked.canceled = canceled;
    if (this->histogramBuckets != nullptr) {
        forked.histogramBuckets = make_unique<vector<int>>(*histogramBuckets);
    }
    return forked;
}

void Timer::setTag(ConstExprStr name, ConstExprStr value) {
    // Check if tag is already set; if so, update value.
    if (tags != nullptr) {
        for (auto &tag : *tags) {
            const auto tagName = tag.first;
            if (tagName.size == name.size && strncmp(tagName.str, name.str, tagName.size) == 0) {
                tag.second = value;
                return;
            }
        }
    }
    // Add new tag.
    if (tags == nullptr) {
        tags = make_unique<vector<pair<ConstExprStr, ConstExprStr>>>();
    }
    tags->push_back(make_pair(name, value));
}

Timer::~Timer() {
    auto clock = clock_gettime_coarse();
    auto dur = microseconds{clock.usec - start.usec};
    auto threshold = microseconds{2'000}; // 2ms
    if (!canceled && dur.usec > threshold.usec) {
        // the trick ^^^ is to skip double comparison in the common case and use the most efficient representation.
        auto durMs = (clock.usec - start.usec) / 1'000;
        log.debug("{}: {}ms", this->name.str, durMs);
        sorbet::timingAdd(this->name, start, clock, move(args), move(tags), self, prev, move(histogramBuckets));
    }
}

} // namespace sorbet
