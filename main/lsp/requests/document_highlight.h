#ifndef RUBY_TYPER_LSP_REQUESTS_DOCUMENT_HIGHLIGHT_H
#define RUBY_TYPER_LSP_REQUESTS_DOCUMENT_HIGHLIGHT_H

#include "main/lsp/LSPTask.h"

namespace sorbet::realmain::lsp {
class TextDocumentPositionParams;
class DocumentHighlightTask final : public LSPRequestTask {
    std::unique_ptr<TextDocumentPositionParams> params;

private:
    std::vector<std::unique_ptr<DocumentHighlight>>
    getHighlightsFromQueryResponse(LSPTypecheckerDelegate &typechecker, std::string_view uri,
                                   const core::GlobalState &gs, core::FileRef fref, bool fileIsTyped,
                                   std::unique_ptr<core::lsp::QueryResponse> resp);

public:
    DocumentHighlightTask(const LSPConfiguration &config, MessageId id,
                          std::unique_ptr<TextDocumentPositionParams> params);

    std::unique_ptr<ResponseMessage> runRequest(LSPTypecheckerDelegate &typechecker) override;
};

} // namespace sorbet::realmain::lsp

#endif
