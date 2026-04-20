#ifndef SORBET_LSP_PREEMPTIONTASKMANAGER_H
#define SORBET_LSP_PREEMPTIONTASKMANAGER_H

#include "core/GlobalState.h"
#include <memory>

namespace sorbet::core::lsp {
class PreemptionTask;
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
    bool tryRunScheduledPreemptionTask(const core::GlobalState &gs, uint16_t currentStratum, bool allowReschedule);
    // Run only from processing thread.
    // Tries to cancel the scheduled preemption task. Returns true if it succeeds.
    bool tryCancelScheduledPreemptionTask(std::shared_ptr<PreemptionTask> &task);
    // Run only from typechecker worker threads. Prevents preemption from occurring while the ReaderMutexLock is alive.
    std::unique_ptr<absl::ReaderMutexLock> lockPreemption() const;
    // (For testing only) Assert that typecheckMutex is held.
    void assertTypecheckMutexHeld();

    // Reset the preemption stratum back to the first stratum.
    void resetPreemptionStratum() {
        // The first time we run a preemption task, it won't know what stratum it needs to be run at, so we
        // optimistically track this as stratum zero, with the assupmtion that rescheduling will be cheap.
        this->runnableAt.store(0);
    }
};

} // namespace sorbet::core::lsp
#endif
