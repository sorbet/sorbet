#include "main/lsp/requests/definition.h"
#include "core/lsp/QueryResponse.h"
#include "main/lsp/json_types.h"

using namespace std;

namespace sorbet::realmain::lsp {
DefinitionTask::DefinitionTask(const LSPConfiguration &config, MessageId id,
                               unique_ptr<TextDocumentPositionParams> params)
    : LSPRequestTask(config, move(id), LSPMethod::TextDocumentDefinition), params(move(params)) {}

unique_ptr<ResponseMessage> DefinitionTask::runRequest(LSPTypecheckerDelegate &typechecker) {
    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::TextDocumentDefinition);
    const core::GlobalState &gs = typechecker.state();
    auto result =
        queryByLoc(typechecker, params->textDocument->uri, *params->position, LSPMethod::TextDocumentDefinition, false);
    if (result.error) {
        // An error happened while setting up the query.
        response->error = move(result.error);
    } else {
        auto &queryResponses = result.responses;
        vector<unique_ptr<Location>> result;
        if (!queryResponses.empty()) {
            const bool fileIsTyped =
                config.uri2FileRef(gs, params->textDocument->uri).data(gs).strictLevel >= core::StrictLevel::True;
            auto resp = move(queryResponses[0]);

            // Only support go-to-definition on constants and fields in untyped files.
            if (auto c = resp->isConstant()) {
                auto sym = c->symbol;
                for (auto loc : sym.locs(gs)) {
                    addLocIfExists(gs, result, loc);
                }
            } else if (resp->isField() || (fileIsTyped && (resp->isIdent() || resp->isLiteral()))) {
                auto retType = resp->getTypeAndOrigins();
                for (auto &originLoc : retType.origins) {
                    addLocIfExists(gs, result, originLoc);
                }
            } else if (fileIsTyped && resp->isDefinition()) {
                auto sym = resp->isDefinition()->symbol;
                for (auto loc : sym.locs(gs)) {
                    addLocIfExists(gs, result, loc);
                }
            } else if (fileIsTyped && resp->isSend()) {
                auto sendResp = resp->isSend();
                auto start = sendResp->dispatchResult.get();
                while (start != nullptr) {
                    if (start->main.method.exists() && !start->main.receiver.isUntyped()) {
                        addLocIfExists(gs, result, start->main.method.data(gs)->loc());
                    }
                    start = start->secondary.get();
                }
            }
        }
        response->result = move(result);
    }
    return response;
}

} // namespace sorbet::realmain::lsp
