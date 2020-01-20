#include "main/lsp/LSPTypecheckerCoordinator.h"
#include "main/lsp/LSPTask.h"

#include "absl/synchronization/notification.h"

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
    absl::Notification complete;
    CounterState counters;

public:
    TypecheckerTask(unique_ptr<LSPTask> task, unique_ptr<LSPTypecheckerDelegate> delegate, bool collectCounters)
        : task(move(task)), delegate(move(delegate)), collectCounters(collectCounters) {}

    void run() override {
        task->run(*delegate);
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

// Special internal tasks that directly operate on `LSPTypechecker`. These are the only tasks that are allowed to
// directly access `LSPTypechecker` (because they do special things). Thus, only `LSPTypecheckerCoordinator` is
// allowed/able to create them.
// TODO(jvilk): These implement `LSPTask` for the convenient `blockUntilComplete` method. Should we move that method
// to `Task` directly?

class InitializeTask : public LSPTask {
    LSPTypechecker &typechecker;
    WorkerPool &workers;
    LSPFileUpdates updates;

public:
    InitializeTask(const LSPConfiguration &config, LSPTypechecker &typechecker, WorkerPool &workers,
                   LSPFileUpdates updates)
        : LSPTask(config, true), typechecker(typechecker), workers(workers), updates(move(updates)){};

    void run(LSPTypecheckerDelegate &_) override {
        typechecker.initialize(move(updates), workers);
    }
};

class SlowPathTypecheckTask : public LSPTask {
    LSPTypechecker &typechecker;
    WorkerPool &workers;
    LSPFileUpdates updates;

public:
    SlowPathTypecheckTask(const LSPConfiguration &config, LSPTypechecker &typechecker, WorkerPool &workers,
                          LSPFileUpdates updates)
        : LSPTask(config, true), typechecker(typechecker), workers(workers), updates(move(updates)){};

    void run(LSPTypecheckerDelegate &_) override {
        const u4 end = updates.versionEnd;
        const u4 start = updates.versionStart;
        // Versions are sequential and wrap around. Use them to figure out how many edits are contained
        // within this update.
        const u4 merged = min(end - start, 0xFFFFFFFF - start + end);
        // Only report stats if the edit was committed.
        if (!typechecker.typecheck(move(updates), workers)) {
            prodCategoryCounterInc("lsp.messages.processed", "sorbet/workspaceEdit");
            prodCategoryCounterAdd("lsp.messages.processed", "sorbet/mergedEdits", merged);
        }
    }
};

class ShutdownTask : public LSPTask {
    LSPTypechecker &typechecker;
    bool &shouldTerminate;
    unique_ptr<core::GlobalState> &gs;

public:
    ShutdownTask(const LSPConfiguration &config, LSPTypechecker &typechecker, bool &shouldTerminate,
                 unique_ptr<core::GlobalState> &gs)
        : LSPTask(config, true), typechecker(typechecker), shouldTerminate(shouldTerminate), gs(gs) {}

    void run(LSPTypecheckerDelegate &_) override {
        shouldTerminate = true;
        gs = typechecker.destroy();
    }
};

}; // namespace

LSPTypecheckerCoordinator::LSPTypecheckerCoordinator(const shared_ptr<const LSPConfiguration> &config,
                                                     WorkerPool &workers)
    : shouldTerminate(false), typechecker(config), config(config), hasDedicatedThread(false), workers(workers),
      emptyWorkers(WorkerPool::create(0, *config->logger)) {}

void LSPTypecheckerCoordinator::asyncRunInternal(shared_ptr<core::lsp::Task> task) {
    if (hasDedicatedThread) {
        tasks.push(move(task), 1);
    } else {
        task->run();
    }
}

void LSPTypecheckerCoordinator::syncRun(unique_ptr<LSPTask> task) {
    // TODO(jvilk): Give single-threaded tasks a single-threaded workerpool once we land preemption.
    absl::Notification notification;
    auto wrappedTask = make_shared<TypecheckerTask>(
        move(task), make_unique<LSPTypecheckerDelegate>(workers, typechecker), hasDedicatedThread);
    asyncRunInternal(wrappedTask);
    wrappedTask->blockUntilComplete();
}

void LSPTypecheckerCoordinator::initialize(unique_ptr<InitializedParams> params) {
    // TODO: Split into a sync and async task when we land preemptible slow path, where the typecheck is async.
    syncRun(make_unique<InitializeTask>(*config, typechecker, workers, move(params->updates)));
}

void LSPTypecheckerCoordinator::typecheckOnSlowPath(LSPFileUpdates updates) {
    // TODO: Make async when we land preemptible slow path.
    syncRun(make_unique<SlowPathTypecheckTask>(*config, typechecker, workers, move(updates)));
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