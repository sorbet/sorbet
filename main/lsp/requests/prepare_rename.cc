#include "main/lsp/requests/prepare_rename.h"
#include "absl/strings/match.h"
#include "core/lsp/QueryResponse.h"
#include "main/lsp/json_types.h"
#include "main/lsp/lsp.h"

using namespace std;

namespace sorbet::realmain::lsp {
namespace {
variant<JSONNullObject, unique_ptr<PrepareRenameResult>> getPrepareRenameResult(const core::GlobalState &gs,
                                                                                core::SymbolRef symbol) {
    auto range = Range::fromLoc(gs, symbol.loc(gs));
    if (range == nullptr) {
        return JSONNullObject();
    }

    auto result = make_unique<PrepareRenameResult>(move(range));
    auto name = symbol.name(gs).show(gs);
    result->placeholder = name;
    return result;
}

variant<JSONNullObject, unique_ptr<PrepareRenameResult>>
getPrepareRenameResultForSend(const core::GlobalState &gs, const core::lsp::SendResponse *sendResp) {
    // The send expression is of the form <receiver>.<method><args>
    // We want to return the range and placeholder for the method part of the expression, because this is what the
    // editor will highlight in the rename UI.

    auto methodNameLoc = sendResp->getMethodNameLoc(gs);
    if (!methodNameLoc) {
        return JSONNullObject();
    }

    auto range = Range::fromLoc(gs, methodNameLoc.value());
    if (range == nullptr) {
        return JSONNullObject();
    }

    auto result = make_unique<PrepareRenameResult>(move(range));
    result->placeholder = methodNameLoc->source(gs);
    return result;
}

} // namespace

PrepareRenameTask::PrepareRenameTask(const LSPConfiguration &config, MessageId id,
                                     unique_ptr<TextDocumentPositionParams> params)
    : LSPRequestTask(config, move(id), LSPMethod::TextDocumentPrepareRename), params(move(params)) {}

unique_ptr<ResponseMessage> PrepareRenameTask::runRequest(LSPTypecheckerDelegate &typechecker) {
    const core::GlobalState &gs = typechecker.state();

    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::TextDocumentPrepareRename);

    prodCategoryCounterInc("lsp.messages.processed", "textDocument.prepareRename");
    auto result = queryByLoc(typechecker, params->textDocument->uri, *params->position,
                             LSPMethod::TextDocumentPrepareRename, false);
    if (result.error) {
        // An error happened while setting up the query.
        response->error = move(result.error);
        return response;
    }

    // An explicit null indicates that we don't support this request (or that nothing was at the location).
    // Note: Need to correctly type variant here so it goes into right 'slot' of result variant.
    response->result = variant<JSONNullObject, unique_ptr<PrepareRenameResult>>(JSONNullObject());
    auto &queryResponses = result.responses;
    if (queryResponses.empty()) {
        return response;
    }
    auto resp = move(queryResponses[0]);
    // We support rename requests from constants, class definitions, and methods.
    if (auto constResp = resp->isConstant()) {
        response->result = getPrepareRenameResult(gs, constResp->symbol);
    } else if (auto defResp = resp->isDefinition()) {
        if (defResp->symbol.isClassOrModule()) {
            response->result = getPrepareRenameResult(gs, defResp->symbol);
        } else if (defResp->symbol.data(gs)->isMethod()) {
            response->result = getPrepareRenameResult(gs, defResp->symbol);
        }
    } else if (auto sendResp = resp->isSend()) {
        response->result = getPrepareRenameResultForSend(gs, sendResp);
    }

    return response;
}

} // namespace sorbet::realmain::lsp
