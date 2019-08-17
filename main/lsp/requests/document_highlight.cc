
#include "absl/strings/match.h"
#include "core/lsp/QueryResponse.h"
#include "main/lsp/lsp.h"

using namespace std;

namespace sorbet::realmain::lsp {

pair<unique_ptr<core::GlobalState>, vector<unique_ptr<DocumentHighlight>>>
LSPLoop::getHighlightsToSymbolInFile(unique_ptr<core::GlobalState> gs, string_view const uri, core::SymbolRef symbol,
                                     vector<unique_ptr<DocumentHighlight>> highlights) const {
    if (symbol.exists()) {
        auto run2 = setupLSPQueryBySymbol(move(gs), symbol, uri);
        gs = move(run2.gs);
        auto locations = extractLocations(*gs, run2.responses);
        for (auto const &location : locations) {
            auto highlight = make_unique<DocumentHighlight>(move(location->range));
            highlights.push_back(move(highlight));
        }
    }
    return make_pair(move(gs), move(highlights));
}

vector<unique_ptr<DocumentHighlight>> locationsToDocumentHighlights(vector<unique_ptr<Location>> const locations) {
    vector<unique_ptr<DocumentHighlight>> highlights;
    for (auto const &location : locations) {
        auto highlight = make_unique<DocumentHighlight>(move(location->range));
        highlights.push_back(move(highlight));
    }
    return highlights;
}

LSPResult LSPLoop::handleTextDocumentDocumentHighlight(unique_ptr<core::GlobalState> gs, const MessageId &id,
                                                       const TextDocumentPositionParams &params) const {
    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::TextDocumentDocumentHighlight);
    prodCategoryCounterInc("lsp.messages.processed", "textDocument.documentHighlight");
    auto uri = params.textDocument->uri;
    auto result = setupLSPQueryByLoc(move(gs), uri, *params.position, LSPMethod::TextDocumentCompletion, false);
    gs = move(result.gs);
    if (result.error) {
        // An error happened while setting up the query.
        response->error = move(result.error);
    } else {
        // An explicit null indicates that we don't support this request (or that nothing was at the location).
        // Note: Need to correctly type variant here so it goes into right 'slot' of result variant.
        response->result = variant<JSONNullObject, vector<unique_ptr<DocumentHighlight>>>(JSONNullObject());
        auto &queryResponses = result.responses;
        if (!queryResponses.empty()) {
            const bool fileIsTyped = uri2FileRef(uri).data(*gs).strictLevel >= core::StrictLevel::True;
            auto resp = move(queryResponses[0]);
            // N.B.: Ignores literals.
            // If file is untyped, only supports find reference requests from constants and class definitions.
            if (auto constResp = resp->isConstant()) {
                std::vector<std::unique_ptr<Location>> locations;
                tie(gs, response->result) = getHighlightsToSymbolInFile(move(gs), uri, constResp->symbol);
            } else if (auto defResp = resp->isDefinition()) {
                if (fileIsTyped || defResp->symbol.data(*gs)->isClass()) {
                    std::vector<std::unique_ptr<Location>> locations;
                    tie(gs, response->result) = getHighlightsToSymbolInFile(move(gs), uri, defResp->symbol);
                }
            } else if (fileIsTyped && resp->isIdent()) {
                auto identResp = resp->isIdent();
                auto loc = identResp->owner.data(*gs)->loc();
                if (loc.exists()) {
                    auto run2 =
                        runQuery(move(gs), core::lsp::Query::createVarQuery(identResp->owner, identResp->variable),
                                 {loc.file()});
                    gs = move(run2.gs);
                    auto locations = extractLocations(*gs, run2.responses);
                    response->result = locationsToDocumentHighlights(move(locations));
                }
            } else if (fileIsTyped && resp->isSend()) {
                auto sendResp = resp->isSend();
                auto start = sendResp->dispatchResult.get();
                vector<unique_ptr<DocumentHighlight>> highlights;
                while (start != nullptr) {
                    if (start->main.method.exists() && !start->main.receiver->isUntyped()) {
                        tie(gs, highlights) =
                            getHighlightsToSymbolInFile(move(gs), uri, start->main.method, move(highlights));
                    }
                    start = start->secondary.get();
                }
                response->result = move(highlights);
            }
        }
    }
    return LSPResult::make(move(gs), move(response));
}

} // namespace sorbet::realmain::lsp
