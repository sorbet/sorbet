#include "main/lsp/requests/command.h"
#include "main/lsp/json_types.h"

using namespace std;

namespace sorbet::realmain::lsp {
ExecuteCommandTask::ExecuteCommandTask(const LSPConfiguration &config, MessageId id,
                                       unique_ptr<ExecuteCommandParams> params)

    : LSPRequestTask(config, move(id), LSPMethod::WorkspaceExecuteCommand), params(move(params)) {}

unique_ptr<ResponseMessage> ExecuteCommandTask::runRequest(LSPTypecheckerInterface &typechecker) {
    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::WorkspaceExecuteCommand);
    auto result = make_unique<ExecuteCommandResponse>(JSONNullObject());
    response->result = move(result);

    return response;
}

} // namespace sorbet::realmain::lsp
