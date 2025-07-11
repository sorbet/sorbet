#include "main/lsp/notifications/indexer_initialization.h"
#include "main/lsp/LSPIndexer.h"

using namespace std;

namespace sorbet::realmain::lsp {

IndexerInitializationTask::IndexerInitializationTask(const LSPConfiguration &config,
                                                     vector<shared_ptr<core::File>> &&files)
    : LSPTask(config, LSPMethod::SorbetIndexerInitialization), files{std::move(files)} {}

LSPTask::Phase IndexerInitializationTask::finalPhase() const {
    return LSPTask::Phase::INDEX;
}

void IndexerInitializationTask::index(LSPIndexer &indexer) {
    indexer.initialize(*this, std::move(this->files));
}

void IndexerInitializationTask::run(LSPTypecheckerDelegate &typechecker) {}

} // namespace sorbet::realmain::lsp
