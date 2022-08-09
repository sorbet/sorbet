#ifndef SORBET_ERROR_QUEUE_H
#define SORBET_ERROR_QUEUE_H

#include "common/concurrency/ConcurrentQueue.h"
#include "core/ErrorFlusher.h"
#include "core/ErrorFlusherStdout.h"
#include "core/ErrorQueueMessage.h"
#include "core/lsp/QueryResponse.h"
#include <atomic>

namespace sorbet {
class FileSystem;

namespace core {
class ErrorQueue {
private:
    void checkOwned();
    UnorderedMap<core::FileRef, std::vector<std::unique_ptr<ErrorQueueMessage>>> drainAll();
    std::shared_ptr<ErrorFlusher> errorFlusher;
    const std::thread::id owner;
    UnorderedMap<core::FileRef, std::vector<std::unique_ptr<ErrorQueueMessage>>> collected;
    ConcurrentUnBoundedQueue<core::ErrorQueueMessage> queue;

public:
    spdlog::logger &logger;
    spdlog::logger &tracer;
    std::atomic<bool> hadCritical{false};
    std::atomic<int> nonSilencedErrorCount{0};
    std::atomic<int> filesFlushedCount{0};

    ErrorQueue(spdlog::logger &logger, spdlog::logger &tracer,
               std::shared_ptr<ErrorFlusher> errorFlusher = std::make_shared<ErrorFlusherStdout>());

    /** register a new error to be reported */
    void pushError(const GlobalState &gs, std::unique_ptr<Error> error);
    void pushQueryResponse(core::FileRef fromFile, std::unique_ptr<lsp::QueryResponse> response);
    bool isEmpty();

    void flushAllErrors(const GlobalState &gs);
    void flushErrorsForFile(const GlobalState &gs, FileRef file);
    bool wouldFlushErrorsForFile(FileRef file) const;

    /** Checks if the queue is empty. Is approximate if there are any concurrent dequeue/enqueue operations */
    bool queueIsEmptyApprox() const;
};

} // namespace core
} // namespace sorbet

#endif
