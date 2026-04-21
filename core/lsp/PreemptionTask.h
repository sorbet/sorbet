#ifndef SORBET_LSP_PREEMPTION_TASK_H
#define SORBET_LSP_PREEMPTION_TASK_H

#include <cstdint>
#include <optional>

namespace sorbet::core::lsp {
// Generic interface for tasks that can run at preemption points.
class PreemptionTask {
public:
    PreemptionTask() = default;
    virtual ~PreemptionTask() = default;

    // Run the preemption task. Returning a non-empty result indicates that there is more work to do, and that it can't
    // be done until the stratum indicated.
    virtual std::optional<uint16_t> run(uint16_t currentStratum) = 0;

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
