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
    std::vector<std::unique_ptr<ErrorQueueMessage>> drainAll();
    std::vector<std::unique_ptr<ErrorQueueMessage>> drainFlushed();
    void collectForFile(core::FileRef whatFile, std::vector<std::unique_ptr<core::ErrorQueueMessage>> &out);
    std::shared_ptr<ErrorFlusher> errorFlusher;
    const std::thread::id owner;
    UnorderedMap<core::FileRef, std::vector<core::ErrorQueueMessage>> collected;
    ConcurrentUnBoundedQueue<core::ErrorQueueMessage> queue;

public:
    spdlog::logger &logger;
    spdlog::logger &tracer;
    std::atomic<bool> hadCritical{false};
    std::atomic<int> nonSilencedErrorCount{0};
    std::atomic<int> filesFlushedCount{0};
    bool ignoreFlushes{false};

    ErrorQueue(spdlog::logger &logger, spdlog::logger &tracer,
               std::shared_ptr<ErrorFlusher> errorFlusher = std::make_shared<ErrorFlusherStdout>());
    ~ErrorQueue();

    /** register a new error to be reported */
    void pushError(const GlobalState &gs, std::unique_ptr<Error> error);
    void pushQueryResponse(std::unique_ptr<lsp::QueryResponse> response);
    /** indicate that errors for `file` should be flushed on next call to to flushErrors */
    void markFileForFlushing(FileRef file);
    /** Extract all query responses. This discards all errors currently present in error Queue */
    std::pair<std::vector<std::unique_ptr<core::Error>>, std::vector<std::unique_ptr<core::lsp::QueryResponse>>>
    drainWithQueryResponses();
    bool isEmpty();
    /** Extract all errors. This discards all query responses currently present in error Queue */
    std::vector<std::unique_ptr<core::Error>> drainAllErrors();

    void flushErrors(bool all = false);
    void flushErrorCount();
    void flushAutocorrects(const GlobalState &gs, FileSystem &fs);

    /** Checks if the queue is empty. Is approximate if there are any concurrent dequeue/enqueue operations */
    bool queueIsEmptyApprox() const;
};

} // namespace core
} // namespace sorbet

#endif
