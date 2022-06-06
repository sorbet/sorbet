#include "main/lsp/requests/document_formatting.h"
#include "main/lsp/LSPOutput.h"
#include "main/lsp/json_types.h"
#include "main/lsp/lsp.h"
#include "subprocess.hpp"

using namespace std;

namespace sorbet::realmain::lsp {
DocumentFormattingTask::DocumentFormattingTask(const LSPConfiguration &config, MessageId id,
                                               std::unique_ptr<DocumentFormattingParams> params)
    : LSPRequestTask(config, move(id), LSPMethod::TextDocumentFormatting), params(move(params)) {}

// Processed on the index thread so it doesn't wait for typechecking.
LSPTask::Phase DocumentFormattingTask::finalPhase() const {
    return Phase::INDEX;
}

enum RubyfmtStatus {
    OK = 0,

    // passed buffer contained a ruby syntax error. Non fatal, user should feel
    // free to continue to call rubyfmt with non-error strings.
    SYNTAX_ERROR = 1,

    // this error is fatal, the calling program should not continue to execute
    // rubyfmt and you should report a bug with the file that crashed rubyfmt
    RIPPER_PARSE_FAILURE = 2,

    // an error occured during IO within the function, should be impossible
    // and most likely indicates a programming error within rubyfmt, please
    // file a bug
    IO_ERROR = 3,

    // some unknown ruby error occured during execution fo Rubyfmt. This indicates
    // a programming error. Please file a bug report and terminate the process
    // and restart.
    OTHER_RUBY_ERROR = 4,

    // Rubyformat failed to initialize.
    // N.B. 101 is the default error code for _any_ Rust panic,
    // but a failure to initialize is the most probable cause for this.
    INITIALIZE_ERROR = 101,

    RUBYFMT_NOT_IN_PATH = 127,
};

void DocumentFormattingTask::displayError(string errorMessage, unique_ptr<ResponseMessage> &response) {
    // Write to both the response (which will get written to the LSP log) and
    // to config.output (which will show a popup in the user's window)
    response->error = make_unique<ResponseError>((int)LSPErrorCodes::RequestFailed, errorMessage);
    config.output->write(make_unique<NotificationMessage>(
        "2.0", LSPMethod::WindowShowMessage, make_unique<ShowMessageParams>(MessageType::Error, errorMessage)));
}

bool DocumentFormattingTask::documentIsFormattable(string_view path) {
    if (config.opts.rubyfmtFormattableDirectories.empty()) {
        return true;
    }
    for (string directory : config.opts.rubyfmtFormattableDirectories) {
        if (path.find(directory) == 0 || path.find(fmt::format("./{}", directory)) == 0) {
            return true;
        }
    }
    return false;
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
        vector<unique_ptr<TextEdit>> edits;

        if (!documentIsFormattable(index.getFile(fref).path())) {
            // If the file isn't allowlisted, return no edits and exit
            result = move(edits);
            response->result = move(result);
            config.output->write(move(response));
            return;
        }

        auto sourceView = index.getFile(fref).source();
        auto process = subprocess::Popen({config.opts.rubyfmtPath}, subprocess::output{subprocess::PIPE},
                                         subprocess::input{subprocess::PIPE});
        auto processResponse = process.communicate(vector<char>(sourceView.begin(), sourceView.end()));
        process.wait();
        auto returnCode = process.retcode();

        std::string formattedContents(processResponse.first.buf.begin(), processResponse.first.buf.end());

        switch (returnCode) {
            case RubyfmtStatus::OK:
                // Construct text edit to replace entire document.
                // Note: VS Code uses 0-indexed lines, so the lineCount will be one more line than the size of the
                // doc.
                edits.emplace_back(
                    make_unique<TextEdit>(make_unique<Range>(make_unique<Position>(0, 0),
                                                             make_unique<Position>(index.getFile(fref).lineCount(), 0)),
                                          formattedContents));
                result = move(edits);
                response->result = move(result);
                break;
            case RubyfmtStatus::SYNTAX_ERROR:
                displayError(fmt::format("`rubyfmt` could not format {} because it contains syntax errors.",
                                         index.getFile(fref).path()),
                             response);
                break;
            case RubyfmtStatus::RIPPER_PARSE_FAILURE:
                displayError(fmt::format("`rubyfmt` failed to deserialize the parse tree from Ripper for {}.\n"
                                         "This is valid Ruby but represents a bug in `rubyfmt`.",
                                         index.getFile(fref).path()),
                             response);
                break;
            case RubyfmtStatus::IO_ERROR:
                displayError(fmt::format("`rubyfmt` encountered an IO error while writing {}.\n"
                                         "This is likely a bug in `rubyfmt`.",
                                         index.getFile(fref).path()),
                             response);
                break;
            case RubyfmtStatus::OTHER_RUBY_ERROR:
                displayError(fmt::format("`rubyfmt` encountered a ruby error while writing {}.\n"
                                         "This is likely a bug in `rubyfmt`.",
                                         index.getFile(fref).path()),
                             response);
                break;
            case RubyfmtStatus::INITIALIZE_ERROR:
                displayError("`rubyfmt` failed to initialize.", response);
                break;
            case RubyfmtStatus::RUBYFMT_NOT_IN_PATH:
                displayError("`rubyfmt` could not be found. Ensure that it is properly configured in your PATH.",
                             response);
                break;
            default:
                displayError(fmt::format("`rubyfmt` encountered an unknown exception (exit code {}).", returnCode),
                             response);
                break;
        }
    }
    config.output->write(move(response));
    return;
}

// Since finalPhase is `preprocess`, this method should never be called.
unique_ptr<ResponseMessage> DocumentFormattingTask::runRequest(LSPTypecheckerInterface &typechecker) {
    Exception::raise("Unimplemented and unused");
}

} // namespace sorbet::realmain::lsp
