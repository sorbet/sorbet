#include "main/lsp/notifications/indexer_initialized.h"
#include "main/lsp/LSPIndexer.h"

namespace sorbet::realmain::lsp {

IndexerInitializedTask::IndexerInitializedTask(const LSPConfiguration &config,
                                               std::unique_ptr<core::GlobalState> initialGS)
    : LSPTask(config, LSPMethod::SorbetIndexerInitialized), initialGS{std::move(initialGS)} {}

LSPTask::Phase IndexerInitializedTask::finalPhase() const {
    return LSPTask::Phase::INDEX;
}

void IndexerInitializedTask::index(LSPIndexer &indexer) {
    indexer.initialize(*this, std::move(this->initialGS));
}

void IndexerInitializedTask::run(LSPTypecheckerInterface &typechecker) {}

} // namespace sorbet::realmain::lsp
