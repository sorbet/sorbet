#include "main/lsp/notifications/initialized.h"
#include "main/lsp/LSPIndexer.h"

using namespace std;

namespace sorbet::realmain::lsp {

InitializedTask::InitializedTask(LSPConfiguration &config)
    : LSPDangerousTypecheckerTask(config, LSPMethod::Initialized), mutableConfig(config), indexer(nullptr){};

void InitializedTask::preprocess(LSPPreprocessor &preprocessor) {
    mutableConfig.markInitialized();
}

void InitializedTask::index(LSPIndexer &indexer) {
    // Hacky: We need to use the indexer, but with the WorkerPool from runSpecial. This is the only task to have this
    // special requirement.
    this->indexer = &indexer;
}

void InitializedTask::runSpecial(LSPTypechecker &typechecker, WorkerPool &workers) {
    prodCategoryCounterInc("lsp.messages.processed", "initialized");
    ENFORCE(this->indexer != nullptr);
    indexer->initialize(updates, workers);
    typechecker.initialize(move(updates), workers);
    // TODO: Make asynchronous.
    complete.Notify();
}

void InitializedTask::schedulerWaitUntilReady() {
    complete.WaitForNotification();
}

bool InitializedTask::needsMultithreading(const LSPIndexer &indexer) const {
    return true;
}

} // namespace sorbet::realmain::lsp