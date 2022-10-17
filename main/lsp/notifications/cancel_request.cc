#include "main/lsp/notifications/cancel_request.h"
#include "main/lsp/LSPPreprocessor.h"
#include "main/lsp/json_types.h"

using namespace std;
namespace sorbet::realmain::lsp {

CancelRequestTask::CancelRequestTask(const LSPConfiguration &config, unique_ptr<CancelParams> params)
    : LSPTask(config, LSPMethod::$CancelRequest), params(move(params)) {}

LSPTask::Phase CancelRequestTask::finalPhase() const {
    return LSPTask::Phase::PREPROCESS;
}

void CancelRequestTask::preprocess(LSPPreprocessor &preprocessor) {
    preprocessor.cancelRequest(*params);
}

void CancelRequestTask::run(LSPTypecheckerDelegate &tc) {}

} // namespace sorbet::realmain::lsp
