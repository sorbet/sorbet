#include "main/lsp/notifications/initialized.h"
#include "common/kvstore/KeyValueStore.h"
#include "main/lsp/LSPIndexer.h"

using namespace std;

namespace sorbet::realmain::lsp {

InitializedTask::InitializedTask(LSPConfiguration &config)
    : LSPTask(config, LSPMethod::Initialized), mutableConfig(config), gs{}, kvstore{} {};

void InitializedTask::preprocess(LSPPreprocessor &preprocessor) {
    mutableConfig.markInitialized();
    this->preprocessor = &preprocessor;
}

void InitializedTask::index(LSPIndexer &indexer) {
    // We need to pause during indexing so that nothing else will try to give work to the index thread while its state
    // is being initialized in the typechecker.
    preprocessor->pause();

    indexer.transferInitializeState(*this);
}

void InitializedTask::run(LSPTypecheckerDelegate &typechecker) {
    ENFORCE(this->gs != nullptr);
    typechecker.initialize(*this, move(this->gs), move(this->kvstore), config, move(this->inputFileNames));
    typechecker.resumeTaskQueue(*this);
}

bool InitializedTask::needsMultithreading(const LSPIndexer &indexer) const {
    return true;
}

void InitializedTask::setGlobalState(unique_ptr<core::GlobalState> gs) {
    this->gs = std::move(gs);
}

void InitializedTask::setKeyValueStore(unique_ptr<KeyValueStore> kvstore) {
    this->kvstore = std::move(kvstore);
}

void InitializedTask::setInputFileNames(vector<string> &&inputFileNames) {
    this->inputFileNames = std::move(inputFileNames);
}

} // namespace sorbet::realmain::lsp
