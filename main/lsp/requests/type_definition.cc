#include "core/lsp/QueryResponse.h"
#include "main/lsp/lsp.h"

using namespace std;

namespace sorbet::realmain::lsp {
LSPResult LSPLoop::handleTextDocumentTypeDefinition(unique_ptr<core::GlobalState> gs, const MessageId &id,
                                                    const TextDocumentPositionParams &params) const {
    Exception::raise("TODO(jez) Implement this");
}
} // namespace sorbet::realmain::lsp
