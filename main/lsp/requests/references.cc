#include "absl/strings/match.h"
#include "core/lsp/QueryResponse.h"
#include "main/lsp/lsp.h"

using namespace std;

namespace sorbet::realmain::lsp {

unique_ptr<core::GlobalState> LSPLoop::handleTextDocumentReferences(unique_ptr<core::GlobalState> gs,
                                                                    const MessageId &id,
                                                                    const ReferenceParams &params) {
    prodCategoryCounterInc("lsp.requests.processed", "textDocument.references");

    auto finalGs = move(gs);
    auto run1 = setupLSPQueryByLoc(move(finalGs), id, params.textDocument->uri, *params.position,
                                   LSPMethod::TextDocumentCompletion(), false);
    finalGs = move(run1.gs);
    auto &queryResponses = run1.responses;
    if (!queryResponses.empty()) {
        auto resp = move(queryResponses[0]);

        if (auto constResp = resp->isConstant()) {
            if (!constResp->dispatchComponents.empty()) {
                auto symRef = constResp->dispatchComponents[0].method;
                auto run2 = setupLSPQueryBySymbol(move(finalGs), symRef, LSPMethod::TextDocumentRefernces());
                finalGs = move(run2.gs);
                vector<unique_ptr<JSONBaseType>> result;
                auto &queryResponses = run2.responses;
                for (auto &q : queryResponses) {
                    result.push_back(loc2Location(*finalGs, q->getLoc()));
                }
                sendResponse(id, result);
                return finalGs;
            }
        }
    }
    // An explicit null indicates that we don't support this request (or that nothing was at the location).
    sendNullResponse(id);
    return finalGs;
}

} // namespace sorbet::realmain::lsp
