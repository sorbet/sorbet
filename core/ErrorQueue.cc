#include "core/ErrorQueue.h"
#include "common/FileSystem.h"
#include "common/Timer.h"
#include "core/Error.h"
#include "core/ErrorFlusherStdout.h"

namespace sorbet::core {

namespace {
// In the case of location ties, determines which query response takes priority in the vector produced by
// drainWithQueryResponses. Larger values means greater specificity.
u2 getQueryResponseTypeSpecificity(const core::lsp::QueryResponse &q) {
    if (q.isEdit()) {
        // Only reported for autocomplete, and should take precedence over anything else reported
        return 8;
    } else if (q.isDefinition()) {
        return 7;
    } else if (q.isSend()) {
        return 6;
    } else if (q.isField()) {
        return 5;
    } else if (q.isIdent()) {
        return 4;
    } else if (q.isConstant()) {
        return 3;
    } else if (q.isLiteral()) {
        return 2;
    } else {
        return 1;
    }
}
} // namespace

using namespace std;

ErrorQueue::ErrorQueue(spdlog::logger &logger, spdlog::logger &tracer, shared_ptr<ErrorFlusher> errorFlusher)
    : errorFlusher(errorFlusher), owner(this_thread::get_id()), logger(logger), tracer(tracer){};

ErrorQueue::~ErrorQueue() {
    if (owner == this_thread::get_id()) {
        flushErrors(true);
    }
}

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
        if (leftTermLoc.endPos() != rightTermLoc.endPos()) {
            return leftTermLoc.endPos() < rightTermLoc.endPos();
        }
        // Locations tie! Tiebreak with the expected specificity of the response.
        return getQueryResponseTypeSpecificity(*left) > getQueryResponseTypeSpecificity(*right);
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

    Timer timeit(tracer, "ErrorQueue::flushErrors");
    vector<unique_ptr<ErrorQueueMessage>> errors;
    if (all) {
        errors = drainAll();
    } else {
        errors = drainFlushed();
    }
    errorFlusher->flushErrors(logger, move(errors));
}

void ErrorQueue::flushErrorCount() {
    errorFlusher->flushErrorCount(logger, nonSilencedErrorCount);
}

void ErrorQueue::flushAutocorrects(const GlobalState &gs, FileSystem &fs) {
    errorFlusher->flushAutocorrects(gs, fs);
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
    filesFlushedCount.fetch_add(1);
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

bool ErrorQueue::queueIsEmptyApprox() const {
    return this->queue.sizeEstimate() == 0;
}
} // namespace sorbet::core
