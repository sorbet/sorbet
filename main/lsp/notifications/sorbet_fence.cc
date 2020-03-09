#include "main/lsp/notifications/sorbet_fence.h"
#include "main/lsp/LSPOutput.h"
#include "main/lsp/json_types.h"

using namespace std;

namespace sorbet::realmain::lsp {
SorbetFenceTask::SorbetFenceTask(const LSPConfiguration &config, int id)
    : LSPTask(config, LSPMethod::SorbetFence), id(id) {}

bool SorbetFenceTask::canPreempt(const LSPIndexer &indexer) const {
    return false;
}

void SorbetFenceTask::run(LSPTypecheckerDelegate &tc) {
    // Send the same fence back to acknowledge the fence.
    // NOTE: Fence is a notification rather than a request so that we don't have to worry about clashes with
    // client-chosen IDs when using fences internally.
    auto response = make_unique<NotificationMessage>("2.0", LSPMethod::SorbetFence, id);
    config.output->write(move(response));
}
} // namespace sorbet::realmain::lsp
