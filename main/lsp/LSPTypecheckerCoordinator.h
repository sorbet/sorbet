#ifndef RUBY_TYPER_LSP_LSPTYPECHECKERCOORDINATOR_H
#define RUBY_TYPER_LSP_LSPTYPECHECKERCOORDINATOR_H

#include "core/lsp/Task.h"
#include "main/lsp/LSPTypechecker.h"

namespace sorbet::realmain::lsp {
/**
 * Handles typechecking and other queries. Can either operate in single-threaded mode (in which lambdas passed to
 * syncRun/asyncRun run-to-completion immediately) or dedicated-thread mode (in which lambdas are enqueued to execute on
 * thread).
 */
class LSPTypecheckerCoordinator final {
    /** Contains a queue of tasks to run on the typechecking thread. */
    BlockingUnBoundedQueue<std::shared_ptr<core::lsp::Task>> tasks;
    /** If 'true', the coordinator should terminate immediately. */
    bool shouldTerminate;
    /** LSPTypecheckerCoordinator delegates typechecking operations to LSPTypechecker. */
    LSPTypechecker typechecker;
    std::shared_ptr<const LSPConfiguration> config;
    /** If 'true', then the typechecker is running on a dedicated thread. */
    bool hasDedicatedThread;

    /**
     * Runs the provided task on the typechecker thread.
     */
    void asyncRunInternal(std::shared_ptr<core::lsp::Task> task);

public:
    LSPTypecheckerCoordinator(const std::shared_ptr<const LSPConfiguration> &config);

    /**
     * Runs lambda with exclusive access to GlobalState. lambda runs on typechecker thread.
     */
    void asyncRun(std::function<void(LSPTypechecker &)> &&lambda);

    /**
     * Like asyncRun, but blocks until `lambda` completes.
     */
    void syncRun(std::function<void(LSPTypechecker &)> &&lambda);

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
