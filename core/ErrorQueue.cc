#include "core/ErrorQueue.h"
#include "core/Error.h"

namespace sorbet::core {

using namespace std;

ErrorQueue::ErrorQueue(spdlog::logger &logger, spdlog::logger &tracer, vector<int> errorCodeWhiteList)
    : errorFlusher(move(errorCodeWhiteList)), owner(this_thread::get_id()), logger(logger), tracer(tracer){};

pair<vector<unique_ptr<core::Error>>, vector<unique_ptr<core::lsp::QueryResponse>>>
ErrorQueue::drainWithQueryResponses() {
    checkOwned();
    vector<unique_ptr<core::lsp::QueryResponse>> outResponses;
    vector<unique_ptr<core::Error>> out;

    auto collected = drainAll();

    out.reserve(collected.size());
    for (auto &msg : collected) {
        if (msg->kind == ErrorQueueMessage::Kind::QueryResponse) {
            outResponses.emplace_back(move(msg->queryResponse));
        }
        if (msg->kind == ErrorQueueMessage::Kind::Error) {
            out.emplace_back(move(msg->error));
        }
    }

    stable_sort(outResponses.begin(), outResponses.end(), [](auto &left, auto &right) -> bool {
        /* we want the most precise information to go first. Normally, they are computed in this order by construction,
         * but threading artifact might reorder them, thus we'd like to sort them */
        auto leftTermLoc = left->getLoc();
        auto rightTermLoc = right->getLoc();
        auto leftLength = leftTermLoc.endPos() - leftTermLoc.beginPos();
        auto rightLength = rightTermLoc.endPos() - rightTermLoc.beginPos();
        if (leftLength != rightLength) {
            return leftLength < rightLength;
        }
        if (leftTermLoc.beginPos() != rightTermLoc.beginPos()) {
            return leftTermLoc.beginPos() < rightTermLoc.beginPos();
        }
        return leftTermLoc.endPos() < rightTermLoc.endPos();
    });

    return make_pair(move(out), move(outResponses));
}

vector<unique_ptr<core::Error>> ErrorQueue::drainAllErrors() {
    return move(drainWithQueryResponses().first);
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

void ErrorQueue::pushError(const core::GlobalState &gs, unique_ptr<core::Error> error) {
    if (!error->isSilenced) {
        this->nonSilencedErrorCount.fetch_add(1);
    }
    core::ErrorQueueMessage msg;
    msg.kind = core::ErrorQueueMessage::Kind::Error;
    msg.whatFile = error->loc.file();
    msg.text = error->toString(gs);
    msg.error = move(error);
    this->queue.push(move(msg), 1);
}

void ErrorQueue::collectForFile(core::FileRef whatFile, vector<unique_ptr<core::ErrorQueueMessage>> &out) {
    auto it = collected.find(whatFile);
    if (it == collected.end()) {
        return;
    }
    for (auto &error : it->second) {
        out.emplace_back(make_unique<core::ErrorQueueMessage>(move(error)));
    }
    collected[whatFile].clear();
};

vector<unique_ptr<core::ErrorQueueMessage>> ErrorQueue::drainFlushed() {
    checkOwned();

    vector<unique_ptr<core::ErrorQueueMessage>> ret;

    core::ErrorQueueMessage msg;
    for (auto result = queue.try_pop(msg); result.gotItem(); result = queue.try_pop(msg)) {
        if (msg.kind == core::ErrorQueueMessage::Kind::Flush) {
            collectForFile(msg.whatFile, ret);
            collectForFile(core::FileRef(), ret);
        } else {
            collected[msg.whatFile].emplace_back(move(msg));
        }
    }

    return ret;
}

void ErrorQueue::markFileForFlushing(core::FileRef file) {
    core::ErrorQueueMessage msg;
    msg.kind = core::ErrorQueueMessage::Kind::Flush;
    msg.whatFile = file;
    this->queue.push(move(msg), 1);
}

void ErrorQueue::pushQueryResponse(unique_ptr<core::lsp::QueryResponse> queryResponse) {
    core::ErrorQueueMessage msg;
    msg.kind = core::ErrorQueueMessage::Kind::QueryResponse;
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
vector<unique_ptr<core::ErrorQueueMessage>> ErrorQueue::drainAll() {
    checkOwned();
    auto out = drainFlushed();

    for (auto &part : collected) {
        for (auto &error : part.second) {
            out.emplace_back(make_unique<core::ErrorQueueMessage>(move(error)));
        }
    }
    collected.clear();

    return out;
}
} // namespace sorbet::core
