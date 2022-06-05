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

    void index(LSPIndexer &index) override;

    std::unique_ptr<ResponseMessage> runRequest(LSPTypecheckerInterface &typechecker) override;

private:
    void displayError(std::string errorMessage, std::unique_ptr<ResponseMessage> &response);
    bool documentIsFormattable(std::string_view path);
};

} // namespace sorbet::realmain::lsp

#endif
