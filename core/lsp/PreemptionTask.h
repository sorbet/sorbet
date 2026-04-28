#ifndef SORBET_LSP_PREEMPTION_TASK_H
#define SORBET_LSP_PREEMPTION_TASK_H

#include "common/common.h"
#include <cstdint>
#include <optional>

namespace sorbet::core::lsp {
// Generic interface for tasks that can run at preemption points.
class PreemptionTask {
public:
    PreemptionTask() = default;
    virtual ~PreemptionTask() = default;

    // There are four possible states that the `RunResult` type represents:
    // 1. No tasks were processed by the preemption task, and it wasn't rescheduled for a later stratum when operating
    //    in package-directed mode. This can occur when the main thread schedules a preemption task because it
    //    determines that there are preemption-supporting tasks at the head of the queue, but those tasks get canceled
    //    before preemption has the opportunity to run.
    // 2. The preemption task successfully handles all of the tasks in the queue that support preemption, and doesn't
    //    need to reschedule itself for a later stratum.
    // 3. The preemption task successfully handles some of the tasks in the queue, but needs to reschedule itself for a
    //    later stratum to handle the rest.
    // 4. The preemption task cannot handle any of the tasks in the queue, but can at a later stratum. It has requested
    //    to be rescheduled to run at that stratum.
    class RunResult {
        bool tasksHandled_ = false;
        bool wasRescheduled_ = false;

        uint16_t rescheduled = 0;

    public:
        RunResult() = default;

        // Indicate that tasks from the queue were handled during preemption.
        void setTasksHandled() {
            this->tasksHandled_ = true;
        }

        // True if the preemption task handled tasks from the queue.
        bool getTasksHandled() const {
            return this->tasksHandled_;
        }

        // Indicate that the preemption task would like to be run again at the provided stratum.
        void setRescheduledStratum(uint16_t stratum) {
            this->wasRescheduled_ = true;
            this->rescheduled = stratum;
        }

        // Fetch the stratum that the preemption task requested to be rescheduled to (only valid if
        // `setRescheduledStratum` has been called).
        uint16_t getRescheduledStratum() const {
            ENFORCE(this->wasRescheduled_);
            return this->rescheduled;
        }

        // True if the preemption task encountered a task that it could process at a later stratum.
        bool wasRescheduled() const {
            return this->wasRescheduled_;
        }

        // Returns true if the preemption task handled some tasks or determined that it needs to be rescheduled.
        bool progress() const {
            return this->tasksHandled_ || this->wasRescheduled_;
        }
    };

    // Run the preemption task. Returning a non-empty result indicates that there is more work to do, and that it can't
    // be done until the stratum indicated.
    virtual RunResult run(uint16_t currentStratum) = 0;

    // Optional hook called when the task has been run to completion. Any notification to unblock other threads should
    // be done here to avoid accidentally unblocking work that could interact with the preemption scheduler.
    virtual void finish() {}

    // Disallow copy/move to force management through smart pointers.
    PreemptionTask(PreemptionTask &) = delete;
    PreemptionTask(const PreemptionTask &) = delete;
    PreemptionTask &operator=(PreemptionTask &&) = delete;
    PreemptionTask &operator=(const PreemptionTask &) = delete;
};
} // namespace sorbet::core::lsp

#endif
