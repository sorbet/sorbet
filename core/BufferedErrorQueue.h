#ifndef SORBET_BUFFERED_ERRORQUEUE_H
#define SORBET_BUFFERED_ERRORQUEUE_H
#include "core/ErrorQueue.h"
#include "spdlog/spdlog.h"

namespace spd = spdlog;

namespace sorbet {
namespace core {
struct ErrorQueueMessage;
class BasicError;

// This is a simple, non-thread-safe, implementation of ErrorQueue that buffers
// all errors in memory for later retrieval.
class BufferedErrorQueue : public ErrorQueue {
private:
    std::vector<std::unique_ptr<ErrorQueueMessage>> errors;
    std::vector<std::unique_ptr<ErrorQueueMessage>> flushedErrors;
    virtual void checkOwned() override;

    virtual std::vector<std::unique_ptr<ErrorQueueMessage>> drainFlushed() override;
    virtual std::vector<std::unique_ptr<ErrorQueueMessage>> drainAll() override;

public:
    BufferedErrorQueue(spd::logger &logger, spd::logger &tracer);
    ~BufferedErrorQueue();

    virtual void pushError(const GlobalState &gs, std::unique_ptr<BasicError> error) override;
    virtual void pushQueryResponse(std::unique_ptr<QueryResponse> error) override;
    virtual void markFileForFlushing(FileRef file) override;
};
} // namespace core
} // namespace sorbet

#endif // SORBET_ERRORQUEUE_H
