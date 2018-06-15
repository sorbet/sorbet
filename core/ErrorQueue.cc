#include "ErrorQueue.h"

using namespace std;

namespace sorbet {
namespace core {
vector<unique_ptr<BasicError>> ErrorQueue::drainErrors() {
    ENFORCE(owner == this_thread::get_id());
    vector<unique_ptr<BasicError>> res;
    for (auto &alreadyCollected : collected) {
        for (auto &entry : alreadyCollected.second) {
            res.emplace_back(move(entry.error));
        }
    }
    collected.clear();
    ErrorQueueMessage msg;
    for (auto result = queue.try_pop(msg); result.gotItem(); result = queue.try_pop(msg)) {
        if (msg.kind == ErrorQueueMessage::Kind::Error) {
            res.emplace_back(move(msg.error));
        }
    }
    return res;
}

ErrorQueue::ErrorQueue(spd::logger &logger, spd::logger &tracer) : logger(logger), tracer(tracer) {
    owner = this_thread::get_id();
}

void ErrorQueue::renderForFile(core::FileRef whatFile, stringstream &critical, stringstream &nonCritical) {
    auto it = collected.find(whatFile);
    if (it == collected.end()) {
        return;
    }
    for (auto &error : it->second) {
        auto &out = error.error->isCritical ? critical : nonCritical;
        out << error.text;
    }
    collected[whatFile].clear();
};

void ErrorQueue::flushErrors(bool all) {
    ENFORCE(owner == this_thread::get_id());

    stringstream critical;
    stringstream nonCritical;

    ErrorQueueMessage msg;
    for (auto result = queue.try_pop(msg); result.gotItem(); result = queue.try_pop(msg)) {
        if (msg.kind == ErrorQueueMessage::Kind::Error) {
            this->errorCount.fetch_add(1);
            collected[msg.whatFile].emplace_back(move(msg));
        } else if (msg.kind == ErrorQueueMessage::Kind::Flush) {
            renderForFile(msg.whatFile, critical, nonCritical);
            renderForFile(core::FileRef(), critical, nonCritical);
        }
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
        this->logger.log(spdlog::level::critical, "{}", critical.str());
    }
    if (nonCritical.tellp() != 0) {
        this->logger.log(spdlog::level::err, "{}", nonCritical.str());
    }
}

ErrorQueue::~ErrorQueue() {
    flushErrors(true);
}
} // namespace core
} // namespace sorbet
