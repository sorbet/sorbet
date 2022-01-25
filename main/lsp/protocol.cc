#include "absl/synchronization/mutex.h"
#include "absl/synchronization/notification.h"
#include "common/EarlyReturnWithCode.h"
#include "common/Timer.h"
#include "common/web_tracer_framework/tracing.h"
#include "core/lsp/TypecheckEpochManager.h"
#include "main/lsp/LSPConfiguration.h"
#include "main/lsp/LSPInput.h"
#include "main/lsp/LSPOutput.h"
#include "main/lsp/LSPPreprocessor.h"
#include "main/lsp/LSPTask.h"
#include "main/lsp/json_types.h"
#include "main/lsp/lsp.h"
#include "main/lsp/watchman/WatchmanProcess.h"

using namespace std;
namespace spd = spdlog;

namespace sorbet::realmain::lsp {

namespace {
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

void tagNewRequest(spd::logger &logger, LSPMessage &msg) {
    msg.latencyTimer =
        make_unique<Timer>("task_latency", initializer_list<int>{50, 100, 250, 500, 1000, 1500, 2000, 2500, 5000, 10000,
                                                                 15000, 20000, 25000, 30000, 35000, 40000});
}
} // namespace

unique_ptr<Joinable> LSPPreprocessor::runPreprocessor(MessageQueueState &messageQueue, absl::Mutex &messageQueueMutex) {
    return runInAThread("lspPreprocess", [this, &messageQueue, &messageQueueMutex] {
        // Propagate the termination flag across the two queues.
        NotifyOnDestruction notifyIncoming(messageQueueMutex, messageQueue.terminate);
        NotifyOnDestruction notifyProcessing(*taskQueueMutex, ABSL_TS_UNCHECKED_READ(taskQueue)->terminate);
        owner = this_thread::get_id();
        while (true) {
            unique_ptr<LSPMessage> msg;
            {
                absl::MutexLock lck(&messageQueueMutex);
                messageQueueMutex.Await(absl::Condition(
                    +[](MessageQueueState *messageQueue) -> bool {
                        return messageQueue->terminate || !messageQueue->pendingRequests.empty();
                    },
                    &messageQueue));
                // Only terminate once incoming queue is drained.
                if (messageQueue.terminate && messageQueue.pendingRequests.empty()) {
                    config->logger->debug("Preprocessor terminating");
                    return;
                }
                msg = move(messageQueue.pendingRequests.front());
                messageQueue.pendingRequests.pop_front();
                // Combine counters with this thread's counters.
                if (!messageQueue.counters.hasNullCounters()) {
                    counterConsume(move(messageQueue.counters));
                }
            }

            preprocessAndEnqueue(move(msg));

            {
                absl::MutexLock lck(taskQueueMutex.get());
                // Merge the counters from all of the worker threads with those stored in
                // taskQueue.
                taskQueue->counters = mergeCounters(move(taskQueue->counters));
                if (taskQueue->terminate) {
                    // We must have processed an exit notification, or one of the downstream threads exited.
                    return;
                }
            }
        }
    });
}

optional<unique_ptr<core::GlobalState>> LSPLoop::runLSP(shared_ptr<LSPInput> input) {
    // Naming convention: thread that executes this function is called processing thread

    // Message queue stores requests that arrive from the client and Watchman. No preprocessing is performed on
    // these messages (e.g., edits are not merged).
    absl::Mutex messageQueueMutex;
    MessageQueueState messageQueue;

    // Notifies threads once LSP is initialized. Used to prevent Watchman thread from enqueueing messages that mutate
    // file state until after initialization.
    absl::Notification initializedNotification;

    auto typecheckThread = typecheckerCoord.startTypecheckerThread();

    unique_ptr<watchman::WatchmanProcess> watchmanProcess;
    const auto &opts = config->opts;
    auto &logger = config->logger;
    if (!opts.disableWatchman) {
        if (opts.rawInputDirNames.size() != 1 || !opts.rawInputFileNames.empty()) {
            auto msg = "Watchman support currently only works when Sorbet is run with a single input directory. If "
                       "Watchman is not needed, run Sorbet with `--disable-watchman`.";
            logger->error(msg);
            auto params = make_unique<ShowMessageParams>(MessageType::Error, msg);
            config->output->write(make_unique<LSPMessage>(
                make_unique<NotificationMessage>("2.0", LSPMethod::WindowShowMessage, move(params))));
            throw EarlyReturnWithCode(1);
        }

        // The lambda below intentionally does not capture `this`.
        watchmanProcess = make_unique<watchman::WatchmanProcess>(
            logger, opts.watchmanPath, opts.rawInputDirNames.at(0), vector<string>({"rb", "rbi"}),
            [&messageQueueMutex, &messageQueue, logger = logger,
             &initializedNotification](std::unique_ptr<WatchmanQueryResponse> response) {
                auto notifMsg =
                    make_unique<NotificationMessage>("2.0", LSPMethod::SorbetWatchmanFileChange, move(response));
                auto msg = make_unique<LSPMessage>(move(notifMsg));
                // Don't start enqueueing requests until LSP is initialized.
                initializedNotification.WaitForNotification();
                {
                    absl::MutexLock lck(&messageQueueMutex);
                    tagNewRequest(*logger, *msg);
                    messageQueue.counters = mergeCounters(move(messageQueue.counters));
                    messageQueue.pendingRequests.push_back(move(msg));
                }
            },
            [&messageQueue, &messageQueueMutex, logger = logger, config = this->config](int watchmanExitCode,
                                                                                        string const &msg) {
                {
                    absl::MutexLock lck(&messageQueueMutex);
                    if (!messageQueue.terminate) {
                        messageQueue.terminate = true;
                        messageQueue.errorCode = watchmanExitCode;
                        if (watchmanExitCode != 0) {
                            auto params = make_unique<ShowMessageParams>(MessageType::Error, msg);
                            config->output->write(make_unique<LSPMessage>(
                                make_unique<NotificationMessage>("2.0", LSPMethod::WindowShowMessage, move(params))));
                        }
                    }
                    logger->debug("Watchman terminating");
                }
            });
    }

    auto readerThread =
        runInAThread("lspReader", [&messageQueue, &messageQueueMutex, logger = logger, input = move(input)] {
            // Thread that executes this lambda is called reader thread.
            // This thread _intentionally_ does not capture `this`.
            NotifyOnDestruction notify(messageQueueMutex, messageQueue.terminate);
            auto timeit = make_unique<Timer>("getNewRequest");
            while (true) {
                auto readResult = input->read();
                if (readResult.result == FileOps::ReadResult::ErrorOrEof) {
                    // Exit loop if there is an error reading from input.
                    break;
                }
                {
                    absl::MutexLock lck(&messageQueueMutex); // guards guardedState.
                    auto &msg = readResult.message;
                    if (msg) {
                        tagNewRequest(*logger, *msg);
                        messageQueue.counters = mergeCounters(move(messageQueue.counters));
                        messageQueue.pendingRequests.push_back(move(msg));
                        // Reset span now that we've found a request.
                        timeit = make_unique<Timer>("getNewRequest");
                    }
                    // Check if it's time to exit.
                    if (messageQueue.terminate) {
                        // Another thread exited.
                        break;
                    }
                }
            }
            logger->debug("Reader thread terminating");
        });

    // Bridges the gap between the {reader, watchman} threads and the typechecking thread.
    auto preprocessingThread = preprocessor.runPreprocessor(messageQueue, messageQueueMutex);

    mainThreadId = this_thread::get_id();
    {
        // Ensure Watchman thread gets unstuck when thread exits prior to initialization.
        NotifyNotificationOnDestruction notify(initializedNotification);
        // Ensure preprocessor, reader, and watchman threads get unstuck when thread exits.
        NotifyOnDestruction notifyIncoming(messageQueueMutex, messageQueue.terminate);
        while (true) {
            unique_ptr<LSPTask> task;
            {
                absl::MutexLock lck(taskQueueMutex.get());
                Timer timeit("idle");
                taskQueueMutex->Await(absl::Condition(
                    +[](TaskQueueState *taskQueue) -> bool {
                        return taskQueue->terminate || (!taskQueue->paused && !taskQueue->pendingTasks.empty());
                    },
                    taskQueue.get()));
                ENFORCE(!taskQueue->paused);
                if (taskQueue->terminate) {
                    if (taskQueue->errorCode != 0) {
                        // Abnormal termination. Exit immediately.
                        typecheckerCoord.shutdown();
                        throw EarlyReturnWithCode(taskQueue->errorCode);
                    } else if (taskQueue->pendingTasks.empty()) {
                        // Normal termination. Wait until all pending requests finish.
                        break;
                    }
                }

                // Before giving up the lock, check if the typechecker is running a slow path and if the task at the
                // head of the queue can preempt. If it is, we may be able to schedule a preemption.
                // Don't bother scheduling tasks to preempt that only need the indexer.
                // N.B.: We check `canPreempt` last as it is mildly expensive for edits (it hashes the files)
                auto &frontTask = taskQueue->pendingTasks.front();
                if (frontTask->finalPhase() == LSPTask::Phase::RUN && epochManager->getStatus().slowPathRunning &&
                    frontTask->canPreempt(indexer)) {
                    absl::Notification finished;
                    string methodStr = convertLSPMethodToString(frontTask->method);
                    auto preemptTask =
                        make_unique<LSPQueuePreemptionTask>(*config, finished, *taskQueueMutex, *taskQueue, indexer);
                    auto scheduleToken = typecheckerCoord.trySchedulePreemption(move(preemptTask));

                    if (scheduleToken != nullptr) {
                        logger->debug("[Processing] Preempting slow path for task {}", methodStr);
                        // Preemption scheduling success!
                        // In this if statement **only**, `taskQueueMutex` protects all accesses to LSPIndexer. This is
                        // needed to linearize the indexing of edits, which may happen in the typechecking thread if a
                        // fast path edit preempts, with the `canPreempt` checks of edits in this thread.
                        auto headOfQueueCanPreempt = [&indexer = this->indexer,
                                                      &taskQueue = this->taskQueue]() -> bool {
                            // Await always holds taskQueueMutex when calling this function, but absl doesn't know that.
                            return ABSL_TS_UNCHECKED_READ(taskQueue)->pendingTasks.empty() ||
                                   !ABSL_TS_UNCHECKED_READ(taskQueue)->pendingTasks.front()->canPreempt(indexer);
                        };
                        // Wait until the head of the queue turns into a non-preemptible task to resume processing the
                        // queue.
                        taskQueueMutex->Await(absl::Condition(&headOfQueueCanPreempt));

                        // The queue is now empty or has a task that cannot preempt. There are two possibilities here:
                        // 1) The scheduled work is now irrelevant because the task that was scheduled is now gone
                        // (e.g., the request was canceled or, in the case of an edit, merged w/ a slow path edit)
                        // 2) The scheduled work has already started (and may have finished).
                        // Pessimistically assume 1) and try to cancel the scheduled preemption.
                        if (!typecheckerCoord.tryCancelPreemption(scheduleToken)) {
                            // Cancelation failed: 2) must be the case. Unlock the queue and wait until task finishes to
                            // avoid races.
                            taskQueueMutex->Unlock();
                            finished.WaitForNotification();
                            taskQueueMutex->Lock();
                            logger->debug("[Processing] Preemption for task {} complete", methodStr);
                        } else {
                            logger->debug("[Processing] Canceled scheduled preemption for task {}", methodStr);
                        }

                        // At this point, we are guaranteed that the scheduled task has run or has been canceled.
                        continue;
                    }
                    // If preemption scheduling failed, then the slow path probably finished just now. Continue as
                    // normal.
                }

                task = move(taskQueue->pendingTasks.front());
                taskQueue->pendingTasks.pop_front();

                // Test only: Collect counters from other threads into this thread for reporting.
                if (task->method == LSPMethod::GETCOUNTERS && !taskQueue->counters.hasNullCounters()) {
                    counterConsume(move(taskQueue->counters));
                }
            }

            logger->debug("[Processing] Running task {} normally", convertLSPMethodToString(task->method));
            runTask(move(task));

            if (config->isInitialized() && !initializedNotification.HasBeenNotified()) {
                initializedNotification.Notify();
            }

            auto currentTime = chrono::steady_clock::now();
            if (shouldSendCountersToStatsd(currentTime)) {
                {
                    // Merge counters from worker threads.
                    absl::MutexLock counterLck(taskQueueMutex.get());
                    if (!taskQueue->counters.hasNullCounters()) {
                        counterConsume(move(taskQueue->counters));
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
