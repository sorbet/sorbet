#include "absl/synchronization/mutex.h"
#include "common/Timer.h"
#include "lsp.h"
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
    LSPLoop::QueueState guardedState{{}, false, false};
    absl::Mutex mtx;

    rapidjson::MemoryPoolAllocator<>
        innerAlloc; // we need objects created by inner thread to outlive the thread itself.

    auto readerThread = runInAThread(
        "lspReader", [&guardedState, &mtx, logger = this->logger, &innerAlloc, &inputStream = this->inputStream] {
            // Thread that executes this lambda is called reader thread.
            // This thread _intentionally_ does not capture `this`.
            NotifyOnDestruction notify(mtx, guardedState.terminate);
            int requestCounter = 0;
            while (true) {
                rapidjson::Document d(&innerAlloc);

                if (!getNewRequest(d, logger, inputStream)) {
                    break;
                }

                absl::MutexLock lck(&mtx); // guards pendingRequests & paused

                try {
                    preprocessRequest(innerAlloc, logger, guardedState, make_unique<LSPMessage>(innerAlloc, d),
                                      requestCounter++);
                } catch (DeserializationError e) {
                    logger->error(fmt::format("Unable to deserialize LSP request: {}", e.what()));
                }
            }
        });

    unique_ptr<core::GlobalState> gs;
    while (true) {
        unique_ptr<LSPMessage> msg;
        {
            absl::MutexLock lck(&mtx);
            mtx.Await(absl::Condition(
                +[](LSPLoop::QueueState *guardedState) -> bool {
                    return guardedState->terminate || (!guardedState->paused && !guardedState->pendingRequests.empty());
                },
                &guardedState));
            ENFORCE(!guardedState.paused);
            if (guardedState.terminate) {
                break;
            }
            msg = move(guardedState.pendingRequests.front());
            guardedState.pendingRequests.pop_front();
        }
        prodCounterInc("lsp.requests.received");
        long requestReceiveTimeNanos = (long)msg->timestamp();

        gs = processRequest(move(gs), *msg);
        timingAdd("processing_time", (Timer::currentTimeNanos() - requestReceiveTimeNanos));
    }
    if (gs) {
        return gs;
    } else {
        return move(initialGS);
    }
}

void LSPLoop::mergeDidChanges(rapidjson::MemoryPoolAllocator<> &alloc,
                              std::deque<std::unique_ptr<LSPMessage>> &pendingRequests) {
    // make pass through pendingRequests and squish any consecutive didChanges that are for the same
    // file together
    // TODO: if we ever support diffs, this would need to be extended
    int requestsMerged = 0;
    int originalSize = pendingRequests.size();
    for (auto it = pendingRequests.begin(); it != pendingRequests.end();) {
        auto &current = *it;
        auto method = LSPMethod::getByName(current->method());
        if (method == LSPMethod::TextDocumentDidChange()) {
            auto currentChanges = DidChangeTextDocumentParams::fromJSONValue(alloc, current->params(), "root.params");
            string_view thisURI = currentChanges->textDocument->uri;
            auto nextIt = it + 1;
            if (nextIt != pendingRequests.end()) {
                auto &next = *nextIt;
                auto nextMethod = LSPMethod::getByName(next->method());
                if (nextMethod == LSPMethod::TextDocumentDidChange()) {
                    auto nextChanges = DidChangeTextDocumentParams::fromJSONValue(alloc, next->params(), "root.params");
                    string_view nextURI = nextChanges->textDocument->uri;
                    if (nextURI == thisURI) {
                        auto &currentUpdates = currentChanges->contentChanges;
                        auto &nextUpdates = nextChanges->contentChanges;
                        std::move(std::begin(nextUpdates), std::end(nextUpdates), std::back_inserter(currentUpdates));
                        nextChanges->contentChanges = move(currentUpdates);
                        requestsMerged += 1;
                        next->setParams(nextChanges->toJSONValue(alloc));
                        it = pendingRequests.erase(it);
                        continue;
                    }
                }
            }
        }
        ++it;
    }
    ENFORCE(pendingRequests.size() + requestsMerged == originalSize);
}

