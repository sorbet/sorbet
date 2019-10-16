#include "core/lsp/QueryResponse.h"
#include "main/lsp/lsp.h"

using namespace std;

namespace sorbet::realmain::lsp {
void LSPLoop::addLocIfExists(const core::GlobalState &gs, vector<unique_ptr<Location>> &locs, core::Loc loc) const {
    auto location = config->loc2Location(gs, loc);
    if (location != nullptr) {
        locs.push_back(std::move(location));
    }
}

LSPResult LSPLoop::handleTextDocumentDefinition(const LSPTypecheckerOps &ops, const MessageId &id,
                                                const TextDocumentPositionParams &params) const {
    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::TextDocumentDefinition);
    prodCategoryCounterInc("lsp.messages.processed", "textDocument.definition");
    const core::GlobalState &gs = ops.gs;
    auto result = queryByLoc(ops, params.textDocument->uri, *params.position, LSPMethod::TextDocumentDefinition, false);
    if (result.error) {
        // An error happened while setting up the query.
        response->error = move(result.error);
    } else {
        auto &queryResponses = result.responses;
        vector<unique_ptr<Location>> result;
        if (!queryResponses.empty()) {
            const bool fileIsTyped =
                config->uri2FileRef(gs, params.textDocument->uri).data(gs).strictLevel >= core::StrictLevel::True;
            auto resp = move(queryResponses[0]);

            // Only support go-to-definition on constants in untyped files.
            if (resp->isConstant() || (fileIsTyped && (resp->isIdent() || resp->isLiteral()))) {
                auto retType = resp->getTypeAndOrigins();
                for (auto &originLoc : retType.origins) {
                    addLocIfExists(gs, result, originLoc);
                }
            } else if (fileIsTyped && resp->isDefinition()) {
                addLocIfExists(gs, result, resp->isDefinition()->termLoc);
            } else if (fileIsTyped && resp->isSend()) {
                auto sendResp = resp->isSend();
                auto start = sendResp->dispatchResult.get();
                while (start != nullptr) {
                    if (start->main.method.exists() && !start->main.receiver->isUntyped()) {
                        addLocIfExists(gs, result, start->main.method.data(gs)->loc());
                    }
                    start = start->secondary.get();
                }
            }
        }
        response->result = move(result);
    }
    return LSPResult::make(move(response));
}

} // namespace sorbet::realmain::lsp
