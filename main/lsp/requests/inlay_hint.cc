#include "main/lsp/requests/inlay_hint.h"
#include "core/lsp/QueryResponse.h"
#include "main/lsp/LSPQuery.h"
#include "main/lsp/json_types.h"
#include "main/lsp/lsp.h"

#include "absl/algorithm/container.h"

using namespace std;

namespace sorbet::realmain::lsp {

InlayHintTask::InlayHintTask(const LSPConfiguration &config, MessageId id, std::unique_ptr<InlayHintParams> params)
    : LSPRequestTask(config, move(id), LSPMethod::TextDocumentInlayHint), params(move(params)) {}

bool InlayHintTask::isDelayable() const {
    return true;
}

unique_ptr<ResponseMessage> InlayHintTask::runRequest(LSPTypecheckerInterface &typechecker) {
    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::TextDocumentInlayHint);
    if (!config.opts.lspInlayHintsEnabled) {
        response->error =
            make_unique<ResponseError>((int)LSPErrorCodes::InvalidRequest,
                                       "The `Inlay Hint` LSP feature is experimental and disabled by default.");
        return response;
    }

    const core::GlobalState &gs = typechecker.state();
    auto &uri = params->textDocument->uri;
    auto result = LSPQuery::byLoc(config, typechecker, uri, *params->range, LSPMethod::TextDocumentInlayHint);
    if (result.error) {
        // An error happened while setting up the query.
        response->error = move(result.error);
        return response;
    }

    // An explicit null indicates that we don't support this request (or that nothing was at the location).
    // Note: Need to correctly type variant here so it goes into right 'slot' of result variant.
    response->result = variant<JSONNullObject, vector<unique_ptr<InlayHint>>>(JSONNullObject());
    auto fref = config.uri2FileRef(gs, uri);
    if (!fref.exists()) {
        return response;
    }

    vector<unique_ptr<InlayHint>> hints;
    auto &queryResponses = result.responses;
    for (auto &queryResponse : queryResponses) {
        // TODO(froydnj): figure out how to handle everything else.
        if (auto *ident = queryResponse->isIdent()) {
            auto position = Position::fromLoc(gs, ident->termLoc.copyWithZeroLength());
            if (position == nullptr) {
                continue;
            }

            auto label = ident->retType.type.show(gs);
            auto hint = make_unique<InlayHint>(move(position), label);
            // TODO(froydnj): is it worth trying to propagate enough information through this
            // to label things as `InlayHintKind::Parameter`?
            hint->kind = InlayHintKind::Type;
            // TODO(froydnj): What do we set tooltip to?  paddingLeft?  paddingRight?
            hint->tooltip = label;
            hints.emplace_back(move(hint));
        }
    }

    response->result = move(hints);
    return response;
}
} // namespace sorbet::realmain::lsp
