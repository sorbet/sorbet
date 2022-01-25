#include "main/lsp/LSPTypecheckerCoordinator.h"
#include "absl/synchronization/notification.h"
#include "common/concurrency/WorkerPool.h"
#include "core/lsp/PreemptionTaskManager.h"
#include "core/lsp/Task.h"
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
class TypecheckerTask final : public core::lsp::Task {
    const unique_ptr<LSPTask> task;
    const unique_ptr<LSPTypecheckerDelegate> delegate;
    const bool collectCounters;
    absl::Notification started;
    absl::Notification complete;
    CounterState counters;
    unique_ptr<Timer> timeUntilRun;

public:
    TypecheckerTask(unique_ptr<LSPTask> task, unique_ptr<LSPTypecheckerDelegate> delegate, bool collectCounters)
        : task(move(task)), delegate(move(delegate)), collectCounters(collectCounters) {}

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
        started.Notify();
        {
            Timer timeit("LSPTask::run");
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

class DangerousTypecheckerTask : public core::lsp::Task {
    unique_ptr<LSPDangerousTypecheckerTask> task;
    LSPTypechecker &typechecker;
    WorkerPool &workers;

public:
    DangerousTypecheckerTask(unique_ptr<LSPDangerousTypecheckerTask> task, LSPTypechecker &typechecker,
                             WorkerPool &workers)
        : task(move(task)), typechecker(typechecker), workers(workers){};

    void run() override {
        Timer timeit("LSPDangerousTypecheckerTask::runSpecial");
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
                                                     WorkerPool &workers)
    : preemptionTaskManager(preemptionTaskManager), shouldTerminate(false),
      typechecker(config, move(preemptionTaskManager)), config(config), hasDedicatedThread(false), workers(workers),
      emptyWorkers(WorkerPool::create(0, *config->logger)) {}

void LSPTypecheckerCoordinator::asyncRunInternal(shared_ptr<core::lsp::Task> task) {
    if (hasDedicatedThread) {
        tasks.push(move(task), 1);
    } else {
        task->run();
    }
}

void LSPTypecheckerCoordinator::syncRun(unique_ptr<LSPTask> task) {
    auto wrappedTask = make_shared<TypecheckerTask>(
        move(task), make_unique<LSPTypecheckerDelegate>(workers, typechecker), hasDedicatedThread);

    asyncRunInternal(wrappedTask);
    wrappedTask->blockUntilComplete();
}

shared_ptr<core::lsp::Task>
LSPTypecheckerCoordinator::trySchedulePreemption(std::unique_ptr<LSPQueuePreemptionTask> preemptTask) {
    auto wrappedTask =
        make_shared<TypecheckerTask>(move(preemptTask), make_unique<LSPTypecheckerDelegate>(*emptyWorkers, typechecker),
                                     /* collectCounters */ false);
    // Plant this timer before scheduling task to preempt, as task could run before we plant the timer!
    wrappedTask->timeLatencyUntilRun(make_unique<Timer>("latency.preempt_slow_path"));
    if (hasDedicatedThread && preemptionTaskManager->trySchedulePreemptionTask(wrappedTask)) {
        // Preempted; task is guaranteed to run by interrupting the slow path.
        return wrappedTask;
    } else {
        // Did not preempt, so don't collect a latency metric.
        wrappedTask->cancelTimeLatencyUntilRun();
        return nullptr;
    }
}

bool LSPTypecheckerCoordinator::tryCancelPreemption(shared_ptr<core::lsp::Task> &preemptTask) {
    return preemptionTaskManager->tryCancelScheduledPreemptionTask(preemptTask);
}

void LSPTypecheckerCoordinator::initialize(unique_ptr<InitializedTask> initializedTask) {
    auto dangerousTask = make_shared<DangerousTypecheckerTask>(move(initializedTask), typechecker, workers);
    asyncRunInternal(dangerousTask);
    dangerousTask->blockUntilReady();
}

void LSPTypecheckerCoordinator::typecheckOnSlowPath(unique_ptr<SorbetWorkspaceEditTask> editTask) {
    auto dangerousTask = make_shared<DangerousTypecheckerTask>(move(editTask), typechecker, workers);
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
            shared_ptr<core::lsp::Task> task;
            // Note: Pass in 'true' for silent to avoid spamming log with wait_pop_timed entries.
            auto result = tasks.wait_pop_timed(task, WorkerPool::BLOCK_INTERVAL(), *config->logger, true);
            if (result.gotItem()) {
                task->run();
            }
        }
    });
}

}; // namespace sorbet::realmain::lsp
