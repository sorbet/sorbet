#include "ConcurrentErrorQueue.h"

using namespace std;

namespace sorbet {
namespace realmain {

void ConcurrentErrorQueue::drainQueue() {
    ENFORCE(owner == this_thread::get_id());

    ErrorQueueMessage msg;
    for (auto result = queue.try_pop(msg); result.gotItem(); result = queue.try_pop(msg)) {
        collected[msg.whatFile].emplace_back(move(msg));
    }
}

vector<ErrorQueueMessage> ConcurrentErrorQueue::drainKind(ErrorQueueMessage::Kind kind) {
    drainQueue();

    vector<ErrorQueueMessage> res;
    for (auto &kv : collected) {
        for (auto it = kv.second.begin(); it != kv.second.end();) {
            if (it->kind == kind) {
                res.emplace_back(move(*it));
                it = kv.second.erase(it);
            } else {
                ++it;
            }
        }
    }
    return res;
}

vector<unique_ptr<core::QueryResponse>> ConcurrentErrorQueue::drainQueryResponses() {
    vector<unique_ptr<core::QueryResponse>> res;
    for (auto &msg : drainKind(ErrorQueueMessage::Kind::QueryResponse)) {
        res.emplace_back(move(msg.queryResponse));
    }

    // TODO: The LSP runner expects calling this method to also clear the errors
    // off the queue.
    for (auto &msg : drainKind(ErrorQueueMessage::Kind::Error)) {
        (void)msg;
    }
    return res;
}

vector<unique_ptr<core::BasicError>> ConcurrentErrorQueue::drainErrors() {
    vector<unique_ptr<core::BasicError>> res;
    for (auto &msg : drainKind(ErrorQueueMessage::Kind::Error)) {
        res.emplace_back(move(msg.error));
    }
    return res;
}

void ConcurrentErrorQueue::renderForFile(core::FileRef whatFile, stringstream &critical, stringstream &nonCritical) {
    auto it = collected.find(whatFile);
    if (it == collected.end()) {
        return;
    }
    for (auto &error : it->second) {
        auto &out = error.error->isCritical ? critical : nonCritical;
        if (out.tellp() != 0) {
            out << '\n';
        }
        out << error.text;

        for (auto &autocorrect : error.error->autocorrects) {
            autocorrects.emplace_back(move(autocorrect));
        }
    }
    collected[whatFile].clear();
};

void ConcurrentErrorQueue::flushErrors(bool all) {
    ENFORCE(owner == this_thread::get_id());

    stringstream critical;
    stringstream nonCritical;

    for (auto &msg : drainKind(ErrorQueueMessage::Kind::Error)) {
        this->errorCount.fetch_add(1);
        collected[msg.whatFile].emplace_back(move(msg));
    }
    for (auto &msg : drainKind(ErrorQueueMessage::Kind::Flush)) {
        renderForFile(msg.whatFile, critical, nonCritical);
        renderForFile(core::FileRef(), critical, nonCritical);
    }

    if (all) {
        for (auto &part : collected) {
            for (auto &error : part.second) {
                auto &out = error.error->isCritical ? critical : nonCritical;
                out << error.text;
            }
        }
        collected.clear();
    }

    if (critical.tellp() != 0) {
        if (!printedAtLeastOneError) {
            this->logger.log(spdlog::level::critical, "{}", critical.str());
            printedAtLeastOneError = true;
        } else {
            this->logger.log(spdlog::level::critical, "\n{}", critical.str());
        }
    }
    if (nonCritical.tellp() != 0) {
        if (!printedAtLeastOneError) {
            this->logger.log(spdlog::level::err, "{}", nonCritical.str());
            printedAtLeastOneError = true;
        } else {
            this->logger.log(spdlog::level::err, "\n{}", nonCritical.str());
        }
    }
}

void ConcurrentErrorQueue::flushErrorCount() {
    if (this->errorCount == 0) {
        this->logger.log(spdlog::level::err, "No errors! Great job.\n", this->errorCount);
    } else {
        this->logger.log(spdlog::level::err, "Errors: {}\n", this->errorCount);
    }
}

void ConcurrentErrorQueue::flushAutocorrects(const core::GlobalState &gs) {
    map<core::FileRef, string> sources;
    for (auto &autocorrect : autocorrects) {
        auto &file = autocorrect.loc.file;
        if (!sources.count(file)) {
            sources[file] = FileOps::read(file.data(gs).path());
        }
    }

    auto toWrite = core::AutocorrectSuggestion::apply(autocorrects, sources);
    for (auto &entry : toWrite) {
        FileOps::write(entry.first.data(gs).path(), entry.second);
    }
    autocorrects.clear();
}

void ConcurrentErrorQueue::flushFile(core::FileRef file) {
    ErrorQueueMessage msg;
    msg.kind = ErrorQueueMessage::Kind::Flush;
    msg.whatFile = file;
    this->queue.push(move(msg), 1);
}

void ConcurrentErrorQueue::pushError(const core::GlobalState &gs, unique_ptr<core::BasicError> error) {
    ErrorQueueMessage msg;
    msg.kind = ErrorQueueMessage::Kind::Error;
    msg.whatFile = error->loc.file;
    msg.text = error->toString(gs);
    msg.error = move(error);
    this->queue.push(move(msg), 1);
}

void ConcurrentErrorQueue::pushQueryResponse(unique_ptr<core::QueryResponse> queryResponse) {
    ErrorQueueMessage msg;
    msg.kind = ErrorQueueMessage::Kind::QueryResponse;
    msg.queryResponse = move(queryResponse);
    this->queue.push(move(msg), 1);
}

ConcurrentErrorQueue::ConcurrentErrorQueue(spd::logger &logger, spd::logger &tracer) : ErrorQueue(logger, tracer) {
    owner = this_thread::get_id();
}

ConcurrentErrorQueue::~ConcurrentErrorQueue() {
    flushErrors(true);
}
} // namespace realmain
} // namespace sorbet
