#include "main/lsp/requests/get_counters.h"
#include "main/lsp/LSPOutput.h"
#include "main/lsp/json_types.h"

using namespace std;

namespace sorbet::realmain::lsp {
GetCountersTask::GetCountersTask(const LSPConfiguration &config, MessageId id)
    : LSPTask(config, LSPMethod::GETCOUNTERS), id(move(id)) {}

LSPTask::Phase GetCountersTask::finalPhase() const {
    return LSPTask::Phase::INDEX;
}

bool GetCountersTask::canPreempt(const LSPIndexer &) const {
    return false;
}

// Has to run on indexing thread, which is what coordinates all threads and collects all of the metrics.
void GetCountersTask::index(LSPIndexer &indexer) {
    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::GETCOUNTERS);
    unique_ptr<SorbetCounters> counters = make_unique<SorbetCounters>();
    counters->counters = getAndClearThreadCounters();
    response->result = move(counters);
    config.output->write(move(response));
}

void GetCountersTask::run(LSPTypecheckerDelegate &typechecker) {}

} // namespace sorbet::realmain::lsp
