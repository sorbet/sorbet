#include "main/lsp/requests/sorbet_error.h"
#include "main/lsp/LSPOutput.h"
#include "main/lsp/json_types.h"

using namespace std;

namespace sorbet::realmain::lsp {
SorbetErrorTask::SorbetErrorTask(const LSPConfiguration &config, unique_ptr<SorbetErrorParams> params,
                                 optional<MessageId> id)
    : LSPTask(config, LSPMethod::SorbetError), params(move(params)), id(move(id)) {}

LSPTask::Phase SorbetErrorTask::finalPhase() const {
    return LSPTask::Phase::PREPROCESS;
}

void SorbetErrorTask::preprocess(LSPPreprocessor &preprocessor) {
    // Don't bother the main thread with these. Quickly handle.
    if (id.has_value()) {
        auto response = make_unique<ResponseMessage>("2.0", id.value(), LSPMethod::SorbetError);
        response->error = make_unique<ResponseError>(params->code, params->message);
        config.output->write(move(response));
    } else {
        if (params->code == (int)LSPErrorCodes::MethodNotFound) {
            // Not an error; we just don't care about this notification type (e.g. TextDocumentDidSave).
            config.logger->debug(params->message);
        } else {
            config.logger->error(params->message);
        }
    }
}

void SorbetErrorTask::run(LSPTypecheckerDelegate &typechecker) {}

} // namespace sorbet::realmain::lsp