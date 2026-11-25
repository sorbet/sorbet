#ifndef RUBY_TYPER_LSP_NOTIFICATIONS_SORBET_WORKSPACE_EDIT_H
#define RUBY_TYPER_LSP_NOTIFICATIONS_SORBET_WORKSPACE_EDIT_H

#include "absl/synchronization/notification.h"
#include "main/lsp/LSPIndexer.h"
#include "main/lsp/LSPTask.h"
#include <variant>

namespace sorbet::realmain::lsp {
class LSPFileUpdates;
class SorbetWorkspaceEditParams;
class SorbetWorkspaceEditTask final : public LSPDangerousTypecheckerTask {
    // The edit hasn't been committed yet (it's in the task queue).
    struct Queued {
        static constexpr std::string_view name = "Queued";

        // Caches the indexer's prediction of the typechecking path for the provided update, along with the files
        // outside of the update that the fast path would also typecheck. Becomes invalidated when the update changes,
        // or when the indexer commits another edit (see `LSPIndexer::isTypecheckingPathValid`).
        mutable std::optional<LSPIndexer::TypecheckingPathResult> provisionalDecision;
    };

    // `index` found too many files to commit serially, so `runSpecial` will commit the edit using the typechecker's
    // workers. The edit takes the slow path.
    struct CommitInRunSpecial {
        static constexpr std::string_view name = "CommitInRunSpecial";

        // HACK: Stash the indexer here for use in `runSpecial`.
        LSPIndexer *indexer;
    };

    // The edit has been committed, and is about to run. `run` or `runSpecial` moves `updates` to the typechecker.
    struct Committed {
        static constexpr std::string_view name = "Committed";

        std::unique_ptr<LSPFileUpdates> updates;
    };

    std::variant<Queued, CommitInRunSpecial, Committed> state;
    std::string_view stateName() const;

    absl::Notification startedNotification;
    std::unique_ptr<Timer> latencyCancelSlowPath;
    std::unique_ptr<SorbetWorkspaceEditParams> params;

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
    core::packages::Stratum preemptionStratum(FileStratumMapping info) const override;

    bool canPreempt(const LSPIndexer &index) const override;
    // The typechecking path that `commitEdit` decided, once this edit has been committed. Before that, the path that
    // the indexer predicts `commitEdit` would decide.
    TypecheckingPath getTypecheckingPath(const LSPIndexer &index) const;
};

} // namespace sorbet::realmain::lsp

#endif
