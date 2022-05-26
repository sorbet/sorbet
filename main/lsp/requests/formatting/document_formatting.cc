#include "main/lsp/requests/formatting/document_formatting.h"
#include "common/common.h"
#include "main/lsp/LSPOutput.h"
#include "main/lsp/json_types.h"
#include "main/lsp/lsp.h"

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
    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::TextDocumentFormatting);
    if (!config.opts.lspDocumentFormatRubyfmtEnabled) {
        response->error = make_unique<ResponseError>(
            (int)LSPErrorCodes::InvalidRequest,
            "The `Document Formatting` LSP feature is experimental and disabled by default.");
        config.output->write(move(response));
        return;
    }

    variant<JSONNullObject, vector<unique_ptr<TextEdit>>> result = JSONNullObject();

    auto fref = index.uri2FileRef(params->textDocument->uri);
    if (fref.exists()) {
        // Ensure that `rubyfmt` is in the user's path
        auto rubyfmt_path = exec("which rubyfmt");
        if (rubyfmt_path.empty()) {
            Exception::raise("Could not find `rubyfmt` in PATH");
        }

        auto formattedContents = exec(fmt::format("rubyfmt {}", index.getFile(fref).path()));

        // Construct text edit to replace entire document.
        vector<unique_ptr<TextEdit>> edits;
        // Note: VS Code uses 0-indexed lines, so the lineCount will be one more line than the size of the doc.
        edits.emplace_back(make_unique<TextEdit>(
            make_unique<Range>(make_unique<Position>(0, 0), make_unique<Position>(index.getFile(fref).lineCount(), 0)),
            formattedContents));
        result = move(edits);
    }
    response->result = move(result);
    config.output->write(move(response));
    return;
}

// Since finalPhase is `preprocess`, this method should never be called.
unique_ptr<ResponseMessage> DocumentFormattingTask::runRequest(LSPTypecheckerInterface &typechecker) {
    Exception::raise("Unimplemented and unused");
}

} // namespace sorbet::realmain::lsp
