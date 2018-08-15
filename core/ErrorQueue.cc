#include "ErrorQueue.h"
#include "Errors.h"
#include "lsp/QueryResponse.h"

namespace sorbet {
namespace core {

using namespace std;

ErrorQueue::ErrorQueue(spdlog::logger &logger, spdlog::logger &tracer) : logger(logger), tracer(tracer) {}

ErrorQueue::~ErrorQueue() {}

std::vector<std::unique_ptr<core::QueryResponse>> ErrorQueue::drainQueryResponses() {
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

std::vector<std::unique_ptr<core::BasicError>> ErrorQueue::drainAllErrors() {
    checkOwned();
    std::vector<std::unique_ptr<core::BasicError>> out;
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
    std::vector<std::unique_ptr<ErrorQueueMessage>> errors;
    if (all) {
        errors = drainAll();
    } else {
        errors = drainFlushed();
    }
    errorFlusher.flushErrors(logger, move(errors));
}

void ErrorQueue::flushErrorCount() {
    errorFlusher.flushErrorCount(logger, errorCount);
}

void ErrorQueue::flushAutocorrects(const GlobalState &gs) {
    errorFlusher.flushAutocorrects(gs);
}

} // namespace core
} // namespace sorbet
