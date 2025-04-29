#include "main/lsp/notifications/indexer_initialization.h"
#include "main/lsp/LSPIndexer.h"

namespace sorbet::realmain::lsp {

IndexerInitializationTask::IndexerInitializationTask(const LSPConfiguration &config)
    : LSPTask(config, LSPMethod::SorbetIndexerInitialization) {}

LSPTask::Phase IndexerInitializationTask::finalPhase() const {
    return LSPTask::Phase::INDEX;
}

void IndexerInitializationTask::index(LSPIndexer &indexer) {
    indexer.initialize(*this);
}

void IndexerInitializationTask::run(LSPTypecheckerDelegate &typechecker) {}

} // namespace sorbet::realmain::lsp
