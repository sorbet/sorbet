#include "absl/synchronization/mutex.h"
#include "common/Timer.h"
#include "lsp.h"
#include <deque>
#include <iostream>

using namespace std;

namespace sorbet::realmain::lsp {

bool safeGetline(istream &is, string &t) {
    t.clear();

    // The characters in the stream are read one-by-one using a streambuf.
    // That is faster than reading them one-by-one using the istream.
    // Code that uses streambuf this way must be guarded by a sentry object.
    // The sentry object performs various tasks,
    // such as thread synchronization and updating the stream state.

    istream::sentry se(is, true);
    streambuf *sb = is.rdbuf();

    for (;;) {
        int c = sb->sbumpc();
        switch (c) {
            case '\n':
                return true;
            case '\r':
                if (sb->sgetc() == '\n')
                    sb->sbumpc();
                return true;
            case streambuf::traits_type::eof():
                // Also handle the case when the last line has no line ending
                if (t.empty())
                    is.setstate(ios::eofbit);
                return false;
            default:
                t += (char)c;
        }
    }
}

bool getNewRequest(rapidjson::Document &d, const shared_ptr<spd::logger> &logger, istream &inputStream) {
    int length = -1;
    {
        string line;
        while (safeGetline(inputStream, line)) {
            logger->trace("raw read: {}", line);
            if (line == "") {
                break;
            }
            sscanf(line.c_str(), "Content-Length: %i", &length);
        }
        logger->trace("final raw read: {}, length: {}", line, length);
    }
    if (length < 0) {
        logger->debug("eof or no \"Content-Length: %i\" header found.");
        return false;
    }

    string json(length, '\0');
    inputStream.read(&json[0], length);
    logger->debug("Read: {}", json);
    if (d.Parse(json.c_str()).HasParseError()) {
        logger->error("Last LSP request: `{}` is not a valid json object", json);
        return false;
    }
    return true;
}

class NotifyOnDestruction {
    absl::Mutex &mutex;
    bool &flag;

public:
    NotifyOnDestruction(absl::Mutex &mutex, bool &flag) : mutex(mutex), flag(flag){};
    ~NotifyOnDestruction() {
        absl::MutexLock lck(&mutex);
        flag = true;
    }
};

unique_ptr<core::GlobalState> LSPLoop::runLSP() {
    // Naming convention: thread that executes this function is called coordinator thread
    struct GuardedState {
        deque<rapidjson::Document> pendingRequests;
        bool terminate = false;
        bool paused = false;
    } guardedState;
    absl::Mutex mtx;

    rapidjson::MemoryPoolAllocator<>
        inner_alloc; // we need objects created by inner thread to outlive the thread itself.

    auto readerThread = runInAThread(
        "lspReader", [&guardedState, &mtx, logger = this->logger, &inner_alloc, &inputStream = this->inputStream] {
            // Thread that executes this lambda is called reader thread.
            // This thread _intentionally_ does not capture `this`.
            NotifyOnDestruction notify(mtx, guardedState.terminate);
            int requestCounter = 0;
            while (true) {
                rapidjson::Document d(&inner_alloc);

                if (!getNewRequest(d, logger, inputStream)) {
                    break;
                }

                d.AddMember("sorbet_counter", requestCounter++, d.GetAllocator());
                d.AddMember("sorbet_recieve_timestamp", (int64_t)Timer::currentTimeNanos(), d.GetAllocator());

                absl::MutexLock lck(&mtx); // guards pendingRequests & paused

                const LSPMethod method = LSPMethod::getByName({d["method"].GetString(), d["method"].GetStringLength()});

                if (method == LSPMethod::Pause()) {
                    ENFORCE(!guardedState.paused);
                    logger->error("Pausing");
                    guardedState.paused = true;
                    continue;
                } else if (method == LSPMethod::Resume()) {
                    logger->error("Resuming");
                    ENFORCE(guardedState.paused);
                    guardedState.paused = false;
                    continue;
                }

                if (method == LSPMethod::CancelRequest()) {
                    // see if they are canceling request that we didn't yet even start processing.
                    rapidjson::Document canceledRequest;
                    auto it = findRequestToBeCancelled(guardedState.pendingRequests, d);
                    if (it != guardedState.pendingRequests.end()) {
                        auto &requestToBeCancelled = *it;
                        requestToBeCancelled.AddMember("canceled", rapidjson::Value(true),
                                                       requestToBeCancelled.GetAllocator());
                        canceledRequest = move(requestToBeCancelled);
                        guardedState.pendingRequests.erase(it);
                        // move the canceled request to the front
                        auto itFront = findFirstPositionAfterLSPInitialization(guardedState.pendingRequests);
                        guardedState.pendingRequests.insert(itFront, move(canceledRequest));
                        LSPLoop::mergeDidChanges(guardedState.pendingRequests);
                    }
                    // if we started processing it already, well... swallow the cancellation request and continue
                    // computing.
                    continue;
                }

                guardedState.pendingRequests.emplace_back(move(d));
                LSPLoop::mergeDidChanges(guardedState.pendingRequests);
            }
        });

    unique_ptr<core::GlobalState> gs;
    while (true) {
        rapidjson::Document doc;
        {
            absl::MutexLock lck(&mtx);
            mtx.Await(absl::Condition(
                +[](GuardedState *guardedState) -> bool {
                    return guardedState->terminate || (!guardedState->paused && !guardedState->pendingRequests.empty());
                },
                &guardedState));
            ENFORCE(!guardedState.paused);
            if (guardedState.terminate) {
                break;
            }
            doc.CopyFrom(guardedState.pendingRequests.front(), alloc);
            guardedState.pendingRequests.pop_front();
        }
        prodCounterInc("lsp.requests.received");
        long requestRecieveTimeNanos = doc["sorbet_recieve_timestamp"].GetInt64();

        gs = processRequest(move(gs), doc);
        timingAdd("processing_time", (Timer::currentTimeNanos() - requestRecieveTimeNanos));
    }
    if (gs) {
        return gs;
    } else {
        return move(initialGS);
    }
}

void LSPLoop::mergeDidChanges(deque<rapidjson::Document> &pendingRequests) {
    // make pass through pendingRequests and squish any consecutive didChanges that are for the same
    // file together
    // TODO: if we ever support diffs, this would need to be extended
    int requestsMerged = 0;
    int originalSize = pendingRequests.size();
    for (auto it = pendingRequests.begin(); it != pendingRequests.end();) {
        auto &current = *it;
        auto method = LSPMethod::getByName({current["method"].GetString(), current["method"].GetStringLength()});
        if (method == LSPMethod::TextDocumentDidChange()) {
            string_view thisURI(current["params"]["textDocument"]["uri"].GetString(),
                                current["params"]["textDocument"]["uri"].GetStringLength());
            auto nextIt = it + 1;
            if (nextIt != pendingRequests.end()) {
                auto &next = *nextIt;
                auto nextMethod = LSPMethod::getByName({next["method"].GetString(), next["method"].GetStringLength()});
                if (nextMethod == LSPMethod::TextDocumentDidChange()) {
                    string_view nextURI(next["params"]["textDocument"]["uri"].GetString(),
                                        next["params"]["textDocument"]["uri"].GetStringLength());
                    if (nextURI == thisURI) {
                        auto currentUpdates = move(current["params"]["contentChanges"]);
                        for (auto &newUpdate : next["params"]["contentChanges"].GetArray()) {
                            currentUpdates.PushBack(move(newUpdate), current.GetAllocator());
                        }
                        next["params"]["contentChanges"] = move(currentUpdates);
                        it = pendingRequests.erase(it);
                        requestsMerged += 1;
                        continue;
                    }
                }
            }
        }
        ++it;
    }
    ENFORCE(pendingRequests.size() + requestsMerged == originalSize);
}

deque<rapidjson::Document>::iterator LSPLoop::findRequestToBeCancelled(deque<rapidjson::Document> &pendingRequests,
                                                                       rapidjson::Document &cancellationRequest) {
    for (auto it = pendingRequests.begin(); it != pendingRequests.end(); ++it) {
        auto &current = *it;
        auto fnd = current.FindMember("id");
        if (fnd != current.MemberEnd()) {
            if (cancellationRequest["params"]["id"].IsString()) {
                if (current["id"].IsString() &&
                    current["id"].GetString() == cancellationRequest["params"]["id"].GetString()) {
                    return it;
                }
            } else if (cancellationRequest["params"]["id"].IsInt()) {
                if (current["id"].IsInt() && current["id"].GetInt() == cancellationRequest["params"]["id"].GetInt()) {
                    return it;
                }
            } else {
                Exception::raise("should never happen. Request id is neither int nor string.");
            }
        }
    }
    return pendingRequests.end();
}

deque<rapidjson::Document>::iterator
LSPLoop::findFirstPositionAfterLSPInitialization(deque<rapidjson::Document> &pendingRequests) {
    for (auto it = pendingRequests.begin(); it != pendingRequests.end(); ++it) {
        auto &current = *it;
        auto method = LSPMethod::getByName({current["method"].GetString(), current["method"].GetStringLength()});
        if (method != LSPMethod::LSPMethod::Initialize() && method != LSPMethod::LSPMethod::Initialized()) {
            return it;
        }
    }
    return pendingRequests.end();
}

void LSPLoop::sendShowMessageNotification(MessageType messageType, string_view message) {
    sendNotification(LSPMethod::WindowShowMessage(), ShowMessageParams(messageType, string(message)));
}

void LSPLoop::sendNullResponse(const MessageId &id) {
    auto resp = ResponseMessage("2.0", id);
    // rapidjson values default to null.
    resp.result = make_unique<rapidjson::Value>();
    sendRaw(resp.toJSON());
}

void LSPLoop::sendResponse(const MessageId &id, const JSONBaseType &result) {
    auto resp = ResponseMessage("2.0", id);
    resp.result = result.toJSONValue(alloc);
    sendRaw(resp.toJSON());
}

void LSPLoop::sendResponse(const MessageId &id, const vector<unique_ptr<JSONBaseType>> &result) {
    auto finalResult = make_unique<rapidjson::Value>();
    finalResult->SetArray();
    for (auto &item : result) {
        finalResult->PushBack(*item->toJSONValue(alloc), alloc);
    }
    auto resp = ResponseMessage("2.0", id);
    resp.result = move(finalResult);
    sendRaw(resp.toJSON());
}

void LSPLoop::sendError(const MessageId &id, int errorCode, string_view errorMsg) {
    auto resp = ResponseMessage("2.0", id);
    resp.error = make_unique<ResponseError>(errorCode, string(errorMsg));
    sendRaw(resp.toJSON());
}

unique_ptr<core::Loc> LSPLoop::lspPos2Loc(core::FileRef fref, const Position &pos, const core::GlobalState &gs) {
    core::Loc::Detail reqPos;
    reqPos.line = pos.line + 1;
    reqPos.column = pos.character + 1;
    auto offset = core::Loc::pos2Offset(fref.data(gs), reqPos);
    return make_unique<core::Loc>(core::Loc(fref, offset, offset));
}

bool LSPLoop::handleReplies(rapidjson::Document &d) {
    if (d.FindMember("result") != d.MemberEnd()) {
        if (d.FindMember("id") != d.MemberEnd()) {
            string key(d["id"].GetString(), d["id"].GetStringLength());
            auto fnd = awaitingResponse.find(key);
            if (fnd != awaitingResponse.end()) {
                auto func = move(fnd->second.onResult);
                awaitingResponse.erase(fnd);
                func(d["result"]);
            }
        }
        return true;
    }

    if (d.FindMember("error") != d.MemberEnd()) {
        if (d.FindMember("id") != d.MemberEnd()) {
            string key(d["id"].GetString(), d["id"].GetStringLength());
            auto fnd = awaitingResponse.find(key);
            if (fnd != awaitingResponse.end()) {
                auto func = move(fnd->second.onError);
                awaitingResponse.erase(fnd);
                func(d["error"]);
            }
        }
        return true;
    }
    return false;
}

void LSPLoop::sendRaw(string_view json) {
    string outResult = fmt::format("Content-Length: {}\r\n\r\n{}", json.length(), json);
    logger->debug("Write: {}\n", json);
    outputStream << outResult << flush;
}

void LSPLoop::sendNotification(LSPMethod meth, const JSONBaseType &data) {
    ENFORCE(meth.isNotification);
    ENFORCE(meth.kind == LSPMethod::Kind::ServerInitiated || meth.kind == LSPMethod::Kind::Both);

    auto notif = NotificationMessage("2.0", meth.name);
    notif.params = data.toJSONValue(alloc);

    sendRaw(notif.toJSON());
}

} // namespace sorbet::realmain::lsp
