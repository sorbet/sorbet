#include "BufferedErrorQueue.h"
#include "core/lsp/QueryResponse.h"

using namespace std;

namespace sorbet {
namespace core {
BufferedErrorQueue::BufferedErrorQueue(spd::logger &logger, spd::logger &tracer) : ErrorQueue(logger, tracer) {}

BufferedErrorQueue::~BufferedErrorQueue() = default;

void BufferedErrorQueue::pushError(const GlobalState &gs, std::unique_ptr<BasicError> error) {
    errors.push_back(move(error));
}

void BufferedErrorQueue::pushQueryResponse(std::unique_ptr<QueryResponse> response) {
    queryResponses.push_back(move(response));
}

void BufferedErrorQueue::flushFile(FileRef file) {}
void BufferedErrorQueue::flushErrors(bool all) {}
void BufferedErrorQueue::flushErrorCount() {}
void BufferedErrorQueue::flushAutocorrects(const core::GlobalState &gs) {}

vector<unique_ptr<QueryResponse>> BufferedErrorQueue::drainQueryResponses() {
    vector<unique_ptr<QueryResponse>> out;
    swap(out, queryResponses);
    return out;
}

vector<unique_ptr<BasicError>> BufferedErrorQueue::drainErrors() {
    vector<unique_ptr<BasicError>> out;
    swap(out, errors);
    return out;
}

} // namespace core
} // namespace sorbet
