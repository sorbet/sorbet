#include "main/lsp/notifications/sorbet_workspace_edit.h"
#include "absl/algorithm/container.h"
#include "core/lsp/TypecheckEpochManager.h"
#include "main/lsp/LSPFileUpdates.h"
#include "main/lsp/LSPIndexer.h"
#include "main/lsp/json_types.h"

using namespace std;

namespace sorbet::realmain::lsp {
SorbetWorkspaceEditTask::SorbetWorkspaceEditTask(const LSPConfiguration &config,
                                                 unique_ptr<SorbetWorkspaceEditParams> params)
    : LSPDangerousTypecheckerTask(config, LSPMethod::SorbetWorkspaceEdit),
      latencyCancelSlowPath(make_unique<Timer>(*config.logger, "latency.cancel_slow_path")), params(move(params)) {
    if (isNoop()) {
        latencyCancelSlowPath->cancel();
    }
};

SorbetWorkspaceEditTask::~SorbetWorkspaceEditTask() = default;

bool SorbetWorkspaceEditTask::isNoop() const {
    // A resync only looks like a no-op: Watchman did not say which files changed, so the indexer fills `updates` in.
    return params->updates.empty() && !params->resyncAllFiles;
}

LSPTask::Phase SorbetWorkspaceEditTask::finalPhase() const {
    if (isNoop()) {
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
    task.cachedFastPathDecision = TypecheckingPath::Slow;
    cachedFastPathDecisionValid = false;
    cachedFastPathDecision = TypecheckingPath::Slow;
}

void SorbetWorkspaceEditTask::preprocess(LSPPreprocessor &preprocessor) {
    // latencyTimer is assigned prior to preprocess.
    if (this->latencyTimer != nullptr && !isNoop()) {
        params->diagnosticLatencyTimers.push_back(
            make_unique<Timer>(this->latencyTimer->clone("last_diagnostic_latency")));
    }
}

void SorbetWorkspaceEditTask::index(LSPIndexer &indexer) {
    // This is the first time that we're able to compare the files in the update set to
    // the indexer's file table. We should determine if any of the updates we've received
    // are no-ops from background notifications, and filter them out to not artificially
    // inflate the number of files that are actually making changes.
    std::erase_if(this->params->updates, [&indexer](auto &file) {
        ENFORCE(file != nullptr);
        return !indexer.wouldUpdateFileTable(*file);
    });

    // A resync is also indexed in `runSpecial`: expanding it reads the whole workspace, which wants a worker pool and
    // must not happen while this thread holds the task queue's lock.
    if (!params->resyncAllFiles && params->updates.size() <= config.opts.lspMaxFilesOnFastPath) {
        updates = indexer.commitEdit(*params);
    } else {
        // HACK: Too many files to `commitEdit` serially. Index in `runSpecial`.
        this->indexer = &indexer;
        ENFORCE(getTypecheckingPath(indexer) != TypecheckingPath::Fast);
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
    if (updates->typecheckingPath != TypecheckingPath::Fast) {
        Exception::raise("Attempted to run a slow path update on the fast path!");
    }
    const auto newEditCount = updates->editCount - updates->committedEditCount;

    // Checks in debug builds that we have exactly 1 diagnostic latency timer per edit
    ENFORCE(latencyTimer == nullptr || newEditCount == params->diagnosticLatencyTimers.size());

    typechecker.typecheckOnFastPath(std::move(updates), move(params->diagnosticLatencyTimers));
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
        updates = indexer->commitEdit(*params, workers);

        // IMPORTANT: We only have unqiue access to the indexer until we notify the LSPTypecheckerCoordinator that we
        // have started, which is done through `startedNotification` below. As there's no mutex that guarantees unique
        // access to the indexer (other than the task queue's mutex) and the LSPTypecheckerCoordinator will allow new
        // tasks to start once we notify through `startedNotification`, we need to make sure that we don't accidentally
        // use `indexer` again after this point.
        indexer = nullptr;
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

    ENFORCE(indexer == nullptr);
    startedNotification.Notify();

    const auto newEditCount = updates->editCount - updates->committedEditCount;

    // Checks in debug builds that we have exactly 1 diagnostic latency timer per edit
    ENFORCE(latencyTimer == nullptr || newEditCount == params->diagnosticLatencyTimers.size());

    // Only report stats if the edit was committed.
    if (typechecker.typecheck(std::move(updates), workers, move(params->diagnosticLatencyTimers))) {
        prodCategoryCounterAdd("lsp.messages.processed", "sorbet.mergedEdits", newEditCount - 1);
    } else if (latencyTimer != nullptr) {
        // Don't report a latency value for canceled slow paths.
        latencyTimer->cancel();
    }
}

void SorbetWorkspaceEditTask::schedulerWaitUntilReady() {
    startedNotification.WaitForNotification();
}

TypecheckingPath SorbetWorkspaceEditTask::getTypecheckingPath(const LSPIndexer &index) const {
    if (updates != nullptr) {
        return updates->typecheckingPath;
    }
    if (params->resyncAllFiles) {
        // What this edit contains is unknown until the indexer has read the workspace, and it takes the slow path
        // either way; see LSPFileUpdates::resyncedAllFiles.
        return TypecheckingPath::Slow;
    }
    if (!cachedFastPathDecisionValid) {
        cachedFastPathDecision = index.getTypecheckingPath(params->updates);
        cachedFastPathDecisionValid = true;
    }
    return cachedFastPathDecision;
}

bool SorbetWorkspaceEditTask::canPreempt(const LSPIndexer &index) const {
    return getTypecheckingPath(index) == TypecheckingPath::Fast;
}

const SorbetWorkspaceEditParams &SorbetWorkspaceEditTask::getParams() const {
    return *params;
}

core::packages::Stratum SorbetWorkspaceEditTask::preemptionStratum(FileStratumMapping info) const {
    vector<string_view> paths;
    paths.reserve(this->params->updates.size());
    absl::c_transform(this->params->updates, back_inserter(paths), [](auto &file) { return file->path(); });
    return info.getStratumForPaths(paths);
}

} // namespace sorbet::realmain::lsp
