#ifndef RUBY_TYPER_LSP_LSPINDEXER_H
#define RUBY_TYPER_LSP_LSPINDEXER_H

#include "common/concurrency/WorkerPool.h"
#include "common/kvstore/KeyValueStore.h"
#include "core/NameHash.h"
#include "core/core.h"
#include "main/lsp/LSPMessage.h"

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
    /** Contains file hashes for the files stored in `initialGS`. Used to determine if an edit can be typechecked
     * incrementally. */
    std::vector<core::FileHash> globalStateHashes;
    /** Contains a copy of the last edit committed on the slow path. Used in slow path cancelation logic. */
    LSPFileUpdates pendingTypecheckUpdates;
    /** Contains a clone of the latency timer for the pending typecheck operation. Is used to ensure that we correctly
     * track the latency of canceled & rescheduled typechecking operations. */
    std::unique_ptr<Timer> pendingTypecheckLatencyTimer;
    std::vector<std::unique_ptr<Timer>> pendingTypecheckDiagnosticLatencyTimers;
    /** Contains globalStateHashes evicted with `pendingTypecheckUpdates`. Used in slow path cancelation logic. */
    UnorderedMap<int, core::FileHash> pendingTypecheckEvictedStateHashes;
    std::unique_ptr<KeyValueStore> kvstore; // always null for now.
    /** A WorkerPool with 0 workers. */
    std::unique_ptr<WorkerPool> emptyWorkers;

    std::vector<core::FileHash> computeFileHashes(const std::vector<std::shared_ptr<core::File>> &files,
                                                  WorkerPool &workers) const;

public:
    LSPIndexer(std::shared_ptr<const LSPConfiguration> config, std::unique_ptr<core::GlobalState> initialGS);
    ~LSPIndexer();

    /** Determines if the given edit can take the fast path relative to the most recently committed edit. */
    bool canTakeFastPath(const SorbetWorkspaceEditParams &params, const std::vector<core::FileHash> &fileHashes) const;

    /**
     * Computes state hashes for the given set of files.
     */
    std::vector<core::FileHash> computeFileHashes(const std::vector<std::shared_ptr<core::File>> &files) const;

    /** Initializes the indexer by indexing and hashing all files in the workspace. Mutates the LSPFileUpdates so it can
     * be passed to the typechecker to initialize it. */
    void initialize(LSPFileUpdates &updates, WorkerPool &workers);

    /**
     * Commits the given edit to `initialGS`, and returns a canonical LSPFileUpdates object containing indexed trees
     * and file hashes. Also handles canceling the running slow path.
     *
     * If `newHashesOrEmpty` contains file hashes, this function avoids re-computing those hashes.
     */
    LSPFileUpdates commitEdit(std::unique_ptr<Timer> &latencyTimer, SorbetWorkspaceEditParams &edit,
                              std::vector<core::FileHash> newHashesOrEmpty);
};

} // namespace sorbet::realmain::lsp

#endif // RUBY_TYPER_LSP_LSPINDEXER_H
