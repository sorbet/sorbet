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

class TerminateOnDestruction final {
    TaskQueue &queue;

public:
    TerminateOnDestruction(TaskQueue &queue) : queue{queue} {}
    ~TerminateOnDestruction() {
        absl::MutexLock lck(queue.getMutex());
        queue.terminate();
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
    msg.latencyTimer = make_unique<Timer>(logger, "task_latency",
                                          initializer_list<int>{50, 100, 250, 500, 1000, 1500, 2000, 2500, 5000, 10000,
                                                                15000, 20000, 25000, 30000, 35000, 40000});
}
} // namespace

unique_ptr<Joinable> LSPPreprocessor::runPreprocessor(MessageQueueState &messageQueue, absl::Mutex &messageQueueMutex) {
    return runInAThread("lspPreprocess", [this, &messageQueue, &messageQueueMutex] {
        // Propagate the termination flag across the two queues.
        NotifyOnDestruction notifyIncoming(messageQueueMutex, messageQueue.terminate);
        TerminateOnDestruction notifyProcessing(*taskQueue);
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
                absl::MutexLock lck(taskQueue->getMutex());
                // Merge the counters from all of the worker threads with those stored in
                // taskQueue.
                taskQueue->getCounters() = mergeCounters(move(taskQueue->getCounters()));
                if (taskQueue->isTerminated()) {
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
            [&messageQueue, &messageQueueMutex, logger = logger,
             config = this->config](int watchmanExitCode, const optional<string> &msg) -> void {
                {
                    absl::MutexLock lck(&messageQueueMutex);
                    if (!messageQueue.terminate) {
                        messageQueue.terminate = true;
                        messageQueue.errorCode = watchmanExitCode;
                        if (watchmanExitCode != 0 && msg.has_value()) {
                            auto params = make_unique<ShowMessageParams>(MessageType::Error, msg.value());
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
            auto timeit = make_unique<Timer>(logger, "getNewRequest");
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
                        timeit = make_unique<Timer>(logger, "getNewRequest");
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
                absl::MutexLock lck(taskQueue->getMutex());
                Timer timeit(logger, "idle");
                taskQueue->getMutex()->Await(absl::Condition(taskQueue.get(), &TaskQueue::ready));
                ENFORCE(!taskQueue->isPaused());
                if (taskQueue->isTerminated()) {
                    if (taskQueue->getErrorCode() != 0) {
                        // Abnormal termination. Exit immediately.
                        typecheckerCoord.shutdown();
                        throw EarlyReturnWithCode(taskQueue->getErrorCode());
                    } else if (taskQueue->tasks().empty()) {
                        // Normal termination. Wait until all pending requests finish.
                        break;
                    }
                }

                // Before giving up the lock, check if the typechecker is running a slow path and if the task at the
                // head of the queue can either operate on stale data (i.e., the GlobalState just prior to the change
                // that required the slow path to run) or preempt the currently running slow path. (Note that we don't
                // bother with either one in cases where we only need the indexer. TODO(aprocter): is that restriction
                // actually appropriate for stale-data tasks?)
                auto &frontTask = taskQueue->tasks().front();

                // Note that running on stale data is an experimental feature, so we hide it behind the
                // --enable-experimental-lsp-stale-state flag.
                if (opts.lspStaleStateEnabled && frontTask->finalPhase() == LSPTask::Phase::RUN &&
                    epochManager->getStatus().slowPathRunning && frontTask->canUseStaleData()) {
                    logger->debug("Trying to run on stale data");

                    frontTask = typecheckerCoord.syncRunOnStaleState(move(frontTask));

                    // If the coordinator has consumed the task, we know it was able to run it on stale state. Pop it
                    // and move on to the next task.
                    if (frontTask == nullptr) {
                        logger->debug("Succeeded in running on stale data");
                        taskQueue->tasks().pop_front();
                    }
                    // If the coordinator has not consumed the task, that means it was not able to run it on stale
                    // state, because no cancellationUndoState was present. There are (we think! see below) two
                    // possibilities here:
                    //
                    //   1. by the time we acquired the cancellationUndoState lock, the typechecker had already
                    //      finished the slow path and nulled out the cancellationUndoState; or
                    //   2. (not sure if this case is actually possible!) we acquired the lock between the time
                    //      slowPathRunning became true and the time that the typechecker actually initialized the
                    //      cancellationUndoState.
                    //
                    // In case 1, we should be able to process the task as normal shortly, since slowPathRunning has
                    // become (or is about to become) false.
                    //
                    // In case 2, we should be able to process the task on stale state once the typechecker initializes
                    // cancellationUndoState.
                    //
                    // To handle both of these cases, we insert a short sleep before heading around for another turn of
                    // the loop.
                    //
                    // TODO(aprocter): Investigate whether case 2 is actually possible, and consider if there's a
                    // better way to handle all of this than sleep-and-retry.
                    else {
                        logger->debug("Failed to grab the stale state, will try again in 100ms");
                        Timer::timedSleep(100ms, *logger, "stale_state.sleep");
                    }

                    continue;
                }

                // If the task can preempt, we may be able to schedule a preemption. Don't bother scheduling tasks to
                // preempt that only need the indexer.
                // N.B.: We check `canPreempt` last as it is mildly expensive for edits (it hashes the files)
                if (frontTask->finalPhase() == LSPTask::Phase::RUN && epochManager->getStatus().slowPathRunning &&
                    frontTask->canPreempt(indexer)) {
                    absl::Notification finished;
                    string methodStr = convertLSPMethodToString(frontTask->method);
                    auto preemptTask = make_unique<LSPQueuePreemptionTask>(*config, finished, *taskQueue, indexer);
                    auto scheduleToken = typecheckerCoord.trySchedulePreemption(move(preemptTask));

                    if (scheduleToken != nullptr) {
                        logger->debug("[Processing] Preempting slow path for task {}", methodStr);
                        // Preemption scheduling success!
                        // In this if statement **only**, `taskQueueMutex` protects all accesses to LSPIndexer. This is
                        // needed to linearize the indexing of edits, which may happen in the typechecking thread if a
                        // fast path edit preempts, with the `canPreempt` checks of edits in this thread.
                        auto headOfQueueCanPreempt = [&indexer = this->indexer,
                                                      &tasks = this->taskQueue->tasks()]() -> bool {
                            // Await always holds taskQueueMutex when calling this function, but absl doesn't know that.
                            return tasks.empty() || !tasks.front()->canPreempt(indexer);
                        };
                        // Wait until the head of the queue turns into a non-preemptible task to resume processing the
                        // queue.
                        taskQueue->getMutex()->Await(absl::Condition(&headOfQueueCanPreempt));

                        // The queue is now empty or has a task that cannot preempt. There are two possibilities here:
                        // 1) The scheduled work is now irrelevant because the task that was scheduled is now gone
                        // (e.g., the request was canceled or, in the case of an edit, merged w/ a slow path edit)
                        // 2) The scheduled work has already started (and may have finished).
                        // Pessimistically assume 1) and try to cancel the scheduled preemption.
                        if (!typecheckerCoord.tryCancelPreemption(scheduleToken)) {
                            // Cancelation failed: 2) must be the case. Unlock the queue and wait until task finishes to
                            // avoid races.
                            taskQueue->getMutex()->Unlock();
                            finished.WaitForNotification();
                            taskQueue->getMutex()->Lock();
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

                task = move(taskQueue->tasks().front());
                taskQueue->tasks().pop_front();

                // Test only: Collect counters from other threads into this thread for reporting.
                if (task->method == LSPMethod::GETCOUNTERS && !taskQueue->getCounters().hasNullCounters()) {
                    counterConsume(move(taskQueue->getCounters()));
                }
            }

            logger->trace("[Processing] Running task {} normally", convertLSPMethodToString(task->method));
            runTask(move(task));

            if (config->isInitialized() && !initializedNotification.HasBeenNotified()) {
                initializedNotification.Notify();
            }

            auto currentTime = chrono::steady_clock::now();
            if (shouldSendCountersToStatsd(currentTime)) {
                {
                    // Merge counters from worker threads.
                    absl::MutexLock counterLck(taskQueue->getMutex());
                    if (!taskQueue->getCounters().hasNullCounters()) {
                        counterConsume(move(taskQueue->getCounters()));
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
