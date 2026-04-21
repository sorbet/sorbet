#include "main/lsp/LSPTypecheckerCoordinator.h"
#include "absl/synchronization/notification.h"
#include "common/concurrency/WorkerPool.h"
#include "core/lsp/PreemptionTask.h"
#include "core/lsp/PreemptionTaskManager.h"
#include "core/lsp/TypecheckEpochManager.h"
#include "main/lsp/LSPTask.h"
#include "main/lsp/notifications/initialized.h"
#include "main/lsp/notifications/sorbet_workspace_edit.h"

namespace sorbet::realmain::lsp {
using namespace std;

namespace {
/**
 * Adapter class from LSPTask to Task. Handles passing a `LSPTypecheckerDelegate` to the `LSPTask`.
 * Why use `Task` internally instead of `LSPTask`? There are four different contexts involved in scheduling a unit of
 * work on the typechecking thread:
 * - request_dispatch.cc: Creates the LSPTask, but has no access to a `LSPTypecheckerDelegate` (intentionally).
 * - Method on LSPTypecheckerCoordinator: Has enough context to create the `LSPTypecheckerDelegate`.
 * - Typechecker thread: Does not have enough context to create the `LSPTypecheckerDelegate`. Also, I'd like to reserve
 * the right to run other types of tasks on this thread when appropriate.
 * - core/lsp/PreemptionTaskManager: Knows nothing about any of this and just wants to run a method with no args.
 */
class TypecheckerTask final : public LSPTypecheckerCoordinator::Task {
    const LSPConfiguration &config;
    const unique_ptr<LSPTask> task;
    const unique_ptr<LSPTypecheckerDelegate> delegate;
    const bool collectCounters;
    absl::Notification complete;
    CounterState counters;
    unique_ptr<Timer> timeUntilRun;

public:
    TypecheckerTask(const LSPConfiguration &config, unique_ptr<LSPTask> task,
                    unique_ptr<LSPTypecheckerDelegate> delegate, bool collectCounters)
        : config(config), task(move(task)), delegate(move(delegate)), collectCounters(collectCounters) {}

    void timeLatencyUntilRun(unique_ptr<Timer> timer) {
        timeUntilRun = move(timer);
    }

    void cancelTimeLatencyUntilRun() {
        if (timeUntilRun != nullptr) {
            timeUntilRun->cancel();
            timeUntilRun = nullptr;
        }
    }

    void run() override {
        // Destruct timer, if specified. Causes metric to be reported.
        timeUntilRun = nullptr;
        {
            Timer timeit(config.logger, "LSPTask::run");
            timeit.setTag("method", task->methodString());
            task->run(*delegate);
        }
        if (collectCounters) {
            counters = getAndClearThreadCounters();
        }
        complete.Notify();
    }

    void blockUntilComplete() {
        complete.WaitForNotification();
        if (collectCounters) {
            counterConsume(move(counters));
        }
    }
};

class DangerousTypecheckerTask : public LSPTypecheckerCoordinator::Task {
    const LSPConfiguration &config;
    unique_ptr<LSPDangerousTypecheckerTask> task;
    LSPTypechecker &typechecker;
    WorkerPool &workers;

public:
    DangerousTypecheckerTask(const LSPConfiguration &config, unique_ptr<LSPDangerousTypecheckerTask> task,
                             LSPTypechecker &typechecker, WorkerPool &workers)
        : config(config), task(move(task)), typechecker(typechecker), workers(workers){};

    void run() override {
        Timer timeit(config.logger, "LSPDangerousTypecheckerTask::runSpecial");
        timeit.setTag("method", task->methodString());
        task->runSpecial(typechecker, workers);
    }

    void blockUntilReady() {
        task->schedulerWaitUntilReady();
    }
};

class ShutdownTask : public LSPTask {
    LSPTypechecker &typechecker;
    bool &shouldTerminate;
    unique_ptr<core::GlobalState> &gs;

public:
    ShutdownTask(const LSPConfiguration &config, LSPTypechecker &typechecker, bool &shouldTerminate,
                 unique_ptr<core::GlobalState> &gs)
        : LSPTask(config, LSPMethod::Exit), typechecker(typechecker), shouldTerminate(shouldTerminate), gs(gs) {}

    bool canPreempt(const LSPIndexer &indexer) const override {
        return false;
    }

    void run(LSPTypecheckerDelegate &_) override {
        shouldTerminate = true;
        gs = typechecker.destroy();
    }
};

}; // namespace

LSPTypecheckerCoordinator::LSPTypecheckerCoordinator(const shared_ptr<const LSPConfiguration> &config,
                                                     shared_ptr<core::lsp::PreemptionTaskManager> preemptionTaskManager,
                                                     WorkerPool &workers, shared_ptr<TaskQueue> taskQueue)
    : preemptionTaskManager(preemptionTaskManager), shouldTerminate(false),
      typechecker(config, move(preemptionTaskManager)), config(config), hasDedicatedThread(false),
      workers(workers), taskQueue{std::move(taskQueue)},
      preemptionWorkers(WorkerPool::create(config->opts.threads, *config->logger)) {}

void LSPTypecheckerCoordinator::asyncRunInternal(shared_ptr<LSPTypecheckerCoordinator::Task> task) {
    if (hasDedicatedThread) {
        tasks.push(move(task), 1);
    } else {
        task->run();
    }
}

void LSPTypecheckerCoordinator::syncRun(unique_ptr<LSPTask> task) {
    auto wrappedTask = make_shared<TypecheckerTask>(
        *config, move(task), make_unique<LSPTypecheckerDelegate>(*taskQueue, workers, typechecker), hasDedicatedThread);

    asyncRunInternal(wrappedTask);
    wrappedTask->blockUntilComplete();
}

namespace {

class PreemptionLoop : public core::lsp::PreemptionTask {
    const LSPConfiguration &config;
    absl::Notification &finished;
    TaskQueue &taskQueue;
    LSPTypecheckerDelegate delegate;
    LSPIndexer &indexer;

