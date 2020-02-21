#include "common/Timer.h"
#include "common/sort.h"
using namespace std;
namespace sorbet {
namespace {
vector<int> ensureSorted(initializer_list<int> input) {
    vector<int> output(input);
    output.push_back(INT_MAX);
    fast_sort(output);
    return output;
}
} // namespace

Timer::Timer(spdlog::logger &log, ConstExprStr name, FlowId prev, initializer_list<pair<ConstExprStr, string>> args,
             chrono::time_point<chrono::steady_clock> start, initializer_list<int> histogramBuckets)
    : log(log), name(name), prev(prev), self{0}, args(args), start(start),
      histogramBuckets(ensureSorted(histogramBuckets)) {}

Timer::Timer(spdlog::logger &log, ConstExprStr name, FlowId prev, initializer_list<pair<ConstExprStr, string>> args,
             initializer_list<int> histogramBuckets)
    : Timer(log, name, prev, args, chrono::steady_clock::now(), histogramBuckets){};

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

Timer Timer::clone(ConstExprStr name) {
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
    auto clock = chrono::steady_clock::now();
    auto dur = clock - start;
    if (!canceled && dur > std::chrono::milliseconds(1)) {
        // the trick ^^^ is to skip double comparison in the common case and use the most efficient representation.
        auto dur = std::chrono::duration<double, std::milli>(clock - start);
        const auto msCount = dur.count();
        log.debug("{}: {}ms", this->name.str, msCount);
        sorbet::timingAdd(this->name, start, clock, move(args), move(tags), self, prev);

        if (!histogramBuckets.empty()) {
            // Find the bucket for this value. The last bucket is INT_MAX, so it's guaranteed to pick one.
            for (const auto bucket : histogramBuckets) {
                if (msCount < bucket) {
                    prodHistogramInc(name, bucket);
                    break;
                }
            }
        }
    }
}

} // namespace sorbet
