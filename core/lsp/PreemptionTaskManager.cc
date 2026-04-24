#include "core/lsp/PreemptionTaskManager.h"
#include "core/ErrorQueue.h"
#include "core/NullFlusher.h"
#include "core/lsp/PreemptionTask.h"
#include "core/lsp/TypecheckEpochManager.h"

using namespace std;

namespace sorbet::core::lsp {

PreemptionTaskManager::PreemptionTaskManager(shared_ptr<TypecheckEpochManager> epochManager)
    : epochManager(move(epochManager)) {}

bool PreemptionTaskManager::trySchedulePreemptionTask(shared_ptr<PreemptionTask> task) {
    TypecheckEpochManager::assertConsistentThread(
        processingThreadId, "PreemptionTaskManager::trySchedulePreemptionTask", "processing thread");
    bool success = false;
    // Need to grab epoch lock so we have accurate information w.r.t. if typechecking is happening / if typechecking was
    // canceled. Avoids races with typechecking thread.
    this->epochManager->withEpochLock([&preemptTask = this->preemptTask, &runnableAt = this->runnableAt, &task,
                                       &success](TypecheckEpochManager::TypecheckingStatus status) -> void {
        // The code should only ever set one preempt function.
        auto existingTask = atomic_load(&preemptTask);
        ENFORCE(existingTask == nullptr);
        if (!status.slowPathRunning || status.slowPathWasCanceled || existingTask != nullptr) {
            // No slow path running, typechecking was canceled so we can't preempt the canceled slow path, or a task
            // is already scheduled. The latter should _never_ occur, as the scheduled task should _block_ the
            // thread that scheduled it.
            return;
        }

        // We preemptively reset the runnableStratum at this point. Because we are already assuming that the task
        // slot is empty, this should not have any affect on an existing task. However, if there was somehow a task
        // already scheduled that had set `runnableAt`, this would cause it to run again immediately and then
        // potentially defer itself again.
        runnableAt.store(0);

        success = atomic_compare_exchange_strong(&preemptTask, &existingTask, move(task));
    });

    return success;
}

PreemptionTask::RunResult PreemptionTaskManager::tryRunScheduledPreemptionTask(const core::GlobalState &gs,
                                                                               uint16_t currentStratum,
                                                                               bool allowReschedule) {
    PreemptionTask::RunResult result;

    TypecheckEpochManager::assertConsistentThread(
        typecheckingThreadId, "PreemptionTaskManager::tryRunScheduledPreemptionTask", "typechecking thread");

    // We can early-exit if we know that it's not possible to run preemption yet, but if we know that rescheduling is
    // not possible, we should run to completion.
    if (allowReschedule && currentStratum < this->runnableAt.load()) {
        return result;
    }

    auto preemptTask = atomic_load(&this->preemptTask);
    if (preemptTask != nullptr &&
        atomic_compare_exchange_strong(&this->preemptTask, &preemptTask, shared_ptr<PreemptionTask>(nullptr))) {
        // Capture with write lock before running task. Ensures that all worker threads park before we proceed.
        absl::MutexLock lock(&typecheckMutex);
        // The error queue is where typechecking puts all typechecking errors. For a given edit, Sorbet LSP runs
        // typechecking and then drains the error queue. If we failed to temporarily swap it out during preemption, the
        // preempted task will see all of the errors that have accumulated thus far on the slow path. Thus, we save the
        // old error queue and replace so new operation starts fresh
        auto previousErrorQueue = move(gs.errorQueue);
        gs.errorQueue = make_shared<core::ErrorQueue>(previousErrorQueue->logger, previousErrorQueue->tracer,
                                                      make_shared<core::NullFlusher>());
        gs.tracer().debug("[Typechecker] Beginning preemption task.");
        result = preemptTask->run(currentStratum);
        if (allowReschedule && result.rescheduled.has_value()) {
            // In this case the task has indicated that there's more work to do, but that it can't occur until the
            // stratum named in `result`. We re-queue the task, and remember the stratum that we can run at so that we
            // can early exit in future calls to `tryRunScheduledPreemptionTask`.
            this->runnableAt.store(*result.rescheduled);

            // As we haven't called `finish` yet, we're assuming unique access to the preemptTask slot: LSPLoop will be
            // blocked as the PreemptionLoop won't have notified it yet.
            auto existingTask = atomic_load(&this->preemptTask);
            ENFORCE(existingTask == nullptr);
            auto success = atomic_compare_exchange_strong(&this->preemptTask, &existingTask, preemptTask);
            ENFORCE(success);

            gs.tracer().debug("[Typechecker] Preemption task deferred.");
        } else {
            preemptTask->finish();
            gs.tracer().debug("[Typechecker] Preemption task complete.");
        }
        gs.errorQueue = move(previousErrorQueue);
    }

    return result;
}

bool PreemptionTaskManager::tryCancelScheduledPreemptionTask(shared_ptr<PreemptionTask> &task) {
    TypecheckEpochManager::assertConsistentThread(
        processingThreadId, "PreemptionTaskManager::tryCancelScheduledPreemptionTask", "processing thread");
    return atomic_compare_exchange_strong(&preemptTask, &task, shared_ptr<PreemptionTask>(nullptr));
}

unique_ptr<absl::ReaderMutexLock> PreemptionTaskManager::lockPreemption() const {
    return make_unique<absl::ReaderMutexLock>(&typecheckMutex);
}

void PreemptionTaskManager::assertTypecheckMutexHeld() {
    typecheckMutex.AssertHeld();
}

} // namespace sorbet::core::lsp
