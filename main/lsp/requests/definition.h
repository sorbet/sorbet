#ifndef RUBY_TYPER_LSP_REQUESTS_DEFINITION_H
#define RUBY_TYPER_LSP_REQUESTS_DEFINITION_H

#include "main/lsp/LSPTask.h"

namespace sorbet::realmain::lsp {
class TextDocumentPositionParams;
class DefinitionTask final : public LSPRequestTask {
    std::unique_ptr<TextDocumentPositionParams> params;
    core::Loc findRequireRelativeLoc(const core::GlobalState &gs,
                                     const std::vector<std::unique_ptr<core::lsp::QueryResponse>> &responses);

public:
    DefinitionTask(const LSPConfiguration &config, MessageId id, std::unique_ptr<TextDocumentPositionParams> params);

    std::unique_ptr<ResponseMessage> runRequest(LSPTypecheckerDelegate &typechecker) override;
};

} // namespace sorbet::realmain::lsp

#endif
