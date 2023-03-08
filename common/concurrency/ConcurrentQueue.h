#ifndef SORBET_CONCURRENTQUEUE_H
#define SORBET_CONCURRENTQUEUE_H
/**
  Those classes wrap a pre-existing concurrent queue to provide a consistent API.
  We use https://github.com/cameron314/concurrentqueue
  It was chosen by @DarkDimius writing a benchmark at the time that's now lost.
  https://max0x7ba.github.io/atomic_queue/html/benchmarks.html seems to contain a similar benchmark.
  In practice, we currently use it in either "single producer N consumers" or "N producer single consumer modes".
*/

#include "blockingconcurrentqueue.h"
#include "common/common.h"
#include "common/timers/Timer.h"
#include <atomic>
#include <chrono>
#include <type_traits>

struct DequeueResult {
    bool returned;
    bool shouldRetry;
    inline bool done() noexcept {
        return !(returned || shouldRetry);
    }
    inline bool gotItem() noexcept {
        return returned;
    }
};

/* A thread safe lock free queue that has safe publication guarantees, that is only used to process N elements */
template <class Elem, class Queue> class AbstractConcurrentBoundedQueue {
    Queue _queue;
    std::atomic<int> elementsLeftToPush; // double serves as a counter and as safe publication marker
    std::atomic<int> elementsPopped;

public:
    const int bound;
    AbstractConcurrentBoundedQueue(int bound) noexcept : elementsLeftToPush(bound), elementsPopped(0), bound(bound) {}
    AbstractConcurrentBoundedQueue(const AbstractConcurrentBoundedQueue &other) = delete;
    AbstractConcurrentBoundedQueue(AbstractConcurrentBoundedQueue &&other) = delete;

    // When `Elem` is a fundamental type (int, bool, etc) push takes a value, but if it's anything else it expects an
    // rvalue reference so that we don't forget to move the argument.
    // TODO: is it valuable to make this check use `std::is_trivially_copyable` instead?
    inline void push(typename std::conditional<std::is_fundamental<Elem>::value, Elem, Elem &&>::type elem,
                     int count) noexcept {
        _queue.enqueue(std::move(elem));
        elementsLeftToPush.fetch_add(-count, std::memory_order_release);
        ENFORCE(elementsLeftToPush.load(std::memory_order_relaxed) >= 0);
    }

    inline DequeueResult try_pop(Elem &elem) noexcept {
        DequeueResult ret;
        ret.shouldRetry = elementsLeftToPush.load(std::memory_order_acquire) != 0;
        ret.returned = _queue.try_dequeue(elem);
        if (ret.returned) {
            elementsPopped.fetch_add(1, std::memory_order_relaxed);
        }
        return ret;
    }

    template <typename Rep, typename Period>
    inline DequeueResult wait_pop_timed(Elem &elem, std::chrono::duration<Rep, Period> const &timeout,
                                        spdlog::logger &log, bool silent = false) noexcept {
        DequeueResult ret;
        if (!sorbet::emscripten_build) {
            ret.shouldRetry = elementsLeftToPush.load(std::memory_order_acquire) != 0;
            if (ret.shouldRetry) {
                std::unique_ptr<sorbet::Timer> time;
                if (!silent) {
                    time = std::make_unique<sorbet::Timer>(log, "wait_pop_timed");
                }
                ret.returned = _queue.wait_dequeue_timed(elem, timeout);
            } else { // all elements has been pushed, no need to wait.
                ret.returned = _queue.try_dequeue(elem);
            }
            if (ret.returned) {
                elementsPopped.fetch_add(1, std::memory_order_relaxed);
            }
            return ret;
        }
        return try_pop(elem);
    }

    int doneEstimate() {
        return elementsPopped.load(std::memory_order_relaxed);
    }

    int enqueuedEstimate() {
        return bound - elementsLeftToPush.load(std::memory_order_relaxed);
    }

    int sizeEstimate() const {
        return _queue.size_approx();
    }
};

template <class Elem>
using ConcurrentBoundedQueue = AbstractConcurrentBoundedQueue<Elem, moodycamel::ConcurrentQueue<Elem>>;

template <class Elem>
class ConcurrentUnBoundedQueue : public AbstractConcurrentBoundedQueue<Elem, moodycamel::ConcurrentQueue<Elem>> {
public:
    ConcurrentUnBoundedQueue() : AbstractConcurrentBoundedQueue<Elem, moodycamel::ConcurrentQueue<Elem>>(INT_MAX){};
};

template <class Elem>
using BlockingBoundedQueue = AbstractConcurrentBoundedQueue<Elem, moodycamel::BlockingConcurrentQueue<Elem>>;

template <class Elem>
class BlockingUnBoundedQueue : public AbstractConcurrentBoundedQueue<Elem, moodycamel::BlockingConcurrentQueue<Elem>> {
public:
    BlockingUnBoundedQueue()
        : AbstractConcurrentBoundedQueue<Elem, moodycamel::BlockingConcurrentQueue<Elem>>(INT_MAX){};
};

#ifdef _MACH_BOOLEAN_H_
// on mac, system headers define FALSE and TRUE as macros. Undefine them so that they don't break parser.
#undef TRUE
#undef FALSE
#endif // _MACH_BOOLEAN_H_

#endif // SORBET_CONCURRENTQUEUE_H
