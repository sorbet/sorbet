#ifndef SORBET_WORKERPOOL_IMPL_H
#define SORBET_WORKERPOOL_IMPL_H
#include "blockingconcurrentqueue.h"
#include "common/concurrency/WorkerPool.h"
#include "common/os/os.h"
#include "spdlog/spdlog.h"
#include <memory>
#include <vector>
namespace sorbet {
class WorkerPoolImpl : public WorkerPool {
    int _size;
    // Tune queue for small size
    struct ConcurrentQueueCustomTraits {
        // General-purpose size type. std::size_t is strongly recommended.
        using size_t = std::size_t;

        // The type used for the enqueue and dequeue indices. Must be at least as
        // large as size_t. Should be significantly larger than the number of elements
        // you expect to hold at once, especially if you have a high turnover rate;
        // for example, on 32-bit x86, if you expect to have over a hundred million
        // elements or pump several million elements through your queue in a very
        // short space of time, using a 32-bit type *may* trigger a race condition.
        // A 64-bit int type is recommended in that case, and in practice will
        // prevent a race condition no matter the usage of the queue. Note that
        // whether the queue is lock-free with a 64-int type depends on the whether
        // std::atomic<std::uint64_t> is lock-free, which is platform-specific.
        using index_t = std::size_t;

        // Internally, all elements are enqueued and dequeued from multi-element
        // blocks; this is the smallest controllable unit. If you expect few elements
        // but many producers, a smaller block size should be favoured. For few producers
        // and/or many elements, a larger block size is preferred. A sane default
        // is provided. Must be a power of 2.
        static const size_t BLOCK_SIZE = 2;

        // For explicit producers (i.e. when using a producer token), the block is
        // checked for being empty by iterating through a list of flags, one per element.
        // For large block sizes, this is too inefficient, and switching to an atomic
        // counter-based approach is faster. The switch is made for block sizes strictly
        // larger than this threshold.
        static const size_t EXPLICIT_BLOCK_EMPTY_COUNTER_THRESHOLD = 32;

        // How many full blocks can be expected for a single explicit producer? This should
        // reflect that number's maximum for optimal performance. Must be a power of 2.
        static const size_t EXPLICIT_INITIAL_INDEX_SIZE = 2;

        // How many full blocks can be expected for a single implicit producer? This should
        // reflect that number's maximum for optimal performance. Must be a power of 2.
        static const size_t IMPLICIT_INITIAL_INDEX_SIZE = 2;

        // The initial size of the hash table mapping thread IDs to implicit producers.
        // Note that the hash is resized every time it becomes half full.
        // Must be a power of two, and either 0 or at least 1. If 0, implicit production
        // (using the enqueue methods without an explicit producer token) is disabled.
        static const size_t INITIAL_IMPLICIT_PRODUCER_HASH_SIZE = 1;

        // Controls the number of items that an explicit consumer (i.e. one with a token)
        // must consume before it causes all consumers to rotate and move on to the next
        // internal queue.
        static const std::uint32_t EXPLICIT_CONSUMER_CONSUMPTION_QUOTA_BEFORE_ROTATE = 256;

        // The maximum number of elements (inclusive) that can be enqueued to a sub-queue.
        // Enqueue operations that would cause this limit to be surpassed will fail. Note
        // that this limit is enforced at the block level (for performance reasons), i.e.
        // it's rounded up to the nearest block size.
        static const size_t MAX_SUBQUEUE_SIZE = 16;

        // Memory allocation can be customized if needed.
        // malloc should return nullptr on failure, and handle alignment like std::malloc.
        static inline void *malloc(size_t size) {
            return std::malloc(size);
        }
        static inline void free(void *ptr) {
            return std::free(ptr);
        }
    };
    using Task_ = std::function<bool()>; // return value indicates if the worker should continie gathering jobs
    using Queue = moodycamel::BlockingConcurrentQueue<Task_, ConcurrentQueueCustomTraits>;
    // ORDER IS IMPORTANT. threads must be killed before Queues.
    std::vector<std::unique_ptr<Queue>> threadQueues;
    std::vector<std::unique_ptr<Joinable>> threads;
    spdlog::logger &logger;

    void multiplexJob_(Task_ t);

public:
    WorkerPoolImpl(int size, spdlog::logger &logger);
    ~WorkerPoolImpl();

    void multiplexJob(std::string_view taskName, Task t) override;
    int size() override;
};
};     // namespace sorbet
#endif // SORBET_WORKERPOOL_IMPL_H
