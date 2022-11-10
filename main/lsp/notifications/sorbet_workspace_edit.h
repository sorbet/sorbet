#ifndef RUBY_TYPER_LSP_NOTIFICATIONS_SORBET_WORKSPACE_EDIT_H
#define RUBY_TYPER_LSP_NOTIFICATIONS_SORBET_WORKSPACE_EDIT_H

#include "absl/synchronization/notification.h"
#include "main/lsp/LSPIndexer.h"
#include "main/lsp/LSPTask.h"

namespace sorbet::realmain::lsp {
class LSPFileUpdates;
class SorbetWorkspaceEditParams;
class SorbetWorkspaceEditTask final : public LSPDangerousTypecheckerTask {
    std::unique_ptr<LSPFileUpdates> updates;
    absl::Notification startedNotification;
    std::unique_ptr<Timer> latencyCancelSlowPath;
    std::unique_ptr<SorbetWorkspaceEditParams> params;
    // Caches the fast path decision for the provided update. Becomes invalidated when the update changes.
    mutable bool cachedFastPathDecisionValid = false;
    mutable TypecheckingPath cachedFastPathDecision = TypecheckingPath::Slow;
    // HACK: In the event that this edit is too large to index serially, stash the indexer here for use in `runSpecial`.
    LSPIndexer *indexer = nullptr;

public:
    SorbetWorkspaceEditTask(const LSPConfiguration &config, std::unique_ptr<SorbetWorkspaceEditParams> params);
    ~SorbetWorkspaceEditTask();

    LSPTask::Phase finalPhase() const override;

    // Used in tests only
    const SorbetWorkspaceEditParams &getParams() const;

    void mergeNewer(SorbetWorkspaceEditTask &task);
    void preprocess(LSPPreprocessor &preprocess) override;
    void index(LSPIndexer &indexer) override;
    void run(LSPTypecheckerDelegate &typechecker) override;
    void runSpecial(LSPTypechecker &typechecker, WorkerPool &workers) override;
    void schedulerWaitUntilReady() override;

    bool canPreempt(const LSPIndexer &index) const override;
    TypecheckingPath getTypecheckingPath(const LSPIndexer &index) const;
    bool needsMultithreading(const LSPIndexer &index) const override;
};

} // namespace sorbet::realmain::lsp

#endif
