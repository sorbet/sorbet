#include "main/lsp/requests/document_formatting.h"
#include "common/FileOps.h"
#include "common/Subprocess.h"
#include "common/common.h"
#include "core/Files.h"
#include "main/lsp/LSPLoop.h"
#include "main/lsp/LSPOutput.h"
#include "main/lsp/json_types.h"

using namespace std;

namespace sorbet::realmain::lsp {
DocumentFormattingTask::DocumentFormattingTask(const LSPConfiguration &config, MessageId id,
                                               std::unique_ptr<DocumentFormattingParams> params)
    : LSPRequestTask(config, move(id), LSPMethod::TextDocumentFormatting), params(move(params)) {}

// Processed on the preprocess thread so it doesn't wait for typechecking.
LSPTask::Phase DocumentFormattingTask::finalPhase() const {
    return Phase::PREPROCESS;
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

void DocumentFormattingTask::preprocess(LSPPreprocessor &preprocessor) {
    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::TextDocumentFormatting);
    if (!config.opts.lspDocumentFormatRubyfmtEnabled) {
        response->error = make_unique<ResponseError>(
            (int)LSPErrorCodes::InvalidRequest,
            "The `Document Formatting` LSP feature is experimental and disabled by default.");
        config.output->write(move(response));
        return;
    }

    variant<JSONNullObject, vector<unique_ptr<TextEdit>>> result = JSONNullObject();
    vector<unique_ptr<TextEdit>> edits;

    auto path = config.remoteName2Local(params->textDocument->uri);

    auto maybeFileContents = preprocessor.maybeGetFileContents(path);
    string_view sourceView;
    if (maybeFileContents.has_value()) {
        sourceView = maybeFileContents.value();
    } else if (sorbet::FileOps::exists(path)) {
        // If the requested file path isn't in the workspace,
        // we won't be able to load it, in which case
        // we leave sourceView as empty and this becomes a no-op

        // In this case, the request is for a file that's
        // not open in the IDE, so we read it from disk instead
        sourceView = sorbet::FileOps::read(path);
    }

    // Don't format `__package.rb` files, since currently formatting them
    // can potentially break some pay-server tooling
    if (!sourceView.empty() && !core::File::isPackagePath(path) && !core::File::isRBIPath(path)) {
        auto originalLineCount = findLineBreaks(sourceView).size() - 1;
        auto processResponse = sorbet::Subprocess::spawn(config.opts.rubyfmtPath, vector<string>(), sourceView);

        auto returnCode = processResponse->status;
        auto formattedContents = processResponse->output;

        switch (returnCode) {
            case RubyfmtStatus::OK:
                // Construct text edit to replace entire document.
                // Note: VS Code uses 0-indexed lines, so the lineCount will be one more line than the size of the
                // doc.
                edits.emplace_back(make_unique<TextEdit>(
                    make_unique<Range>(make_unique<Position>(0, 0), make_unique<Position>(originalLineCount, 0)),
                    formattedContents));
                result = move(edits);
                response->result = move(result);
                break;
            case RubyfmtStatus::SYNTAX_ERROR:
                // result is already JSONNullObject, so return null
                response->result = move(result);
                break;
            case RubyfmtStatus::RIPPER_PARSE_FAILURE:
                displayError(fmt::format("`rubyfmt` failed to deserialize the parse tree from Ripper for {}.\n"
                                         "This is valid Ruby but represents a bug in `rubyfmt`.",
                                         path),
                             response);
                break;
            case RubyfmtStatus::IO_ERROR:
                displayError(fmt::format("`rubyfmt` encountered an IO error while writing {}.\n"
                                         "This is likely a bug in `rubyfmt`.",
                                         path),
                             response);
                break;
            case RubyfmtStatus::OTHER_RUBY_ERROR:
                displayError(fmt::format("`rubyfmt` encountered a ruby error while writing {}.\n"
                                         "This is likely a bug in `rubyfmt`.",
                                         path),
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
    } else {
        // `result` here is already a `JSONNullObject`, we just need to explicitly set it.
        response->result = move(result);
    }

    config.output->write(move(response));
    return;
}

// Since finalPhase is `preprocess`, this method should never be called.
unique_ptr<ResponseMessage> DocumentFormattingTask::runRequest(LSPTypecheckerDelegate &typechecker) {
    Exception::raise("Unimplemented and unused");
}

} // namespace sorbet::realmain::lsp
