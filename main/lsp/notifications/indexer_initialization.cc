#include "main/lsp/notifications/indexer_initialization.h"
#include "main/lsp/LSPIndexer.h"

namespace sorbet::realmain::lsp {

IndexerInitializationTask::IndexerInitializationTask(const LSPConfiguration &config,
                                                     std::unique_ptr<core::GlobalState> initialGS)
    : LSPTask(config, LSPMethod::SorbetIndexerInitialization), initialGS{std::move(initialGS)} {}

LSPTask::Phase IndexerInitializationTask::finalPhase() const {
    return LSPTask::Phase::INDEX;
}

void IndexerInitializationTask::index(LSPIndexer &indexer) {
    indexer.initialize(*this, std::move(this->initialGS));
}

void IndexerInitializationTask::run(LSPTypecheckerDelegate &typechecker) {}

} // namespace sorbet::realmain::lsp
