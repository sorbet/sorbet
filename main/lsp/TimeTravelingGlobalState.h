#ifndef RUBY_TYPER_LSP_TIMETRAVELINGGLOBALSTATE_H
#define RUBY_TYPER_LSP_TIMETRAVELINGGLOBALSTATE_H

#include "common/concurrency/WorkerPool.h"
#include "common/kvstore/KeyValueStore.h"
#include "core/NameHash.h"
#include "core/core.h"
#include "main/lsp/LSPConfiguration.h"
#include "main/lsp/json_types.h"
#include <deque>

namespace sorbet::realmain::lsp {

/**
 * A class that encloses a global state and allows one to 'time-travel' it between versions.
 * It also time-travels an array of file hashes corresponding to the files in the time-traveled global state.
 * Updates cannot be re-ordered, so updates that are undone are assumed to eventually become re-applied.
 */
class TimeTravelingGlobalState final {
private:
    // Contains file updates and their respective hash updates.
    struct GlobalStateUpdate {
        std::vector<std::shared_ptr<core::File>> fileUpdates;
        std::vector<core::FileHash> hashUpdates;
    };

    /**
     * An undo/redo log entry for message with internal id `messageId`.
     * Applying the updates to initialGS will apply or undo the edit with that messageId, depending on if it is in the
     * undo or redo log.
     */
    struct TimeTravelUpdate {
        int version = 0;
        GlobalStateUpdate update;
        GlobalStateUpdate undoUpdate;
    };
    const LSPConfiguration config;
    std::shared_ptr<spdlog::logger> logger;
    WorkerPool &workers;
    std::unique_ptr<KeyValueStore> kvstore; // always null for now.
    std::unique_ptr<core::GlobalState> gs;

    // Indicates the current version of `gs`.
    int activeVersion = 0;
    // Indicates the last version committed to `gs` and present in the log.
    int latestVersion = 0;

    // Get all updates that fall within the range (start, end), exclusive of endpoints.
    std::vector<TimeTravelUpdate *> updatesBetweenExclusive(int start, int end);

    // Log of applied updates, from earliest to latest.
    // We frequently delete from the back to prune history, so we use a deque rather than a vector.
    std::deque<TimeTravelUpdate> log;

    // Contains file hashes for the current version of global state.
    std::vector<core::FileHash> globalStateHashes;

    // Internal function: Applies given update (or undoes it) and appropriately updates `activeVersion`.
    std::vector<core::FileRef> applyUpdate(TimeTravelUpdate &update, bool undo);

    std::vector<core::FileHash> computeStateHashes(const std::vector<std::shared_ptr<core::File>> &files) const;

public:
    TimeTravelingGlobalState(const LSPConfiguration &config, const std::shared_ptr<spdlog::logger> &logger,
                             WorkerPool &workers, std::unique_ptr<core::GlobalState> gs, int initialVersion);

    /**
     * Travels GlobalState forwards and backwards in time. No-op if version == current version.
     */
    void travel(int version);

    const core::GlobalState &getGlobalState() const;
    const std::vector<core::FileHash> &getGlobalStateHashes() const;

    /**
     * Prunes entries in the time travel log that fall before version.
     * Note: Supports a version ID that wraps around.
     */
    void pruneBefore(int version);

    /**
     * Applies the given update to GlobalState, appends a corresponding entry to the undo log, and indexes the update.
     * Mutates update to include file hashes and indexes.
     */
    void commitEdits(int version, LSPFileUpdates &update);

    /**
     * Indexes all workspace files from file system and hashes them.
     * This runs code that is not considered performance critical and this is expected to be slow.
     */
    std::vector<ast::ParsedFile> indexFromFileSystem();

    /**
     * Returns `true` if the given changes can run on the fast path relative to the current version of global state.
     * TODO(jvilk): Return reason and track slow path stats.
     */
    bool canTakeFastPath(const LSPFileUpdates &updates) const;
};

} // namespace sorbet::realmain::lsp
#endif // RUBY_TYPER_LSP_TIMETRAVELINGGLOBALSTATE_H