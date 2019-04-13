#include "absl/synchronization/mutex.h"
#include "common/Timer.h"
#include "lsp.h"
#include "main/lsp/watchman/WatchmanProcess.h"
#include "main/options/options.h" // For EarlyReturnWithCode.
#include <iostream>

using namespace std;

namespace sorbet::realmain::lsp {

/**
 * Attempts to read an LSP message from the file descriptor. Stores message into readMessage.
 *
 * Extra bits read are stored into `buffer`.
 *
 * Returns:
 * - 1 if a message was read successfully. `readMessage` contains the message.
 * - 0 if a timeout occurred before a message was completely read.
 * - -1 if an error or EOF occurs.
 */
int getNewRequest(rapidjson::MemoryPoolAllocator<> &alloc, const shared_ptr<spd::logger> &logger, int inputFd,
                  string &buffer, unique_ptr<LSPMessage> &readMessage) {
    int length = -1;
    string allRead;
    {
        string line;
        // Break and return if a timeout occurs.
        for (;;) {
            int result = FileOps::readLineFromFd(inputFd, line, buffer);
            if (result != 1) {
                // Line not read. Abort. Store what was read thus far back into buffer
                // for use in next call to function.
                buffer = absl::StrCat(allRead, buffer);
                return result;
            }
            absl::StrAppend(&allRead, line, "\n");
            if (line == "\r") {
                // End of headers.
                break;
            }
            sscanf(line.c_str(), "Content-Length: %i\r", &length);
        }
        logger->trace("final raw read: {}, length: {}", allRead, length);
    }

    if (length < 0) {
        logger->debug("No \"Content-Length: %i\" header found.");
        // Throw away what we've read and start over.
        return 0;
    }

    if (buffer.length() < length) {
        // Need to read more.
        int moreNeeded = length - buffer.length();
        vector<char> buf(moreNeeded);
        int result = FileOps::readFd(inputFd, buf);
        if (result > 0) {
            buffer.append(buf.begin(), buf.begin() + result);
        }
        if (result != moreNeeded) {
            // Didn't get enough or an error occurred.
            // Return read data to `buffer`.
            buffer = absl::StrCat(allRead, buffer);
            return result > 0 ? 0 : result;
        }
    }

    ENFORCE(buffer.length() >= length);

    string json = buffer.substr(0, length);
    buffer.erase(0, length);
    readMessage = LSPMessage::fromClient(alloc, json);
    return 1;
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
    LSPLoop::QueueState guardedState{{}, false, false, 0};
    absl::Mutex mtx;

    unique_ptr<watchman::WatchmanProcess> watchmanProcess;
    if (!opts.disableWatchman) {
        if (opts.rawInputDirNames.size() == 1 && opts.rawInputFileNames.size() == 0) {
            // The lambda below intentionally does not capture `this`.
            watchmanProcess = make_unique<watchman::WatchmanProcess>(
                logger, opts.watchmanPath, opts.rawInputDirNames.at(0), vector<string>({"rb", "rbi"}),
                [&guardedState, &mtx, logger = this->logger](rapidjson::MemoryPoolAllocator<> &alloc,
                                                             std::unique_ptr<WatchmanQueryResponse> response) {
                    auto notifMsg =
                        make_unique<NotificationMessage>("2.0", LSPMethod::SorbetWatchmanFileChange, move(response));
                    auto msg = make_unique<LSPMessage>(move(notifMsg));
                    {
                        absl::MutexLock lck(&mtx); // guards guardedState
                        // Merge with any existing pending watchman file updates.
                        enqueueRequest(alloc, logger, guardedState, move(msg), true);
                    }
                },
                [&guardedState, &mtx](int watchmanExitCode) {
                    {
                        absl::MutexLock lck(&mtx); // guards guardedState
                        if (!guardedState.terminate) {
                            guardedState.terminate = true;
                            guardedState.errorCode = watchmanExitCode;
                        }
                    }
                });
        } else {
            logger->error("Watchman support currently only works when Sorbet is run with a single input directory. If "
                          "Watchman is not needed, run Sorbet with `--disable-watchman`.");
            throw options::EarlyReturnWithCode(1);
        }
    }

    // !!DO NOT USE OUTSIDE OF READER THREAD!!
    // We need objects created by the reader thread to outlive the thread itself. At the same time, MemoryPoolAllocator
    // is not thread-safe.
    // TODO(jvilk): This (+ Watchman's alloc) leak memory. If we stop using JSON values internally, then we can clear
    // them after every request and only use them for intermediate objects generated during parsing.
    rapidjson::MemoryPoolAllocator<> readerAlloc;
    auto readerThread =
        runInAThread("lspReader", [&guardedState, &mtx, logger = this->logger, &readerAlloc, inputFd = this->inputFd] {
            // Thread that executes this lambda is called reader thread.
            // This thread _intentionally_ does not capture `this`.
            NotifyOnDestruction notify(mtx, guardedState.terminate);
            string buffer;
            while (true) {
                unique_ptr<LSPMessage> msg;
                int result = getNewRequest(readerAlloc, logger, inputFd, buffer, msg);
                {
                    absl::MutexLock lck(&mtx); // guards guardedState.
                    if (result == 1) {
                        ENFORCE(msg);
                        enqueueRequest(readerAlloc, logger, guardedState, move(msg), true);
                    }
                    // Check if it's time to exit.
                    if (guardedState.terminate) {
                        // Another thread exited.
                        break;
                    } else if (result == -1) {
                        // An error or EOF occurred. Tell main thread to exit.
                        guardedState.terminate = true;
                        guardedState.errorCode = 0;
                        break;
                    }
                }
            }
        });

    mainThreadId = this_thread::get_id();
    unique_ptr<core::GlobalState> gs;
    while (true) {
        unique_ptr<LSPMessage> msg;
        {
            absl::MutexLock lck(&mtx);
            Timer timeit(logger, "idle");
            mtx.Await(absl::Condition(
                +[](LSPLoop::QueueState *guardedState) -> bool {
                    return guardedState->terminate || (!guardedState->paused && !guardedState->pendingRequests.empty());
                },
                &guardedState));
            ENFORCE(!guardedState.paused);
            if (guardedState.terminate) {
                if (guardedState.errorCode != 0) {
                    // Abnormal termination.
                    throw options::EarlyReturnWithCode(guardedState.errorCode);
                } else if (guardedState.pendingRequests.empty()) {
                    // Normal termination. Wait until all pending requests finish.
                    break;
                }
            }
            msg = move(guardedState.pendingRequests.front());
            guardedState.pendingRequests.pop_front();
        }
        prodCounterInc("lsp.messages.received");
        auto requestReceiveTime = msg->timestamp;
        gs = processRequest(move(gs), *msg);
        auto currentTime = chrono::steady_clock::now();
        auto processingTime = currentTime - requestReceiveTime;
        auto processingTime_ns = chrono::duration_cast<chrono::nanoseconds>(processingTime);
        timingAdd("processing_time", processingTime_ns.count());
        if (shouldSendCountersToStatsd(currentTime)) {
            {
                // Merge counters from worker threads.
                absl::MutexLock counterLck(&mtx);
                if (!guardedState.counters.hasNullCounters()) {
                    counterConsume(move(guardedState.counters));
                }
            }
            sendCountersToStatsd(currentTime);
        }
    }

    if (gs) {
        return gs;
    } else {
        return move(initialGS);
    }
}

void LSPLoop::mergeFileChanges(rapidjson::MemoryPoolAllocator<> &alloc,
                               deque<unique_ptr<LSPMessage>> &pendingRequests) {
    // Squish any consecutive didChanges that are for the same file together, and combine all Watchman file system
    // updates into a single update.
    // TODO: if we ever support diffs, this would need to be extended
    int didChangeRequestsMerged = 0;
    int foundWatchmanRequests = 0;
    chrono::time_point<chrono::steady_clock> firstWatchmanTimestamp;
    int firstWatchmanCounter;
    int originalSize = pendingRequests.size();
    UnorderedSet<string> updatedFiles;
    for (auto it = pendingRequests.begin(); it != pendingRequests.end();) {
        auto &current = *it;
        if (current->isNotification()) {
            auto method = current->method();
            auto &params = current->asNotification().params;
            if (method == LSPMethod::TextDocumentDidChange) {
                auto &currentChanges = get<unique_ptr<DidChangeTextDocumentParams>>(params);
                string_view thisURI = currentChanges->textDocument->uri;
                auto nextIt = it + 1;
                if (nextIt != pendingRequests.end()) {
                    auto &next = *nextIt;
                    auto nextMethod = next->method();
                    if (nextMethod == LSPMethod::TextDocumentDidChange && next->isNotification()) {
                        auto &nextNotif = next->asNotification();
                        auto &nextChanges = get<unique_ptr<DidChangeTextDocumentParams>>(nextNotif.params);
                        string_view nextURI = nextChanges->textDocument->uri;
                        if (nextURI == thisURI) {
                            auto &currentUpdates = currentChanges->contentChanges;
                            auto &nextUpdates = nextChanges->contentChanges;
                            std::move(std::begin(nextUpdates), std::end(nextUpdates),
                                      std::back_inserter(currentUpdates));
                            nextChanges->contentChanges = move(currentUpdates);
                            didChangeRequestsMerged += 1;
                            nextNotif.params = move(nextChanges);
                            it = pendingRequests.erase(it);
                            continue;
                        }
                    }
                }
            } else if (method == LSPMethod::SorbetWatchmanFileChange) {
                auto &changes = get<unique_ptr<WatchmanQueryResponse>>(params);
                updatedFiles.insert(changes->files.begin(), changes->files.end());
                if (foundWatchmanRequests == 0) {
                    // Use timestamp/counter from the earliest file system change.
                    firstWatchmanTimestamp = current->timestamp;
                    firstWatchmanCounter = current->counter;
                }
                foundWatchmanRequests++;
                it = pendingRequests.erase(it);
                continue;
            }
        }
        ++it;
    }
    ENFORCE(pendingRequests.size() + didChangeRequestsMerged + foundWatchmanRequests == originalSize);

    prodCategoryCounterAdd("lsp.messages.processed", "text_document_did_change_merged", didChangeRequestsMerged);

    /**
     * If we found any watchman updates, inject a single update that contains all of the FS updates.
     *
     * Ordering with textDocumentDidChange notifications effectively does *not* matter. We ignore Watchman updates on
     * files that are open in the editor, and re-pick up the file system version when the editor closes the file.
     *
     * It's possible that this reordering will cause transient errors if the user intended the TextDocumentDidChange to
     * apply against the other changed files on disk. For example, the user may have git pulled in changes to bar.rb,
     * and are editing foo.rb which requires the updated bar.rb to pass typechecking. However, we already have that
     * problem with the devbox rsync loop + autogen latency.
     *
     * I (jvilk) think the harm of these transient errors is small compared to the potential delay in this common
     * scenario:
     * 1. User runs git pull.
     * 2. User begins editing file.
     * 3. While user is editing file, pay up is rsyncing changes + autogen is running.
     * 4. Sorbet is picking up user's edits mixed in with file system updates.
     *
     * If DidChange acts like a fence for combining file system updates, then Sorbet may have to run the slow path many
     * times to catch up with the file system changes.
     *
     * TODO(jvilk): Since the slow path is slow, I actually think that we may want to (eventually) combine all file
     * updates from different sources into a single event to make catching up faster. Then, we can merge all DidChange
     * and Watchman updates into a single update.
     */
    if (foundWatchmanRequests > 0) {
        prodCategoryCounterAdd("lsp.messages.processed", "watchman_file_change_merged", foundWatchmanRequests - 1);
        // Enqueue the changes at *back* of the queue. Defers Sorbet from processing updates until the last
        // possible moment.
        auto watchmanUpdates =
            make_unique<WatchmanQueryResponse>("", "", false, vector<string>(updatedFiles.begin(), updatedFiles.end()));
        auto notifMsg =
            make_unique<NotificationMessage>("2.0", LSPMethod::SorbetWatchmanFileChange, move(watchmanUpdates));
        auto msg = make_unique<LSPMessage>(move(notifMsg));
        msg->counter = firstWatchmanCounter;
        msg->timestamp = firstWatchmanTimestamp;
        pendingRequests.push_back(move(msg));
    }
}

void LSPLoop::enqueueRequest(rapidjson::MemoryPoolAllocator<> &alloc, const shared_ptr<spd::logger> &logger,
                             LSPLoop::QueueState &state, std::unique_ptr<LSPMessage> msg, bool collectThreadCounters) {
    try {
        msg->counter = state.requestCounter++;
        msg->timestamp = chrono::steady_clock::now();

        const LSPMethod method = msg->method();
        if (method == LSPMethod::$CancelRequest) {
            // see if they are canceling request that we didn't yet even start processing.
            auto it = findRequestToBeCancelled(state.pendingRequests,
                                               *get<unique_ptr<CancelParams>>(msg->asNotification().params));
            if (it != state.pendingRequests.end() && (*it)->isRequest()) {
                auto canceledRequest = move(*it);
                canceledRequest->canceled = true;
                state.pendingRequests.erase(it);
                // move the canceled request to the front
                auto itFront = findFirstPositionAfterLSPInitialization(state.pendingRequests);
                state.pendingRequests.insert(itFront, move(canceledRequest));
                LSPLoop::mergeFileChanges(alloc, state.pendingRequests);
            }
            // if we started processing it already, well... swallow the cancellation request and
            // continue computing.
        } else if (method == LSPMethod::PAUSE) {
            ENFORCE(!state.paused);
            logger->error("Pausing");
            state.paused = true;
        } else if (method == LSPMethod::RESUME) {
            logger->error("Resuming");
            ENFORCE(state.paused);
            state.paused = false;
        } else if (method == LSPMethod::Exit) {
            // Don't override previous error code if already terminated.
            if (!state.terminate) {
                state.terminate = true;
                state.errorCode = 0;
            }
        } else if (method == LSPMethod::SorbetError) {
            // Place errors at the *front* of the queue.
            // Otherwise, they could prevent mergeFileChanges from merging adjacent updates.
            state.pendingRequests.insert(findFirstPositionAfterLSPInitialization(state.pendingRequests), move(msg));
        } else {
            state.pendingRequests.push_back(move(msg));
            LSPLoop::mergeFileChanges(alloc, state.pendingRequests);
        }
    } catch (DeserializationError e) {
        logger->error("Unable to deserialize LSP request: {}", e.what());
    }

    if (collectThreadCounters) {
        if (!state.counters.hasNullCounters()) {
            counterConsume(move(state.counters));
        }
        state.counters = getAndClearThreadCounters();
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
        auto method = current->method();
        if (method != LSPMethod::Initialize && method != LSPMethod::Initialized) {
            return it;
        }
    }
    return pendingRequests.end();
}

void LSPLoop::sendShowMessageNotification(MessageType messageType, string_view message) {
    sendNotification(NotificationMessage("2.0", LSPMethod::WindowShowMessage,
                                         make_unique<ShowMessageParams>(messageType, string(message))));
}

// Is this a notification the server should be sending?
bool isServerNotification(const LSPMethod method) {
    switch (method) {
        case LSPMethod::$CancelRequest:
        case LSPMethod::TextDocumentPublishDiagnostics:
        case LSPMethod::WindowShowMessage:
        case LSPMethod::SorbetShowOperation:
            return true;
        default:
            return false;
    }
}

void LSPLoop::sendNotification(const NotificationMessage &msg) {
    ENFORCE(isServerNotification(msg.method));
    sendRaw(msg.toJSON());
}

void LSPLoop::sendResponse(const ResponseMessage &resp) {
    ENFORCE(resp.result || resp.error, "A valid ResponseMessage must have a result or an error.");
    sendRaw(resp.toJSON());
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
                auto func = move(fnd->second);
                awaitingResponse.erase(fnd);
                func(resp);
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

} // namespace sorbet::realmain::lsp
