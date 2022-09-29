#include "main/lsp/requests/prepare_rename.h"
#include "absl/strings/match.h"
#include "core/lsp/QueryResponse.h"
#include "main/lsp/LSPLoop.h"
#include "main/lsp/LSPQuery.h"
#include "main/lsp/json_types.h"

using namespace std;

namespace sorbet::realmain::lsp {
namespace {
variant<JSONNullObject, unique_ptr<PrepareRenameResult>> getPrepareRenameResult(const core::GlobalState &gs,
                                                                                core::SymbolRef symbol) {
    if (symbol.isMethod()) {
        auto def = symbol.loc(gs).source(gs);
        if (def.has_value()) {
            const vector<string> unsupportedDefPrefixes{"attr_reader", "attr_accessor", "attr_writer"};
            for (auto u : unsupportedDefPrefixes) {
                if (absl::StartsWith(*def, u)) {
                    return JSONNullObject();
                }
            }
        }
    }

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
getPrepareRenameResultForIdent(const core::GlobalState &gs, const core::lsp::IdentResponse *identResp) {
    if (!identResp->termLoc.exists()) {
        ENFORCE(false);
        return JSONNullObject();
    }

    // The loc for the instance variable local in `attr_reader :foo`
    // corresponds to `foo`, but we don't want to permit renames on such
    // things.  This is a heuristic to rule out such cases.
    auto adjusted = identResp->termLoc.adjust(gs, -1, 0);
    if (adjusted.exists()) {
        auto source = adjusted.source(gs);
        // Check for symbols and strings.
        if (source.has_value() && (source.value()[0] == ':' || source.value()[0] == '"' || source.value()[0] == '\'')) {
            return JSONNullObject();
        }
    }

    auto range = Range::fromLoc(gs, identResp->termLoc);
    if (range == nullptr) {
        return JSONNullObject();
    }

    auto result = make_unique<PrepareRenameResult>(move(range));
    result->placeholder = identResp->variable._name.show(gs);
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
    // TODO(froydnj): this returns `FUNC=` for setters from `attr_accessor`.  we
    // will disallow renaming such things in renaming proper, but it's not clear
    // what to do about setters that don't result from `attr_{writer,accessor}`.
    // Regardless, using the method name seems better than showing `FUNC =`,
    // which is what we would get from `methodNameLoc.source(gs)`.
    result->placeholder = sendResp->callerSideName.show(gs);
    return result;
}

} // namespace

PrepareRenameTask::PrepareRenameTask(const LSPConfiguration &config, MessageId id,
                                     unique_ptr<TextDocumentPositionParams> params)
    : LSPRequestTask(config, move(id), LSPMethod::TextDocumentPrepareRename), params(move(params)) {}

unique_ptr<ResponseMessage> PrepareRenameTask::runRequest(LSPTypecheckerInterface &typechecker) {
    const core::GlobalState &gs = typechecker.state();

    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::TextDocumentPrepareRename);

    prodCategoryCounterInc("lsp.messages.processed", "textDocument.prepareRename");
    auto result = LSPQuery::byLoc(config, typechecker, params->textDocument->uri, *params->position,
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
    if (auto constResp = resp->isConstant()) {
        response->result = getPrepareRenameResult(gs, constResp->symbolBeforeDealias);
    } else if (auto defResp = resp->isMethodDef()) {
        response->result = getPrepareRenameResult(gs, defResp->symbol);
    } else if (auto sendResp = resp->isSend()) {
        response->result = getPrepareRenameResultForSend(gs, sendResp);
    } else if (auto identResp = resp->isIdent()) {
        response->result = getPrepareRenameResultForIdent(gs, identResp);
    } else if (auto fieldResp = resp->isField()) {
        response->result = getPrepareRenameResult(gs, fieldResp->symbol);
    }

    return response;
}

} // namespace sorbet::realmain::lsp
