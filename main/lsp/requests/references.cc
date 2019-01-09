#include "absl/strings/match.h"
#include "core/lsp/QueryResponse.h"
#include "main/lsp/lsp.h"

using namespace std;

namespace sorbet::realmain::lsp {

unique_ptr<core::GlobalState> LSPLoop::handleTextDocumentReferences(unique_ptr<core::GlobalState> gs,
                                                                    rapidjson::Document &d) {
    prodCategoryCounterInc("lsp.requests.processed", "textDocument.references");

    auto finalGs = move(gs);
    auto run1 = setupLSPQueryByLoc(move(finalGs), d, LSPMethod::TextDocumentCompletion(), false);
    finalGs = move(run1.gs);
    auto &queryResponses = run1.responses;
    if (!queryResponses.empty()) {
        auto resp = move(queryResponses[0]);

        auto receiverType = resp->receiver.type;
        if (resp->kind == core::QueryResponse::Kind::CONSTANT && !resp->dispatchComponents.empty()) {
            auto symRef = resp->dispatchComponents[0].method;
            auto run2 = setupLSPQueryBySymbol(move(finalGs), symRef, LSPMethod::TextDocumentRefernces());
            finalGs = move(run2.gs);
            vector<unique_ptr<JSONBaseType>> result;
            auto &queryResponses = run2.responses;
            for (auto &q : queryResponses) {
                result.push_back(loc2Location(*finalGs, q->termLoc));
            }
            sendResult(d, result);
            return finalGs;
        } else if (resp->kind == core::QueryResponse::Kind::IDENT) {
            // TODO: find them in tree
        }
    }
    // An explicit null indicates that we don't support this request (or that nothing was at the location).
    rapidjson::Value nullValue;
    sendResult(d, nullValue);
    return finalGs;
}

} // namespace sorbet::realmain::lsp
