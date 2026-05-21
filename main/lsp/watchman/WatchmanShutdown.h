#ifndef RUBY_TYPER_LSP_WATCHMAN_WATCHMANSHUTDOWN_H
#define RUBY_TYPER_LSP_WATCHMAN_WATCHMANSHUTDOWN_H

#include <chrono>
#include <csignal>
#include <thread>

namespace sorbet::realmain::lsp::watchman {

// Attempt to shut down the watchman CLI cleanly. The CLI's `-p` (persistent) mode keeps a unix
// socket connection to the watchman daemon open until its stdin closes AND its parent has gone
// away. If we just let Popen go out of scope we close the FDs, but cpp-subprocess never reaps —
// the daemon-connection socket can outlive sorbet, leaving the daemon to grow thread/port
// counts unboundedly across editor restarts. So: close stdin, give it a beat to exit on EOF,
// then SIGTERM if it's still around, give it another beat to handle the signal, then SIGKILL
// as a last resort so a wedged watchman cannot block sorbet from exiting. Reap unconditionally.
//
// Templated on the subprocess type so tests can inject a FakePopen without spawning a real child
// process. Each step is best-effort wrapped so a failure in one stage cannot prevent the rest of
// the shutdown sequence from running.
template <typename Subprocess> void shutdownWatchmanChild(Subprocess &p) {
    // Returns true if the child exited (or poll failed, which we treat as "gone").
    auto pollUntilExitedFor = [&p](int attempts) -> bool {
        for (int i = 0; i < attempts; i++) {
            try {
                if (p.poll() != -1) {
                    return true;
                }
            } catch (...) {
                return true;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
        return false;
    };

    try {
        p.close_input();
    } catch (...) {
        // best-effort
    }
    // 10 * 20ms = 200ms grace period for the EOF-driven shutdown.
    if (pollUntilExitedFor(10)) {
        return;
    }
    try {
        p.kill(SIGTERM);
    } catch (...) {
        // best-effort
    }
    // 100 * 20ms = 2s grace period for SIGTERM to be handled before we escalate.
    if (pollUntilExitedFor(100)) {
        return;
    }
    try {
        p.kill(SIGKILL);
    } catch (...) {
        // best-effort
    }
    // After SIGKILL the kernel terminates the child essentially immediately, except in the edge
    // case of uninterruptible kernel sleep (D-state, typically an unresponsive filesystem) — there
    // the signal is queued but cannot be delivered until the syscall returns, and a blocking
    // wait() would hang. So we poll (non-blocking waitpid(WNOHANG) under the hood) instead of
    // wait(). A successful poll reaps the child. If the bounded grace period elapses without a
    // reap, give up rather than block sorbet's shutdown — any leaked zombie is cleaned up by the
    // kernel when sorbet exits, which is the very next thing that happens.
    pollUntilExitedFor(25);
}

} // namespace sorbet::realmain::lsp::watchman

#endif // RUBY_TYPER_LSP_WATCHMAN_WATCHMANSHUTDOWN_H
