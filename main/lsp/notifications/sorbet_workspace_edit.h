#ifndef RUBY_TYPER_LSP_NOTIFICATIONS_SORBET_WORKSPACE_EDIT_H
#define RUBY_TYPER_LSP_NOTIFICATIONS_SORBET_WORKSPACE_EDIT_H

#include "absl/synchronization/notification.h"
#include "main/lsp/LSPTask.h"

namespace sorbet::realmain::lsp {
class SorbetWorkspaceEditTask final : public LSPDangerousTypecheckerTask {
    std::unique_ptr<LSPFileUpdates> updates;
    absl::Notification startedNotification;
    std::unique_ptr<Timer> latencyCancelSlowPath;
    std::unique_ptr<SorbetWorkspaceEditParams> params;
    // Caches the file hashes computed for `canPreempt` to avoid recomputing them later.
    mutable std::vector<core::FileHash> cachedFileHashesOrEmpty;
    // Caches the fast path decision made with cachedFileHashesOrEmpty.
    // Is only valid if `cachedFileHashesOrEmpty` is not empty.
    mutable bool cachedFastPathDecision = false;

public:
    SorbetWorkspaceEditTask(const LSPConfiguration &config, std::unique_ptr<SorbetWorkspaceEditParams> params);

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
    bool canTakeFastPath(const LSPIndexer &index) const;
    bool needsMultithreading(const LSPIndexer &index) const override;
};

} // namespace sorbet::realmain::lsp

#endif
