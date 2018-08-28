#include "lsp.h"
#include <condition_variable> // std::condition_variable
#include <deque>
#include <mutex> // std::mutex, std::unique_lock

using namespace std;

namespace sorbet {
namespace realmain {
namespace lsp {

bool safeGetline(istream &is, string &t) {
    t.clear();

    // The characters in the stream are read one-by-one using a std::streambuf.
    // That is faster than reading them one-by-one using the std::istream.
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

bool getNewRequest(rapidjson::Document &d, const shared_ptr<spd::logger> &logger) {
    int length = -1;
    {
        string line;
        while (safeGetline(cin, line)) {
            logger->trace("raw read: {}", line);
            if (line == "") {
                break;
            }
            sscanf(line.c_str(), "Content-Length: %i", &length);
        }
        logger->trace("final raw read: {}, length: {}", line, length);
    }
    if (length < 0) {
        logger->debug("eof");
        return false;
    }

    string json(length, '\0');
    cin.read(&json[0], length);
    logger->debug("Read: {}", json);
    if (d.Parse(json.c_str()).HasParseError()) {
        logger->error("Last LSP request: `{}` is not a valid json object", json);
        return false;
    }
    return true;
}

void LSPLoop::runLSP() {
    // Naming convention: thread that executes this function is called coordinator thread
    deque<rapidjson::Document> pendingRequests;
    bool terminate = false;
    mutex mtx;
    condition_variable cv;
    rapidjson::MemoryPoolAllocator<>
        inner_alloc; // we need objects created by inner thread to outlive the thread itself.

    auto readerThread = runInAThread([&pendingRequests, &mtx, &cv, &terminate, logger = this->logger, &inner_alloc] {
        // Thread that executes this lambda is called reader thread.
        // This thread _intentionally_ does not capture `this`.
        unique_lock<mutex> globalLock(mtx, defer_lock);
        while (!terminate) {
            rapidjson::Document d(&inner_alloc);
            if (!getNewRequest(d, logger)) {
                terminate = true;
                break;
            }
            unique_ptr<unique_lock<mutex>> lck = globalLock ? nullptr : make_unique<unique_lock<mutex>>(mtx);
            if (!pendingRequests.empty()) {
                // see if we can be smarter about requests that are waiting to be processed.

                const LSPMethod method = LSPMethod::getByName({d["method"].GetString(), d["method"].GetStringLength()});
                if (method == LSPMethod::Pause()) {
                    if (!globalLock) {
                        ENFORCE(lck);
                        globalLock = move(*lck);
                    }
                    continue;
                } else if (method == LSPMethod::Resume()) {
                    if (globalLock) {
                        globalLock.unlock();
                    }
                    continue;
                } else if (method == LSPMethod::TextDocumentDidChange()) {
                    // see if previous notification is modification of the same file, if so, take the most recent
                    // version.
                    // TODO: if we ever support diffs, this would need to be extended.
                    auto &prev = pendingRequests.back();
                    auto prevMethod =
                        LSPMethod::getByName({prev["method"].GetString(), prev["method"].GetStringLength()});
                    if (prevMethod == LSPMethod::TextDocumentDidChange()) {
                        string thisURI(d["params"]["textDocument"]["uri"].GetString(),
                                       d["params"]["textDocument"]["uri"].GetStringLength());
                        string prevURI(prev["params"]["textDocument"]["uri"].GetString(),
                                       prev["params"]["textDocument"]["uri"].GetStringLength());
                        if (prevURI == thisURI) {
                            pendingRequests.pop_back();
                        }
                    }
                } else if (method == LSPMethod::CancelRequest()) {
                    // see if they are cancelling request that we didn't yet even start processing.
                    rapidjson::Document canceledRequest;
                    auto it = findRequestToBeCancelled(pendingRequests, d);
                    if (it != pendingRequests.end()) {
                        auto &requestToBeCancelled = *it;
                        requestToBeCancelled.AddMember("cancelled", move(d), requestToBeCancelled.GetAllocator());
                        canceledRequest = move(requestToBeCancelled);
                        pendingRequests.erase(it);
                        // move the cancelled request to the front
                        auto itFront = findFirstPositionAfterLSPInitialization(pendingRequests);
                        pendingRequests.insert(itFront, move(canceledRequest));
                        LSPLoop::mergeDidChanges(pendingRequests);
                    }

                    // if we started processing it already, well... swallow the cancellation request and continue
                    // computing.
                    continue;
                }
            }

            pendingRequests.emplace_back(move(d));
            cv.notify_one();
        }
        {
            // signal that coordinator thread should die
            unique_ptr<unique_lock<mutex>> lck = globalLock ? nullptr : make_unique<unique_lock<mutex>>(mtx);
            terminate = true;
            cv.notify_one();
        }
    });

    while (true) {
        rapidjson::Document doc;
        {
            unique_lock<mutex> lck(mtx);
            while (pendingRequests.empty() && !terminate) {
                cv.wait(lck);
            }
            if (terminate) {
                return;
            }
            doc.CopyFrom(pendingRequests.front(), alloc);
            pendingRequests.pop_front();
        }
        if (!handleReplies(doc)) {
            processRequest(doc);
        }
    }
}

void LSPLoop::mergeDidChanges(deque<rapidjson::Document> &pendingRequests) {
    // make pass through pendingRequests and squish any consecutive didChanges that are for the same
    // file together
    for (auto it = pendingRequests.begin(); it != pendingRequests.end();) {
        auto &current = *it;
        auto method = LSPMethod::getByName({current["method"].GetString(), current["method"].GetStringLength()});
        if (method == LSPMethod::TextDocumentDidChange()) {
            string thisURI(current["params"]["textDocument"]["uri"].GetString(),
                           current["params"]["textDocument"]["uri"].GetStringLength());
            auto nextIt = it + 1;
            if (nextIt != pendingRequests.end()) {
                string nextURI((*nextIt)["params"]["textDocument"]["uri"].GetString(),
                               (*nextIt)["params"]["textDocument"]["uri"].GetStringLength());
                if (nextURI == thisURI) {
                    it = pendingRequests.erase(it);
                    continue;
                }
            }
        }
        ++it;
    }
}

std::deque<rapidjson::Document>::iterator LSPLoop::findRequestToBeCancelled(deque<rapidjson::Document> &pendingRequests,
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
                Error::raise("should never happen. Request id is neither int nor string.");
            }
        }
    }
    return pendingRequests.end();
}

std::deque<rapidjson::Document>::iterator
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

void LSPLoop::sendShowMessageNotification(int messageType, string message) {
    rapidjson::Value result;
    result.SetObject();
    result.AddMember("type", messageType, alloc);
    result.AddMember("message", message, alloc);
    sendNotification(LSPMethod::WindowShowMessage(), result);
}

void LSPLoop::sendResult(rapidjson::Document &forRequest, rapidjson::Value &result) {
    forRequest.AddMember("result", result, alloc);
    forRequest.RemoveMember("method");
    forRequest.RemoveMember("params");
    sendRaw(forRequest);
}

void LSPLoop::sendError(rapidjson::Document &forRequest, int errorCode, string errorStr) {
    forRequest.RemoveMember("method");
    forRequest.RemoveMember("params");
    forRequest.RemoveMember("cancelled");
    rapidjson::Value error;
    error.SetObject();
    error.AddMember("code", errorCode, alloc);
    rapidjson::Value message(errorStr.c_str(), alloc);
    error.AddMember("message", message, alloc);
    forRequest.AddMember("error", error, alloc);
    sendRaw(forRequest);
}

unique_ptr<core::Loc> LSPLoop::lspPos2Loc(core::FileRef fref, rapidjson::Document &d, const core::GlobalState &gs) {
    if (!d.HasMember("params") || !d["params"].HasMember("position") ||
        !d["params"]["position"].HasMember("character") || !d["params"]["position"].HasMember("line")) {
        sendError(d, (int)LSPErrorCodes::InvalidRequest,
                  "Request must have a \"params\" field that has a nested \"position\", with nested \"line\" and "
                  "\"character\"");
        return nullptr;
    }
    core::Loc::Detail reqPos;
    reqPos.line = d["params"]["position"]["line"].GetInt() + 1;
    reqPos.column = d["params"]["position"]["character"].GetInt() + 1;
    auto offset = core::Loc::pos2Offset(fref, reqPos, *finalGs);
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

void LSPLoop::sendRaw(rapidjson::Document &raw) {
    if (raw.FindMember("jsonrpc") == raw.MemberEnd()) {
        raw.AddMember("jsonrpc", "2.0", alloc);
    }
    rapidjson::StringBuffer strbuf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(strbuf);
    raw.Accept(writer);
    string outResult = fmt::format("Content-Length: {}\r\n\r\n{}", strbuf.GetLength(), strbuf.GetString());
    logger->debug("Write: {}\n", strbuf.GetString());
    cout << outResult << flush;
}

void LSPLoop::sendNotification(LSPMethod meth, rapidjson::Value &data) {
    ENFORCE(meth.isNotification);
    ENFORCE(meth.kind == LSPMethod::Kind::ServerInitiated || meth.kind == LSPMethod::Kind::Both);
    rapidjson::Document request(&alloc);
    request.SetObject();
    string idStr = fmt::format("ruby-typer-req-{}", ++requestCounter);

    request.AddMember("method", meth.name, alloc);
    request.AddMember("params", data, alloc);

    sendRaw(request);
}

void LSPLoop::sendRequest(LSPMethod meth, rapidjson::Value &data, function<void(rapidjson::Value &)> onComplete,
                          function<void(rapidjson::Value &)> onFail) {
    ENFORCE(!meth.isNotification);
    ENFORCE(meth.kind == LSPMethod::Kind::ServerInitiated || meth.kind == LSPMethod::Kind::Both);
    rapidjson::Document request(&alloc);
    request.SetObject();
    string idStr = fmt::format("ruby-typer-req-{}", ++requestCounter);

    request.AddMember("id", idStr, alloc);
    request.AddMember("method", meth.name, alloc);
    request.AddMember("params", data, alloc);

    awaitingResponse[idStr] = ResponseHandler{move(onComplete), move(onFail)};

    sendRaw(request);
}

} // namespace lsp
} // namespace realmain
} // namespace sorbet
