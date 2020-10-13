#include "main/lsp/requests/rename.h"
#include "absl/strings/match.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_split.h"
#include "core/lsp/QueryResponse.h"
#include "main/lsp/ShowOperation.h"
#include "main/lsp/json_types.h"
#include "main/lsp/lsp.h"
#include <stdio.h>

using namespace std;

namespace sorbet::realmain::lsp {

namespace {
bool isValidRenameLocation(const core::SymbolRef &symbol, const core::GlobalState &gs,
                           unique_ptr<ResponseMessage> &response) {
    auto locs = symbol.data(gs)->locs();
    string filetype;
    for (auto loc : locs) {
        if (loc.file().data(gs).isRBI()) {
            filetype = ".rbi";
        } else if (loc.file().data(gs).isPayload()) {
            filetype = "payload";
        }

        if (!filetype.empty()) {
            auto error =
                fmt::format("Renaming constants defined in {} files is not supported; symbol {} is defined at {}",
                            filetype, symbol.data(gs)->name.show(gs), loc.filePosToString(gs));
            response->error = make_unique<ResponseError>((int)LSPErrorCodes::InvalidRequest, error);

            return false;
        }
    }
    return true;
}
} // namespace

variant<JSONNullObject, unique_ptr<WorkspaceEdit>>
RenameTask::getRenameEdits(LSPTypecheckerDelegate &typechecker, core::SymbolRef symbol, std::string_view newName) {
    const core::GlobalState &gs = typechecker.state();
    vector<unique_ptr<Location>> references = getReferencesToSymbol(typechecker, symbol);
    auto originalName = symbol.data(gs)->name.toString(gs);
    auto we = make_unique<WorkspaceEdit>();

    UnorderedMap<string, vector<unique_ptr<TextEdit>>> edits;
    for (auto &location : references) {
        // Get text at location.
        auto fref = config.uri2FileRef(gs, location->uri);
        if (fref.data(gs).isPayload() || fref.data(gs).isRBI()) {
            // We don't support renaming things in payload files.
            return JSONNullObject();
        }

        auto loc = location->range->toLoc(gs, fref);
        auto source = loc.source(gs);
        std::vector<std::string> strs = absl::StrSplit(source, "::");
        strs[strs.size() - 1] = string(newName);
        edits[location->uri].push_back(make_unique<TextEdit>(move(location->range), absl::StrJoin(strs, "::")));
    }

    vector<unique_ptr<TextDocumentEdit>> textDocEdits;
    for (auto &item : edits) {
        // TODO: Version.
        textDocEdits.push_back(make_unique<TextDocumentEdit>(
            make_unique<VersionedTextDocumentIdentifier>(item.first, JSONNullObject()), move(item.second)));
    }
    we->documentChanges = move(textDocEdits);

    return we;
}

RenameTask::RenameTask(const LSPConfiguration &config, MessageId id, unique_ptr<RenameParams> params)
    : LSPRequestTask(config, move(id), LSPMethod::TextDocumentRename), params(move(params)) {}

unique_ptr<ResponseMessage> RenameTask::runRequest(LSPTypecheckerDelegate &typechecker) {
    const core::GlobalState &gs = typechecker.state();

    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::TextDocumentRename);
    if (!config.opts.lspRenameEnabled) {
        response->error = make_unique<ResponseError>(
            (int)LSPErrorCodes::InvalidRequest, "The `Rename` LSP feature is experimental and disabled by default.");
        return response;
    }

    prodCategoryCounterInc("lsp.messages.processed", "textDocument.rename");

    if (params->newName.empty()) {
        response->error =
            make_unique<ResponseError>((int)LSPErrorCodes::InvalidRequest, "No new name provided for rename request.");
        return response;
    }

    // Sanity check the text.
    if (islower(params->newName[0])) {
        response->error = make_unique<ResponseError>((int)LSPErrorCodes::InvalidRequest,
                                                     "Constant names must begin with an uppercase letter.");
        return response;
    }

    ShowOperation op(config, ShowOperation::Kind::Rename);

    auto result = queryByLoc(typechecker, params->textDocument->uri, *params->position, LSPMethod::TextDocumentRename);
    if (result.error) {
        // An error happened while setting up the query.
        response->error = move(result.error);
    } else {
        // An explicit null indicates that we don't support this request (or that nothing was at the location).
        // Note: Need to correctly type variant here so it goes into right 'slot' of result variant.
        response->result = variant<JSONNullObject, unique_ptr<WorkspaceEdit>>(JSONNullObject());
        auto &queryResponses = result.responses;
        if (!queryResponses.empty()) {
            auto resp = move(queryResponses[0]);
            // Only supports rename requests from constants and class definitions.
            if (auto constResp = resp->isConstant()) {
                if (isValidRenameLocation(constResp->symbol, gs, response)) {
                    response->result = getRenameEdits(typechecker, constResp->symbol, params->newName);
                }
            } else if (auto defResp = resp->isDefinition()) {
                if (defResp->symbol.data(gs)->isClassOrModule()) {
                    if (isValidRenameLocation(defResp->symbol, gs, response)) {
                        response->result = getRenameEdits(typechecker, defResp->symbol, params->newName);
                    }
                }
            }
        }
    }

    return response;
}

} // namespace sorbet::realmain::lsp
