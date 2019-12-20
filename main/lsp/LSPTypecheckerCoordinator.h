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

    // A worker pool with typically as many threads as cores. Can only be used during synchronous blocking operations.
    WorkerPool &workers;

    /**
     * Runs the provided task on the typechecker thread.
     */
    void asyncRunInternal(std::shared_ptr<core::lsp::Task> task);

public:
    LSPTypecheckerCoordinator(const std::shared_ptr<const LSPConfiguration> &config, WorkerPool &workers);

    /**
     * Schedules a task on the typechecker thread, and blocks until `lambda` completes.
     * TODO(jvilk): Make tasks scheduled this way preempt the slow path.
     */
    void syncRun(std::function<void(LSPTypechecker &)> &&lambda);

    /**
     * syncRun, except the function receives direct access to WorkerPool and can perform multithreaded operations.
     * TODO(jvilk): This method will _wait_ for a running slow path before running, as workers cannot be used
     * concurrently with a typechecking operation.
     */
    void syncRunMultithreaded(std::function<void(LSPTypechecker &, WorkerPool &)> &&lambda);

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
