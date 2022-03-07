#include "main/lsp/requests/document_formatting.h"
#include "experimental/rubyfmt/rubyfmt.h"
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
        auto source = index.getFile(fref).source();
        auto formatResult = experimental::rubyfmt::format(source);
        switch (formatResult.status) {
            // Note: I use different line numbers for the below exceptions so they show up differently in crash logs.
            case experimental::rubyfmt::RubyfmtFormatError::RUBYFMT_FORMAT_ERROR_IO_ERROR:
                // Fatal error -- crash
                Exception::raise("Rubyfmt reported an IO error");
            case experimental::rubyfmt::RubyfmtFormatError::RUBYFMT_OTHER_RUBY_ERROR:
                // Fatal error -- crash
                Exception::raise("Rubyfmt reported a Ruby error");
            case experimental::rubyfmt::RubyfmtFormatError::RUBYFMT_INITIALIZE_ERROR:
                // Fatal error -- crash
                Exception::raise("Rubyfmt failed to initialize");
            case experimental::rubyfmt::RubyfmtFormatError::RUBYFMT_FORMAT_ERROR_RIPPER_PARSE_FAILURE:
            case experimental::rubyfmt::RubyfmtFormatError::RUBYFMT_FORMAT_ERROR_SYNTAX_ERROR:
                // Non-fatal error. Returns null for result.
                config.logger->debug("Rubyfmt returned non-fatal error code `{}` for file `{}`", formatResult.status,
                                     params->textDocument->uri);
                break;
            case experimental::rubyfmt::RubyfmtFormatError::RUBYFMT_FORMAT_ERROR_OK:
                // Construct text edit to replace entire document.
                vector<unique_ptr<TextEdit>> edits;
                // Note: VS Code uses 0-indexed lines, so the lineCount will be one more line than the size of the doc.
                edits.emplace_back(
                    make_unique<TextEdit>(make_unique<Range>(make_unique<Position>(0, 0),
                                                             make_unique<Position>(index.getFile(fref).lineCount(), 0)),
                                          formatResult.formatted));
                result = move(edits);
                break;
        }
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
