#include "main/lsp/notifications/initialized.h"
#include "common/kvstore/KeyValueStore.h"
#include "main/lsp/LSPIndexer.h"

using namespace std;

namespace sorbet::realmain::lsp {

InitializedTask::InitializedTask(LSPConfiguration &config)
    : LSPTask(config, LSPMethod::Initialized), mutableConfig(config), gs{}, kvstore{} {};

void InitializedTask::preprocess(LSPPreprocessor &preprocessor) {
    mutableConfig.markInitialized();
}

void InitializedTask::index(LSPIndexer &indexer) {
    indexer.transferInitializeState(*this);
}

void InitializedTask::run(LSPTypecheckerInterface &typechecker) {
    ENFORCE(this->gs != nullptr);
    typechecker.initialize(*this, std::move(this->gs), std::move(this->kvstore));
}

bool InitializedTask::needsMultithreading(const LSPIndexer &indexer) const {
    return true;
}

void InitializedTask::setGlobalState(std::unique_ptr<core::GlobalState> gs) {
    this->gs = std::move(gs);
}

void InitializedTask::setKeyValueStore(std::unique_ptr<KeyValueStore> kvstore) {
    this->kvstore = std::move(kvstore);
}

} // namespace sorbet::realmain::lsp
