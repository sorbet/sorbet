#ifndef RUBY_TYPER_LSP_REQUESTS_CODE_ACTION_H
#define RUBY_TYPER_LSP_REQUESTS_CODE_ACTION_H

#include "main/lsp/LSPTask.h"
#include "main/lsp/json_types.h"

namespace sorbet::realmain::lsp {
class CodeActionParams;
class CodeActionTask final : public LSPRequestTask {
    std::unique_ptr<CodeActionParams> params;

public:
    CodeActionTask(const LSPConfiguration &config, MessageId id, std::unique_ptr<CodeActionParams> params);

    std::unique_ptr<ResponseMessage> runRequest(LSPTypecheckerInterface &typechecker) override;

    bool canUseStaleData() const override;

private:
    std::vector<std::unique_ptr<TextDocumentEdit>> getMoveMethodEdits(const LSPConfiguration &config,
                                                                      const core::GlobalState &gs,
                                                                      const core::lsp::MethodDefResponse &definition,
                                                                      LSPTypecheckerInterface &typechecker);
};

} // namespace sorbet::realmain::lsp

#endif
