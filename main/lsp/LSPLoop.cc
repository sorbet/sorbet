#include "main/lsp/LSPLoop.h"
#include "absl/synchronization/mutex.h"
#include "absl/synchronization/notification.h"
#include "common/EarlyReturnWithCode.h"
#include "common/concurrency/WorkerPool.h"
#include "common/kvstore/KeyValueStore.h"
#include "common/statsd/statsd.h"
#include "common/timers/Timer.h"
#include "common/web_tracer_framework/tracing.h"
#include "core/errors/internal.h"
#include "core/errors/namer.h"
#include "core/errors/resolver.h"
#include "core/lsp/PreemptionTaskManager.h"
#include "core/lsp/TypecheckEpochManager.h"
#include "main/lsp/LSPConfiguration.h"
#include "main/lsp/LSPInput.h"
#include "main/lsp/LSPOutput.h"
#include "main/lsp/LSPPreprocessor.h"
#include "main/lsp/LSPTask.h"
#include "main/lsp/json_types.h"
#include "main/lsp/notifications/sorbet_workspace_edit.h"
#include "main/lsp/watchman/WatchmanProcess.h"
#include "sorbet_version/sorbet_version.h"

using namespace std;

namespace sorbet::realmain::lsp {

LSPLoop::LSPLoop(std::unique_ptr<core::GlobalState> initialGS, WorkerPool &workers,
                 const std::shared_ptr<LSPConfiguration> &config, std::unique_ptr<KeyValueStore> kvstore)
    : config(config), taskQueue(make_shared<TaskQueue>()), epochManager(initialGS->epochManager),
      preprocessor(config, taskQueue),
      typecheckerCoord(config, make_shared<core::lsp::PreemptionTaskManager>(initialGS->epochManager), workers,
                       taskQueue),
      indexer(config, move(initialGS), move(kvstore)), emptyWorkers(WorkerPool::create(0, *config->logger)),
      lastMetricUpdateTime(chrono::steady_clock::now()) {}

constexpr chrono::minutes STATSD_INTERVAL = chrono::minutes(5);

bool LSPLoop::shouldSendCountersToStatsd(chrono::time_point<chrono::steady_clock> currentTime) const {
    // If --web-trace-file, always flush after every task (probably: someone is debugging).
    // Otherwise, batch up connections to hitting statsd (probably: normal mode of operation).
    // Note: passing --web-trace-file will override the "only send every STATSD_INTERVAL" for
    // statsd reporting. So it's *likely* bad to pass all of `--lsp`, `--statsd-host`, and
    // `--web-trace-file` at the same time.
    return !config->opts.webTraceFile.empty() ||
           (!config->opts.statsdHost.empty() && (currentTime - lastMetricUpdateTime) > STATSD_INTERVAL);
}

void LSPLoop::sendCountersToStatsd(chrono::time_point<chrono::steady_clock> currentTime) {
    Timer timeit(config->logger, "LSPLoop::sendCountersToStatsd");
    ENFORCE(this_thread::get_id() == mainThreadId, "sendCounterToStatsd can only be called from the main LSP thread.");
    const auto &opts = config->opts;
    // Record process and version stats. Do this BEFORE clearing the thread counters!
    StatsD::addStandardMetrics();
    auto counters = getAndClearThreadCounters();
    if (!opts.statsdHost.empty()) {
        lastMetricUpdateTime = currentTime;
        auto prefix = fmt::format("{}.lsp.counters", opts.statsdPrefix);
        StatsD::submitCounters(counters, opts.statsdHost, opts.statsdPort, prefix);
    }

    if (!opts.webTraceFile.empty()) {
        timeit.setTag("webtracefile", "true");
        web_tracer_framework::Tracing::storeTraces(counters, opts.webTraceFile);
    } else {
        timeit.setTag("webtracefile", "false");
    }
}

namespace {
class TypecheckCountTask : public LSPTask {
    int &count;

public:
    TypecheckCountTask(const LSPConfiguration &config, int &count)
        : LSPTask(config, LSPMethod::SorbetError), count(count) {}

    bool canPreempt(const LSPIndexer &indexer) const override {
        return false;
    }

