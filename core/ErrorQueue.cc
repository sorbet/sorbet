#include "core/ErrorQueue.h"
#include "core/Errors.h"
#include "lsp/QueryResponse.h"

namespace sorbet::core {

using namespace std;

ErrorQueue::ErrorQueue(spdlog::logger &logger, spdlog::logger &tracer) : logger(logger), tracer(tracer) {}

ErrorQueue::~ErrorQueue() {}

vector<unique_ptr<core::QueryResponse>> ErrorQueue::drainQueryResponses() {
    checkOwned();
    vector<unique_ptr<core::QueryResponse>> out;

    auto collected = drainAll();

    for (auto &msg : collected) {
        if (msg->kind == ErrorQueueMessage::Kind::QueryResponse) {
            out.emplace_back(move(msg->queryResponse));
        }
    }

    return out;
}

vector<unique_ptr<core::BasicError>> ErrorQueue::drainAllErrors() {
    checkOwned();
    vector<unique_ptr<core::BasicError>> out;
    auto collected = drainAll();

    for (auto &msg : collected) {
        if (msg->kind == ErrorQueueMessage::Kind::Error) {
            out.emplace_back(move(msg->error));
        }
    }

    return out;
}

void ErrorQueue::flushErrors(bool all) {
    checkOwned();
    if (ignoreFlushes) {
        return;
    }
    vector<unique_ptr<ErrorQueueMessage>> errors;
    if (all) {
        errors = drainAll();
    } else {
        errors = drainFlushed();
    }
    errorFlusher.flushErrors(logger, move(errors));
}

void ErrorQueue::flushErrorCount() {
    errorFlusher.flushErrorCount(logger, nonSilencedErrorCount);
}

void ErrorQueue::flushAutocorrects(const GlobalState &gs) {
    errorFlusher.flushAutocorrects(gs);
}

} // namespace sorbet::core
