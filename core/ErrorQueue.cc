#include "core/ErrorQueue.h"
#include "common/FileSystem.h"
#include "common/timers/Timer.h"
#include "core/Error.h"
#include "core/ErrorFlusherStdout.h"

namespace sorbet::core {

using namespace std;

ErrorQueueMessage ErrorQueueMessage::clone() {
    ErrorQueueMessage newMsg;

    newMsg.kind = this->kind;
    newMsg.whatFile = this->whatFile;
    newMsg.text = this->text;
    if (this->error) {
        newMsg.error = make_unique<Error>(*this->error);
    }
    if (this->queryResponse) {
        newMsg.queryResponse = make_unique<lsp::QueryResponse>(*this->queryResponse);
    }
    return newMsg;
}

ErrorQueue::ErrorQueue(spdlog::logger &logger, spdlog::logger &tracer, shared_ptr<ErrorFlusher> errorFlusher)
    : errorFlusher(errorFlusher), owner(this_thread::get_id()), logger(logger), tracer(tracer){};

void ErrorQueue::flushAllErrors(const GlobalState &gs) {
    checkOwned();

    Timer timeit(tracer, "ErrorQueue::flushAllErrors");
    auto collectedErrors = drainAll();

    for (auto &it : collectedErrors) {
        errorFlusher->flushErrors(logger, gs, it.first, move(it.second));
    }
}

bool ErrorQueue::wouldFlushErrorsForFile(FileRef file) const {
    // Must be able to to call this from multiple threads.
    // No checkedOwned call, and instead explicitly only use `const &` here.
    const auto &flusher = *errorFlusher;
    return flusher.wouldFlushErrors(file);
}

void ErrorQueue::flushErrorsForFile(const GlobalState &gs, FileRef file) {
    checkOwned();

    filesFlushedCount.fetch_add(1);
    Timer timeit(tracer, "ErrorQueue::flushErrorsForFile");

    core::ErrorQueueMessage msg;
    for (auto result = queue.try_pop(msg); result.gotItem(); result = queue.try_pop(msg)) {
        collected[msg.whatFile].emplace_back(make_unique<ErrorQueueMessage>(move(msg)));
    }

    errorFlusher->flushErrors(logger, gs, file, move(collected[file]));
}

void ErrorQueue::pushError(const core::GlobalState &gs, unique_ptr<core::Error> error) {
    core::ErrorQueueMessage msg;
    msg.kind = core::ErrorQueueMessage::Kind::Error;
    msg.whatFile = error->loc.file();
    if (!error->isSilenced) {
        this->nonSilencedErrorCount.fetch_add(1);
        // Serializing errors is expensive, so we only serialize them if the error isn't silenced.
        msg.text = error->toString(gs);
    }
    msg.error = move(error);
    this->queue.push(move(msg), 1);
}

void ErrorQueue::pushQueryResponse(core::FileRef fromFile, unique_ptr<core::lsp::QueryResponse> queryResponse) {
    core::ErrorQueueMessage msg;
    msg.kind = core::ErrorQueueMessage::Kind::QueryResponse;
    msg.whatFile = fromFile;
    msg.queryResponse = move(queryResponse);
    this->queue.push(move(msg), 1);
}

void ErrorQueue::checkOwned() {
    ENFORCE(owner == this_thread::get_id());
}

bool ErrorQueue::isEmpty() {
    checkOwned();
    return collected.empty();
}

UnorderedMap<core::FileRef, vector<unique_ptr<core::ErrorQueueMessage>>> ErrorQueue::drainAll() {
    checkOwned();

    core::ErrorQueueMessage msg;
    for (auto result = queue.try_pop(msg); result.gotItem(); result = queue.try_pop(msg)) {
        collected[msg.whatFile].emplace_back(make_unique<ErrorQueueMessage>(move(msg)));
    }

    auto out = std::move(collected);

    collected.clear();

    return out;
}

bool ErrorQueue::queueIsEmptyApprox() const {
    return this->queue.sizeEstimate() == 0;
}
} // namespace sorbet::core
