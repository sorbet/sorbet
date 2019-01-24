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
    prodCategoryCounterInc("lsp.requests.processed", "textDocument.definition");
    vector<unique_ptr<JSONBaseType>> result;
    auto finalGs = move(gs);
    auto run = setupLSPQueryByLoc(move(finalGs), id, params.textDocument->uri, *params.position,
                                  LSPMethod::TextDocumentDefinition(), true);
    finalGs = move(run.gs);
    auto &queryResponses = run.responses;
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
}

} // namespace sorbet::realmain::lsp
