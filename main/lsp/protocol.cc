#include "absl/synchronization/mutex.h"
#include "absl/synchronization/notification.h"
#include "common/Timer.h"
#include "common/web_tracer_framework/tracing.h"
#include "lsp.h"
#include "main/lsp/LSPInput.h"
#include "main/lsp/LSPPreprocessor.h"
#include "main/lsp/watchman/WatchmanProcess.h"
#include "main/options/options.h" // For EarlyReturnWithCode.
#include <iostream>

using namespace std;

namespace sorbet::realmain::lsp {

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

CounterState mergeCounters(CounterState counters) {
    if (!counters.hasNullCounters()) {
        counterConsume(move(counters));
    }
    return getAndClearThreadCounters();
}

void tagNewRequest(const std::shared_ptr<spd::logger> &logger, LSPMessage &msg) {
    Timer timeit(logger, "tagNewRequest");
    msg.startTracers.push_back(timeit.getFlowEdge());
    msg.timer = make_unique<Timer>(logger, "processing_time");
}

unique_ptr<Joinable> LSPPreprocessor::runPreprocessor(QueueState &incomingQueue, absl::Mutex &incomingMtx,
                                                      QueueState &processingQueue, absl::Mutex &processingMtx) {
    ENFORCE(owner == this_thread::get_id());
    return runInAThread("lspPreprocess", [this, &incomingQueue, &incomingMtx, &processingQueue, &processingMtx] {
        // Propagate the termination flag across the two queues.
        NotifyOnDestruction notifyIncoming(incomingMtx, incomingQueue.terminate);
        NotifyOnDestruction notifyProcessing(processingMtx, processingQueue.terminate);
        ttgs.switchToNewThread();
        owner = this_thread::get_id();
        while (true) {
            unique_ptr<LSPMessage> msg;
            {
                absl::MutexLock lck(&incomingMtx);
                incomingMtx.Await(absl::Condition(
                    +[](QueueState *incomingQueue) -> bool {
                        return incomingQueue->terminate || !incomingQueue->pendingRequests.empty();
                    },
                    &incomingQueue));
                // Only terminate once incoming queue is drained.
                if (incomingQueue.terminate && incomingQueue.pendingRequests.empty()) {
                    config->logger->debug("Preprocessor terminating");
                    return;
                }
                msg = move(incomingQueue.pendingRequests.front());
                incomingQueue.pendingRequests.pop_front();
                // Combine counters with this thread's counters.
                if (!incomingQueue.counters.hasNullCounters()) {
                    counterConsume(move(incomingQueue.counters));
                }
            }

            preprocessAndEnqueue(processingQueue, move(msg), processingMtx);

            {
                absl::MutexLock lck(&processingMtx);
                // Merge the counters from all of the worker threads with those stored in
                // processingQueue.
                processingQueue.counters = mergeCounters(move(processingQueue.counters));
            }
        }
    });
}

void LSPLoop::maybeStartCommitSlowPathEdit(const LSPMessage &msg) const {
    if (msg.isNotification() && msg.method() == LSPMethod::SorbetWorkspaceEdit) {
        // While we're holding the queue lock (and preventing new messages from entering), start a
        // commit for an epoch if this message will trigger a cancelable slow path.
        const auto &params = get<unique_ptr<SorbetWorkspaceEditParams>>(msg.asNotification().params);
        if (!params->updates.canTakeFastPath && params->updates.updatedGS.has_value()) {
            auto &gs = params->updates.updatedGS.value();
            gs->startCommitEpoch(params->updates.versionStart - 1, params->updates.versionEnd);
        }
    }
}

optional<unique_ptr<core::GlobalState>> LSPLoop::runLSP(shared_ptr<LSPInput> input) {
    // Naming convention: thread that executes this function is called typechecking thread

    // Incoming queue stores requests that arrive from the client and Watchman. No preprocessing is performed on
    // these messages (e.g., edits are not merged).
    absl::Mutex incomingMtx;
    QueueState incomingQueue;

    // Notifies threads once LSP is initialized. Used to prevent Watchman thread from enqueueing messages that mutate
    // file state until after initialization.
    absl::Notification initializedNotification;

    // Processing queue contains preprocessed messages that are ready to be processed (e.g., edits are merged).
    absl::Mutex processingMtx;
    QueueState processingQueue;

    auto typecheckThread = typecheckerCoord.startTypecheckerThread();

    unique_ptr<watchman::WatchmanProcess> watchmanProcess;
    const auto &opts = config->opts;
    auto &logger = config->logger;
    if (!opts.disableWatchman) {
        if (opts.rawInputDirNames.size() != 1 || !opts.rawInputFileNames.empty()) {
            logger->error("Watchman support currently only works when Sorbet is run with a single input directory. If "
                          "Watchman is not needed, run Sorbet with `--disable-watchman`.");
            throw options::EarlyReturnWithCode(1);
        }

        // The lambda below intentionally does not capture `this`.
        watchmanProcess = make_unique<watchman::WatchmanProcess>(
            logger, opts.watchmanPath, opts.rawInputDirNames.at(0), vector<string>({"rb", "rbi"}),
            [&incomingQueue, &incomingMtx, logger = logger,
             &initializedNotification](std::unique_ptr<WatchmanQueryResponse> response) {
                auto notifMsg =
                    make_unique<NotificationMessage>("2.0", LSPMethod::SorbetWatchmanFileChange, move(response));
                auto msg = make_unique<LSPMessage>(move(notifMsg));
                // Don't start enqueueing requests until LSP is initialized.
                initializedNotification.WaitForNotification();
                {
                    absl::MutexLock lck(&incomingMtx);
                    tagNewRequest(logger, *msg);
                    incomingQueue.counters = mergeCounters(move(incomingQueue.counters));
                    incomingQueue.pendingRequests.push_back(move(msg));
                }
            },
            [&incomingQueue, &incomingMtx, logger = logger](int watchmanExitCode) {
                {
                    absl::MutexLock lck(&incomingMtx);
                    if (!incomingQueue.terminate) {
                        incomingQueue.terminate = true;
                        incomingQueue.errorCode = watchmanExitCode;
                    }
                    logger->debug("Watchman terminating");
                }
            });
    }

    auto readerThread = runInAThread("lspReader", [&incomingQueue, &incomingMtx, logger = logger, input = move(input)] {
        // Thread that executes this lambda is called reader thread.
        // This thread _intentionally_ does not capture `this`.
        NotifyOnDestruction notify(incomingMtx, incomingQueue.terminate);
        try {
            auto timeit = make_unique<Timer>(logger, "getNewRequest");
            while (true) {
                auto msg = input->read();
                {
                    absl::MutexLock lck(&incomingMtx); // guards guardedState.
                    if (msg) {
                        tagNewRequest(logger, *msg);
                        incomingQueue.counters = mergeCounters(move(incomingQueue.counters));
                        incomingQueue.pendingRequests.push_back(move(msg));
                        // Reset span now that we've found a request.
                        timeit = make_unique<Timer>(logger, "getNewRequest");
                    }
                    // Check if it's time to exit.
                    if (incomingQueue.terminate) {
                        // Another thread exited.
                        break;
                    }
                }
            }
        } catch (FileReadException e) {
            // Failed to read from input stream. Ignore. NotifyOnDestruction will take care of exiting cleanly.
        }
        logger->debug("Reader thread terminating");
    });

    // Bridges the gap between the {reader, watchman} threads and the typechecking thread.
    auto preprocessingThread = preprocessor.runPreprocessor(incomingQueue, incomingMtx, processingQueue, processingMtx);

    mainThreadId = this_thread::get_id();
    {
        // Ensure Watchman thread gets unstuck when thread exits prior to initialization.
        NotifyNotificationOnDestruction notify(initializedNotification);
        // Ensure preprocessor, reader, and watchman threads get unstuck when thread exits.
        NotifyOnDestruction notifyIncoming(incomingMtx, incomingQueue.terminate);
        bool exitProcessed = false;
        while (true) {
            unique_ptr<LSPMessage> msg;
            bool hasMoreMessages;
            {
                absl::MutexLock lck(&processingMtx);
                Timer timeit(logger, "idle");
                processingMtx.Await(absl::Condition(
                    +[](QueueState *processingQueue) -> bool {
                        return processingQueue->terminate ||
                               (!processingQueue->paused && !processingQueue->pendingRequests.empty());
                    },
                    &processingQueue));
                ENFORCE(!processingQueue.paused);
                if (processingQueue.terminate) {
                    if (processingQueue.errorCode != 0) {
                        // Abnormal termination.
                        typecheckerCoord.shutdown();
                        throw options::EarlyReturnWithCode(processingQueue.errorCode);
                    } else if (exitProcessed || processingQueue.pendingRequests.empty()) {
                        // Normal termination. Wait until all pending requests finish or we process an exit.
                        break;
                    }
                }
                msg = move(processingQueue.pendingRequests.front());
                processingQueue.pendingRequests.pop_front();
                hasMoreMessages = !processingQueue.pendingRequests.empty();
                exitProcessed = msg->isNotification() && msg->method() == LSPMethod::Exit;
                maybeStartCommitSlowPathEdit(*msg);
            }
            prodCounterInc("lsp.messages.received");
            processRequestInternal(*msg);

            if (config->isInitialized() && !initializedNotification.HasBeenNotified()) {
                initializedNotification.Notify();
            }

            auto currentTime = chrono::steady_clock::now();
            if (shouldSendCountersToStatsd(currentTime)) {
                {
                    // Merge counters from worker threads.
                    absl::MutexLock counterLck(&processingMtx);
                    if (!processingQueue.counters.hasNullCounters()) {
                        counterConsume(move(processingQueue.counters));
                    }
                }
                sendCountersToStatsd(currentTime);
            }
            logger->flush();
        }
    }

    logger->debug("Processor terminating");
    auto gs = typecheckerCoord.shutdown();
    if (gs) {
        return gs;
    } else {
        return nullopt;
    }
}

} // namespace sorbet::realmain::lsp
