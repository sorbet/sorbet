#ifndef RUBY_TYPER_LSP_TIMETRAVELINGGLOBALSTATE_H
#define RUBY_TYPER_LSP_TIMETRAVELINGGLOBALSTATE_H

#include "common/common.h"
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
 * Progressing past `index` isn't supported. Actual naming/resolving/typechecking should be done on deep copies
 */
class TimeTravelingGlobalState final {
private:
    // Contains file updates and their respective hash+index updates.
    struct GlobalStateUpdate {
        std::vector<std::shared_ptr<core::File>> fileUpdates;
        std::vector<core::FileHash> hashUpdates;
        // Note: Undo updates do not contain indexes.
        std::vector<ast::ParsedFile> updatedFileIndexes;
    };

    /**
     * An undo/redo log entry for message with internal id `messageId`.
     * Applying the updates to initialGS will apply or undo the edit with that messageId, depending on if it is in the
     * undo or redo log.
     */
    struct TimeTravelUpdate {
        u4 version = 0;
        bool hasNewFiles;
        GlobalStateUpdate update;
        GlobalStateUpdate undoUpdate;
    };
    std::shared_ptr<const LSPConfiguration> config;
    std::unique_ptr<KeyValueStore> kvstore; // always null for now.
    std::unique_ptr<core::GlobalState> gs;

    // Indicates the current version of `gs`. May be a version that comes before `latestVersion` if `gs` has traveled
    // back in time.
    u4 activeVersion = 0;
    // Indicates the version number of the latest file update encountered. It has been committed to the `log`.
    u4 latestVersion = 0;

    // Get all updates that fall within the range (start, end), exclusive of endpoints.
    std::vector<TimeTravelUpdate *> updatesBetweenExclusive(u4 start, u4 end);

    // Log of applied updates, from earliest to latest.
    // We frequently delete from the back to prune history, so we use a deque rather than a vector.
    std::deque<TimeTravelUpdate> log;

    // Contains file hashes for the current version of global state.
    std::vector<core::FileHash> globalStateHashes;

    // Internal function: Applies given update (or undoes it) and appropriately updates `activeVersion`.
    std::vector<core::FileRef> applyUpdate(TimeTravelUpdate &update, bool undo);

    std::vector<core::FileHash> computeStateHashes(u4 version,
                                                   const std::vector<std::shared_ptr<core::File>> &files) const;

public:
    TimeTravelingGlobalState(const std::shared_ptr<LSPConfiguration> &config, std::unique_ptr<core::GlobalState> gs,
                             u4 initialVersion);

    /**
     * Travels GlobalState forwards and backwards in time. No-op if version == current version.
     */
    void travel(u4 version);

    /**
     * Indicates that TTGS operations will happen on a new thread. Used to work around error queue ENFORCEs.
     */
    void switchToNewThread();

    const core::GlobalState &getGlobalState() const;
    const std::vector<core::FileHash> &getGlobalStateHashes() const;

    /**
     * Prunes entries in the time travel log that fall before version.
     * Note: Supports a version ID that wraps around.
     */
    void pruneBefore(u4 version);

    /**
     * Applies the given update to GlobalState, appends a corresponding entry to the undo log, and indexes the update.
     * Mutates update to include file hashes and indexes.
     */
    void commitEdits(LSPFileUpdates &update);

    /**
     * Indexes all workspace files from file system and hashes them.
     * This runs code that is not considered performance critical and this is expected to be slow.
     */
    std::vector<ast::ParsedFile> indexFromFileSystem();

    /**
     * Returns `true` if the given changes can run on the fast path relative to the provided version of global state.
     * Only requires `updatedFileHashes` and `updatedFiles`.
     *
     * TODO(jvilk): Return reason and track slow path stats.
     */
    bool canTakeFastPath(u4 fromVersion, const LSPFileUpdates &updates);

    /**
     * Get a combined LSPFileUpdates containing edits from [fromId, toId] (inclusive).
     */
    LSPFileUpdates getCombinedUpdates(u4 fromId, u4 toId);

    /**
     * Returns true if `a` comes before `b`.
     */
    bool comesBefore(u4 a, u4 b) const;

    /**
     * If version `a` comes before `b`, returns `a` else `b`.
     */
    u4 minVersion(u4 a, u4 b) const;
};

} // namespace sorbet::realmain::lsp
#endif // RUBY_TYPER_LSP_TIMETRAVELINGGLOBALSTATE_H
