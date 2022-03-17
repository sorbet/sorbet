#include "main/lsp/notifications/indexer_initialized.h"
#include "common/kvstore/KeyValueStore.h"
#include "main/lsp/LSPIndexer.h"

namespace sorbet::realmain::lsp {

IndexerInitializedTask::IndexerInitializedTask(const LSPConfiguration &config)
    : LSPTask(config, LSPMethod::SorbetIndexerInitialized), initialGS{}, kvstore{} {}

LSPTask::Phase IndexerInitializedTask::finalPhase() const {
    return LSPTask::Phase::INDEX;
}

void IndexerInitializedTask::setIndexerState(std::unique_ptr<core::GlobalState> initialGS,
                                             std::unique_ptr<KeyValueStore> kvstore) {
    this->initialGS = std::move(initialGS);
    this->kvstore = std::move(kvstore);
}

void IndexerInitializedTask::index(LSPIndexer &indexer) {
    indexer.takeInitializedState(*this, std::move(this->initialGS), std::move(this->kvstore));
}

void IndexerInitializedTask::run(LSPTypecheckerInterface &typechecker) {}

} // namespace sorbet::realmain::lsp
