#include "main/lsp/requests/references.h"
#include "core/lsp/QueryResponse.h"
#include "main/lsp/LSPQuery.h"
#include "main/lsp/ShowOperation.h"
#include "main/lsp/json_types.h"

using namespace std;

namespace sorbet::realmain::lsp {

ReferencesTask::ReferencesTask(const LSPConfiguration &config, MessageId id, std::unique_ptr<ReferenceParams> params)
    : LSPRequestTask(config, move(id), LSPMethod::TextDocumentReferences), params(move(params)) {}

bool ReferencesTask::needsMultithreading(const LSPIndexer &indexer) const {
    return true;
}

unique_ptr<ResponseMessage> ReferencesTask::runRequest(LSPTypecheckerInterface &typechecker) {
    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::TextDocumentReferences);
    ShowOperation op(config, ShowOperation::Kind::References);

    const core::GlobalState &gs = typechecker.state();
    auto result = queryByLoc(config, typechecker, params->textDocument->uri, *params->position,
                             LSPMethod::TextDocumentReferences, false);
    if (result.error) {
        // An error happened while setting up the query.
        response->error = move(result.error);
        return response;
    }

    // An explicit null indicates that we don't support this request (or that nothing was at the location).
    // Note: Need to correctly type variant here so it goes into right 'slot' of result variant.
    response->result = variant<JSONNullObject, vector<unique_ptr<Location>>>(JSONNullObject());
    auto &queryResponses = result.responses;
    if (!queryResponses.empty()) {
        const bool fileIsTyped =
            config.uri2FileRef(gs, params->textDocument->uri).data(gs).strictLevel >= core::StrictLevel::True;
        auto resp = move(queryResponses[0]);
        // N.B.: Ignores literals.
        // If file is untyped, only supports find reference requests from constants and class definitions.
        if (auto constResp = resp->isConstant()) {
            response->result =
                extractLocations(typechecker.state(), getReferencesToSymbol(typechecker, constResp->symbol));
        } else if (auto fieldResp = resp->isField()) {
            // This could be a `prop` or `attr_*`, which have multiple associated symbols.
            response->result = extractLocations(
                typechecker.state(),
                getReferencesToAccessor(typechecker, getAccessorInfo(typechecker.state(), fieldResp->symbol),
                                        fieldResp->symbol));
        } else if (auto defResp = resp->isMethodDef()) {
            if (fileIsTyped) {
                // This could be a `prop` or `attr_*`, which have multiple associated symbols.
                response->result = extractLocations(
                    typechecker.state(),
                    getReferencesToAccessor(typechecker, getAccessorInfo(typechecker.state(), defResp->symbol),
                                            defResp->symbol));
            }
        } else if (fileIsTyped && resp->isIdent()) {
            auto identResp = resp->isIdent();
            auto loc = identResp->termLoc;
            if (loc.exists()) {
                auto run2 = typechecker.query(
                    core::lsp::Query::createVarQuery(identResp->enclosingMethod, identResp->variable), {loc.file()});
                response->result = extractLocations(gs, run2.responses);
            }
        } else if (fileIsTyped && resp->isSend()) {
            auto sendResp = resp->isSend();
            auto start = sendResp->dispatchResult.get();
            vector<unique_ptr<core::lsp::QueryResponse>> responses;
            while (start != nullptr) {
                if (start->main.method.exists() && !start->main.receiver.isUntyped()) {
                    // This could be a `prop` or `attr_*`, which has multiple associated symbols.
                    responses =
                        getReferencesToAccessor(typechecker, getAccessorInfo(typechecker.state(), start->main.method),
                                                start->main.method, move(responses));
                }
                start = start->secondary.get();
            }
            response->result = extractLocations(typechecker.state(), responses);
        }
    }
    return response;
}

} // namespace sorbet::realmain::lsp
