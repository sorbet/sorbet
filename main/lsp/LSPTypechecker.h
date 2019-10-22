#ifndef RUBY_TYPER_LSP_LSPTYPECHECKER_H
#define RUBY_TYPER_LSP_LSPTYPECHECKER_H

#include "ast/ast.h"
#include "common/concurrency/WorkerPool.h"
#include "common/kvstore/KeyValueStore.h"
#include "core/ErrorQueue.h"
#include "core/NameHash.h"
#include "core/core.h"
#include "json_types.h"
#include "main/lsp/LSPConfiguration.h"

namespace sorbet::realmain::lsp {

struct LSPQueryResult {
    std::vector<std::unique_ptr<core::lsp::QueryResponse>> responses;
    // (Optional) Error that occurred during the query that you can pass on to the client.
    std::unique_ptr<ResponseError> error = nullptr;
};

class TypecheckRun final {
public:
    // Errors encountered during typechecking.
    std::vector<std::unique_ptr<core::Error>> errors;
    // The set of files that were typechecked for errors.
    std::vector<core::FileRef> filesTypechecked;
    // The edit applied to `gs`.
    LSPFileUpdates updates;
    // Specifies if the typecheck run took the fast or slow path.
    bool tookFastPath;
    // Specifies if the typecheck run was canceled.
    bool canceled = false;

    TypecheckRun(std::vector<std::unique_ptr<core::Error>> errors = {},
                 std::vector<core::FileRef> filesTypechecked = {}, LSPFileUpdates updates = {},
                 bool tookFastPath = false);

    // Make a canceled TypecheckRun.
    static TypecheckRun makeCanceled();
};

/**
 * Contains the LSPTypechecker state that is needed to cancel a running slow path operation and any subsequent fast path
 * operations that have preempted it.
 */
class LSPTypecheckerUndoState final {
public:
    // The versionEnd of the update that caused this state to come into existance in the first place.
    u4 version;
    // Stores the pre-slow-path global state.
    std::unique_ptr<core::GlobalState> gs;
    // Stores index trees containing data stored in `gs` that have been evacuated during the slow path operation.
    UnorderedMap<int, ast::ParsedFile> indexed;
    // Stores file hashes that have been evacuated during the slow path operation.
    UnorderedMap<int, core::FileHash> globalStateHashes;
    // Stores the index trees stored in `gs` that were evacuated because the slow path operation replaced `gs`.
    UnorderedMap<int, ast::ParsedFile> indexedFinalGS;
    // Stores the list of files that had errors before the slow path began.
    std::vector<core::FileRef> filesThatHaveErrors;

    LSPTypecheckerUndoState(u4 version, std::unique_ptr<core::GlobalState> oldGS,
                            UnorderedMap<int, ast::ParsedFile> oldIndexedFinalGS,
                            std::vector<core::FileRef> oldFilesThatHaveErrors);

    /**
     * Records that the given items were evicted from LSPTypechecker following a typecheck run.
     */
    void recordEvictedState(ast::ParsedFile replacedIndexTree, core::FileHash replacedStateHash);
};

/**
 * Provides lambdas with a set of operations that they are allowed to do with the LSPTypechecker.
 */
class LSPTypechecker final {
    /** Contains the ID of the thread responsible for typechecking. */
    std::thread::id typecheckerThreadId;
    /** A mutex that protects global state on non-typechecker threads. Grabbed on typechecker thread _only_ when gs is
     * going to be updated, and on secondary threads for RO operations. */
    absl::Mutex gsMutex;
    /** GlobalState used for typechecking. Mutable because typechecking routines, even when not changing the GlobalState
     * instance, actively consume and replace GlobalState. */
    mutable std::unique_ptr<core::GlobalState> gs;
    /** Trees that have been indexed (with initialGS) and can be reused between different runs */
    std::vector<ast::ParsedFile> indexed;
    /** Trees that have been indexed (with finalGS) and can be reused between different runs */
    UnorderedMap<int, ast::ParsedFile> indexedFinalGS;
    /** Hashes of global states obtained by resolving every file in isolation. Used for fastpath. */
    std::vector<core::FileHash> globalStateHashes;
    /** List of files that had errors in last run*/
    std::vector<core::FileRef> filesThatHaveErrors;
    std::unique_ptr<KeyValueStore> kvstore; // always null for now.
    /** Set only when typechecking is happening on the slow path. Contains all of the state needed to restore
     * LSPTypechecker to its pre-slow-path state. */
    std::optional<LSPTypecheckerUndoState> cancellationUndoState;
    std::shared_ptr<std::shared_ptr<std::function<void()>>> preemptFunction;

