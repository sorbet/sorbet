#include "main/lsp/requests/document_formatting.h"
#include "absl/strings/ascii.h"
#include "main/lsp/json_types.h"
#include "main/lsp/lsp.h"

using namespace std;

namespace sorbet::realmain::lsp {
DocumentFormattingTask::DocumentFormattingTask(const LSPConfiguration &config, MessageId id, std::unique_ptr<DocumentFormattingParams> params)
    : LSPRequestTask(config, move(id), LSPMethod::TextDocumentFormatting), params(move(params)) {}


unique_ptr<ResponseMessage> DocumentFormattingTask::runRequest(LSPTypecheckerDelegate &typechecker) {
    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::TextDocumentFormatting);
    if (!config.opts.lspDocumentFormatRubyfmtEnabled) {
        response->error =
            make_unique<ResponseError>((int)LSPErrorCodes::InvalidRequest,
                                       "The `Document Formatting` LSP feature is experimental and disabled by default.");
        return response;
    }

    auto range = make_unique<Range>(make_unique<Position>(0, 0), make_unique<Position>(100000000000, 100));
    auto te = make_unique<TextEdit>(move(range), "lolololololol");
    auto tes = vector<unique_ptr<TextEdit>>();
    tes.push_back(move(te));
    variant<JSONNullObject, vector<unique_ptr<TextEdit>>> tmp = move(tes);
    response->result = move(tmp);
    return response;
}
}
