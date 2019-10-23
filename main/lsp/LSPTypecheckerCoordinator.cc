#include "main/lsp/LSPTypecheckerCoordinator.h"

#include "absl/synchronization/notification.h"

namespace sorbet::realmain::lsp {
using namespace std;

LSPTypecheckerCoordinator::LSPTypecheckerCoordinator(const shared_ptr<const LSPConfiguration> &config)
    : shouldTerminate(false), typechecker(config), config(config), hasDedicatedThread(false) {}

void LSPTypecheckerCoordinator::asyncRunInternal(function<void()> &&lambda) {
    if (hasDedicatedThread) {
        lambdas.push(move(lambda), 1);
    } else {
        lambda();
    }
}

void LSPTypecheckerCoordinator::asyncRun(function<void(LSPTypechecker &)> &&lambda) {
    asyncRunInternal([&typechecker = this->typechecker, lambda]() -> void { lambda(typechecker); });
}

void LSPTypecheckerCoordinator::syncRun(function<void(LSPTypechecker &)> &&lambda) {
    absl::Notification notification;
    CounterState typecheckerCounters;
    // If typechecker is running on a dedicated thread, then we need to merge its metrics w/ coordinator thread's so we
    // report them.
    // Note: Capturing notification by reference is safe here, we we wait for the notification to happen prior to
    // returning.
    asyncRunInternal([&typechecker = this->typechecker, lambda, &notification, &typecheckerCounters,
                      hasDedicatedThread = this->hasDedicatedThread]() -> void {
        lambda(typechecker);
        if (hasDedicatedThread) {
            typecheckerCounters = getAndClearThreadCounters();
        }
        notification.Notify();
    });
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
            function<void()> lambda;
            auto result = lambdas.wait_pop_timed(lambda, WorkerPool::BLOCK_INTERVAL(), *config->logger);
            if (result.gotItem()) {
                lambda();
            }
        }
    });
}

}; // namespace sorbet::realmain::lsp