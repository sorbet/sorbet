#ifndef SORBET_WORKERPOOL_H
#define SORBET_WORKERPOOL_H

#include "spdlog/spdlog.h"
namespace sorbet {
class WorkerPool {
public:
    inline static constexpr std::chrono::milliseconds BLOCK_INTERVAL() {
        using namespace std::chrono_literals;
        // NOTE: This value materially impacts IDE responsiveness during typechecking; the typechecking thread wakes up
        // at this interval and checks if it should do other work.
        return 20ms;
    }
    using Task = std::function<void()>;
    static std::unique_ptr<WorkerPool> create(int size, spdlog::logger &logger);

    // Run a job on each of the workers in the pool. This method returns after queueing the work on the pool, under the
    // expectation that the caller will coordinate reading results and waiting for jobs to finish.
    virtual void multiplexJob(std::string_view taskName, Task t) = 0;

    // A version of multiplexJob that can be used when the caller is only interested in blocking until the workers have
    // finished.
    virtual void multiplexJobWait(std::string_view taskName, Task t) = 0;

    virtual ~WorkerPool() = 0;
    virtual int size() = 0;
    WorkerPool() = default;
    WorkerPool(WorkerPool &) = delete;
    WorkerPool(const WorkerPool &) = delete;
    WorkerPool &operator=(WorkerPool &&) = delete;
    WorkerPool &operator=(const WorkerPool &) = delete;
};
};     // namespace sorbet
#endif // SORBET_WORKERPOOL_H
