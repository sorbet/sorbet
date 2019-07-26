#include "core/lsp/QueryResponse.h"
#include "main/lsp/lsp.h"

using namespace std;

namespace sorbet::realmain::lsp {
void LSPLoop::addLocIfExists(const core::GlobalState &gs, vector<unique_ptr<Location>> &locs, core::Loc loc) {
    if (loc.file().exists()) {
        locs.push_back(loc2Location(gs, loc, true));
    }
}

LSPResult LSPLoop::handleTextDocumentDefinition(unique_ptr<core::GlobalState> gs, const MessageId &id,
                                                const TextDocumentPositionParams &params) {
    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::TextDocumentDefinition);
    if (!opts.lspGoToDefinitionEnabled) {
        response->error =
            make_unique<ResponseError>((int)LSPErrorCodes::InvalidRequest,
                                       "The `Go To Definition` LSP feature is experimental and disabled by default.");
        return LSPResult::make(move(gs), move(response));
    }

    prodCategoryCounterInc("lsp.messages.processed", "textDocument.definition");
    auto result = setupLSPQueryByLoc(move(gs), params.textDocument->uri, *params.position,
                                     LSPMethod::TextDocumentDefinition, false);
    if (auto run = get_if<TypecheckRun>(&result)) {
        gs = move(run->gs);
        auto &queryResponses = run->responses;
        vector<unique_ptr<Location>> result;
        if (!queryResponses.empty()) {
            const bool fileIsTyped =
                uri2FileRef(params.textDocument->uri).data(*gs).strictLevel >= core::StrictLevel::True;
            auto resp = move(queryResponses[0]);

            // Only support go-to-definition on constants in untyped files.
            if (resp->isConstant() || (fileIsTyped && (resp->isIdent() || resp->isLiteral()))) {
                auto retType = resp->getTypeAndOrigins();
                for (auto &originLoc : retType.origins) {
                    addLocIfExists(*gs, result, originLoc);
                }
            } else if (fileIsTyped && resp->isDefinition()) {
                result.push_back(loc2Location(*gs, resp->isDefinition()->termLoc, true));
            } else if (fileIsTyped && resp->isSend()) {
                auto sendResp = resp->isSend();
                auto start = sendResp->dispatchResult.get();
                while (start != nullptr) {
                    if (start->main.method.exists() && !start->main.receiver->isUntyped()) {
                        addLocIfExists(*gs, result, start->main.method.data(*gs)->loc());
                    }
                    start = start->secondary.get();
                }
            }
        }
        response->result = move(result);
    } else if (auto error = get_if<pair<unique_ptr<ResponseError>, unique_ptr<core::GlobalState>>>(&result)) {
        // An error happened while setting up the query.
        response->error = move(error->first);
        gs = move(error->second);
    } else {
        // Should never happen, but satisfy the compiler.
        ENFORCE(false, "Internal error: setupLSPQueryByLoc returned invalid value.");
    }
    return LSPResult::make(move(gs), move(response));
}

} // namespace sorbet::realmain::lsp
