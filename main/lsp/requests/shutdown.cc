#include "main/lsp/requests/shutdown.h"
#include "main/lsp/json_types.h"

using namespace std;

namespace sorbet::realmain::lsp {
ShutdownTask::ShutdownTask(const LSPConfiguration &config, MessageId id)
    : LSPRequestTask(config, id, LSPMethod::Shutdown) {}

bool ShutdownTask::canPreempt(const LSPIndexer &indexer) const {
    return true;
}

unique_ptr<ResponseMessage> ShutdownTask::runRequest(LSPTypecheckerDelegate &ts) {
    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::Shutdown);
    response->result = JSONNullObject();
    return response;
}

} // namespace sorbet::realmain::lsp
