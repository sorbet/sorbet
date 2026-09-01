#ifndef RUBY_TYPER_LSP_WATCHMAN_WATCHMANPROCESS_H
#define RUBY_TYPER_LSP_WATCHMAN_WATCHMANPROCESS_H

#include "absl/synchronization/mutex.h"
#include "absl/synchronization/notification.h"
#include "common/common.h"
#include "core/core.h"
#include "main/lsp/MessageQueueState.h"
#include "spdlog/spdlog.h"

#include <chrono>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace sorbet::realmain::lsp {
class WatchmanQueryResponse;
class WatchmanStateEnter;
class WatchmanStateLeave;
class LSPConfiguration;
class LSPMessage;
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

    // The fields below are only touched by the watchman reader thread.

    // Notifications that arrived before the LSP loop finished initializing. The preprocessor rejects messages
    // that arrive before then, and blocking the reader instead would stall the watchman CLI's stdout, so they
    // are held here and released once `initializedNotification` fires.
    std::vector<std::unique_ptr<LSPMessage>> heldUntilInitialized;
    // The clock of the most recent watchman response. After the CLI dies, the changes since this clock are what
    // the replacement subscription would otherwise have missed.
    std::optional<std::string> lastClock;

    // Why one `watchman -j -p` child stopped being useful.
    enum class SubscriptionEnd {
        // `exitWithCode` ran (Sorbet is shutting down).
        Stopped,
        // The child exited or its stdout hit EOF, watchman answered a command with an error, or the changes made
        // while the previous child was down could not be fetched.
        ChildExited,
        // The clock of the previous child belonged to a different watchman instance, so the changes made while
        // the child was down cannot be recovered.
        DeltaLost,
    };

    struct SubscriptionRun {
        SubscriptionEnd end;
        // Whether watchman acknowledged the subscription during this run.
        bool subscribed;
    };

    enum class DeltaFetch { Delivered, Lost, Failed };

    /**
     * Starts up a Watchman subprocess and begins processing file changes. Runs in a dedicated thread.
     */
    void start();

    // Resolves the directory watchman should watch, honoring `watchmanNamespace` (cleared when it does not apply).
    std::string resolveWatchRoot();

    // Runs one watchman CLI child to completion: spawns it, subscribes and forwards its responses until it exits
    // or Sorbet stops. When a previous child had delivered a clock, the changes since that clock are fetched and
    // forwarded right after watchman acknowledges the new subscription.
    SubscriptionRun runSubscription(const std::string &watchRoot, const std::string &subscriptionName);

    // Asks watchman (a one-shot `watchman -j` child) for the files that changed after `since` and forwards them.
    DeltaFetch fetchChangesSince(const std::string &watchRoot, const std::string &since);

    // Removes the namespace directory from the paths watchman reports, when namespacing is active.
    void stripNamespace(WatchmanQueryResponse &response) const;

    // Sleeps for `delay` in small steps, returning early once stopped.
    void sleepUnlessStopped(std::chrono::milliseconds delay);

    void exitWithCode(int code, const std::optional<std::string> &);

    bool isStopped();

    void enqueueNotification(std::unique_ptr<NotificationMessage> notification);

    // Moves every held notification onto the message queue once the LSP loop has initialized.
    void releaseHeldNotifications();

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
