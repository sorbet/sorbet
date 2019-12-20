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
    : shouldTerminate(false), typechecker(config), config(config), hasDedicatedThread(false), workers(workers) {}

void LSPTypecheckerCoordinator::asyncRunInternal(shared_ptr<core::lsp::Task> task) {
    if (hasDedicatedThread) {
        tasks.push(move(task), 1);
    } else {
        task->run();
    }
}

void LSPTypecheckerCoordinator::syncRun(function<void(LSPTypechecker &)> &&lambda) {
    absl::Notification notification;
    CounterState typecheckerCounters;
    // If typechecker is running on a dedicated thread, then we need to merge its metrics w/ coordinator thread's so we
    // report them.
    // Note: Capturing notification by reference is safe here, we we wait for the notification to happen prior to
    // returning.
    asyncRunInternal(
        make_shared<LambdaTask>([&typechecker = this->typechecker, lambda, &notification, &typecheckerCounters,
                                 hasDedicatedThread = this->hasDedicatedThread]() -> void {
            lambda(typechecker);
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

void LSPTypecheckerCoordinator::syncRunMultithreaded(std::function<void(LSPTypechecker &, WorkerPool &)> &&lambda) {
    absl::Notification notification;
    CounterState typecheckerCounters;
    // If typechecker is running on a dedicated thread, then we need to merge its metrics w/ coordinator thread's so we
    // report them.
    // Note: Capturing notification by reference is safe here, we we wait for the notification to happen prior to
    // returning.
    asyncRunInternal(
        make_shared<LambdaTask>([&typechecker = this->typechecker, lambda, &notification, &typecheckerCounters,
                                 hasDedicatedThread = this->hasDedicatedThread, &workers = this->workers]() -> void {
            lambda(typechecker, workers);
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

unique_ptr<core::GlobalState> LSPTypecheckerCoordinator::shutdown() {
    unique_ptr<core::GlobalState> gs;
    syncRun([&](auto &typechecker) -> void {
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