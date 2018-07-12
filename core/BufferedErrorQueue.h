#ifndef SORBET_BUFFERED_ERRORQUEUE_H
#define SORBET_BUFFERED_ERRORQUEUE_H
#include "Errors.h"
#include "spdlog/spdlog.h"

namespace spd = spdlog;

namespace sorbet {
namespace core {

// This is a simple, non-thread-safe, implementation of ErrorQueue that buffers
// all errors in memory for later retrieval.
class BufferedErrorQueue : public ErrorQueue {
private:
    std::vector<std::unique_ptr<QueryResponse>> queryResponses;
    std::vector<std::unique_ptr<BasicError>> errors;

public:
    BufferedErrorQueue(spd::logger &logger, spd::logger &tracer);
    ~BufferedErrorQueue();

    virtual void pushError(const GlobalState &gs, std::unique_ptr<BasicError> error) override;
    virtual void pushQueryResponse(std::unique_ptr<QueryResponse> error) override;
    virtual void flushFile(FileRef file) override;
    virtual void flushErrors(bool all = false) override;

    std::vector<std::unique_ptr<QueryResponse>> drainQueryResponses();
    std::vector<std::unique_ptr<BasicError>> drainErrors();
};
} // namespace core
} // namespace sorbet

#endif // SORBET_ERRORQUEUE_H
