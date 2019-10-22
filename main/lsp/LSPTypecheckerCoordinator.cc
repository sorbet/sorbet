#include "main/lsp/LSPTypecheckerCoordinator.h"

#include "absl/synchronization/notification.h"

namespace sorbet::realmain::lsp {
using namespace std;

LSPTypecheckerCoordinator::LSPTypecheckerCoordinator(const shared_ptr<const LSPConfiguration> &config)
    : shouldTerminate(false), typechecker(config), config(config), hasDedicatedThread(false) {}

void LSPTypecheckerCoordinator::asyncRunInternal(function<void()> &&lambda, bool canPreemptSlowPath) {
    if (hasDedicatedThread) {
        // We can only preempt if this lambda will run next. We have no notion of a queue of preemption lambdas.
        if (canPreemptSlowPath && lambdas.enqueuedEstimate() - lambdas.doneEstimate() == 0) {
            if (typechecker.tryPreemptSlowPath(lambda)) {
                // lambda is guaranteed to preempt slow path.
                config->logger->debug("Preempting slow path.");
                return;
            } else {
                config->logger->debug("Preemption attempt failed: tryPreemptSlowPath returned false.");
            }
        }

        // Debug
        if (canPreemptSlowPath && lambdas.enqueuedEstimate() - lambdas.doneEstimate() > 0) {
            config->logger->debug("Preemption attempt failed: Blocking request exists in queue.");
        } else {
            config->logger->debug("Request cannot preempt.");
        }

        lambdas.push(move(lambda), 1);
    } else {
        lambda();
    }
}

void LSPTypecheckerCoordinator::asyncRun(function<void(LSPTypechecker &)> &&lambda, bool waitUntilTaskStarts) {
    absl::Notification notification;
    asyncRunInternal(
        [&typechecker = this->typechecker, lambda, waitUntilTaskStarts, &notification]() -> void {
            if (waitUntilTaskStarts) {
                notification.Notify();
            }
            lambda(typechecker);
        },
        false);

    if (waitUntilTaskStarts) {
        notification.WaitForNotification();
    }
}

void LSPTypecheckerCoordinator::syncRun(function<void(LSPTypechecker &)> &&lambda, bool canPreemptSlowPath) {
    absl::Notification notification;
    CounterState typecheckerCounters;
    // If typechecker is running on a dedicated thread, then we need to merge its metrics w/ coordinator thread's so we
    // report them.
    // Note: Capturing notification by reference is safe here, we we wait for the notification to happen prior to
    // returning.
    asyncRunInternal(
        [&typechecker = this->typechecker, lambda, &notification, &typecheckerCounters,
         hasDedicatedThread = this->hasDedicatedThread]() -> void {
            lambda(typechecker);
            if (hasDedicatedThread) {
                typecheckerCounters = getAndClearThreadCounters();
            }
            notification.Notify();
        },
        canPreemptSlowPath);
    notification.WaitForNotification();
    if (hasDedicatedThread) {
        counterConsume(move(typecheckerCounters));
    }
}

unique_ptr<core::GlobalState> LSPTypecheckerCoordinator::shutdown() {
    unique_ptr<core::GlobalState> gs;
    syncRun(
        [&](auto &typechecker) -> void {
            shouldTerminate = true;
            gs = typechecker.destroy();
        },
        false);
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
            // Note: Pass in 'true' for silent to avoid spamming log with wait_pop_timed entries.
            auto result = lambdas.wait_pop_timed(lambda, WorkerPool::BLOCK_INTERVAL(), *config->logger, true);
            if (result.gotItem()) {
                lambda();
            }
        }
    });
}

}; // namespace sorbet::realmain::lsp