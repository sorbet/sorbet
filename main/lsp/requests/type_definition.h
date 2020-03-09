#ifndef RUBY_TYPER_LSP_REQUESTS_TYPE_DEFINITION_H
#define RUBY_TYPER_LSP_REQUESTS_TYPE_DEFINITION_H

#include "main/lsp/LSPTask.h"

namespace sorbet::realmain::lsp {
class TextDocumentPositionParams;
class TypeDefinitionTask final : public LSPRequestTask {
    std::unique_ptr<TextDocumentPositionParams> params;

public:
    TypeDefinitionTask(const LSPConfiguration &config, MessageId id,
                       std::unique_ptr<TextDocumentPositionParams> params);

    std::unique_ptr<ResponseMessage> runRequest(LSPTypecheckerDelegate &typechecker) override;
};

} // namespace sorbet::realmain::lsp

#endif
