#ifndef RUBY_TYPER_LSP_WATCHMAN_WATCHMANPROCESS_H
#define RUBY_TYPER_LSP_WATCHMAN_WATCHMANPROCESS_H

#include "absl/synchronization/mutex.h"
#include "absl/synchronization/notification.h"
#include "common/common.h"
#include "core/core.h"
#include "main/lsp/MessageQueueState.h"
#include "spdlog/spdlog.h"
#include <chrono>

namespace sorbet::realmain::lsp {
class WatchmanQueryResponse;
class WatchmanStateEnter;
class WatchmanStateLeave;
class LSPConfiguration;
class NotificationMessage;
} // namespace sorbet::realmain::lsp

namespace sorbet::realmain::lsp::watchman {
class WatchmanProcess {
protected:
    std::shared_ptr<spdlog::logger> logger;

private:
    const std::string watchmanPath;
    const std::string workSpace;
    const std::vector<std::string> extensions;
    const std::unique_ptr<Joinable> thread;
    // Mutex that must be held before reading or writing stopped.
    absl::Mutex mutex;
    // If true, the process has been stopped.
    bool stopped = false;

    MessageQueueState &messageQueue;
    absl::Mutex &messageQueueMutex;
    absl::Notification &initializedNotification;
    const std::shared_ptr<const LSPConfiguration> config;
    std::string watchmanNamespace;

    // The most recent clock watchman has told us this subscription is caught up to. Set only from responses that
    // establish or advance that view — the `subscribe` ack and the file change responses — and used as the `since` of
    // the next subscription. Touched only by the watchman thread, so it needs no lock.
    std::optional<std::string> lastClock;

    // How one watchman CLI session — one child process, one subscription — came to an end.
    enum class SessionOutcome {
        // Sorbet is shutting down, or has decided to stop watching. Do not spawn watchman again.
        Stopped,
        // Lost the connection to the CLI child. Recoverable by subscribing again.
        Disconnected,
    };

    /**
     * Starts up a Watchman subprocess and begins processing file changes, replacing it if the connection to it is lost.
     * Runs in a dedicated thread.
     */
    void start();

    /**
     * Resolves the path to hand watchman as the subscription root, and decides whether namespacing applies to this
     * workspace. Clears `watchmanNamespace` when it does not.
     */
    std::string resolveWatchmanRoot();

    /**
     * Spawns one watchman CLI child, subscribes, and pumps its responses into the message queue until either Sorbet
     * stops or the child does. Reaps the child before returning.
     */
    SessionOutcome runSubscription(std::string_view root, std::string_view subscriptionName);

    /**
     * Asks watchman, over a connection of its own, for the changes that landed since `sinceClock` — the window a
     * replacement subscription does not cover. Best effort: logs and returns if watchman cannot answer.
     */
    void catchUpSince(std::string_view root, std::string_view sinceClock);

    /**
     * Remembers the clock a watchman response was stamped with, if it carries one, as the point a replacement
     * subscription resumes from. Only responses that mean "you have been told everything up to here" — the `subscribe`
     * ack and the file change responses — carry a clock that is safe to resume from; a state-enter/state-leave clock
     * does not, since watchman can still owe us file changes from before it.
     */
    void rememberClock(std::optional<std::string> clock);

    /**
     * Sleeps for `duration`, in short slices, returning early once stopped so that shutdown does not have to wait out a
     * restart delay.
     */
    void sleepUnlessStopped(std::chrono::milliseconds duration);

    void exitWithCode(int code, const std::optional<std::string> &);

    bool isStopped();

    void enqueueNotification(std::unique_ptr<NotificationMessage> notification);

    void processQueryResponse(std::unique_ptr<sorbet::realmain::lsp::WatchmanQueryResponse>);

    void processStateEnter(std::unique_ptr<sorbet::realmain::lsp::WatchmanStateEnter>);

    void processStateLeave(std::unique_ptr<sorbet::realmain::lsp::WatchmanStateLeave>);

    void processExit(int core, const std::optional<std::string> &);

public:
    /**
     * Immediately starts a Watchman subprocess and begins processing file updates in the provided
     * workspace folder. Passes file updates to `processUpdate` function.
     */
    WatchmanProcess(std::shared_ptr<spdlog::logger> logger, std::string_view watchmanPath, std::string_view workSpace,
                    std::vector<std::string> extensions, MessageQueueState &messageQueue,
                    absl::Mutex &messageQueueMutex, absl::Notification &initializedNotification,
                    std::shared_ptr<const LSPConfiguration> config, std::string_view watchmanNamespace);

    ~WatchmanProcess();

    WatchmanProcess(const WatchmanProcess &&) = delete;
    WatchmanProcess(WatchmanProcess &) = delete;
    void operator=(const WatchmanProcess &) = delete;
};
} // namespace sorbet::realmain::lsp::watchman

#endif // RUBY_TYPER_LSP_WATCHMAN_WATCHMANPROCESS_H
