#ifndef RUBY_TYPER_LSP_LSPTYPECHECKERCOORDINATOR_H
#define RUBY_TYPER_LSP_LSPTYPECHECKERCOORDINATOR_H

#include "common/concurrency/ConcurrentQueue.h"
#include "main/lsp/LSPTypechecker.h"

namespace sorbet::core::lsp {
class PreemptionTaskManager;
class Task;
} // namespace sorbet::core::lsp

namespace sorbet::realmain::lsp {
class LSPTask;
class LSPQueuePreemptionTask;
class InitializedTask;
class SorbetWorkspaceEditTask;
class TaskQueue;
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

    // A handle to the task pool that the preprocessor reads from, to enable feedback to the indexer when initializing
    // the global state for the first time.
    std::shared_ptr<TaskQueue> taskQueue;

    // An empty workerpool with 0 threads. Runs all work on the thread using it.
    std::unique_ptr<WorkerPool> emptyWorkers;

    /**
     * Runs the provided task on the typechecker thread.
     */
    void asyncRunInternal(std::shared_ptr<core::lsp::Task> task);

public:
    LSPTypecheckerCoordinator(const std::shared_ptr<const LSPConfiguration> &config,
                              std::shared_ptr<core::lsp::PreemptionTaskManager> preemptionTaskManager,
                              WorkerPool &workers, std::shared_ptr<TaskQueue> taskQueue);

    /**
     * Runs the given typecheck task asynchronously on the slow path.
     */
    void typecheckOnSlowPath(std::unique_ptr<SorbetWorkspaceEditTask> typecheckTask);

    /**
     * Schedules a task to run on the typechecker thread. Blocks until it completes.
     */
    void syncRun(std::unique_ptr<LSPTask> task);

    /**
     * Attempts to schedule a task to preempt the slow path. Returns the scheduled task if it succeeds, or nullptr
     * otherwise. The scheduled task should only be used in `tryCancelPreemption` to cancel the scheduled preemption
     * task.
     *
     * Does not block. It is the responsibility of the caller to properly block. Should only be used in one place
     * in `protocol.cc`.
     */
    std::shared_ptr<core::lsp::Task> trySchedulePreemption(std::unique_ptr<LSPQueuePreemptionTask> preemptTask);

    /**
     * Attempts to cancel the scheduled preeemption task. Returns true if it succeeds, or false if the task has or will
     * run.
     */
    bool tryCancelPreemption(std::shared_ptr<core::lsp::Task> &preemptTask);

    /**
     * Safely shuts down the typechecker and returns the final GlobalState object. Blocks until typechecker completes
     * final operation.
     */
    std::unique_ptr<core::GlobalState> shutdown();

    /** Runs the typechecker in a dedicated thread. */
    std::unique_ptr<Joinable> startTypecheckerThread();

    /**
     * (For tests only) Set a flag that forces the slow path to block indefinitely after saving undo state. Setting
     * this flag to `false` will immediately unblock any currently blocked slow paths.
     */
    void setSlowPathBlocked(bool blocked) {
        typechecker.setSlowPathBlocked(blocked);
    }
};
} // namespace sorbet::realmain::lsp

#endif
