#include "absl/synchronization/mutex.h"
#include "absl/synchronization/notification.h"
#include "common/Timer.h"
#include "common/web_tracer_framework/tracing.h"
#include "lsp.h"
#include "main/lsp/watchman/WatchmanProcess.h"
#include "main/options/options.h" // For EarlyReturnWithCode.
#include <iostream>

using namespace std;

namespace sorbet::realmain::lsp {

/**
 * Attempts to read an LSP message from the file descriptor. Returns a nullptr if it fails.
 *
 * Extra bits read are stored into `buffer`.
 *
 * Throws an exception on read error or EOF.
 */
unique_ptr<LSPMessage> getNewRequest(const shared_ptr<spd::logger> &logger, int inputFd, string &buffer) {
    int length = -1;
    string allRead;
    {
        // Break and return if a timeout occurs. Bound loop to prevent infinite looping here. There's typically only two
        // lines in a header.
        for (int i = 0; i < 10; i += 1) {
            auto maybeLine = FileOps::readLineFromFd(inputFd, buffer);
            if (!maybeLine) {
                // Line not read. Abort. Store what was read thus far back into buffer
                // for use in next call to function.
                buffer = absl::StrCat(allRead, buffer);
                return nullptr;
            }
            const string &line = *maybeLine;
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
        logger->trace("No \"Content-Length: %i\" header found.");
        // Throw away what we've read and start over.
        return nullptr;
    }

    if (buffer.length() < length) {
        // Need to read more.
        int moreNeeded = length - buffer.length();
        vector<char> buf(moreNeeded);
        int result = FileOps::readFd(inputFd, buf);
        if (result > 0) {
            buffer.append(buf.begin(), buf.begin() + result);
        }
        if (result == -1) {
            Exception::raise("Error reading file or EOF.");
        }
        if (result != moreNeeded) {
            // Didn't get enough data. Return read data to `buffer`.
            buffer = absl::StrCat(allRead, buffer);
            return nullptr;
        }
    }

    ENFORCE(buffer.length() >= length);

    string json = buffer.substr(0, length);
    buffer.erase(0, length);
    return LSPMessage::fromClient(json);
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

class NotifyNotificationOnDestruction {
    absl::Notification &notification;

public:
    NotifyNotificationOnDestruction(absl::Notification &notif) : notification(notif){};
    ~NotifyNotificationOnDestruction() {
        if (!notification.HasBeenNotified()) {
            notification.Notify();
        }
    }
};

unique_ptr<core::GlobalState> LSPLoop::runLSP() {
    // Naming convention: thread that executes this function is called coordinator thread
    LSPLoop::QueueState guardedState{{}, false, false, 0};
    absl::Mutex mtx;
    absl::Notification initializedNotification;

    unique_ptr<watchman::WatchmanProcess> watchmanProcess;
    if (!opts.disableWatchman) {
        if (opts.rawInputDirNames.size() == 1 && opts.rawInputFileNames.size() == 0) {
            // The lambda below intentionally does not capture `this`.
            watchmanProcess = make_unique<watchman::WatchmanProcess>(
                logger, opts.watchmanPath, opts.rawInputDirNames.at(0), vector<string>({"rb", "rbi"}),
                [&guardedState, &mtx, logger = this->logger,
                 &initializedNotification](std::unique_ptr<WatchmanQueryResponse> response) {
                    auto notifMsg =
                        make_unique<NotificationMessage>("2.0", LSPMethod::SorbetWatchmanFileChange, move(response));
                    auto msg = make_unique<LSPMessage>(move(notifMsg));
                    // Don't start enqueueing requests until LSP is initialized.
                    initializedNotification.WaitForNotification();
                    {
                        absl::MutexLock lck(&mtx); // guards guardedState
                        // Merge with any existing pending watchman file updates.
                        enqueueRequest(logger, guardedState, move(msg), true);
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

    auto readerThread =
        runInAThread("lspReader", [&guardedState, &mtx, logger = this->logger, inputFd = this->inputFd] {
            // Thread that executes this lambda is called reader thread.
            // This thread _intentionally_ does not capture `this`.
            NotifyOnDestruction notify(mtx, guardedState.terminate);
            string buffer;
            try {
                auto timeit = make_unique<Timer>(logger, "getNewRequest");
                while (true) {
                    auto msg = getNewRequest(logger, inputFd, buffer);
                    {
                        absl::MutexLock lck(&mtx); // guards guardedState.
                        if (msg) {
                            enqueueRequest(logger, guardedState, move(msg), true);
                            // Reset span now that we've found a request.
                            timeit = make_unique<Timer>(logger, "getNewRequest");
                        }
                        // Check if it's time to exit.
                        if (guardedState.terminate) {
                            // Another thread exited.
                            break;
                        }
                    }
                }
            } catch (FileReadException e) {
                // Failed to read from input stream. Ignore. NotifyOnDestruction will take care of exiting cleanly.
            }
        });

    mainThreadId = this_thread::get_id();
    unique_ptr<core::GlobalState> gs;
    {
        // Ensure Watchman thread gets unstuck when thread exits.
        NotifyNotificationOnDestruction notify(initializedNotification);
        while (true) {
            unique_ptr<LSPMessage> msg;
            {
                absl::MutexLock lck(&mtx);
                Timer timeit(logger, "idle");
                mtx.Await(absl::Condition(
                    +[](LSPLoop::QueueState *guardedState) -> bool {
                        return guardedState->terminate ||
                               (!guardedState->paused && !guardedState->pendingRequests.empty());
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
            auto result = processRequest(move(gs), *msg);
            gs = move(result.gs);
            for (auto &msg : result.responses) {
                sendMessage(*msg);
            }

            if (initialized && !initializedNotification.HasBeenNotified()) {
                initializedNotification.Notify();
            }

            auto currentTime = chrono::steady_clock::now();
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
    }

    if (gs) {
        return gs;
    } else {
        return move(initialGS);
    }
}

/**
 * Returns true if the given message is a workspace edit that can be merged with other workspace edits.
 * If it can be merged, it moves the message's contents into edits or updatedFiles and updates counters ('preMerge').
 */
bool tryPreMerge(LSPMessage &current, SorbetWorkspaceEditCounts &counts,
                 vector<unique_ptr<SorbetWorkspaceEdit>> &changes, UnorderedSet<string> &updatedFiles) {
    if (current.isNotification()) {
        auto &params = current.asNotification().params;
        switch (current.method()) {
            case LSPMethod::TextDocumentDidOpen: {
                counts.textDocumentDidOpen++;
                changes.push_back(make_unique<SorbetWorkspaceEdit>(
                    SorbetWorkspaceEditType::EditorOpen, move(get<unique_ptr<DidOpenTextDocumentParams>>(params))));
                return true;
            }
            case LSPMethod::TextDocumentDidChange: {
                counts.textDocumentDidChange++;
                changes.push_back(make_unique<SorbetWorkspaceEdit>(
                    SorbetWorkspaceEditType::EditorChange, move(get<unique_ptr<DidChangeTextDocumentParams>>(params))));
                return true;
            }
            case LSPMethod::TextDocumentDidClose: {
                counts.textDocumentDidClose++;
                changes.push_back(make_unique<SorbetWorkspaceEdit>(
                    SorbetWorkspaceEditType::EditorClose, move(get<unique_ptr<DidCloseTextDocumentParams>>(params))));
                return true;
            }
            case LSPMethod::SorbetWatchmanFileChange: {
                counts.sorbetWatchmanFileChange++;
                auto &changes = get<unique_ptr<WatchmanQueryResponse>>(params);
                updatedFiles.insert(changes->files.begin(), changes->files.end());
                return true;
            }
            case LSPMethod::SorbetWorkspaceEdit: {
                auto &editParams = get<unique_ptr<SorbetWorkspaceEditParams>>(params);
                counts.textDocumentDidOpen += editParams->counts->textDocumentDidOpen;
                counts.textDocumentDidChange += editParams->counts->textDocumentDidChange;
                counts.textDocumentDidClose += editParams->counts->textDocumentDidClose;
                counts.sorbetWatchmanFileChange += editParams->counts->sorbetWatchmanFileChange;
                for (auto &edit : editParams->changes) {
                    if (edit->type == SorbetWorkspaceEditType::FileSystem) {
                        auto &changes = get<unique_ptr<WatchmanQueryResponse>>(edit->contents);
                        updatedFiles.insert(changes->files.begin(), changes->files.end());
                    } else {
                        changes.push_back(move(edit));
                    }
                }
                return true;
            }
            default:
                return false;
        }
    }
    return false;
}

// Returns a new LSPMessage if a merge should be performed. Returns nullptr otherwise.
unique_ptr<LSPMessage> performMerge(const UnorderedSet<string> &updatedFiles,
                                    vector<unique_ptr<SorbetWorkspaceEdit>> &consecutiveWorkspaceEdits,
                                    unique_ptr<SorbetWorkspaceEditCounts> &counts) {
    if (updatedFiles.size() > 0) {
        consecutiveWorkspaceEdits.push_back(make_unique<SorbetWorkspaceEdit>(
            SorbetWorkspaceEditType::FileSystem,
            make_unique<WatchmanQueryResponse>("", "", false,
                                               vector<string>(updatedFiles.begin(), updatedFiles.end()))));
    }
    if (consecutiveWorkspaceEdits.size() > 0) {
        auto notif = make_unique<NotificationMessage>(
            "2.0", LSPMethod::SorbetWorkspaceEdit,
            make_unique<SorbetWorkspaceEditParams>(move(counts), move(consecutiveWorkspaceEdits)));
        return make_unique<LSPMessage>(move(notif));
    }
    // No merge.
    return nullptr;
}

void LSPLoop::mergeFileChanges(deque<unique_ptr<LSPMessage>> &pendingRequests) {
    int requestsMergedCounter = 0;
    const int originalSize = pendingRequests.size();
    auto counts = make_unique<SorbetWorkspaceEditCounts>(0, 0, 0, 0);
    vector<unique_ptr<SorbetWorkspaceEdit>> consecutiveWorkspaceEdits;
    UnorderedSet<string> updatedFiles;
    FlowId firstMergedTimestamp{0};
    int firstMergedCounter = 0;

    for (auto it = pendingRequests.begin(); it != pendingRequests.end();) {
        auto &current = *it;
        const bool preMerged = tryPreMerge(*current, *counts, consecutiveWorkspaceEdits, updatedFiles);
        if (preMerged) {
            requestsMergedCounter++;
            if (firstMergedTimestamp.id == 0) {
                firstMergedTimestamp = current->startTracer;
                firstMergedCounter = current->counter;
            }
            // N.B.: Advances `it` to next item.
            it = pendingRequests.erase(it);
        }

        // Enqueue a merge update if we've encountered a message we couldn't merge, or we are at the end of the queue.
        if (!preMerged || it == pendingRequests.end()) {
            auto mergedMessage = performMerge(updatedFiles, consecutiveWorkspaceEdits, counts);
            if (mergedMessage != nullptr) {
                // If we merge n requests into 1 request, then we've only decreased the queue size by n - 1.
                requestsMergedCounter--;
                mergedMessage->startTracer = firstMergedTimestamp;
                mergedMessage->counter = firstMergedCounter;

                // Insert merged updates, then push iterator back to where it was.
                it = pendingRequests.insert(it, move(mergedMessage)) + 1;

                // Clear state for next round.
                counts = make_unique<SorbetWorkspaceEditCounts>(0, 0, 0, 0);
                firstMergedTimestamp.id = 0;
                firstMergedCounter = 0;
                updatedFiles.clear();
                consecutiveWorkspaceEdits.clear();
            }
        }

        if (!preMerged) {
            // preMerged is only `false` if `it` points to a non-mergeable item right now.
            ENFORCE(it != pendingRequests.end());
            // No messages were merged, so `it` needs to be advanced.
            it++;
        }
    }
    ENFORCE(pendingRequests.size() + requestsMergedCounter == originalSize);
}

void LSPLoop::enqueueRequest(const shared_ptr<spd::logger> &logger, LSPLoop::QueueState &state,
                             std::unique_ptr<LSPMessage> msg, bool collectThreadCounters) {
    Timer timeit(logger, "enqueueRequest");
    msg->counter = state.requestCounter++;
    msg->startTracer = timeit.getFlowEdge();

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
            LSPLoop::mergeFileChanges(state.pendingRequests);
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
        state.pendingRequests.push_back(move(msg));
    } else if (method == LSPMethod::SorbetError) {
        // Place errors at the *front* of the queue.
        // Otherwise, they could prevent mergeFileChanges from merging adjacent updates.
        state.pendingRequests.insert(findFirstPositionAfterLSPInitialization(state.pendingRequests), move(msg));
    } else {
        state.pendingRequests.push_back(move(msg));
        LSPLoop::mergeFileChanges(state.pendingRequests);
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
    sendMessage(LSPMessage(make_unique<NotificationMessage>(
        "2.0", LSPMethod::WindowShowMessage, make_unique<ShowMessageParams>(messageType, string(message)))));
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

unique_ptr<core::Loc> LSPLoop::lspPos2Loc(core::FileRef fref, const Position &pos, const core::GlobalState &gs) {
    core::Loc::Detail reqPos;
    reqPos.line = pos.line + 1;
    reqPos.column = pos.character + 1;
    auto offset = core::Loc::pos2Offset(fref.data(gs), reqPos);
    return make_unique<core::Loc>(core::Loc(fref, offset, offset));
}

void LSPLoop::sendMessage(const LSPMessage &msg) {
    if (msg.isResponse()) {
        ENFORCE(msg.asResponse().result || msg.asResponse().error,
                "A valid ResponseMessage must have a result or an error.");
    } else if (msg.isNotification()) {
        ENFORCE(isServerNotification(msg.method()));
    }
    auto json = msg.toJSON();
    string outResult = fmt::format("Content-Length: {}\r\n\r\n{}", json.length(), json);
    logger->debug("Write: {}\n", json);
    outputStream << outResult << flush;
}

} // namespace sorbet::realmain::lsp
