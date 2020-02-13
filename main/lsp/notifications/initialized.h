#ifndef RUBY_TYPER_LSP_NOTIFICATIONS_INITIALIZED_H
#define RUBY_TYPER_LSP_NOTIFICATIONS_INITIALIZED_H

#include "absl/synchronization/notification.h"
#include "main/lsp/LSPTask.h"

namespace sorbet::realmain::lsp {
class InitializedTask final : public LSPDangerousTypecheckerTask {
    LSPConfiguration &mutableConfig;
    LSPFileUpdates updates;
    absl::Notification complete;
    LSPIndexer *indexer;

public:
    InitializedTask(LSPConfiguration &config);

    void preprocess(LSPPreprocessor &preprocessor) override;
    void index(LSPIndexer &indexer) override;
    void runSpecial(LSPTypechecker &typechecker, WorkerPool &workers) override;
    void schedulerWaitUntilReady() override;

    bool needsMultithreading(const LSPIndexer &indexer) const override;
};
} // namespace sorbet::realmain::lsp

#endif
