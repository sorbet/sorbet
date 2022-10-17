#ifndef RUBY_TYPER_LSP_REQUESTS_INITIALIZE_H
#define RUBY_TYPER_LSP_REQUESTS_INITIALIZE_H

#include "main/lsp/LSPTask.h"

namespace sorbet::realmain::lsp {
class InitializeParams;
class InitializeTask final : public LSPRequestTask {
    LSPConfiguration &mutableConfig;
    std::unique_ptr<InitializeParams> params;

public:
    InitializeTask(LSPConfiguration &config, MessageId id, std::unique_ptr<InitializeParams> params);

    static const std::vector<std::string> TRIGGER_CHARACTERS;

protected:
    bool canPreempt(const LSPIndexer &indexer) const override;

    void preprocess(LSPPreprocessor &preprocessor) override;

    std::unique_ptr<ResponseMessage> runRequest(LSPTypecheckerDelegate &ts) override;
};
} // namespace sorbet::realmain::lsp

#endif
