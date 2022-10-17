#ifndef RUBY_TYPER_LSP_NOTIFICATIONS_WATCHMAN_STATE_LEAVE_H
#define RUBY_TYPER_LSP_NOTIFICATIONS_WATCHMAN_STATE_LEAVE_H

#include "main/lsp/LSPTask.h"

namespace sorbet::realmain::lsp {
class WatchmanStateLeaveTask final : public LSPTask {
public:
    const std::unique_ptr<WatchmanStateLeave> params;

    WatchmanStateLeaveTask(const LSPConfiguration &config, std::unique_ptr<WatchmanStateLeave> params);

    LSPTask::Phase finalPhase() const override;

    void preprocess(LSPPreprocessor &preprocessor) override;

    void run(LSPTypecheckerDelegate &tc) override;
};
} // namespace sorbet::realmain::lsp

#endif
