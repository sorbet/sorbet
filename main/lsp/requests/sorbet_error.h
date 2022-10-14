#ifndef RUBY_TYPER_LSP_REQUESTS_UNKNOWN_REQUEST_H
#define RUBY_TYPER_LSP_REQUESTS_UNKNOWN_REQUEST_H

#include "main/lsp/LSPTask.h"

namespace sorbet::realmain::lsp {
class SorbetErrorParams;
class SorbetErrorTask final : public LSPTask {
    std::unique_ptr<SorbetErrorParams> params;
    std::optional<MessageId> id;

public:
    SorbetErrorTask(const LSPConfiguration &config, std::unique_ptr<SorbetErrorParams> params,
                    std::optional<MessageId> id = std::nullopt);

    LSPTask::Phase finalPhase() const override;

    void preprocess(LSPPreprocessor &preprocessor) override;

    void run(LSPTypecheckerDelegate &typechecker) override;
};

} // namespace sorbet::realmain::lsp

#endif
