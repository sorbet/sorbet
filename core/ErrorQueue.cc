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

void ErrorQueue::flushAllErrors(GlobalState &gs) {
    checkOwned();

    Timer timeit(tracer, "ErrorQueue::flushAllErrors");
    auto collectedErrors = drainAll();

    for (auto &[file, errors] : gs.errors) {
        move(errors.begin(), errors.end(), std::back_inserter(collectedErrors[file]));
        errors.clear();
    }
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

void ErrorQueue::flushErrors(const GlobalState &gs, FileRef file,
                             std::vector<std::unique_ptr<ErrorQueueMessage>> errors) {
    errorFlusher->flushErrors(logger, gs, file, move(errors));
}

vector<unique_ptr<ErrorQueueMessage>> ErrorQueue::flushErrorsForFile(const GlobalState &gs, FileRef file) {
    checkOwned();

    Timer timeit(tracer, "ErrorQueue::flushErrorsForFile");

    vector<unique_ptr<ErrorQueueMessage>> newErrors;
    core::ErrorQueueMessage msg;
    for (auto result = queue.try_pop(msg); result.gotItem(); result = queue.try_pop(msg)) {
        newErrors.emplace_back(make_unique<ErrorQueueMessage>(msg.clone()));
        collected[msg.whatFile].emplace_back(make_unique<ErrorQueueMessage>(move(msg)));
    }

    auto prevErrorsIt = gs.errors.find(file);
    if (prevErrorsIt != gs.errors.end()) {
        auto &prevErrors = (*prevErrorsIt).second;
        for (auto &e : prevErrors) {
            auto cloned = make_unique<ErrorQueueMessage>(e->clone());
            collected[file].emplace_back(move(cloned));
        }
    }

    errorFlusher->flushErrors(logger, gs, file, move(collected[file]));

    return newErrors;
}

void ErrorQueue::flushButRetainErrorsForFile(GlobalState &gs, FileRef file) {
    checkOwned();

    Timer timeit(tracer, "ErrorQueue::flushButRetainErrorsForFile");

    core::ErrorQueueMessage msg;
    for (auto result = queue.try_pop(msg); result.gotItem(); result = queue.try_pop(msg)) {
        if (!result.gotItem()) {
            continue;
        }

        collected[msg.whatFile].emplace_back(make_unique<ErrorQueueMessage>(move(msg)));
    }

    for (auto &e : collected[file]) {
        gs.errors[file].emplace_back(move(e));
    }
    collected[file].clear();

    std::vector<std::unique_ptr<core::ErrorQueueMessage>> errorsToFlush;
    for (auto &e : gs.errors[file]) {
        auto cloned = make_unique<core::ErrorQueueMessage>(e->clone());
        errorsToFlush.emplace_back(move(cloned));
    }

    errorFlusher->flushErrors(logger, gs, file, move(errorsToFlush));
};

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
