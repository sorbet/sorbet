#ifndef RUBY_TYPER_LSP_NOTIFICATIONS_SORBET_PAUSE_H
#define RUBY_TYPER_LSP_NOTIFICATIONS_SORBET_PAUSE_H

#include "main/lsp/LSPTask.h"

namespace sorbet::realmain::lsp {
class SorbetPauseTask final : public LSPTask {
public:
    SorbetPauseTask(const LSPConfiguration &config);

    LSPTask::Phase finalPhase() const override;

    void preprocess(LSPPreprocessor &preprocessor) override;

    void run(LSPTypecheckerDelegate &tc) override;
};
} // namespace sorbet::realmain::lsp

#endif
