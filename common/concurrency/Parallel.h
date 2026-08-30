#ifndef SORBET_CONCURRENCY_PARALLEL_H
#define SORBET_CONCURRENCY_PARALLEL_H

#include "common/concurrency/ConcurrentQueue.h"
#include "common/concurrency/WorkerPool.h"

namespace sorbet {

class Parallel {
public:
    // Parallelize running `body` over all of the elements of `args` using the worker pool provided. This function will
    // return when all arguments have been processed.
    //
    // Additionally, if the argument list is small enough, or the worker pool is empty, iteration will take place on the
    // current thread instead of on the threads of the worker pool.
    template <typename T, typename Fn>
    static void iterate(WorkerPool &workers, std::string_view taskName, absl::Span<T> args, Fn body) {
        if (args.size() <= 1 || workers.size() == 0) {
            for (T &arg : args) {
                std::invoke(body, arg);
            }
        } else {
            auto taskq = std::make_shared<ConcurrentBoundedQueue<size_t>>(args.size());
            for (size_t i = 0; i < args.size(); ++i) {
                taskq->push(i, 1);
            }

            workers.multiplexJobWait(taskName, [taskq, &args, body]() mutable {
                size_t idx;
                for (auto result = taskq->try_pop(idx); !result.done(); result = taskq->try_pop(idx)) {
                    if (result.gotItem()) {
                        T &arg = args[idx];
                        std::invoke(body, arg);
                    }
                }
            });
        }
    }

    // Like `iterate`, but `body` receives runs of up to `chunkSize` consecutive elements of `args` (as an
    // `absl::Span<T>`), for work that is far cheaper per element than a queue operation.
    template <typename T, typename Fn>
    static void iterateChunked(WorkerPool &workers, std::string_view taskName, absl::Span<T> args, size_t chunkSize,
                               Fn body) {
        ENFORCE(chunkSize > 0);
        std::vector<absl::Span<T>> chunks;
        chunks.reserve((args.size() + chunkSize - 1) / chunkSize);
        for (size_t start = 0; start < args.size(); start += chunkSize) {
            chunks.emplace_back(args.subspan(start, chunkSize));
        }
        iterate(workers, taskName, absl::MakeSpan(chunks), [&body](absl::Span<T> &chunk) { std::invoke(body, chunk); });
    }
};

} // namespace sorbet

#endif