    void run(LSPTypecheckerDelegate &tc) override {
        count = tc.state().lspTypecheckCount;
    }
};
} // namespace

int LSPLoop::getTypecheckCount() {
    int count = 0;
    typecheckerCoord.syncRun(make_unique<TypecheckCountTask>(*config, count));
    return count;
}

namespace {
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

} // namespace

void LSPLoop::processRequest(const string &json) {
    vector<unique_ptr<LSPMessage>> messages;
    messages.push_back(LSPMessage::fromClient(json));
    LSPLoop::processRequests(move(messages));
}

void LSPLoop::processRequest(std::unique_ptr<LSPMessage> msg) {
    vector<unique_ptr<LSPMessage>> messages;
    messages.push_back(move(msg));
    processRequests(move(messages));
}

void LSPLoop::processRequests(vector<unique_ptr<LSPMessage>> messages) {
    for (auto &message : messages) {
        preprocessor.preprocessAndEnqueue(move(message));
    }

    std::vector<std::unique_ptr<LSPTask>> tasks;
    while (true) {
        {
            absl::MutexLock lck(taskQueue->getMutex());
            ENFORCE(!taskQueue->isPaused(), "__PAUSE__ not supported in single-threaded mode.");
            auto &queuedTasks = taskQueue->tasks();
            tasks.reserve(queuedTasks.size());
            for (auto &task : queuedTasks) {
                tasks.emplace_back(move(task));
            }
            queuedTasks.clear();
        }

        if (tasks.empty()) {
            break;
        }

        for (auto &task : tasks) {
            runTask(std::move(task));
        }
        tasks.clear();
    }
}

void LSPLoop::runTask(unique_ptr<LSPTask> task) {
    prodCategoryCounterInc("lsp.messages.processed", task->methodString());
    {
        Timer timeit(config->logger, "LSPTask::index");
        timeit.setTag("method", task->methodString());
        task->index(this->indexer);
    }
    if (task->finalPhase() == LSPTask::Phase::INDEX) {
        // Task doesn't need the typechecker.
        return;
    }

    // Check if the task is a dangerous task, as those are scheduled specially.
    if (auto *dangerousTask = dynamic_cast<LSPDangerousTypecheckerTask *>(task.get())) {
        if (auto *editTask = dynamic_cast<SorbetWorkspaceEditTask *>(dangerousTask)) {
            unique_ptr<SorbetWorkspaceEditTask> edit(editTask);
            (void)task.release();
            switch (edit->getTypecheckingPath(indexer)) {
                case TypecheckingPath::Fast: {
                    // Can run on fast path synchronously; it should complete quickly.
                    typecheckerCoord.syncRun(move(edit));
                    break;
                }
                case TypecheckingPath::Slow: {
                    // Must run on slow path; this method is async in multithreaded environments, and blocks in
                    // single threaded environments.
                    typecheckerCoord.typecheckOnSlowPath(move(edit));
                    break;
                }
            }

        } else {
            // Must be a new type of dangerous task we don't know about.
            // Please do not add new dangerous tasks to the codebase. Try to surface whatever functionality you
            // require safely through LSPTypecheckerCoordinator.
            ENFORCE(false);
        }
    } else {
        // Run synchronously.
        typecheckerCoord.syncRun(move(task));
    }
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

        watchmanProcess = make_unique<watchman::WatchmanProcess>(
            logger, opts.watchmanPath, opts.rawInputDirNames.at(0), vector<string>({"rb", "rbi"}), messageQueue,
            messageQueueMutex, initializedNotification, this->config);
    }

    auto readerThread =
        runInAThread("lspReader", [&messageQueue, &messageQueueMutex, logger = logger, input = move(input)] {
            // Thread that executes this lambda is called reader thread.
            // This thread _intentionally_ does not capture `this`.
            MessageQueueState::NotifyOnDestruction notify(messageQueue, messageQueueMutex);
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
                        msg->tagNewRequest(*logger);
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
        MessageQueueState::NotifyOnDestruction notifyIncoming(messageQueue, messageQueueMutex);
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
                // head of the queue can preempt. If it is, we may be able to schedule a preemption.
                // Don't bother scheduling tasks to preempt that only need the indexer.
                // N.B.: We check `canPreempt` last as it is mildly expensive for edits (it hashes the files)
                auto &frontTask = taskQueue->tasks().front();

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
