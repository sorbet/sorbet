#include "main/lsp/requests/inlay_hint.h"
#include "core/lsp/QueryResponse.h"
#include "main/lsp/LSPLoop.h"
#include "main/lsp/LSPQuery.h"
#include "main/lsp/json_types.h"

#include "absl/algorithm/container.h"

using namespace std;

namespace sorbet::realmain::lsp {

InlayHintTask::InlayHintTask(const LSPConfiguration &config, MessageId id,
                             std::unique_ptr<InlayHintParams> params)
    : LSPRequestTask(config, move(id), LSPMethod::TextDocumentInlayHint), params(move(params)) {}

bool InlayHintTask::isDelayable() const {
    return true;
}

unique_ptr<ResponseMessage> InlayHintTask::runRequest(LSPTypecheckerDelegate &typechecker) {
    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::TextDocumentInlayHint);
    if (!config.opts.lspInlayHintsEnabled) {
        response->error =
            make_unique<ResponseError>((int)LSPErrorCodes::InvalidRequest,
                                       "The `Inlay Hint` LSP feature is experimental and disabled by default.");
        return response;
    }

    return response;
}
}
