#include "main/lsp/requests/prepare_rename.h"
#include "absl/strings/match.h"
#include "core/lsp/QueryResponse.h"
#include "main/lsp/json_types.h"
#include "main/lsp/lsp.h"

using namespace std;

namespace sorbet::realmain::lsp {
unique_ptr<PrepareRenameResult> getPrepareRenameResult(const core::GlobalState &gs, core::SymbolRef symbol) {
    auto range = Range::fromLoc(gs, symbol.data(gs)->loc());
    auto result = make_unique<PrepareRenameResult>(move(range));
    auto name = symbol.data(gs)->name.show(gs);
    result->placeholder = name;
    return result;
}

PrepareRenameTask::PrepareRenameTask(const LSPConfiguration &config, MessageId id,
                                     unique_ptr<TextDocumentPositionParams> params)
    : LSPRequestTask(config, move(id), LSPMethod::TextDocumentPrepareRename), params(move(params)) {}

unique_ptr<ResponseMessage> PrepareRenameTask::runRequest(LSPTypecheckerDelegate &typechecker) {
    const core::GlobalState &gs = typechecker.state();

    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::TextDocumentPrepareRename);
    if (!config.opts.lspRenameEnabled) {
        response->error = make_unique<ResponseError>(
            (int)LSPErrorCodes::InvalidRequest, "The `Rename` LSP feature is experimental and disabled by default.");
        return response;
    }

    prodCategoryCounterInc("lsp.messages.processed", "textDocument.prepareRename");
    auto result = queryByLoc(typechecker, params->textDocument->uri, *params->position,
                             LSPMethod::TextDocumentPrepareRename, false);

    if (result.error) {
        // An error happened while setting up the query.
        response->error = move(result.error);
    } else {
        // An explicit null indicates that we don't support this request (or that nothing was at the location).
        // Note: Need to correctly type variant here so it goes into right 'slot' of result variant.
        response->result = variant<JSONNullObject, unique_ptr<PrepareRenameResult>>(JSONNullObject());
        auto &queryResponses = result.responses;
        if (!queryResponses.empty()) {
            auto resp = move(queryResponses[0]);
            // Only supports rename requests from constants and class definitions.
            if (auto constResp = resp->isConstant()) {
                response->result = getPrepareRenameResult(gs, constResp->symbol);
            } else if (auto defResp = resp->isDefinition()) {
                if (defResp->symbol.data(gs)->isClassOrModule()) {
                    response->result = getPrepareRenameResult(gs, defResp->symbol);
                }
            }
        }
    }
    return response;
}

} // namespace sorbet::realmain::lsp
