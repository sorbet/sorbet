#ifndef RUBY_TYPER_LSP_LSPTYPECHECKERCOORDINATOR_H
#define RUBY_TYPER_LSP_LSPTYPECHECKERCOORDINATOR_H

#include "main/lsp/LSPTypechecker.h"

namespace sorbet::realmain::lsp {

/**
 * Handles typechecking and other queries. Can either operate in single-threaded mode (in which lambdas passed to
 * syncRun/asyncRun run-to-completion immediately) or dedicated-thread mode (in which lambdas are enqueued to execute on
 * thread).
 */
class LSPTypecheckerCoordinator final {
    /** Contains a queue of functions to run on the typechecking thread. */
    BlockingUnBoundedQueue<std::function<void()>> lambdas;
    /** If 'true', the coordinator should terminate immediately. */
    bool shouldTerminate;
    /** LSPTypecheckerCoordinator delegates typechecking operations to LSPTypechecker. */
    LSPTypechecker typechecker;
    std::shared_ptr<const LSPConfiguration> config;
    /** If 'true', then the typechecker is running on a dedicated thread. */
    bool hasDedicatedThread;

    /**
     * Runs the provided function on the typechecker thread.
     */
    void asyncRunInternal(std::function<void()> &&lambda, bool canPreemptSlowPath);

public:
    LSPTypecheckerCoordinator(const std::shared_ptr<const LSPConfiguration> &config);

    /**
     * Runs lambda with exclusive access to GlobalState. lambda runs on typechecker thread. These cannot preeempt the
     * slow path because we do not support having a queue of preempting lambdas.
     * If `waitUntilTaskStarts` is `true`, then this method will block until lambda starts running.
     */
    void asyncRun(std::function<void(LSPTypechecker &)> &&lambda, bool waitUntilTaskStarts);

    /**
     * Like asyncRun, but blocks until `lambda` completes. These can preeempt a currently running slow path.
     */
    void syncRun(std::function<void(LSPTypechecker &)> &&lambda, bool canPreemptSlowPath);

    /**
     * Safely shuts down the typechecker and returns the final GlobalState object. Blocks until typechecker completes
     * final operation.
     */
    std::unique_ptr<core::GlobalState> shutdown();

    /** Runs the typechecker in a dedicated thread. */
    std::unique_ptr<Joinable> startTypecheckerThread();
};
} // namespace sorbet::realmain::lsp

#endif
