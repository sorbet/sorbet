#ifndef SORBET_EXECUTE_COMMAND_H
#define SORBET_EXECUTE_COMMAND_H

#include "main/lsp/LSPTask.h"

namespace sorbet::realmain::lsp {
class ExecuteCommandParams;
class ExecuteCommandTask final : public LSPRequestTask {
    std::unique_ptr<ExecuteCommandParams> params;

public:
    ExecuteCommandTask(const LSPConfiguration &config, MessageId id, std::unique_ptr<ExecuteCommandParams> params);

    std::unique_ptr<ResponseMessage> runRequest(LSPTypecheckerInterface &typechecker) override;
};

} // namespace sorbet::realmain::lsp

#endif
