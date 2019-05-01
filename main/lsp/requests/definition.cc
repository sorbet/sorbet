#include "core/lsp/QueryResponse.h"
#include "main/lsp/lsp.h"

using namespace std;

namespace sorbet::realmain::lsp {
void LSPLoop::addLocIfExists(const core::GlobalState &gs, vector<unique_ptr<Location>> &locs, core::Loc loc) {
    if (loc.file().exists()) {
        locs.push_back(loc2Location(gs, loc));
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
                                     LSPMethod::TextDocumentDefinition, true);
    if (auto run = get_if<TypecheckRun>(&result)) {
        gs = move(run->gs);
        auto &queryResponses = run->responses;
        vector<unique_ptr<Location>> result;
        if (!queryResponses.empty()) {
            auto resp = move(queryResponses[0]);

            if (auto identResp = resp->isIdent()) {
                for (auto &originLoc : identResp->retType.origins) {
                    addLocIfExists(*gs, result, originLoc);
                }
            } else if (auto defResp = resp->isDefinition()) {
                result.push_back(loc2Location(*gs, defResp->termLoc));
            } else {
                for (auto &component : resp->getDispatchComponents()) {
                    if (component.method.exists()) {
                        addLocIfExists(*gs, result, component.method.data(*gs)->loc());
                    }
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
