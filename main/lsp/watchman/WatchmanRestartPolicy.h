#ifndef RUBY_TYPER_LSP_WATCHMAN_WATCHMANRESTARTPOLICY_H
#define RUBY_TYPER_LSP_WATCHMAN_WATCHMANRESTARTPOLICY_H

#include <chrono>
#include <optional>

namespace sorbet::realmain::lsp::watchman {

// Decides whether losing the watchman CLI is worth another spawn, and how long to wait first.
//
// Losing the connection is not the same as watchman being unusable, so a session that was demonstrably working earns an
// immediate resubscribe. One that was not spends from a small budget of increasingly-delayed retries, after which
// Sorbet exits with an error rather than respawning watchman forever. Both signals are needed to call a session
// working: without the acknowledgement a session that never watched anything refills the budget by lasting, and without
// the duration a subscription watchman accepts and drops immediately retries with no delay.
class WatchmanRestartPolicy {
public:
    static constexpr std::chrono::milliseconds HEALTHY_SESSION = std::chrono::seconds{10};

    // Four retries, so at most 100 + 200 + 400 + 800 = 1.5s is spent telling a broken watchman from an unlucky one.
    static constexpr int MAX_CONSECUTIVE_UNHEALTHY_SESSIONS = 5;
    static constexpr std::chrono::milliseconds FIRST_RETRY_DELAY{100};

    // Returns how long to wait before spawning watchman again, or `nullopt` to stop trying.
    std::optional<std::chrono::milliseconds> delayBeforeRestart(std::chrono::milliseconds sessionDuration,
                                                                bool subscribed) {
        if (subscribed && sessionDuration >= HEALTHY_SESSION) {
            consecutiveUnhealthySessions = 0;
            return std::chrono::milliseconds{0};
        }

        consecutiveUnhealthySessions++;
        if (consecutiveUnhealthySessions >= MAX_CONSECUTIVE_UNHEALTHY_SESSIONS) {
            return std::nullopt;
        }

        return FIRST_RETRY_DELAY * (1 << (consecutiveUnhealthySessions - 1));
    }

    int unhealthySessionCount() const {
        return consecutiveUnhealthySessions;
    }

private:
    int consecutiveUnhealthySessions = 0;
};

} // namespace sorbet::realmain::lsp::watchman

#endif // RUBY_TYPER_LSP_WATCHMAN_WATCHMANRESTARTPOLICY_H