    unique_ptr<Timer> timeUntilRun;

public:
    PreemptionLoop(const LSPConfiguration &config, absl::Notification &finished, TaskQueue &taskQueue,
                   WorkerPool &preemptionWorkers, LSPTypechecker &typechecker, LSPIndexer &indexer)
        : config{config}, finished{finished}, taskQueue{taskQueue},
          delegate(this->taskQueue, preemptionWorkers, typechecker), indexer{indexer} {}

    void timeLatencyUntilRun(unique_ptr<Timer> timer) {
        timeUntilRun = move(timer);
    }

    void cancelTimeLatencyUntilRun() {
        if (timeUntilRun != nullptr) {
            timeUntilRun->cancel();
            timeUntilRun = nullptr;
        }
    }

    optional<uint16_t> run(uint16_t currentStratum) override {
        // Destruct timer, if specified. Causes metric to be reported.
        this->timeUntilRun = nullptr;

        Timer timeit(config.logger, "preemption_loop");
        for (;;) {
            unique_ptr<LSPTask> task;
            {
                absl::MutexLock lck(taskQueue.getMutex());
                if (!indexer.preemptionPossible(taskQueue.tasks())) {
                    break;
                }
                task = move(taskQueue.tasks().front());
                taskQueue.tasks().pop_front();

                {
                    Timer timeit(config.logger, "LSPTask::index");
                    timeit.setTag("method", task->methodString());
                    // Index while holding lock to prevent races with processing thread.
                    task->index(indexer);
                }
            }
            prodCategoryCounterInc("lsp.messages.processed", task->methodString());

            if (task->finalPhase() == LSPTask::Phase::INDEX) {
                continue;
            }
            Timer timeit(config.logger, "LSPTask::run");
            timeit.setTag("method", task->methodString());
            task->run(this->delegate);
        }

        return nullopt;
    }

    void finish() override {
        finished.Notify();
    }
};

} // namespace

shared_ptr<core::lsp::PreemptionTask> LSPTypecheckerCoordinator::trySchedulePreemption(absl::Notification &finished,
                                                                                       TaskQueue &taskQueue,
                                                                                       LSPIndexer &indexer) {
    auto preemptionLoop =
        make_shared<PreemptionLoop>(*config, finished, taskQueue, *preemptionWorkers, typechecker, indexer);
    // Plant this timer before scheduling task to preempt, as task could run before we plant the timer!
    preemptionLoop->timeLatencyUntilRun(make_unique<Timer>(*config->logger, "latency.preempt_slow_path"));
    if (hasDedicatedThread && preemptionTaskManager->trySchedulePreemptionTask(preemptionLoop)) {
        // Preempted; task is guaranteed to run by interrupting the slow path.
        return preemptionLoop;
    } else {
        // Did not preempt, so don't collect a latency metric.
        preemptionLoop->cancelTimeLatencyUntilRun();
        return nullptr;
    }
}

bool LSPTypecheckerCoordinator::tryCancelPreemption(shared_ptr<core::lsp::PreemptionTask> &preemptTask) {
    return preemptionTaskManager->tryCancelScheduledPreemptionTask(preemptTask);
}

void LSPTypecheckerCoordinator::typecheckOnSlowPath(unique_ptr<SorbetWorkspaceEditTask> editTask) {
    auto dangerousTask = make_shared<DangerousTypecheckerTask>(*config, move(editTask), typechecker, workers);
    asyncRunInternal(dangerousTask);
    dangerousTask->blockUntilReady();
}

unique_ptr<core::GlobalState> LSPTypecheckerCoordinator::shutdown() {
    unique_ptr<core::GlobalState> gs;
    // shouldTerminate and gs are captured by reference.
    syncRun(make_unique<ShutdownTask>(*config, typechecker, shouldTerminate, gs));
    return gs;
}

unique_ptr<Joinable> LSPTypecheckerCoordinator::startTypecheckerThread() {
    if (hasDedicatedThread) {
        Exception::raise("Typechecker already started on a dedicated thread.");
    }

    hasDedicatedThread = true;
    return runInAThread("Typechecker", [&]() -> void {
        typechecker.changeThread();

        while (!shouldTerminate) {
            shared_ptr<Task> task;
            // Note: Pass in 'true' for silent to avoid spamming log with wait_pop_timed entries.
            auto result = tasks.wait_pop_timed(task, WorkerPool::BLOCK_INTERVAL(), *config->logger, true);
            if (result.gotItem()) {
                task->run();
            }
        }
    });
}

}; // namespace sorbet::realmain::lsp
