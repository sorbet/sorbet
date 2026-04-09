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
    // Ask the LSP stdin reader to exit its polling loop. On the next poll
    // cycle (within the 100 ms select timeout, or immediately if the signal
    // was delivered to the reader thread and interrupted its select call),
    // LSPFDInput::read() will return ErrorOrEof, which propagates through
    // the reader → message queue → preprocessor → task queue chain and
    // brings the main LSP loop down cleanly.
    g_shutdownRequested.store(true, memory_order_relaxed);
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
