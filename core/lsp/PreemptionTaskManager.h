#ifndef SORBET_LSP_PREEMPTIONTASKMANAGER_H
#define SORBET_LSP_PREEMPTIONTASKMANAGER_H

#include "core/GlobalState.h"
#include "core/lsp/PreemptionTask.h"
#include <memory>

namespace sorbet::core::lsp {
class TypecheckEpochManager;
class PreemptionTaskManager final {
private:
    // Used to pre-empt typechecking (post-resolver).
    // - Worker threads grab as a reader lock, and routinely gives up and re-acquire the lock to allow other requests to
    // pre-empt.
    // - Typechecking coordinator thread grabs as a writer lock when there's a preemption function, which halts all
    // worker threads.
    mutable absl::Mutex typecheckMutex;
    std::shared_ptr<PreemptionTask> preemptTask;
    std::shared_ptr<TypecheckEpochManager> epochManager;
    // Thread ID of the typechecking thread. Lazily set.
    std::optional<std::thread::id> typecheckingThreadId;
    // Thread ID of the processing thread. Lazily set.
    std::optional<std::thread::id> processingThreadId;

    // If a preemption task has rescheduled itself, this is the stratum that it indicated it would be runnable at.
    //
    // This is written from two contexts:
    // 1. `trySchedulePreemptionTask`, where it is reset to zero when preemption task scheduling is successful,
    // 2. and `tryRunScheduledPreemptionTask`, where it's set if the task indicates that it can't run until some future
    //    stratum N.
    //
    // These two writes are safe for the following reasons:
    // 1. No other preemption task will be scheduled, when its set in `trySchedulePreemptionTask`, and the only
    //    implementation of `PreemptionTask` that we run outside of tasks is `PreemptionLoop`, which blocks the main
    //    thread while it's scheduled.
    // 2. No other preemption task will be scheduled while we're running the current one, as the main thread will be
    //    blocked waiting for a notification from the running task's `finish` method. Again, this logic is pretty
    //    specific to the `PreemptionLoop` implementation, but that's also the only non-test implementation of
    //    `PreemeptionTask`.
    std::atomic<uint16_t> runnableAt = 0;

public:
    PreemptionTaskManager(std::shared_ptr<TypecheckEpochManager> epochManager);
    // Run only from processing thread.
    // Attempts to preempt a running slow path to run the provided task. If it returns true, the task is guaranteed
    // to run.
    bool trySchedulePreemptionTask(std::shared_ptr<PreemptionTask> task);
    // Run only from the typechecking thread.
    // Runs the scheduled preemption task, if any.
    // Must be called with `allowReschedule` set to false on the last run to ensure that the preemption task gets
    // cleared out. Otherwise there is the possibility that it might get rescheduled, which would cause problems the
    // next time preemption was scheduled from the main thread. Handles running task with a fresh errorQueue, and
    // restoring previous errorQueue when done.
    PreemptionTask::RunResult tryRunScheduledPreemptionTask(const core::GlobalState &gs, uint16_t currentStratum,
                                                            bool allowReschedule);
    // Run only from processing thread.
    // Tries to cancel the scheduled preemption task. Returns true if it succeeds.
    bool tryCancelScheduledPreemptionTask(std::shared_ptr<PreemptionTask> &task);
    // Run only from typechecker worker threads. Prevents preemption from occurring while the ReaderMutexLock is alive.
    std::unique_ptr<absl::ReaderMutexLock> lockPreemption() const;
    // (For testing only) Assert that typecheckMutex is held.
    void assertTypecheckMutexHeld();
};

} // namespace sorbet::core::lsp
#endif
