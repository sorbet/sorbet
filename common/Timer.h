#ifndef SORBET_TIMER_H
#define SORBET_TIMER_H
#include "common/Counters.h"
#include <memory>
#include <string>
#include <thread>

namespace sorbet {

class Timer {
    Timer(ConstExprStr name, FlowId prev, std::initializer_list<std::pair<ConstExprStr, std::string>> args,
          microseconds start, std::initializer_list<int> histogramBuckets);

public:
    Timer(ConstExprStr name);
    Timer(ConstExprStr name, std::initializer_list<int> histogramBuckets);
    Timer(ConstExprStr name, FlowId prev);
    Timer(ConstExprStr name, std::initializer_list<std::pair<ConstExprStr, std::string>> args);
    Timer(ConstExprStr name, FlowId prev, std::initializer_list<std::pair<ConstExprStr, std::string>> args);
    Timer(ConstExprStr name, FlowId prev, std::initializer_list<std::pair<ConstExprStr, std::string>> args,
          std::initializer_list<int> histogramBuckets);
    // Delete copy constructor to avoid accidentally copying and reporting a timer twice.
    Timer(const Timer &) = delete;
    // Define custom move constructor to avoid reporting moved timers.
    Timer(Timer &&);

    ~Timer();

    FlowId getFlowEdge();

    // Don't report timer when it gets destructed.
    void cancel();

    // Add a tag to the statsd metrics for this timer. Will not appear in traces.
    void setTag(ConstExprStr name, ConstExprStr value);

    // Creates a new timer with the same start time, tags, args, and name.
    Timer clone() const;

    // Creates a new timer with the same start time, tags, and args but a different name.
    Timer clone(ConstExprStr name) const;

    // TODO We could add more overloads for this if we need them (to create other kinds of Timers)
    // We could also make this more generic to allow more sleep duration types.
    static void timedSleep(const std::chrono::microseconds &sleep_duration, ConstExprStr name) {
        Timer timer(name);
        std::this_thread::sleep_for(sleep_duration);
    }

    static microseconds clock_gettime_coarse();

    void setEndTime();

private:
    ConstExprStr name;
    FlowId prev;
    FlowId self;
    // 'args' appear in traces, but not in statsd metrics because they can cause an explosion in cardinality
    std::unique_ptr<std::vector<std::pair<ConstExprStr, std::string>>> args;
    // 'tags' appear in statsd metrics but not in traces. They are ConstExprStr to limit cardinality.
    std::unique_ptr<std::vector<std::pair<ConstExprStr, ConstExprStr>>> tags;
    // It would be far better for type safety to store this as a std::chrono::time_point,
    // but we don't know the clock a priori, because it's platform specific.
    const microseconds start;
    // If not empty, report the time for this timer in the given histogram, where each entry forms an upper bound
    // on a bucket.
    std::unique_ptr<std::vector<int>> histogramBuckets;
    bool canceled = false;
    microseconds endTime;
};
} // namespace sorbet

#endif
