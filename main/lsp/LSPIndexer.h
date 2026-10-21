#ifndef RUBY_TYPER_LSP_LSPINDEXER_H
#define RUBY_TYPER_LSP_LSPINDEXER_H

#include "core/core.h"
#include "main/lsp/LSPFileUpdates.h"
#include "main/lsp/LSPMessage.h"
#include "main/lsp/LSPPreprocessor.h"
#include "main/lsp/notifications/indexer_initialization.h"
#include "main/lsp/notifications/initialized.h"

namespace sorbet {
class WorkerPool;
class KeyValueStore;
} // namespace sorbet

namespace sorbet::realmain::lsp {

class SorbetWorkspaceEditParams;
class LSPConfiguration;

/**
 * The indexer keeps a GlobalState object up-to-date with the latest edits, maintains a set of FileHashes for
 * every file in the workspace, and uses said hashes to decide if edits can be incrementally typechecked on the
 * fast path or necessitate retypechecking the world on the slow path.
 *
 * The actual work of typechecking is undertaken by LSPTypechecker.
 */
class LSPIndexer final {
public:
    struct TypecheckingPathResult {
        TypecheckingPath path = TypecheckingPath::Slow;
        LSPFileUpdates::FastPathFilesToTypecheckResult files;
    };

private:
    bool initialized = false;
    /** Encapsulates the active configuration for the language server. */
    const std::shared_ptr<const LSPConfiguration> config;
    /** Global state that we keep up-to-date with file edits. We do _not_ typecheck using this global state, as it will
     * never have a populated symbol table. */
    std::unique_ptr<core::GlobalState> gs;
    /** Key-value store used during initialization. */
    std::unique_ptr<KeyValueStore> kvstore;
    /**
     * What the indexer knows about the slow path that the typechecker is running: the edits that the slow path is
     * typechecking, plus the fast path edits that have preempted it. A newer slow path edit that cancels this slow path
     * must include all of them, because canceling restores the typechecker to its state from before the slow path.
     *
     * Only read while a slow path is running. After the slow path finishes, this is stale until the next slow path
     * edit replaces it.
     */
    struct PendingSlowPath {
        /** Contains a copy of the last edit committed on the slow path. Used in slow path cancelation logic. */
        LSPFileUpdates updates;
        /** Contains a clone of the latency timers for each new edit in the pending typecheck operation. Is used to
         * ensure that we correctly track the latency of canceled & rescheduled typechecking operations. */
        std::vector<std::unique_ptr<Timer>> diagnosticLatencyTimers;
        /** Contains files evicted by `updates`. Used to make fast path decisions in the immediate past. */
        UnorderedMap<core::FileRef, std::shared_ptr<core::File>> evictedFiles;
    };
    PendingSlowPath pendingSlowPath;
    /** A WorkerPool with 0 workers. */
    std::unique_ptr<WorkerPool> emptyWorkers;
    /** Incremented whenever the files in `gs` change. */
    uint32_t version = 0;

    void computeFileHashes(const std::vector<std::shared_ptr<core::File>> &files, WorkerPool &workers) const;

    /**
     * Decides the typechecking path of an edit that `commitEdit` is committing, and records the fast path files in
     * `edit`. It compares the file hashes in the files in `edit` to those in `evictedFiles` and `gs` (in that order).
     *
     * `predictTypecheckingPath` predicts this decision for edits that haven't been committed yet.
     */
    TypecheckingPath
    decideTypecheckingPath(LSPFileUpdates &edit,
                           const UnorderedMap<core::FileRef, std::shared_ptr<core::File>> &evictedFiles) const;

    /**
     * Called by `commitEdit` once it has decided `update`'s typechecking path. If a slow path is running and `update`
     * takes the slow path too, cancels the running slow path, and merges `pendingSlowPath` into `update` (and its
     * timers into `diagnosticLatencyTimers`), deciding the merged update's path again.
     *
     * Canceling must happen on the processing thread (`TypecheckEpochManager::tryCancelSlowPath` asserts that).
     * `commitEdit` also runs on the typechecker thread, but never cancels there: in a preemption, the update takes the
     * fast path (the prediction that let it preempt matches the decision), and in `runSpecial`, no slow path is
     * running.
     */
    void tryCancelSlowPath(LSPFileUpdates &update,
                           UnorderedMap<core::FileRef, std::shared_ptr<core::File>> &newlyEvictedFiles,
                           std::vector<std::unique_ptr<Timer>> &diagnosticLatencyTimers);

