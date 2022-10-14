#ifndef RUBY_TYPER_LSP_REQUESTS_HOVER_H
#define RUBY_TYPER_LSP_REQUESTS_HOVER_H

#include "main/lsp/LSPTask.h"

namespace sorbet::realmain::lsp {
class TextDocumentPositionParams;
class HoverTask final : public LSPRequestTask {
    std::unique_ptr<TextDocumentPositionParams> params;

public:
    HoverTask(const LSPConfiguration &config, MessageId id, std::unique_ptr<TextDocumentPositionParams> params);

    std::unique_ptr<ResponseMessage> runRequest(LSPTypecheckerDelegate &typechecker) override;
};

} // namespace sorbet::realmain::lsp

#endif
