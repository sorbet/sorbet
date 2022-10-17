#include "main/lsp/requests/code_action_resolve.h"
#include "main/lsp/LSPQuery.h"
#include "main/lsp/MoveMethod.h"
#include "main/lsp/ShowOperation.h"

using namespace std;
namespace sorbet::realmain::lsp {
CodeActionResolveTask::CodeActionResolveTask(const LSPConfiguration &config, MessageId id,
                                             unique_ptr<CodeAction> params)
    : LSPRequestTask(config, move(id), LSPMethod::CodeActionResolve), params(move(params)) {}

unique_ptr<ResponseMessage> CodeActionResolveTask::runRequest(LSPTypecheckerDelegate &typechecker) {
    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::CodeActionResolve);
    if (params->kind != CodeActionKind::RefactorExtract || !params->data.has_value()) {
        response->error =
            make_unique<ResponseError>((int)LSPErrorCodes::InvalidRequest, "Invalid `codeAction/resolve` request");
        return response;
    }
    const auto &actualParams = *params->data;

    const auto queryResult = LSPQuery::byLoc(config, typechecker, actualParams->textDocument->uri,
                                             *actualParams->range->start, LSPMethod::CodeActionResolve, false);

    if (queryResult.error != nullptr) {
        response->error =
            make_unique<ResponseError>((int)LSPErrorCodes::InvalidRequest, "Invalid `codeAction/resolve` request");
        return response;
    }

    ShowOperation op(config, ShowOperation::Kind::MoveMethod);

    for (const auto &resp : queryResult.responses) {
        if (const auto def = resp->isMethodDef()) {
            auto action = make_unique<CodeAction>("Move method to a new module");
            action->kind = CodeActionKind::RefactorExtract;
            auto workspaceEdit = make_unique<WorkspaceEdit>();
            workspaceEdit->documentChanges = getMoveMethodEdits(typechecker, config, *def);
            action->edit = move(workspaceEdit);
            response->result = move(action);
        }
    }

    return response;
}

} // namespace sorbet::realmain::lsp
