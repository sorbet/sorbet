#include "main/lsp/LSPOutput.h"
#include "main/lsp/json_types.h"
#include "main/lsp/lsp.h"
#include "main/lsp/requests/formatting/document_formatting.h"

using namespace std;

namespace sorbet::realmain::lsp {
DocumentFormattingTask::DocumentFormattingTask(const LSPConfiguration &config, MessageId id,
                                               std::unique_ptr<DocumentFormattingParams> params)
    : LSPRequestTask(config, move(id), LSPMethod::TextDocumentFormatting), params(move(params)) {}

// Processed on the index thread so it doesn't wait for typechecking.
LSPTask::Phase DocumentFormattingTask::finalPhase() const {
    return Phase::INDEX;
}

void DocumentFormattingTask::index(LSPIndexer &index) {
    // no-op in emscripten to references to `popen`
    return;
}

// Since finalPhase is `preprocess`, this method should never be called.
unique_ptr<ResponseMessage> DocumentFormattingTask::runRequest(LSPTypecheckerInterface &typechecker) {
    Exception::raise("Unimplemented and unused");
}

} // namespace sorbet::realmain::lsp
