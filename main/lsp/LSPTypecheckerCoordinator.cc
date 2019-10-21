#include "main/lsp/LSPTypecheckerCoordinator.h"

#include "absl/synchronization/notification.h"

namespace sorbet::realmain::lsp {
using namespace std;

LSPTypecheckerCoordinator::LSPTypecheckerCoordinator(const shared_ptr<const LSPConfiguration> &config)
    : typecheckerThreadId(this_thread::get_id()), shouldTerminate(false), typechecker(config), config(config) {}

void LSPTypecheckerCoordinator::asyncRunInternal(function<void()> &&lambda) {
    if (this_thread::get_id() == typecheckerThreadId) {
        // Single-threaded mode.
        lambda();
    } else {
        lambdas.push(move(lambda), 1);
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
    const bool isMultithreaded = this_thread::get_id() == typecheckerThreadId;
    // Note: Capturing notification by reference is safe here, we we wait for the notification to happen prior to
    // returning.
    asyncRunInternal(
        [&typechecker = this->typechecker, lambda, &notification, &typecheckerCounters, isMultithreaded]() -> void {
            lambda(typechecker);
            if (isMultithreaded) {
                typecheckerCounters = getAndClearThreadCounters();
            }
            notification.Notify();
        });
    if (isMultithreaded) {
        counterConsume(move(typecheckerCounters));
    }
    notification.WaitForNotification();
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
    if (typecheckerThreadId != this_thread::get_id()) {
        Exception::raise("Typechecker already started on a different thread.");
    }

    return runInAThread("Typechecker", [&]() -> void {
        typecheckerThreadId = this_thread::get_id();
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