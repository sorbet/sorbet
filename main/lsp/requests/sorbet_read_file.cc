#include "main/lsp/requests/sorbet_read_file.h"
#include "main/lsp/json_types.h"

using namespace std;

namespace sorbet::realmain::lsp {

SorbetReadFileTask::SorbetReadFileTask(const LSPConfiguration &config, MessageId id,
                                       std::unique_ptr<TextDocumentIdentifier> params)
    : LSPRequestTask(config, move(id), LSPMethod::SorbetReadFile), params(move(params)) {}

unique_ptr<ResponseMessage> SorbetReadFileTask::runRequest(LSPTypecheckerDelegate &typechecker) {
    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::SorbetReadFile);
    auto fref = config.uri2FileRef(typechecker.state(), params->uri);
    if (fref.exists()) {
        response->result =
            make_unique<TextDocumentItem>(params->uri, "ruby", 0, string(fref.data(typechecker.state()).source()));
    } else {
        response->error = make_unique<ResponseError>((int)LSPErrorCodes::InvalidParams,
                                                     fmt::format("Did not find file at uri {} in {}", params->uri,
                                                                 convertLSPMethodToString(LSPMethod::SorbetReadFile)));
    }
    return response;
}

} // namespace sorbet::realmain::lsp