#include "main/lsp/LSPTypecheckerCoordinator.h"

#include "absl/synchronization/notification.h"

namespace sorbet::realmain::lsp {
using namespace std;

namespace {
// TODO(jvilk): Switch LSPTypecheckerCoordinator interface to use a type of task rather than lambdas.
class LambdaTask : public core::lsp::Task {
private:
    const function<void()> lambda;

public:
    LambdaTask(function<void()> &&lambda) : lambda(move(lambda)) {}
    void run() override {
        lambda();
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

void LSPTypecheckerCoordinator::syncRun(function<void(LSPTypecheckerDelegate &)> &&lambda, bool multithreaded) {
    absl::Notification notification;
    CounterState typecheckerCounters;
    // If typechecker is running on a dedicated thread, then we need to merge its metrics w/ coordinator thread's so we
    // report them.
    // Note: Capturing notification by reference is safe here, we we wait for the notification to happen prior to
    // returning.
    asyncRunInternal(make_shared<LambdaTask>(
        [&typechecker = this->typechecker, &workers = multithreaded ? workers : *emptyWorkers, lambda, &notification,
         &typecheckerCounters, hasDedicatedThread = this->hasDedicatedThread]() -> void {
            LSPTypecheckerDelegate d(workers, typechecker);
            lambda(d);
            if (hasDedicatedThread) {
                typecheckerCounters = getAndClearThreadCounters();
            }
            notification.Notify();
        }));
    notification.WaitForNotification();
    if (hasDedicatedThread) {
        counterConsume(move(typecheckerCounters));
    }
}

void LSPTypecheckerCoordinator::initialize(unique_ptr<InitializedParams> params) {
    // TODO: Make async when we land preemptible slow path.
    syncRun([&typechecker = this->typechecker, &workers = workers, &params = params](auto &tcd) -> void {
        auto &updates = params->updates;
        typechecker.initialize(move(updates), workers);
    });
}

void LSPTypecheckerCoordinator::typecheckOnSlowPath(LSPFileUpdates updates) {
    // TODO: Make async when we land preemptible slow path.
    syncRun([&typechecker = this->typechecker, &workers = workers, &updates = updates](auto &tcd) -> void {
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
    });
}

unique_ptr<core::GlobalState> LSPTypecheckerCoordinator::shutdown() {
    unique_ptr<core::GlobalState> gs;
    syncRun([&](auto &tcd) -> void {
        shouldTerminate = true;
        gs = typechecker.destroy();
    });
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