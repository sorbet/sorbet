#include "absl/strings/match.h"
#include "core/lsp/QueryResponse.h"
#include "main/lsp/lsp.h"

using namespace std;

namespace sorbet::realmain::lsp {

void LSPLoop::handleTextDocumentReferences(rapidjson::Value &result, rapidjson::Document &d) {
    prodCategoryCounterInc("lsp.requests.processed", "textDocument.references");
    // we're postponing setting the type of `result` because we want to return `null` in case we don't handle that
    // location yet.

    if (auto run1 = setupLSPQueryByLoc(d, LSPMethod::TextDocumentCompletion(), false)) {
        auto &queryResponses = run1->responses;
        if (!queryResponses.empty()) {
            auto resp = move(queryResponses[0]);

            auto receiverType = resp->receiver.type;
            if (resp->kind == core::QueryResponse::Kind::CONSTANT && !resp->dispatchComponents.empty()) {
                auto symRef = resp->dispatchComponents[0].method;
                if (auto run2 = setupLSPQueryBySymbol(symRef, LSPMethod::TextDocumentRefernces())) {
                    result.SetArray();
                    auto &queryResponses = run2->responses;
                    for (auto &q : queryResponses) {
                        result.PushBack(loc2Location(q->termLoc), alloc);
                    }
                }

            } else if (resp->kind == core::QueryResponse::Kind::IDENT) {
                // TODO: find them in tree
            }
        }
        sendResult(d, result);
    }
}

} // namespace sorbet::realmain::lsp
