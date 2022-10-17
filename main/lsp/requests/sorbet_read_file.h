#ifndef RUBY_TYPER_LSP_REQUESTS_SORBET_READ_FILE_H
#define RUBY_TYPER_LSP_REQUESTS_SORBET_READ_FILE_H

#include "main/lsp/LSPTask.h"

namespace sorbet::realmain::lsp {
class TextDocumentIdentifier;
class SorbetReadFileTask final : public LSPRequestTask {
    std::unique_ptr<TextDocumentIdentifier> params;

public:
    SorbetReadFileTask(const LSPConfiguration &config, MessageId id, std::unique_ptr<TextDocumentIdentifier> params);

    std::unique_ptr<ResponseMessage> runRequest(LSPTypecheckerDelegate &typechecker) override;
};

} // namespace sorbet::realmain::lsp

#endif
