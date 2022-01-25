#include "main/lsp/notifications/sorbet_workspace_edit.h"
#include "core/lsp/TypecheckEpochManager.h"
#include "main/lsp/LSPFileUpdates.h"
#include "main/lsp/LSPIndexer.h"
#include "main/lsp/json_types.h"

using namespace std;

namespace sorbet::realmain::lsp {
SorbetWorkspaceEditTask::SorbetWorkspaceEditTask(const LSPConfiguration &config,
                                                 unique_ptr<SorbetWorkspaceEditParams> params)
    : LSPDangerousTypecheckerTask(config, LSPMethod::SorbetWorkspaceEdit),
      latencyCancelSlowPath(make_unique<Timer>("latency.cancel_slow_path")), params(move(params)) {
    if (this->params->updates.empty()) {
        latencyCancelSlowPath->cancel();
    }
};

SorbetWorkspaceEditTask::~SorbetWorkspaceEditTask() = default;

LSPTask::Phase SorbetWorkspaceEditTask::finalPhase() const {
    if (params->updates.empty()) {
        // Early-dispatch no-op edits. These can happen if the user opens or changes a file that is not within the
        // current workspace.
        return LSPTask::Phase::PREPROCESS;
    } else {
        return LSPTask::Phase::RUN;
    }
}

void SorbetWorkspaceEditTask::mergeNewer(SorbetWorkspaceEditTask &task) {
    // Merging is only supported *before* we index this update.
    ENFORCE(updates == nullptr && task.updates == nullptr);
    params->merge(*task.params);
    // Don't report a latency metric for merged edits.
    if (task.latencyTimer) {
        task.latencyTimer->cancel();
    }
    if (task.latencyCancelSlowPath) {
        task.latencyCancelSlowPath->cancel();
    }

    // This cached information is now invalid.
    task.cachedFastPathDecisionValid = false;
    task.cachedFastPathDecision = false;
    cachedFastPathDecisionValid = false;
    cachedFastPathDecision = false;
}

void SorbetWorkspaceEditTask::preprocess(LSPPreprocessor &preprocessor) {
    // latencyTimer is assigned prior to preprocess.
    if (this->latencyTimer != nullptr && !params->updates.empty()) {
        params->diagnosticLatencyTimers.push_back(
            make_unique<Timer>(this->latencyTimer->clone("last_diagnostic_latency")));
    }
}

void SorbetWorkspaceEditTask::index(LSPIndexer &indexer) {
    if (params->updates.size() <= config.opts.lspMaxFilesOnFastPath) {
        updates = make_unique<LSPFileUpdates>(indexer.commitEdit(*params));
    } else {
        // HACK: Too many files to `commitEdit` serially. Index in `runSpecial`.
        this->indexer = &indexer;
        ENFORCE(canTakeFastPath(indexer) == false);
    }
}

void SorbetWorkspaceEditTask::run(LSPTypecheckerDelegate &typechecker) {
    if (latencyTimer != nullptr) {
        latencyTimer->setTag("path", "fast");
    }
    ENFORCE(updates != nullptr);
    ENFORCE(this->indexer == nullptr);
    if (!updates->canceledSlowPath) {
        latencyCancelSlowPath->cancel();
    }
    // Trigger destructor of Timer, which reports metric.
    latencyCancelSlowPath = nullptr;
    // For consistency; I don't expect this notification to be used for fast path edits.
    startedNotification.Notify();
    if (!updates->canTakeFastPath) {
        Exception::raise("Attempted to run a slow path update on the fast path!");
    }
    const auto newEditCount = updates->editCount - updates->committedEditCount;

    // Checks in debug builds that we have exactly 1 diagnostic latency timer per edit
    ENFORCE(latencyTimer == nullptr || newEditCount == params->diagnosticLatencyTimers.size());

    typechecker.typecheckOnFastPath(move(*updates), move(params->diagnosticLatencyTimers));
    prodCategoryCounterAdd("lsp.messages.processed", "sorbet.mergedEdits", newEditCount - 1);
}

void SorbetWorkspaceEditTask::runSpecial(LSPTypechecker &typechecker, WorkerPool &workers) {
    if (latencyTimer != nullptr) {
        latencyTimer->setTag("path", "slow");
    }
    if (indexer) {
        ENFORCE(updates == nullptr);
        // Using the `indexer` here is safe; the indexing thread is blocked until `startedNotification.Notify` is called
        // later in this function.
        // This is really gnarly; there's got to be a cleaner way to do threading here. We can't move this out because
        // we need `workers`, which is a resource that is explicitly managed by typechecking.
        updates = make_unique<LSPFileUpdates>(indexer->commitEdit(*params, workers));
    } else {
        ENFORCE(updates != nullptr);
    }

    if (!updates->canceledSlowPath) {
        latencyCancelSlowPath->cancel();
    }
    // Trigger destructor of Timer, which reports metric.
    latencyCancelSlowPath = nullptr;
    // Inform the epoch manager that we're going to perform a cancelable typecheck, then notify the
    // processing thread that it's safe to move on.
    typechecker.state().epochManager->startCommitEpoch(updates->epoch);
    startedNotification.Notify();
    const auto newEditCount = updates->editCount - updates->committedEditCount;

    // Checks in debug builds that we have exactly 1 diagnostic latency timer per edit
    ENFORCE(latencyTimer == nullptr || newEditCount == params->diagnosticLatencyTimers.size());

    // Only report stats if the edit was committed.
    if (typechecker.typecheck(move(*updates), workers, move(params->diagnosticLatencyTimers))) {
        prodCategoryCounterAdd("lsp.messages.processed", "sorbet.mergedEdits", newEditCount - 1);
    } else if (latencyTimer != nullptr) {
        // Don't report a latency value for canceled slow paths.
        latencyTimer->cancel();
    }
}

void SorbetWorkspaceEditTask::schedulerWaitUntilReady() {
    startedNotification.WaitForNotification();
}

bool SorbetWorkspaceEditTask::canTakeFastPath(const LSPIndexer &index) const {
    if (updates != nullptr) {
        return updates->canTakeFastPath;
    }
    if (!cachedFastPathDecisionValid) {
        cachedFastPathDecision = index.canTakeFastPath(params->updates);
        cachedFastPathDecisionValid = true;
    }
    return cachedFastPathDecision;
}

bool SorbetWorkspaceEditTask::canPreempt(const LSPIndexer &index) const {
    return canTakeFastPath(index);
}

bool SorbetWorkspaceEditTask::needsMultithreading(const LSPIndexer &index) const {
    return !canTakeFastPath(index);
}

const SorbetWorkspaceEditParams &SorbetWorkspaceEditTask::getParams() const {
    return *params;
}

} // namespace sorbet::realmain::lsp
