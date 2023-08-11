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
    bool initialized = false;
    /** Encapsulates the active configuration for the language server. */
    const std::shared_ptr<const LSPConfiguration> config;
    /** Global state that we keep up-to-date with file edits. We do _not_ typecheck using this global state! We clone
     * this global state every time we need to perform a slow path typechecking operation. */
    std::unique_ptr<core::GlobalState> initialGS;
    /** Key-value store used during initialization. */
    std::unique_ptr<KeyValueStore> kvstore;
    /** Contains a copy of the last edit committed on the slow path. Used in slow path cancelation logic. */
    LSPFileUpdates pendingTypecheckUpdates;
    /** Contains a clone of the latency timers for each new edit in the pending typecheck operation. Is used to ensure
     * that we correctly track the latency of canceled & rescheduled typechecking operations. */
    std::vector<std::unique_ptr<Timer>> pendingTypecheckDiagnosticLatencyTimers;
    /** Contains files evicted by `pendingTypecheckUpdates`. Used to make fast path decisions in the immediate past. */
    UnorderedMap<core::FileRef, std::shared_ptr<core::File>> evictedFiles;
    /** A WorkerPool with 0 workers. */
    std::unique_ptr<WorkerPool> emptyWorkers;

    void computeFileHashes(const std::vector<std::shared_ptr<core::File>> &files, WorkerPool &workers) const;

    /**
     * Determines if the given edit can take the fast path relative to the most recently committed edit.
     * It compares the file hashes in the files in `edit` to those in `evictedFiles` and `initialGS` (in that order).
     */
    TypecheckingPath
    getTypecheckingPath(const LSPFileUpdates &edit,
                        const UnorderedMap<core::FileRef, std::shared_ptr<core::File>> &evictedFiles) const;
    /**
     * INVARIANT: `changedFiles` must have hashes computed.
     */
    TypecheckingPath
    getTypecheckingPathInternal(const std::vector<std::shared_ptr<core::File>> &changedFiles,
                                const UnorderedMap<core::FileRef, std::shared_ptr<core::File>> &evictedFiles) const;

public:
    LSPIndexer(std::shared_ptr<const LSPConfiguration> config, std::unique_ptr<core::GlobalState> initialGS,
               std::unique_ptr<KeyValueStore> kvstore);
    ~LSPIndexer();

    /**
     * Determines if the given files can take the fast path relative to the latest committed edit.
     */
    TypecheckingPath getTypecheckingPath(const std::vector<std::shared_ptr<core::File>> &changedFiles) const;

    /**
     * Computes state hashes for the given set of files. Is a no-op if the provided files all have hashes.
     */
    void computeFileHashes(const std::vector<std::shared_ptr<core::File>> &files) const;

    /**
     * Initializes the indexer with the state produced on the typechecking thread.
     */
    void initialize(IndexerInitializationTask &task, std::unique_ptr<core::GlobalState> initialGS);

    /**
     * Commits the given edit to `initialGS`, and returns a canonical LSPFileUpdates object containing indexed trees
     * and file hashes. Also handles canceling the running slow path.
     */
    LSPFileUpdates commitEdit(SorbetWorkspaceEditParams &edit, WorkerPool &workers);
    LSPFileUpdates commitEdit(SorbetWorkspaceEditParams &edit);

    /**
     * Retrieves the file ref for the given file, if exists.
     */
    core::FileRef uri2FileRef(std::string_view uri) const;

    /**
     * Given a file ref _that exists_, return the underlying file.
     */
    const core::File &getFile(core::FileRef fref) const;

    /**
     * Given a reference to the InitializedTask, transfer ownership of the global state out for initialization in the
     * typechecker thread. The task argument is unused, and is present only to make it difficult to get the global state
     * out in a context that's not the InitializedTask's index function.
     */
    void transferInitializeState(InitializedTask &task);

    void updateGsFromOptions(const DidChangeConfigurationParams &options) const;
};

} // namespace sorbet::realmain::lsp

#endif // RUBY_TYPER_LSP_LSPINDEXER_H
