#ifndef SORBET_CONCURRENCY_PARALLEL_H
#define SORBET_CONCURRENCY_PARALLEL_H

#include "absl/types/span.h"
#include "common/concurrency/ConcurrentIndex.h"
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
            // We can avoid the use of a `shared_ptr` here because we're using `multiplexJobWait` below. That ensures
            // that this function won't return before the workers have all completed.
            ConcurrentIndex index(args.size());

            workers.multiplexJobWait(taskName, [&index, &args, body]() mutable {
                while (auto ix = index.next()) {
                    T &arg = args[*ix];
                    std::invoke(body, arg);
                }
            });
        }
    }
};

} // namespace sorbet

#endif
