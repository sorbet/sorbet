#ifndef RUBY_TYPER_LSP_NOTIFICATIONS_SORBET_WORKSPACE_EDIT_H
#define RUBY_TYPER_LSP_NOTIFICATIONS_SORBET_WORKSPACE_EDIT_H

#include "absl/synchronization/notification.h"
#include "main/lsp/LSPIndexer.h"
#include "main/lsp/LSPTask.h"
#include <variant>

namespace sorbet::realmain::lsp {
class LSPFileUpdates;
class SorbetWorkspaceEditParams;
/**
 * An edit to one or more files. Its lifecycle:
 *
 * 1. Queued. The preprocessor creates the task, and may merge newer edits into it (`mergeNewer`). So may the
 *    processing thread, when it promotes a slow path edit past a preemption (`LSPLoop::promoteSlowPathEdits`). While a
 *    slow path is running, `canPreempt` and `preemptionStratum` use the indexer's prediction of the typechecking path
 *    that committing the edit would decide (`cachedPrediction`).
 *
 * 2. Committed. `index` commits the edit to the indexer (`LSPIndexer::commitEdit`), which decides its typechecking path
 *    and returns the `LSPFileUpdates` for the typechecker (`updates`). An edit with too many files to commit serially
 *    is instead committed at the start of `runSpecial`, which can use the typechecker's workers.
 *
 *    An edit is only committed right before it runs: either by the processing thread, which then hands it to the
 *    typechecker, or by a preemption, which then runs it. So the task queue never holds a committed edit, and merging
 *    only ever happens before committing.
 *
 * 3. Run. On the fast path, `run` typechecks it, either normally or while preempting a slow path. On the slow path,
 *    `runSpecial` starts a cancelable slow path, and lets the processing thread continue once it has started
 *    (`schedulerWaitUntilReady`). A later slow path edit can cancel that slow path, in which case the later edit's
 *    update includes this edit's files too (see `LSPIndexer::commitEdit`).
 */
class SorbetWorkspaceEditTask final : public LSPDangerousTypecheckerTask {
    // The edit hasn't been committed yet (it's in the task queue).
    struct Queued {
        // Caches the indexer's prediction of the typechecking path for the provided update, along with the files
        // outside of the update that the fast path will also typecheck (e.g., because they mention a method whose
        // definition changed). Becomes invalidated when the update changes, or when the indexer commits another edit
        // (e.g., one ahead of this one in the queue).
        mutable std::optional<LSPIndexer::TypecheckingPathResult> cachedPrediction;
        mutable uint32_t cachedPredictionVersion = 0;
    };

    // `index` found too many files to commit serially, so `runSpecial` will commit the edit using the typechecker's
    // workers. The edit takes the slow path.
    struct CommitInRunSpecial {
        // HACK: Stash the indexer here for use in `runSpecial`.
        LSPIndexer *indexer;
    };

    // The edit has been committed, and is about to run. `run` or `runSpecial` moves `updates` to the typechecker.
    struct Committed {
        std::unique_ptr<LSPFileUpdates> updates;
    };

    std::variant<Queued, CommitInRunSpecial, Committed> state;

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
