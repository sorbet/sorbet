#ifndef RUBY_TYPER_LSP_NOTIFICATIONS_WATCHMAN_STATE_ENTER_H
#define RUBY_TYPER_LSP_NOTIFICATIONS_WATCHMAN_STATE_ENTER_H

#include "main/lsp/LSPTask.h"

namespace sorbet::realmain::lsp {
class WatchmanStateEnterTask final : public LSPTask {
public:
    const std::unique_ptr<WatchmanStateEnter> params;

    WatchmanStateEnterTask(const LSPConfiguration &config, std::unique_ptr<WatchmanStateEnter> params);

    LSPTask::Phase finalPhase() const override;

    void preprocess(LSPPreprocessor &preprocessor) override;

    void run(LSPTypecheckerDelegate &tc) override;
};
} // namespace sorbet::realmain::lsp

#endif