    /**
     * Called by `commitEdit` last. A slow path update replaces `pendingSlowPath`, and a fast path update is merged
     * into it (as having preempted the running slow path, if there is one).
     */
    void recordInPendingSlowPath(const LSPFileUpdates &update,
                                 UnorderedMap<core::FileRef, std::shared_ptr<core::File>> newlyEvictedFiles,
                                 const std::vector<std::unique_ptr<Timer>> &diagnosticLatencyTimers);

    /**
     * The computation shared by `decideTypecheckingPath` and `predictTypecheckingPath`.
     *
     * INVARIANT: `changedFiles` must have hashes computed.
     */
    TypecheckingPathResult
    computeTypecheckingPath(const std::vector<std::shared_ptr<core::File>> &changedFiles,
                            const UnorderedMap<core::FileRef, std::shared_ptr<core::File>> &evictedFiles) const;

public:
    LSPIndexer(std::shared_ptr<const LSPConfiguration> config, std::unique_ptr<core::GlobalState> gs,
               std::unique_ptr<KeyValueStore> kvstore);
    ~LSPIndexer();

    /**
     * Predicts the typechecking path that `commitEdit` would decide for these files, if it committed them now (relative
     * to the latest committed edit), without committing them. When they can take the fast path, the result also
     * describes which files outside of `changedFiles` the fast path would typecheck.
     *
     * Used to decide whether a queued edit can preempt a slow path, and at which stratum.
     */
    TypecheckingPathResult predictTypecheckingPath(const std::vector<std::shared_ptr<core::File>> &changedFiles) const;

    /**
     * Changes whenever the files in `gs` change, so that a result computed against them (like
     * `predictTypecheckingPath`) can tell whether it's stale.
     */
    uint32_t getVersion() const {
        return version;
    }

    /**
     * Computes state hashes for the given set of files. Is a no-op if the provided files all have hashes.
     */
    void computeFileHashes(const std::vector<std::shared_ptr<core::File>> &files) const;

    /**
     * Finalizes indexer initialization.
     */
    void initialize(IndexerInitializationTask &task, std::vector<std::shared_ptr<core::File>> &&files);

    /**
     * Commits the given edit to `gs`, and returns a canonical LSPFileUpdates object containing indexed trees
     * and file hashes. Also handles canceling the running slow path.
     */
    std::unique_ptr<LSPFileUpdates> commitEdit(SorbetWorkspaceEditParams &edit, WorkerPool &workers);
    std::unique_ptr<LSPFileUpdates> commitEdit(SorbetWorkspaceEditParams &edit);

    /**
     * Retrieves the file ref for the given file, if exists.
     */
    core::FileRef uri2FileRef(std::string_view uri) const;

    /**
     * Given a file ref _that exists_, return the underlying file.
     */
    const core::File &getFile(core::FileRef fref) const;

    /**
     * Given a File, return true if it is new, or differs from the version currently in the indexer's file table.
     */
    bool wouldUpdateFileTable(const core::File &file) const;

    /**
     * Given a reference to the InitializedTask, transfer ownership of the global state out for initialization in the
     * typechecker thread. The task argument is unused, and is present only to make it difficult to get the global state
     * out in a context that's not the InitializedTask's index function.
     */
    void transferInitializeState(InitializedTask &task);

    void updateConfigAndGsFromOptions(const DidChangeConfigurationParams &options) const;

    // Determines if it's possible for the head of the task queue to run during preemption.
    bool preemptionPossible(const TaskQueue::QueueType &tasks) const;
};

} // namespace sorbet::realmain::lsp

#endif // RUBY_TYPER_LSP_LSPINDEXER_H
