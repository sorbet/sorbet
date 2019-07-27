#include "core/lsp/QueryResponse.h"
#include "main/lsp/lsp.h"

using namespace std;

namespace sorbet::realmain::lsp {
LSPResult LSPLoop::handleTextDocumentCodeAction(unique_ptr<core::GlobalState> gs, const MessageId &id,
                                                const TextDocumentCodeActionParams &params) {
    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::TextDocumentCodeAction);

    prodCategoryCounterInc("lsp.messages.processed", "textDocument.codeAction");

    return LSPResult::make(move(gs), move(response));
}
} // namespace sorbet::realmain::lsp
