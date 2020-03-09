#ifndef RUBY_TYPER_LSP_REQUESTS_DOCUMENT_SYMBOL_H
#define RUBY_TYPER_LSP_REQUESTS_DOCUMENT_SYMBOL_H

#include "main/lsp/LSPTask.h"

namespace sorbet::realmain::lsp {
class DocumentSymbolParams;
class DocumentSymbolTask final : public LSPRequestTask {
    std::unique_ptr<DocumentSymbolParams> params;

public:
    DocumentSymbolTask(const LSPConfiguration &config, MessageId id, std::unique_ptr<DocumentSymbolParams> params);

    bool isDelayable() const override;

    std::unique_ptr<ResponseMessage> runRequest(LSPTypecheckerDelegate &typechecker) override;
};

} // namespace sorbet::realmain::lsp

#endif
