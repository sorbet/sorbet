#include "main/lsp/requests/get_counters.h"
#include "main/lsp/lsp.h"

using namespace std;

namespace sorbet::realmain::lsp {
GetCountersTask::GetCountersTask(const LSPConfiguration &config, MessageId id)
    : LSPRequestTask(config, move(id), LSPMethod::GETCOUNTERS) {}

std::unique_ptr<ResponseMessage> GetCountersTask::runRequest(LSPTypecheckerDelegate &typechecker) {
    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::GETCOUNTERS);
    unique_ptr<SorbetCounters> counters = make_unique<SorbetCounters>();
    counters->counters = getAndClearThreadCounters();
    response->result = move(counters);
    return response;
}

} // namespace sorbet::realmain::lsp
