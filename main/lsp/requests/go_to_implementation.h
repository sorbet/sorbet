#ifndef RUBY_TYPER_LSP_REQUESTS_GO_TO_IMPLEMENTATION_H
#define RUBY_TYPER_LSP_REQUESTS_GO_TO_IMPLEMENTATION_H

#include "main/lsp/LSPTask.h"

namespace sorbet::realmain::lsp {
class ImplementationParams;

class GoToImplementationTask final : public LSPRequestTask {
    std::unique_ptr<ImplementationParams> params;

public:
    GoToImplementationTask(const LSPConfiguration &config, MessageId id, std::unique_ptr<ImplementationParams> params);

    std::unique_ptr<ResponseMessage> runRequest(LSPTypecheckerDelegate &typechecker) override;
};

} // namespace sorbet::realmain::lsp

#endif