    std::shared_ptr<const LSPConfiguration> config;

    /** Conservatively reruns entire pipeline without caching any trees. Returns `true` if slow path successfully
     * completed, or `false` if it was canceled. If `typecheckLock` is supplied, operation is cancelable and
     * preemptible. */
    bool runSlowPath(LSPFileUpdates updates, bool cancelableAndPreemptible);
    /** Runs incremental typechecking on the provided updates. */
    TypecheckRun runFastPath(LSPFileUpdates updates) const;

    /**
     * Sends diagnostics from a typecheck run to the client.
     * `version` specifies the version of the file updates that produced these diagnostics. Used to prevent emitting
     * outdated diagnostics from a slow path run if they had already been re-typechecked on the fast path.
     */
    void pushDiagnostics(u4 version, std::vector<core::FileRef> filesTypechecked,
                         std::vector<std::unique_ptr<core::Error>> errors);

    void commitFileUpdates(LSPFileUpdates &updates, bool tookFastPath, bool couldBeCanceled);

    void commitTypecheckRun(TypecheckRun run);

    /**
     * Undoes the given slow path changes on LSPTypechecker, and clears the client's error list for any files that were
     * newly introduced with the canceled update. Returns a list of files that need to be retypechecked to update their
     * error lists.
     */
    std::vector<core::FileRef> restore(LSPTypecheckerUndoState &undoState);

public:
    LSPTypechecker(const std::shared_ptr<const LSPConfiguration> &config);
    ~LSPTypechecker() = default;

    /**
     * Conducts the first typechecking pass of the session, and initializes `gs`, `index`, and `globalStateHashes`
     * variables. Must be called before typecheck and other functions work.
     *
     * Writes all diagnostic messages to LSPOutput.
     */
    void initialize(LSPFileUpdates updates);

    /**
     * Typechecks the given input. Returns 'true' if the updates were committed, or 'false' if typechecking was
     * canceled. Guaranteed to return `true` if `cancelableAndPreemptible` is `false`.
     */
    bool typecheck(LSPFileUpdates updates, bool cancelableAndPreemptible);

    /**
     * Re-typechecks the provided input to re-produce error messages. Input *must* match already committed state!
     * Provided to facilitate code actions.
     */
    TypecheckRun retypecheck(LSPFileUpdates updates) const;

    /** Runs the provided query against the given files, and returns matches. */
    LSPQueryResult query(const core::lsp::Query &q, const std::vector<core::FileRef> &filesForQuery) const;

    /**
     * Returns the parsed file for the given file, up to the index passes (does not include resolver passes).
     */
    const ast::ParsedFile &getIndexed(core::FileRef fref) const;

    /**
     * Returns the parsed files for the given files, including resolver.
     */
    std::vector<ast::ParsedFile> getResolved(const std::vector<core::FileRef> &frefs) const;

    /**
     * Returns the hashes of all committed files.
     */
    const std::vector<core::FileHash> &getFileHashes() const;

    /**
     * Returns the currently active GlobalState.
     */
    const core::GlobalState &state() const;

    /**
     * Called by LSPTypecheckerCoordinator to indicate that typechecking will occur on the current thread.
     */
    void changeThread();

    /**
     * Returns the typechecker's internal global state, which effectively destroys the typechecker for further use.
     */
    std::unique_ptr<core::GlobalState> destroy();

    /**
     * If the slow path is running, preempt it by running this lambda.
     * Returns 'true' if the lambda will definitely preempt the slow path.
     */
    bool tryPreemptSlowPath(std::function<void()> &lambda);
};

} // namespace sorbet::realmain::lsp
#endif
