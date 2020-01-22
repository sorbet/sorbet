#ifndef RUBY_TYPER_LSP_LSPTYPECHECKERCOORDINATOR_H
#define RUBY_TYPER_LSP_LSPTYPECHECKERCOORDINATOR_H

#include "core/lsp/Task.h"
#include "main/lsp/LSPTypechecker.h"

namespace sorbet::core::lsp {
class PreemptionTaskManager;
}

namespace sorbet::realmain::lsp {
class LSPTask;
/**
 * Handles typechecking and other queries. Can either operate in single-threaded mode (in which lambdas passed to
 * syncRun/asyncRun run-to-completion immediately) or dedicated-thread mode (in which lambdas are enqueued to execute on
 * thread).
 */
class LSPTypecheckerCoordinator final {
    /** Contains a queue of tasks to run on the typechecking thread. */
    BlockingUnBoundedQueue<std::shared_ptr<core::lsp::Task>> tasks;
    /** Used to preempt running slow paths. */
    std::shared_ptr<core::lsp::PreemptionTaskManager> preemptionTaskManager;
    /** If 'true', the coordinator should terminate immediately. */
    bool shouldTerminate;
    /** LSPTypecheckerCoordinator delegates typechecking operations to LSPTypechecker. */
    LSPTypechecker typechecker;
    std::shared_ptr<const LSPConfiguration> config;
    /** If 'true', then the typechecker is running on a dedicated thread. */
    bool hasDedicatedThread;

    // A worker pool with typically as many threads as cores. Can only be used during synchronous blocking operations.
    WorkerPool &workers;

    // An empty workerpool with 0 threads. Runs all work on the thread using it.
    std::unique_ptr<WorkerPool> emptyWorkers;

    /**
     * Runs the provided task on the typechecker thread.
     */
    void asyncRunInternal(std::shared_ptr<core::lsp::Task> task);

public:
    LSPTypecheckerCoordinator(const std::shared_ptr<const LSPConfiguration> &config,
                              std::shared_ptr<core::lsp::PreemptionTaskManager> preemptionTaskManager,
                              WorkerPool &workers);

    /**
     * Initializes typechecker and runs typechecking for the first time.
     * TODO(jvilk): Make non-blocking when we implement preemption.
     */
    void initialize(LSPFileUpdates initializeUpdate);

    /**
     * Typechecks the given updates on the slow path. Blocks until the typechecking actually begins.
     */
    void typecheckOnSlowPath(LSPFileUpdates updates);

    /**
     * Schedules a task on the typechecker thread, and blocks until `task` completes. If the task has
     * `enableMultithreaded` set to "true", then the given task is allowed to use the full threadpool at the cost of not
     * being able to preempt slow path typechecking.
     * TODO(jvilk): Make single-threaded tasks scheduled this way preempt the slow path.
     */
    void syncRun(std::unique_ptr<LSPTask> task);

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
