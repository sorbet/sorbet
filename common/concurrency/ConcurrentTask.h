#ifndef SORBET_CONCURRENTTASK_H
#define SORBET_CONCURRENTTASK_H

#include "absl/synchronization/barrier.h"
#include "common/ConstExprStr.h"
#include "common/Counters.h"
#include "common/concurrency/ConcurrentQueue.h"
#include "common/concurrency/WorkerPool.h"

namespace sorbet {
template <typename I, typename WorkerState, bool CollectCounters = false> class ConcurrentTask {
    std::shared_ptr<ConcurrentBoundedQueue<I>> inputq;
    ConstExprStr metricName;
    ConstExprStr workerMetricName;
    const size_t size;
    spdlog::logger &tracer;
    WorkerPool &workers;

public:
    ConcurrentTask(ConstExprStr metricName, ConstExprStr workerMetricName, size_t size, spdlog::logger &tracer,
                   WorkerPool &workers)
        : inputq(std::make_shared<ConcurrentBoundedQueue<I>>(size)), metricName(metricName),
          workerMetricName(workerMetricName), size(size), tracer(tracer), workers(workers) {}

    void enqueue(I &&input) {
        inputq->push(std::move(input), 1);
    }

    void run(std::function<void(I &&, WorkerState &)> processTask, std::function<void(WorkerState &&)> combineOutput) {
        ENFORCE_NO_TIMER(inputq->hasAllInput(), "Input queue is not filled; did you specify the wrong queue size?");
        Timer timeit(tracer, metricName);

        auto outputq = std::make_shared<BlockingBoundedQueue<WorkerState>>(size);
        std::shared_ptr<BlockingBoundedQueue<CounterState>> counters;
        if (CollectCounters) {
            // If workers is empty, the main thread will run the job.
            counters = std::make_shared<BlockingBoundedQueue<CounterState>>(std::max(workers.size(), 1));
        }

        // Note: Cannot be stack allocated; see docs for absl::Barrier. The +1 is for the control thread.
        auto workerBarrier = new absl::Barrier(workers.size() + 1);

        // Note: We must copy `barrier` pointer into the lambda, as `delete barrier` races with the coordinator thread
        // destroying the stack frame.
        workers.multiplexJob(metricName.str, [&tracer = this->tracer, &workerMetricName = this->workerMetricName,
                                              inputq = this->inputq, outputq, processTask, counters,
                                              &workers = this->workers, barrier = workerBarrier]() {
            {
                Timer timeit(tracer, workerMetricName);
                I input;
                size_t count = 0;
                WorkerState state;
                for (auto result = inputq->try_pop(input); !result.done(); result = inputq->try_pop(input)) {
                    if (result.gotItem()) {
                        count++;
                        processTask(std::move(input), state);
                    }
                }
                if (count > 0) {
                    outputq->push(std::move(state), count);
                }

                if (CollectCounters) {
                    counters->push(getAndClearThreadCounters(), 1);
                }
            }

            // Prevent deadlock with an empty workerpool, where all logic runs on one thread.
            // After this LOC, it is no longer safe to use `this` or refer to any variables in the `run` stack
            // frame.
            if (workers.size() > 0 && barrier->Block()) {
                delete barrier;
            }
        });

        {
            WorkerState threadResult;
            for (auto result = outputq->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), tracer);
                 !result.done(); result = outputq->wait_pop_timed(threadResult, WorkerPool::BLOCK_INTERVAL(), tracer)) {
                if (result.gotItem()) {
                    combineOutput(std::move(threadResult));
                }
            }

            if (CollectCounters) {
                CounterState threadCounters;
                for (auto result = counters->wait_pop_timed(threadCounters, WorkerPool::BLOCK_INTERVAL(), tracer);
                     !result.done();
                     result = counters->wait_pop_timed(threadCounters, WorkerPool::BLOCK_INTERVAL(), tracer)) {
                    if (result.gotItem()) {
                        counterConsume(std::move(threadCounters));
                    }
                }
            }
        }

        // Wait for all workers to complete before exiting. Prevents the scenario where this function completes before
        // all worker threads, which causes a SEGFAULT as the thread tries to access deallocated objects.
        // That scenario is possible if one thread isn't scheduled to run until after all other threads have finished
        // processing work.
        if (workerBarrier->Block()) {
            delete workerBarrier;
        }
    }
};
} // namespace sorbet

#endif