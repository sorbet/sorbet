#include "main/lsp/notifications/initialized.h"
#include "main/lsp/LSPIndexer.h"

using namespace std;

namespace sorbet::realmain::lsp {

InitializedTask::InitializedTask(LSPConfiguration &config)
    : LSPTask(config, LSPMethod::Initialized), mutableConfig(config), gs{}, sessionCache{} {};

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
    typechecker.initialize(*this, std::move(this->gs), std::move(this->sessionCache), config);
    typechecker.resumeTaskQueue(*this);
}

bool InitializedTask::canPreempt(const LSPIndexer &indexer) const {
    return false;
}

void InitializedTask::setGlobalState(unique_ptr<core::GlobalState> gs) {
    this->gs = std::move(gs);
}

void InitializedTask::setSessionCache(unique_ptr<cache::SessionCache> sessionCache) {
    this->sessionCache = std::move(sessionCache);
}

} // namespace sorbet::realmain::lsp
