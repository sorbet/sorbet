#include "doctest/doctest.h"
// has to go first as it violates our requirements
#include "main/lsp/watchman/WatchmanRestartPolicy.h"
#include <chrono>

using namespace std;

namespace sorbet::realmain::lsp::watchman::test {
namespace {

constexpr chrono::milliseconds HEALTHY = WatchmanRestartPolicy::HEALTHY_SESSION;
constexpr chrono::milliseconds INSTANT{0};

// The case this whole class exists for: a subscription that ran for minutes and then lost its connection is evidence
// that watchman works, so it is worth resubscribing right now rather than after a delay.
TEST_CASE("delayBeforeRestart retries a long-lived session immediately") {
    WatchmanRestartPolicy policy;

    CHECK(policy.delayBeforeRestart(chrono::minutes{6}) == chrono::milliseconds{0});
    CHECK(policy.shortSessionCount() == 0);
}

// A session exactly at the threshold counts as healthy: the boundary belongs to the side that keeps Sorbet alive.
TEST_CASE("delayBeforeRestart treats a session at the threshold as healthy") {
    WatchmanRestartPolicy policy;

    CHECK(policy.delayBeforeRestart(HEALTHY) == chrono::milliseconds{0});
    CHECK(policy.delayBeforeRestart(HEALTHY - chrono::milliseconds{1}) == chrono::milliseconds{100});
}

// Sessions that die on arrival are the crash-loop case. Back off, but only far enough to tell "broken" from "unlucky",
// then stop so that Sorbet exits with an error the editor can show instead of respawning watchman forever.
TEST_CASE("delayBeforeRestart backs off over consecutive short sessions, then gives up") {
    WatchmanRestartPolicy policy;

    CHECK(policy.delayBeforeRestart(INSTANT) == chrono::milliseconds{100});
    CHECK(policy.delayBeforeRestart(INSTANT) == chrono::milliseconds{200});
    CHECK(policy.delayBeforeRestart(INSTANT) == chrono::milliseconds{400});
    CHECK(policy.delayBeforeRestart(INSTANT) == chrono::milliseconds{800});

    CHECK_FALSE(policy.delayBeforeRestart(INSTANT).has_value());
    CHECK(policy.shortSessionCount() == WatchmanRestartPolicy::MAX_CONSECUTIVE_SHORT_SESSIONS);
}

// Giving up is meant for a watchman that cannot hold a subscription at all, not for one that has had a bad minute over
// the course of a long editor session. One session that lasts spends nothing from the budget and refills it.
TEST_CASE("delayBeforeRestart forgives short sessions once one lasts") {
    WatchmanRestartPolicy policy;

    CHECK(policy.delayBeforeRestart(INSTANT) == chrono::milliseconds{100});
    CHECK(policy.delayBeforeRestart(INSTANT) == chrono::milliseconds{200});

    CHECK(policy.delayBeforeRestart(HEALTHY) == chrono::milliseconds{0});
    CHECK(policy.shortSessionCount() == 0);

    // Back to the start of the backoff, not to where the earlier run of failures left off.
    CHECK(policy.delayBeforeRestart(INSTANT) == chrono::milliseconds{100});
}

} // namespace
} // namespace sorbet::realmain::lsp::watchman::test
