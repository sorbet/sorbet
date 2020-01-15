#include "main/lsp/LSPTypecheckerCoordinator.h"
#include "main/lsp/LSPTask.h"

#include "absl/synchronization/notification.h"

namespace sorbet::realmain::lsp {
using namespace std;

namespace {
class TypecheckerTask final : public core::lsp::Task {
    unique_ptr<LSPTask> task;
    unique_ptr<LSPTypecheckerDelegate> delegate;
    absl::Notification complete;

public:
    TypecheckerTask(unique_ptr<LSPTask> task, unique_ptr<LSPTypecheckerDelegate> delegate)
        : task(move(task)), delegate(move(delegate)) {}

    void run() override {
        task->run(*delegate);
        complete.Notify();
    }

    void blockUntilComplete() {
        complete.WaitForNotification();
    }
};

// Special internal tasks

class InitializeTask : public LSPTask {
    LSPTypechecker &typechecker;
    WorkerPool &workers;
    LSPFileUpdates updates;

public:
    InitializeTask(const LSPConfiguration &config, LSPTypechecker &typechecker, WorkerPool &workers,
                   LSPFileUpdates updates)
        : LSPTask(config), typechecker(typechecker), workers(workers), updates(move(updates)){};

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
        : LSPTask(config), typechecker(typechecker), workers(workers), updates(move(updates)){};

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
        : LSPTask(config), typechecker(typechecker), shouldTerminate(shouldTerminate), gs(gs) {}

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

void LSPTypecheckerCoordinator::syncRun(unique_ptr<LSPTask> task, bool multithreaded) {
    absl::Notification notification;
    CounterState typecheckerCounters;
    auto wrappedTask =
        make_shared<TypecheckerTask>(move(task), make_unique<LSPTypecheckerDelegate>(workers, typechecker));
    asyncRunInternal(wrappedTask);
    wrappedTask->blockUntilComplete();
    if (hasDedicatedThread) {
        counterConsume(move(typecheckerCounters));
    }
}

void LSPTypecheckerCoordinator::initialize(unique_ptr<InitializedParams> params) {
    // TODO: Make async when we land preemptible slow path.
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