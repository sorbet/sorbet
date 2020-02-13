#ifndef RUBY_TYPER_LSP_NOTIFICATIONS_EXIT_H
#define RUBY_TYPER_LSP_NOTIFICATIONS_EXIT_H

#include "main/lsp/LSPTask.h"

namespace sorbet::realmain::lsp {
class ExitTask final : public LSPTask {
public:
    const int exitCode;

    ExitTask(const LSPConfiguration &config, int exitCode);

    LSPTask::Phase finalPhase() const override;

    void preprocess(LSPPreprocessor &preprocessor) override;

    void run(LSPTypecheckerDelegate &tc) override;
};
} // namespace sorbet::realmain::lsp

#endif
