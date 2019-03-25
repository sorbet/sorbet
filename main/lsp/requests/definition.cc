#include "core/lsp/QueryResponse.h"
#include "main/lsp/lsp.h"

using namespace std;

namespace sorbet::realmain::lsp {
void LSPLoop::addLocIfExists(const core::GlobalState &gs, vector<unique_ptr<JSONBaseType>> &locs, core::Loc loc) {
    if (loc.file().exists()) {
        locs.push_back(loc2Location(gs, loc));
    }
}

unique_ptr<core::GlobalState> LSPLoop::handleTextDocumentDefinition(unique_ptr<core::GlobalState> gs,
                                                                    const MessageId &id,
                                                                    const TextDocumentPositionParams &params) {
    if (!opts.lspGoToDefinitionEnabled) {
        sendError(id, (int)LSPErrorCodes::InvalidRequest,
                  "The `Go To Definition` LSP feature is experimental and disabled by default.");
        return gs;
    }

    prodCategoryCounterInc("lsp.messages.processed", "textDocument.definition");
    auto result = setupLSPQueryByLoc(move(gs), params.textDocument->uri, *params.position,
                                     LSPMethod::TextDocumentDefinition(), true);
    if (auto run = get_if<TypecheckRun>(&result)) {
        auto finalGs = move(run->gs);
        auto &queryResponses = run->responses;
        vector<unique_ptr<JSONBaseType>> result;
        if (!queryResponses.empty()) {
            auto resp = move(queryResponses[0]);

            if (auto identResp = resp->isIdent()) {
                for (auto &originLoc : identResp->retType.origins) {
                    addLocIfExists(*finalGs, result, originLoc);
                }
            } else if (auto defResp = resp->isDefinition()) {
                result.push_back(loc2Location(*finalGs, defResp->termLoc));
            } else {
                for (auto &component : resp->getDispatchComponents()) {
                    if (component.method.exists()) {
                        addLocIfExists(*finalGs, result, component.method.data(*finalGs)->loc());
                    }
                }
            }
        }

        sendResponse(id, result);
        return finalGs;
    } else if (auto error = get_if<pair<unique_ptr<ResponseError>, unique_ptr<core::GlobalState>>>(&result)) {
        // An error happened while setting up the query.
        sendError(id, move(error->first));
        return move(error->second);
    } else {
        // Should never happen, but satisfy the compiler.
        ENFORCE(false, "Internal error: setupLSPQueryByLoc returned invalid value.");
        return nullptr;
    }
}

} // namespace sorbet::realmain::lsp
