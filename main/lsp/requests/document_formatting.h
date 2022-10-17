#ifndef RUBY_TYPER_LSP_REQUESTS_DOCUMENT_FORMAT_H
#define RUBY_TYPER_LSP_REQUESTS_DOCUMENT_FORMAT_H

#include "main/lsp/LSPTask.h"

namespace sorbet::realmain::lsp {
class DocumentFormattingParams;
class DocumentFormattingTask final : public LSPRequestTask {
    std::unique_ptr<DocumentFormattingParams> params;

public:
    DocumentFormattingTask(const LSPConfiguration &config, MessageId id,
                           std::unique_ptr<DocumentFormattingParams> params);

    Phase finalPhase() const override;

    void preprocess(LSPPreprocessor &preprocessor) override;

    std::unique_ptr<ResponseMessage> runRequest(LSPTypecheckerDelegate &typechecker) override;

private:
    void displayError(std::string errorMessage, std::unique_ptr<ResponseMessage> &response);
};

} // namespace sorbet::realmain::lsp

#endif
