#include "doctest/doctest.h"
// has to go first as it violates our requirements
#include "main/lsp/watchman/WatchmanShutdown.h"
#include <csignal>
#include <exception>
#include <limits>
#include <stdexcept>
#include <vector>

using namespace std;

namespace sorbet::realmain::lsp::watchman::test {
namespace {

// A drop-in stand-in for `subprocess::Popen` that records the calls
// `shutdownWatchmanChild` makes and lets each test script the `poll()` response.
struct FakePopen {
    int closeInputCalls = 0;
    int pollCalls = 0;
    std::vector<int> killSignals;

    // `poll()` returns -1 (still running) until the `pollCalls`'th call, at which point it
    // returns `exitCode`. INT_MAX means "never exits on its own".
    int pollsBeforeExit = numeric_limits<int>::max();
    int exitCode = 0;

    // If set, `close_input()` rethrows this. Used to verify the best-effort wrappers.
    exception_ptr throwOnCloseInput;
    bool throwOnPoll = false;
    bool throwOnKill = false;

    void close_input() {
        closeInputCalls++;
        if (throwOnCloseInput) {
            rethrow_exception(throwOnCloseInput);
        }
    }

    int poll() {
        pollCalls++;
        if (throwOnPoll) {
            throw runtime_error("poll failed");
        }
        if (pollCalls >= pollsBeforeExit) {
            return exitCode;
        }
        return -1;
    }

    void kill(int sig) {
        killSignals.push_back(sig);
        if (throwOnKill) {
            throw runtime_error("kill failed");
        }
    }
};

// Happy path: child exits on stdin EOF before we run out of poll attempts.
// We should close stdin, observe the exit via poll, and skip the kill escalation.
TEST_CASE("shutdownWatchmanChild reaps via stdin EOF") {
    FakePopen p;
    p.pollsBeforeExit = 3;

    shutdownWatchmanChild(p);

    CHECK(p.closeInputCalls == 1);
    CHECK(p.pollCalls == 3);
    CHECK(p.killSignals.empty());
}

// Fallback path: child exits on SIGTERM within the grace period. We should SIGTERM, observe the
// exit via poll, and skip the SIGKILL escalation entirely.
TEST_CASE("shutdownWatchmanChild SIGTERMs and returns when the child responds") {
    FakePopen p;
    // Survive the 10 EOF polls, exit on the 15th poll overall (5 polls into the SIGTERM grace).
    p.pollsBeforeExit = 15;

    shutdownWatchmanChild(p);

    CHECK(p.closeInputCalls == 1);
    CHECK(p.pollCalls == 15);
    CHECK(p.killSignals == vector<int>{SIGTERM});
}

// Wedged path: child ignores SIGTERM. After the SIGTERM grace period we must SIGKILL.
// We rely on non-blocking poll (not wait()) to reap so that a D-state child cannot hang us.
TEST_CASE("shutdownWatchmanChild SIGKILLs when SIGTERM is ignored and reaps via poll") {
    FakePopen p;
    // Exits at the very end of the post-SIGKILL poll grace period (10 EOF + 100 SIGTERM + 25 KILL).
    p.pollsBeforeExit = 135;

    shutdownWatchmanChild(p);

    CHECK(p.closeInputCalls == 1);
    CHECK(p.pollCalls == 135);
    CHECK(p.killSignals == vector<int>{SIGTERM, SIGKILL});
}

// D-state guarantee: even if the child never exits after SIGKILL, we must not block sorbet's
// shutdown. The post-SIGKILL poll loop is bounded; we return rather than calling a blocking wait.
TEST_CASE("shutdownWatchmanChild does not block when SIGKILL is undeliverable") {
    FakePopen p;
    p.pollsBeforeExit = numeric_limits<int>::max();

    shutdownWatchmanChild(p);

    CHECK(p.closeInputCalls == 1);
    CHECK(p.pollCalls == 135);
    CHECK(p.killSignals == vector<int>{SIGTERM, SIGKILL});
}

// Best-effort wrapping: an exception in close_input must not prevent the rest of the sequence
// from running, and must not propagate out (the caller is a noexcept destructor).
TEST_CASE("shutdownWatchmanChild swallows close_input exceptions and continues") {
    FakePopen p;
    p.throwOnCloseInput = make_exception_ptr(runtime_error("EPIPE"));
    p.pollsBeforeExit = numeric_limits<int>::max();

    CHECK_NOTHROW(shutdownWatchmanChild(p));

    CHECK(p.closeInputCalls == 1);
    CHECK(p.pollCalls == 135);
    CHECK(p.killSignals == vector<int>{SIGTERM, SIGKILL});
}

// If poll() throws, we treat that as "child is gone" and stop early — kill does not run.
// This mirrors the production behavior where a throwing poll usually means the child was already
// reaped out from under us (e.g. someone else called wait()).
TEST_CASE("shutdownWatchmanChild stops cleanly when poll throws") {
    FakePopen p;
    p.throwOnPoll = true;

    CHECK_NOTHROW(shutdownWatchmanChild(p));

    CHECK(p.closeInputCalls == 1);
    CHECK(p.pollCalls == 1);
    CHECK(p.killSignals.empty());
}

// Final guarantee: kill is best-effort. A throwing SIGTERM must not stop us from escalating
// to SIGKILL, and a throwing SIGKILL must not propagate out of the noexcept destructor.
TEST_CASE("shutdownWatchmanChild swallows kill exceptions and still escalates") {
    FakePopen p;
    p.pollsBeforeExit = numeric_limits<int>::max();
    p.throwOnKill = true;

    CHECK_NOTHROW(shutdownWatchmanChild(p));

    CHECK(p.killSignals == vector<int>{SIGTERM, SIGKILL});
}

} // namespace
} // namespace sorbet::realmain::lsp::watchman::test
