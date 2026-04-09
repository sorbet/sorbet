#include "main/lsp/LSPSignalHandler.h"
#include <atomic>
#include <csignal>
#include <unistd.h>

using namespace std;

namespace sorbet::realmain::lsp {
namespace {

atomic<pid_t> g_watchmanChildPid{0};
atomic<bool> g_shutdownRequested{false};

void sigtermHandler(int /*sig*/) {
    // Kill the Watchman client right away. This ensures it does not outlive
    // Sorbet even if a subsequent SIGKILL arrives before the clean-shutdown
    // cascade has a chance to run the WatchmanProcess RAII guard.
    pid_t pid = g_watchmanChildPid.load(memory_order_relaxed);
    if (pid > 0) {
        ::kill(pid, SIGTERM);
    }
    // Ask the LSP stdin reader to exit its polling loop. This starts the clean
    // shutdown cascade in case we have time to run it (e.g. graceful SIGTERM
    // that isn't followed by SIGKILL).
    g_shutdownRequested.store(true, memory_order_relaxed);
    // Restore the default SIGTERM disposition and re-raise so the process
    // actually terminates promptly. Without this, Sorbet would stay alive until
    // the current type-checking task finishes — potentially many minutes on a
    // large codebase — since the main loop only checks for shutdown between
    // tasks. The Watchman client has already been signaled above; any zombie
    // will be reaped by init. Both signal() and raise() are async-signal-safe.
    ::signal(SIGTERM, SIG_DFL);
    ::raise(SIGTERM);
}

} // namespace

void registerWatchmanChildPid(pid_t pid) {
    g_watchmanChildPid.store(pid, memory_order_relaxed);
}

bool isLSPShutdownRequested() {
    return g_shutdownRequested.load(memory_order_relaxed);
}

bool installLSPSigtermHandler(struct sigaction *prev) {
    struct sigaction sa {};
    sa.sa_handler = sigtermHandler;
    sigemptyset(&sa.sa_mask);
    return sigaction(SIGTERM, &sa, prev) == 0;
}

} // namespace sorbet::realmain::lsp
