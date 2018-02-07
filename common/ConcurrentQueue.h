#ifndef SRUBY_CONCURRENTQUEUE_H
#define SRUBY_CONCURRENTQUEUE_H
#include "blockingconcurrentqueue.h"
#include <atomic>
#include <chrono>

struct DequeueResult {
    bool returned;
    bool shouldRetry;
    inline bool done() {
        return !(returned || shouldRetry);
    }
    inline bool gotItem() {
        return returned;
    }
};

/* A thread safe lock free queue that has safe publication guarantees, that is only used to process N elements */
template <class Elem, class Queue> class AbstractConcurrentBoundedQueue {
    Queue _queue;
    const int bound;
    std::atomic<int> elementsLeftToPush; // double serves as a counter and as safe publication marker
    std::atomic<int> elementsPopped;

public:
    AbstractConcurrentBoundedQueue(int bound) : bound(bound), elementsLeftToPush(bound), elementsPopped(0) {}
    AbstractConcurrentBoundedQueue(const AbstractConcurrentBoundedQueue &other) = delete;
    AbstractConcurrentBoundedQueue(AbstractConcurrentBoundedQueue &&other) = delete;

    inline void push(Elem &&elem, int count) {
        _queue.enqueue(std::move(elem));
        elementsLeftToPush.fetch_add(-count, std::memory_order_release);
        ENFORCE(elementsLeftToPush.load(std::memory_order_relaxed) >= 0);
    }

    inline DequeueResult try_pop(Elem &elem) {
        DequeueResult ret;
        ret.shouldRetry = elementsLeftToPush.load(std::memory_order_acquire) != 0;
        ret.returned = _queue.try_dequeue(elem);
        if (ret.returned) {
            elementsPopped.fetch_add(1, std::memory_order_relaxed);
        }
        return ret;
    }

    template <typename Rep, typename Period>
    inline DequeueResult wait_pop_timed(Elem &elem, std::chrono::duration<Rep, Period> const &timeout) {
        DequeueResult ret;
        ret.shouldRetry = elementsLeftToPush.load(std::memory_order_acquire) != 0;
        if (ret.shouldRetry) {
            ret.returned = _queue.wait_dequeue_timed(elem, timeout);
        } else { // all elements has been pushed, no need to wait.
            ret.returned = _queue.try_dequeue(elem);
        }
        if (ret.returned) {
            elementsPopped.fetch_add(1, std::memory_order_relaxed);
        }
        return ret;
    }

    int doneEstimate() {
        return elementsPopped.load(std::memory_order_relaxed);
    }

    int enqueuedEstimate() {
        return bound - elementsLeftToPush.load(std::memory_order_relaxed);
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

#endif // SRUBY_CONCURRENTQUEUE_H
