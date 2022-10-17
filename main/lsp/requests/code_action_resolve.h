#ifndef SORBET_CODE_ACTION_RESOLVE_H
#define SORBET_CODE_ACTION_RESOLVE_H

#include "main/lsp/LSPTask.h"

namespace sorbet::realmain::lsp {
class CodeAction;
class CodeActionResolveTask final : public LSPRequestTask {
    std::unique_ptr<CodeAction> params;

public:
    CodeActionResolveTask(const LSPConfiguration &config, MessageId id, std::unique_ptr<CodeAction> params);

    std::unique_ptr<ResponseMessage> runRequest(LSPTypecheckerDelegate &typechecker) override;
};

} // namespace sorbet::realmain::lsp

#endif
