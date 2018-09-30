#include "ConcurrentErrorQueue.h"

using namespace std;

namespace sorbet::realmain {

void ConcurrentErrorQueue::pushError(const core::GlobalState &gs, unique_ptr<core::BasicError> error) {
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

void ConcurrentErrorQueue::collectForFile(core::FileRef whatFile, vector<unique_ptr<core::ErrorQueueMessage>> &out) {
    auto it = collected.find(whatFile);
    if (it == collected.end()) {
        return;
    }
    for (auto &error : it->second) {
        out.emplace_back(make_unique<core::ErrorQueueMessage>(move(error)));
    }
    collected[whatFile].clear();
};

vector<unique_ptr<core::ErrorQueueMessage>> ConcurrentErrorQueue::drainFlushed() {
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

void ConcurrentErrorQueue::markFileForFlushing(core::FileRef file) {
    core::ErrorQueueMessage msg;
    msg.kind = core::ErrorQueueMessage::Kind::Flush;
    msg.whatFile = file;
    this->queue.push(move(msg), 1);
}

void ConcurrentErrorQueue::pushQueryResponse(unique_ptr<core::QueryResponse> queryResponse) {
    core::ErrorQueueMessage msg;
    msg.kind = core::ErrorQueueMessage::Kind::QueryResponse;
    msg.queryResponse = move(queryResponse);
    this->queue.push(move(msg), 1);
}

ConcurrentErrorQueue::ConcurrentErrorQueue(spd::logger &logger, spd::logger &tracer) : ErrorQueue(logger, tracer) {
    owner = this_thread::get_id();
}

ConcurrentErrorQueue::~ConcurrentErrorQueue() {}

void ConcurrentErrorQueue::checkOwned() {
    ENFORCE(owner == this_thread::get_id());
}

vector<unique_ptr<core::ErrorQueueMessage>> ConcurrentErrorQueue::drainAll() {
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
} // namespace sorbet::realmain
