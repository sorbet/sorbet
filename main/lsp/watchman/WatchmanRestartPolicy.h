#ifndef RUBY_TYPER_LSP_WATCHMAN_WATCHMANRESTARTPOLICY_H
#define RUBY_TYPER_LSP_WATCHMAN_WATCHMANRESTARTPOLICY_H

#include <chrono>
#include <optional>

namespace sorbet::realmain::lsp::watchman {

// Decides whether losing the watchman CLI is worth another spawn, and how long to wait first.
//
// Losing the connection is not the same as watchman being unusable. The CLI exits when the daemon closes its socket
// mid-response, and the daemon does that for reasons that say nothing about the health of the watch: it sends
// subscription payloads with the client socket in blocking mode and gives up on a write that makes no progress for 60
// seconds (watchman's `kWriteTimeout`), which is what happens to a client whose reader has stopped draining — Sorbet's
// own watchman thread parks in `enqueueNotification` until the initial typecheck finishes. The daemon logs nothing on
// that path, so from Sorbet's side an entirely healthy watchman is indistinguishable from a broken one except by how
// long the subscription lasted before it died.
//
// So: a session that was demonstrably doing its job earns an immediate resubscribe and resets the budget. A session
// that was not spends from a small budget of increasingly-delayed retries, and when that runs out we stop and let
// Sorbet exit with an error the editor shows. A watchman that cannot hold a working subscription for a few seconds is
// not something a retry loop should paper over.
//
// "Doing its job" takes both of the things that can go wrong to answer, because neither alone bounds the loop:
//
//   - Long enough. A subscription that watchman acknowledged and then dropped in milliseconds, over and over, would
//     spin without delay if the acknowledgement were the whole test.
//   - Acknowledged. A session that sat there for a minute without watchman ever accepting the subscription was never
//     watching anything, so its length is not evidence of health.
class WatchmanRestartPolicy {
public:
    // A session at least this long *and* carrying a subscription acknowledgement is evidence that spawning watchman
    // and subscribing both work.
    static constexpr std::chrono::milliseconds HEALTHY_SESSION = std::chrono::seconds{10};

    // How many unhealthy sessions in a row we tolerate before giving up. Four retries, so at most
    // 100 + 200 + 400 + 800 = 1.5s of delay is spent proving that watchman is broken rather than unlucky.
    static constexpr int MAX_CONSECUTIVE_UNHEALTHY_SESSIONS = 5;

    // Delay before retrying the nth consecutive unhealthy session: FIRST_RETRY_DELAY * 2^(n-1).
    static constexpr std::chrono::milliseconds FIRST_RETRY_DELAY{100};

    // Call once per ended session, with how long it lasted and whether watchman ever acknowledged its subscription.
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
