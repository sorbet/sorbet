#include "doctest/doctest.h"
// has to go first as it violates our requirements
#include "main/lsp/watchman/WatchmanRestartPolicy.h"
#include <chrono>

using namespace std;

namespace sorbet::realmain::lsp::watchman::test {
namespace {

constexpr chrono::milliseconds HEALTHY = WatchmanRestartPolicy::HEALTHY_SESSION;
constexpr chrono::milliseconds INSTANT{0};
constexpr bool SUBSCRIBED = true;
constexpr bool NEVER_SUBSCRIBED = false;

// The case this whole class exists for: a subscription that ran for minutes and then lost its connection is evidence
// that watchman works, so it is worth resubscribing right now rather than after a delay.
TEST_CASE("delayBeforeRestart retries a long-lived subscribed session immediately") {
    WatchmanRestartPolicy policy;

    CHECK(policy.delayBeforeRestart(chrono::minutes{6}, SUBSCRIBED) == chrono::milliseconds{0});
    CHECK(policy.unhealthySessionCount() == 0);
}

// A session exactly at the threshold counts as healthy: the boundary belongs to the side that keeps Sorbet alive.
TEST_CASE("delayBeforeRestart treats a session at the threshold as healthy") {
    WatchmanRestartPolicy policy;

    CHECK(policy.delayBeforeRestart(HEALTHY, SUBSCRIBED) == chrono::milliseconds{0});
    CHECK(policy.delayBeforeRestart(HEALTHY - chrono::milliseconds{1}, SUBSCRIBED) == chrono::milliseconds{100});
}

// Length alone is not health. A session that sat there without watchman ever accepting the subscription was not
// watching anything, however long it lasted, so it must not refill the budget.
TEST_CASE("delayBeforeRestart does not trust a long session that was never subscribed") {
    WatchmanRestartPolicy policy;

    CHECK(policy.delayBeforeRestart(chrono::minutes{6}, NEVER_SUBSCRIBED) == chrono::milliseconds{100});
    CHECK(policy.unhealthySessionCount() == 1);
}

// And acknowledgement alone is not health either. A subscription watchman accepts and then drops immediately, over and
// over, has to back off rather than spin.
TEST_CASE("delayBeforeRestart does not trust a subscribed session that died immediately") {
    WatchmanRestartPolicy policy;

    CHECK(policy.delayBeforeRestart(INSTANT, SUBSCRIBED) == chrono::milliseconds{100});
    CHECK(policy.delayBeforeRestart(INSTANT, SUBSCRIBED) == chrono::milliseconds{200});
}

// Sessions that die on arrival are the crash-loop case. Back off, but only far enough to tell "broken" from "unlucky",
// then stop so that Sorbet exits with an error the editor can show instead of respawning watchman forever.
TEST_CASE("delayBeforeRestart backs off over consecutive unhealthy sessions, then gives up") {
    WatchmanRestartPolicy policy;

    CHECK(policy.delayBeforeRestart(INSTANT, NEVER_SUBSCRIBED) == chrono::milliseconds{100});
    CHECK(policy.delayBeforeRestart(INSTANT, NEVER_SUBSCRIBED) == chrono::milliseconds{200});
    CHECK(policy.delayBeforeRestart(INSTANT, NEVER_SUBSCRIBED) == chrono::milliseconds{400});
    CHECK(policy.delayBeforeRestart(INSTANT, NEVER_SUBSCRIBED) == chrono::milliseconds{800});

    CHECK_FALSE(policy.delayBeforeRestart(INSTANT, NEVER_SUBSCRIBED).has_value());
    CHECK(policy.unhealthySessionCount() == WatchmanRestartPolicy::MAX_CONSECUTIVE_UNHEALTHY_SESSIONS);
}

// Giving up is meant for a watchman that cannot hold a subscription at all, not for one that has had a bad minute over
// the course of a long editor session. One session that lasts spends nothing from the budget and refills it.
TEST_CASE("delayBeforeRestart forgives unhealthy sessions once one is healthy") {
    WatchmanRestartPolicy policy;

    CHECK(policy.delayBeforeRestart(INSTANT, NEVER_SUBSCRIBED) == chrono::milliseconds{100});
    CHECK(policy.delayBeforeRestart(INSTANT, NEVER_SUBSCRIBED) == chrono::milliseconds{200});

    CHECK(policy.delayBeforeRestart(HEALTHY, SUBSCRIBED) == chrono::milliseconds{0});
    CHECK(policy.unhealthySessionCount() == 0);

    // Back to the start of the backoff, not to where the earlier run of failures left off.
    CHECK(policy.delayBeforeRestart(INSTANT, NEVER_SUBSCRIBED) == chrono::milliseconds{100});
}

} // namespace
} // namespace sorbet::realmain::lsp::watchman::test
