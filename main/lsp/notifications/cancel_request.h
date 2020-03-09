#ifndef RUBY_TYPER_LSP_NOTIFICATIONS_CANCEL_REQUEST_H
#define RUBY_TYPER_LSP_NOTIFICATIONS_CANCEL_REQUEST_H

#include "main/lsp/LSPTask.h"

namespace sorbet::realmain::lsp {
class CancelParams;
class CancelRequestTask final : public LSPTask {
    const std::unique_ptr<CancelParams> params;

public:
    CancelRequestTask(const LSPConfiguration &config, std::unique_ptr<CancelParams> params);

    LSPTask::Phase finalPhase() const override;

    void preprocess(LSPPreprocessor &preprocessor) override;

    void run(LSPTypecheckerDelegate &tc) override;
};
} // namespace sorbet::realmain::lsp

#endif