void LSPLoop::preprocessRequest(rapidjson::MemoryPoolAllocator<> &alloc, const shared_ptr<spd::logger> &logger,
                                LSPLoop::QueueState &state, std::unique_ptr<LSPMessage> msg, int requestCounter) {
    try {
        msg->setCounter(requestCounter);
        msg->setTimestamp((double)Timer::currentTimeNanos());

        const LSPMethod method = LSPMethod::getByName(msg->method());
        if (method == LSPMethod::CancelRequest()) {
            // see if they are canceling request that we didn't yet even start processing.
            auto it = findRequestToBeCancelled(state.pendingRequests,
                                               *CancelParams::fromJSONValue(alloc, msg->params(), "root.params"));
            if (it != state.pendingRequests.end() && (*it)->isRequest()) {
                auto &requestToBeCancelled = (*it)->asRequest();
                requestToBeCancelled.canceled = true;
                auto canceledRequest = move(*it);
                state.pendingRequests.erase(it);
                // move the canceled request to the front
                auto itFront = findFirstPositionAfterLSPInitialization(state.pendingRequests);
                state.pendingRequests.insert(itFront, move(canceledRequest));
                LSPLoop::mergeDidChanges(alloc, state.pendingRequests);
            }
            // if we started processing it already, well... swallow the cancellation request and
            // continue computing.
        } else if (method == LSPMethod::Pause()) {
            ENFORCE(!state.paused);
            logger->error("Pausing");
            state.paused = true;
        } else if (method == LSPMethod::Resume()) {
            logger->error("Resuming");
            ENFORCE(state.paused);
            state.paused = false;
        } else {
            state.pendingRequests.emplace_back(move(msg));
            LSPLoop::mergeDidChanges(alloc, state.pendingRequests);
        }
    } catch (DeserializationError e) {
        logger->error(fmt::format("Unable to deserialize LSP request: {}", e.what()));
    }
}

std::deque<std::unique_ptr<LSPMessage>>::iterator
LSPLoop::findRequestToBeCancelled(std::deque<std::unique_ptr<LSPMessage>> &pendingRequests,
                                  const CancelParams &cancelParams) {
    for (auto it = pendingRequests.begin(); it != pendingRequests.end(); ++it) {
        auto &current = *it;
        if (current->isRequest()) {
            auto &request = current->asRequest();
            if (request.id == cancelParams.id) {
                return it;
            }
        }
    }
    return pendingRequests.end();
}

std::deque<std::unique_ptr<LSPMessage>>::iterator
LSPLoop::findFirstPositionAfterLSPInitialization(std::deque<std::unique_ptr<LSPMessage>> &pendingRequests) {
    for (auto it = pendingRequests.begin(); it != pendingRequests.end(); ++it) {
        auto &current = *it;
        auto method = LSPMethod::getByName(current->method());
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

void LSPLoop::sendError(const MessageId &id, unique_ptr<ResponseError> error) {
    auto resp = ResponseMessage("2.0", id);
    resp.error = move(error);
    sendRaw(resp.toJSON());
}

void LSPLoop::sendError(const MessageId &id, int errorCode, string_view errorMsg) {
    sendError(id, make_unique<ResponseError>(errorCode, string(errorMsg)));
}

unique_ptr<core::Loc> LSPLoop::lspPos2Loc(core::FileRef fref, const Position &pos, const core::GlobalState &gs) {
    core::Loc::Detail reqPos;
    reqPos.line = pos.line + 1;
    reqPos.column = pos.character + 1;
    auto offset = core::Loc::pos2Offset(fref.data(gs), reqPos);
    return make_unique<core::Loc>(core::Loc(fref, offset, offset));
}

bool LSPLoop::handleReplies(const LSPMessage &msg) {
    if (msg.isResponse()) {
        auto &resp = msg.asResponse();
        auto id = resp.id;
        if (auto stringId = get_if<string>(&id)) {
            auto fnd = awaitingResponse.find(*stringId);
            if (fnd != awaitingResponse.end()) {
                auto func = move(fnd->second.onResult);
                awaitingResponse.erase(fnd);

                if (resp.error.has_value()) {
                    auto &error = *resp.error;
                    func(*error->toJSONValue(alloc));
                } else if (resp.result.has_value()) {
                    auto &result = *resp.result;
                    func(*result);
                }
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
