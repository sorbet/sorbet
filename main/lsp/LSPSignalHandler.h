#ifndef RUBY_TYPER_LSP_SIGNAL_HANDLER_H
#define RUBY_TYPER_LSP_SIGNAL_HANDLER_H

#include <csignal>
#include <sys/types.h>

namespace sorbet::realmain::lsp {

// Register the PID of the active Watchman client subprocess so that it is
// killed immediately when SIGTERM is received. Pass 0 to deregister.
// Safe to call from any thread; uses only a lock-free atomic store.
void registerWatchmanChildPid(pid_t pid);

// Returns true if a graceful shutdown has been requested by the SIGTERM handler.
// Checked by LSPFDInput::read() so the LSP stdin reader exits its polling loop.
bool isLSPShutdownRequested();

// Install a SIGTERM handler for LSP mode. The handler:
//   1. Sends SIGTERM to the registered Watchman child pid immediately, so the
//      child does not outlive Sorbet even if a subsequent SIGKILL prevents
//      the RAII guard from running.
//   2. Sets a flag polled by LSPFDInput::read(), which unblocks the reader
//      thread and starts the clean shutdown cascade.
//
// Does not set SA_RESTART so that blocking system calls (e.g. select) in the
// reader thread return EINTR and unblock right away when the signal is
// delivered to that thread.
//
// Saves the previous handler in *prev if non-null. Returns true on success.
bool installLSPSigtermHandler(struct sigaction *prev = nullptr);

} // namespace sorbet::realmain::lsp

#endif // RUBY_TYPER_LSP_SIGNAL_HANDLER_H
