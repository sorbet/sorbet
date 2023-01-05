#include "main/lsp/requests/document_highlight.h"
#include "absl/strings/match.h"
#include "core/lsp/QueryResponse.h"
#include "main/lsp/LSPQuery.h"
#include "main/lsp/json_types.h"

using namespace std;

namespace sorbet::realmain::lsp {

namespace {
vector<unique_ptr<DocumentHighlight>> locationsToDocumentHighlights(string_view uri,
                                                                    vector<unique_ptr<Location>> const locations) {
    vector<unique_ptr<DocumentHighlight>> highlights;
    for (auto const &location : locations) {
        // The query may pick up secondary files required for accurate querying (e.g., package files)
        if (location->uri == uri) {
            auto highlight = make_unique<DocumentHighlight>(move(location->range));
            highlights.push_back(move(highlight));
        }
    }
    return highlights;
}
} // namespace

DocumentHighlightTask::DocumentHighlightTask(const LSPConfiguration &config, MessageId id,
                                             unique_ptr<TextDocumentPositionParams> params)
    : LSPRequestTask(config, move(id), LSPMethod::TextDocumentDocumentHighlight), params(move(params)) {}

unique_ptr<ResponseMessage> DocumentHighlightTask::runRequest(LSPTypecheckerDelegate &typechecker) {
    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::TextDocumentDocumentHighlight);
    if (!config.opts.lspDocumentHighlightEnabled) {
        response->error = make_unique<ResponseError>(
            (int)LSPErrorCodes::InvalidRequest, "The `Highlight` LSP feature is experimental and disabled by default.");
        return response;
    }

    const core::GlobalState &gs = typechecker.state();
    auto uri = params->textDocument->uri;
    auto result = LSPQuery::byLoc(config, typechecker, params->textDocument->uri, *params->position,
                                  LSPMethod::TextDocumentDocumentHighlight, false);
    if (result.error) {
        // An error happened while setting up the query.
        response->error = move(result.error);
        return response;
    }

    // An explicit null indicates that we don't support this request (or that nothing was at the location).
    // Note: Need to correctly type variant here so it goes into right 'slot' of result variant.
    response->result = variant<JSONNullObject, vector<unique_ptr<DocumentHighlight>>>(JSONNullObject());
    auto fref = config.uri2FileRef(typechecker.state(), uri);
    if (!fref.exists()) {
        return response;
    }
    auto &queryResponses = result.responses;
    if (!queryResponses.empty()) {
        auto file = config.uri2FileRef(gs, uri);
        if (!file.exists()) {
            return response;
        }
        const bool fileIsTyped = file.data(gs).strictLevel >= core::StrictLevel::True;
        auto resp = move(queryResponses[0]);
        // N.B.: Ignores literals.
        // If file is untyped, only supports find reference requests from constants and class definitions.
        if (auto constResp = resp->isConstant()) {
            response->result =
                getHighlights(typechecker, getReferencesToSymbolInFile(typechecker, fref, constResp->symbol));
        } else if (auto fieldResp = resp->isField()) {
            // This could be a `prop` or `attr_*`, which have multiple associated symbols.
            response->result = getHighlights(
                typechecker, getReferencesToAccessorInFile(typechecker, fref,
                                                           getAccessorInfo(typechecker.state(), fieldResp->symbol),
                                                           fieldResp->symbol));
        } else if (auto defResp = resp->isMethodDef()) {
            if (fileIsTyped) {
                // This could be a `prop` or `attr_*`, which have multiple associated symbols.
                response->result = getHighlights(
                    typechecker, getReferencesToAccessorInFile(typechecker, fref,
                                                               getAccessorInfo(typechecker.state(), defResp->symbol),
                                                               defResp->symbol));
            }
        } else if (fileIsTyped && resp->isIdent()) {
            auto identResp = resp->isIdent();
            auto loc = identResp->termLoc;
            if (loc.exists()) {
                auto run2 = typechecker.query(core::lsp::Query::createVarQuery(identResp->enclosingMethod,
                                                                               identResp->enclosingMethodLoc,
                                                                               identResp->variable),
                                              {loc.file()});
                auto locations = extractLocations(gs, run2.responses);
                response->result = locationsToDocumentHighlights(uri, move(locations));
            }
        } else if (fileIsTyped && resp->isSend()) {
            auto sendResp = resp->isSend();
            auto start = sendResp->dispatchResult.get();
            vector<unique_ptr<core::lsp::QueryResponse>> references;
            while (start != nullptr) {
                if (start->main.method.exists() && !start->main.receiver.isUntyped()) {
                    // This could be a `prop` or `attr_*`, which have multiple associated symbols.
                    references = getReferencesToAccessorInFile(typechecker, fref,
                                                               getAccessorInfo(typechecker.state(), start->main.method),
                                                               start->main.method, move(references));
                }
                start = start->secondary.get();
            }
            response->result = getHighlights(typechecker, references);
        }
    }
    return response;
}

} // namespace sorbet::realmain::lsp
