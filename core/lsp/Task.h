#ifndef SORBET_LSP_TASK_H
#define SORBET_LSP_TASK_H

namespace sorbet::core::lsp {
// Generic, multi-use task interface.
// TODO(jvilk): Will use for tasks that preempt IDE slow path typechecking. It's in core because GlobalState will
// reference it.
class Task {
public:
    Task() = default;
    virtual ~Task() = default;

    virtual void run() = 0;
};
} // namespace sorbet::core::lsp

#endif
