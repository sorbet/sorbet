#include "main/lsp/requests/code_action_resolve.h"

using namespace std;
namespace sorbet::realmain::lsp {
CodeActionResolveTask::CodeActionResolveTask(const LSPConfiguration &config, MessageId id,
                                             unique_ptr<CodeAction> params)
    : LSPRequestTask(config, move(id), LSPMethod::CodeActionResolve), params(move(params)) {}

unique_ptr<ResponseMessage> CodeActionResolveTask::runRequest(LSPTypecheckerInterface &typechecker) {
    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::TextDocumentCodeAction);
    auto result = make_unique<CodeActionResolveResponse>();
    result->result = move(params);
    return response;
}

} // namespace sorbet::realmain::lsp
