#include "absl/strings/match.h"
#include "core/lsp/QueryResponse.h"
#include "main/lsp/lsp.h"

using namespace std;

namespace sorbet::realmain::lsp {

unique_ptr<core::GlobalState> LSPLoop::handleTextDocumentReferences(unique_ptr<core::GlobalState> gs,
                                                                    const MessageId &id,
                                                                    const ReferenceParams &params) {
    if (!opts.lspFindReferencesEnabled) {
        sendError(id, (int)LSPErrorCodes::InvalidRequest,
                  "The `Find References` LSP feature is experimental and disabled by default.");
        return gs;
    }

    prodCategoryCounterInc("lsp.messages.processed", "textDocument.references");

    auto result = setupLSPQueryByLoc(move(gs), params.textDocument->uri, *params.position,
                                     LSPMethod::TextDocumentCompletion, false);
    if (auto run1 = get_if<TypecheckRun>(&result)) {
        auto finalGs = move(run1->gs);
        auto &queryResponses = run1->responses;
        if (!queryResponses.empty()) {
            auto resp = move(queryResponses[0]);

            if (auto constResp = resp->isConstant()) {
                if (!constResp->dispatchComponents.empty()) {
                    auto symRef = constResp->dispatchComponents[0].method;
                    auto run2 = setupLSPQueryBySymbol(move(finalGs), symRef);
                    finalGs = move(run2.gs);
                    vector<unique_ptr<JSONBaseType>> result;
                    auto &queryResponses = run2.responses;
                    for (auto &q : queryResponses) {
                        result.push_back(loc2Location(*finalGs, q->getLoc()));
                    }
                    sendResponse(id, result);
                    return finalGs;
                }
            } else if (auto identResp = resp->isIdent()) {
                std::vector<std::shared_ptr<core::File>> files;
                auto run2 =
                    runLSPQuery(move(finalGs), core::lsp::Query::createVarQuery(identResp->owner, identResp->variable),
                                files, true);
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
        // An explicit null indicates that we don't support this request (or that nothing was at the location).
        sendNullResponse(id);
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
