#ifndef RUBY_TYPER_LSP_REQUESTS_WORKSPACE_SYMBOLS_H
#define RUBY_TYPER_LSP_REQUESTS_WORKSPACE_SYMBOLS_H

#include "main/lsp/LSPTask.h"

namespace sorbet::realmain::lsp {
class WorkspaceSymbolParams;
class WorkspaceSymbolsTask final : public LSPRequestTask {
    std::unique_ptr<WorkspaceSymbolParams> params;

public:
    WorkspaceSymbolsTask(const LSPConfiguration &config, MessageId id, std::unique_ptr<WorkspaceSymbolParams> params);

    bool isDelayable() const override;

    std::unique_ptr<ResponseMessage> runRequest(LSPTypecheckerDelegate &typechecker) override;
};

} // namespace sorbet::realmain::lsp

#endif
