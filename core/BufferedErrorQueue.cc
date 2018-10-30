#include "core/BufferedErrorQueue.h"
#include "core/lsp/QueryResponse.h"

using namespace std;

namespace sorbet::core {
BufferedErrorQueue::BufferedErrorQueue(spd::logger &logger, spd::logger &tracer, vector<int> errorCodeWhiteList)
    : ErrorQueue(logger, tracer, errorCodeWhiteList) {}

BufferedErrorQueue::~BufferedErrorQueue() = default;

void BufferedErrorQueue::pushError(const GlobalState &gs, unique_ptr<Error> error) {
    if (!error->isSilenced) {
        this->nonSilencedErrorCount.fetch_add(1);
    }
    auto msg = make_unique<ErrorQueueMessage>();
    msg->text = error->toString(gs);
    msg->error = move(error);
    msg->kind = ErrorQueueMessage::Kind::Error;
    errors.emplace_back(move(msg));
}

void BufferedErrorQueue::pushQueryResponse(unique_ptr<QueryResponse> response) {
    auto msg = make_unique<ErrorQueueMessage>();
    msg->queryResponse = move(response);
    msg->kind = ErrorQueueMessage::Kind::QueryResponse;
    errors.emplace_back(move(msg));
}

vector<unique_ptr<ErrorQueueMessage>> BufferedErrorQueue::drainFlushed() {
    vector<unique_ptr<ErrorQueueMessage>> out;
    swap(out, flushedErrors);
    return out;
}

vector<unique_ptr<ErrorQueueMessage>> BufferedErrorQueue::drainAll() {
    vector<unique_ptr<ErrorQueueMessage>> out;
    swap(out, flushedErrors);
    for (auto &msg : errors) {
        out.emplace_back(move(msg));
    }
    errors.clear();
    return out;
}

void BufferedErrorQueue::checkOwned() {}

void BufferedErrorQueue::markFileForFlushing(FileRef file) {
    this->errors.erase(remove_if(
        this->errors.begin(), this->errors.end(), [file, &flushedErrors = this->flushedErrors](auto &msg) -> bool {
            if (msg->kind != ErrorQueueMessage::Kind::Error || msg->error->loc.file() != file) {
                return false;
            }
            flushedErrors.emplace_back(move(msg));
            return true;
        }));
}

} // namespace sorbet::core
