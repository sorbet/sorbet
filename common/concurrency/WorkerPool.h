#ifndef SORBET_WORKERPOOL_H
#define SORBET_WORKERPOOL_H

#include "spdlog/spdlog.h"
namespace sorbet {
class WorkerPool {
public:
    class MultiplexCleanup {
    public:
        MultiplexCleanup() = default;

        MultiplexCleanup(MultiplexCleanup &&) = default;
        MultiplexCleanup &operator=(MultiplexCleanup &&) = default;

        MultiplexCleanup(const MultiplexCleanup &) = delete;
        MultiplexCleanup &operator=(const MultiplexCleanup &) = delete;

        // It would be neat if we could say that this method can only be called once.
        // One way to do that would be to make this a static method somewhere, and accept the
        // receiver as a && value. But clang currently won't check use-after-move errors in Sorbet,
        // so there's no point to factoring it like that IMO.
        void cleanup(WorkerPool &workers) {
            workers.consumeCounters();
        }
    };

    inline static constexpr std::chrono::milliseconds BLOCK_INTERVAL() {
        using namespace std::chrono_literals;
        // NOTE: This value materially impacts IDE responsiveness during typechecking; the typechecking thread wakes up
        // at this interval and checks if it should do other work.
        return 20ms;
    }
    using Task = std::function<void()>;
    static std::unique_ptr<WorkerPool> create(int size, spdlog::logger &logger);

    // TODO(jez) C++20 lets us put this message into the nodiscard attribute as a string
    // You must call multiplexCleanup.consumeCounters(workers) from the main thread after you're
    // done processing all thread results.
    [[nodiscard]] virtual MultiplexCleanup multiplexJob(std::string_view taskName, Task t) = 0;
    virtual ~WorkerPool() = 0;
    virtual int size() = 0;
    WorkerPool() = default;
    WorkerPool(WorkerPool &) = delete;
    WorkerPool(const WorkerPool &) = delete;
    WorkerPool &operator=(WorkerPool &&) = delete;
    WorkerPool &operator=(const WorkerPool &) = delete;

private:
    friend MultiplexCleanup;

    virtual void consumeCounters() = 0;
};
};     // namespace sorbet
#endif // SORBET_WORKERPOOL_H
