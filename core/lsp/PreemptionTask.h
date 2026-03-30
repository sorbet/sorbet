#ifndef SORBET_LSP_PREEMPTION_TASK_H
#define SORBET_LSP_PREEMPTION_TASK_H

namespace sorbet::core::lsp {
// Generic interface for tasks that can run at preemption points.
class PreemptionTask {
public:
    PreemptionTask() = default;
    virtual ~PreemptionTask() = default;

    virtual void run() = 0;

    // Disallow copy/move to force management through smart pointers.
    PreemptionTask(PreemptionTask &) = delete;
    PreemptionTask(const PreemptionTask &) = delete;
    PreemptionTask &operator=(PreemptionTask &&) = delete;
    PreemptionTask &operator=(const PreemptionTask &) = delete;
};
} // namespace sorbet::core::lsp

#endif
