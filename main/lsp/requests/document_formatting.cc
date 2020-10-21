#include "main/lsp/requests/document_formatting.h"
#include "experimental/rubyfmt/rubyfmt.h"
#include "main/lsp/json_types.h"
#include "main/lsp/lsp.h"

using namespace std;

namespace sorbet::realmain::lsp {
DocumentFormattingTask::DocumentFormattingTask(const LSPConfiguration &config, MessageId id,
                                               std::unique_ptr<DocumentFormattingParams> params)
    : LSPRequestTask(config, move(id), LSPMethod::TextDocumentFormatting), params(move(params)) {}

unique_ptr<ResponseMessage> DocumentFormattingTask::runRequest(LSPTypecheckerDelegate &typechecker) {
    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::TextDocumentFormatting);
    if (!config.opts.lspDocumentFormatRubyfmtEnabled) {
        response->error = make_unique<ResponseError>(
            (int)LSPErrorCodes::InvalidRequest,
            "The `Document Formatting` LSP feature is experimental and disabled by default.");
        return response;
    }

    variant<JSONNullObject, vector<unique_ptr<TextEdit>>> result = JSONNullObject();

    auto fref = config.uri2FileRef(typechecker.state(), params->textDocument->uri);
    if (fref.exists()) {
        auto formatResult = experimental::rubyfmt::format(fref.data(typechecker.state()).source());
        switch (formatResult.status) {
            // Note: I use different line numbers for the below exceptions so they show up differently in crash logs.
            case Rubyfmt_FormatError::RUBYFMT_FORMAT_ERROR_IO_ERROR:
                // Fatal error -- crash
                Exception::raise("Rubyfmt reported an IO error");
            case Rubyfmt_FormatError::RUBYFMT_OTHER_RUBY_ERROR:
                // Fatal error -- crash
                Exception::raise("Rubyfmt reported a Ruby error");
            case Rubyfmt_FormatError::RUBYFMT_FORMAT_ERROR_RIPPER_PARSE_FAILURE:
            case Rubyfmt_FormatError::RUBYFMT_FORMAT_ERROR_SYNTAX_ERROR:
                // Non-fatal error. Returns null for result.
                config.logger->debug("Rubyfmt returned non-fatal error code `{}` for file `{}`", formatResult.status,
                                     fref.data(typechecker.state()).path());
                break;
            case Rubyfmt_FormatError::RUBYFMT_FORMAT_ERROR_OK:
                // Construct text edit to replace entire document.
                core::Loc loc(fref, 0, fref.data(typechecker.state()).source().length());
                vector<unique_ptr<TextEdit>> edits;
                edits.emplace_back(
                    make_unique<TextEdit>(Range::fromLoc(typechecker.state(), loc), move(formatResult.formatted)));
                response->result = move(edits);
                break;
        }
    }
    return response;
}
} // namespace sorbet::realmain::lsp
