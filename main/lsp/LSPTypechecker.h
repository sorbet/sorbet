#ifndef RUBY_TYPER_LSP_LSPTYPECHECKER_H
#define RUBY_TYPER_LSP_LSPTYPECHECKER_H

#include "ast/ast.h"
#include "core/ErrorFlusher.h"
#include "core/core.h"
#include "main/lsp/ErrorReporter.h"
#include "main/lsp/LSPConfiguration.h"
#include "main/lsp/LSPFileUpdates.h"

namespace sorbet {
class WorkerPool;
class KeyValueStore;
} // namespace sorbet

namespace sorbet::core::lsp {
class PreemptionTaskManager;
class QueryResponse;
} // namespace sorbet::core::lsp

namespace sorbet::realmain::lsp {
class ResponseError;
class InitializedTask;
class TaskQueue;
class DidChangeConfigurationParams;

struct LSPQueryResult {
    std::vector<std::unique_ptr<core::lsp::QueryResponse>> responses;
    // (Optional) Error that occurred during the query that you can pass on to the client.
    std::unique_ptr<ResponseError> error;
};

class UndoState;

/**
 * Encapsulates typechecker operations and enforces that they happen on a single thread.
 */
class LSPTypechecker final {
    /** Contains the ID of the thread responsible for typechecking. */
    std::thread::id typecheckerThreadId;
    /**
     * GlobalState used for typechecking. It is replaced during indexing on the slow path, and is always replaced with
     * a GlobalState that derives from the one created during LSP initialization. This derivation is guaranteed by
     * passing a copy of that GlobalState to the indexer thread at the end of initialization, and then copying that
     * GlobalState back over to the typechecker to use as the starting point for the next slow path.
     */
    std::unique_ptr<core::GlobalState> gs;
    /**
     * Trees that have been indexed with the GlobalState that was originally supplied during initialization
     * (initialGS). As all values of this->gs will derive from that initial GlobalState, none of these trees will be
     * re-indexed for subsequent slow path runs. An additional consequence of this->gs deriving from the initialization
     * value of GlobalState is that they will continue to be valid when used with this->gs (see the comment on this->gs
     * for an explanation of how this derivation is ensured).
     *
     * WARNING:
     * Updates to this vector can happen through this->commitFileUpdates and LSPFileUpdates::updatedFileIndexes, however
     * those updates will only be valid when used with the indexer's GlobalState. As a result it is absolutely necessary
     * to ensure that any updates to this vector are either already valid with this->gs, or have a corresponding tree in
     * this->indexedFinalGS, as otherwise there is a possibility that the tree used will reference names that aren't
     * consistent with this->gs. Once a slow path is kicked off this problem will resolve itself, as this->gs will be
     * re-initialized with a copy of the indexer's GlobalState, making all of the names stored in the trees of
     * this->indexed valid again.
     */
    std::vector<ast::ParsedFile> indexed;
    /**
     * Trees that have been indexed with this->gs between slow path runs, which means that they may have names that are
     * not present in the name table of the indexer. This is a sparse diff of trees indexed by position in
     * this->indexed, and should be consulted before this->indexed when looking up trees. Pelase see the WARNING section
     * of the comment on this->indexed for more context about why. This lookup strategy is implemented by
     * this->getIndexed. All of the trees in this map are valid to use with this->gs.
     */
    UnorderedMap<int, ast::ParsedFile> indexedFinalGS;

    /** Set only when typechecking is happening on the slow path. Contains all of the state needed to restore
     * LSPTypechecker to its pre-slow-path state. Can be null, which indicates that no slow path is currently running */
    std::unique_ptr<UndoState> cancellationUndoState;

    std::shared_ptr<const LSPConfiguration> config;
    /** Used to preempt running slow paths. */
    std::shared_ptr<core::lsp::PreemptionTaskManager> preemptManager;
    /** Used for assertions. Indicates if `initialize` has been run. */
    bool initialized = false;

    std::shared_ptr<ErrorReporter> errorReporter;

    /** Used in tests to force the slow path to block just after cancellation state has been set. */
    bool slowPathBlocked ABSL_GUARDED_BY(slowPathBlockedMutex) = false;
    absl::Mutex slowPathBlockedMutex;

    enum class SlowPathMode {
        Init,
        Cancelable,
    };

    struct SlowPathResult {
        // True when the changes from the slow path have been committed. A false value here indicates that the slow path
        // operation was cancelled.
        bool committed = false;

        // When the slow path was run with `SlowPathMode::Init`, this is a copy of the GlobalState that is taken right
        // after indexing has completed. This GlobalState is suitable to be given to the indexer thread, as its name
        // table will be populated with the current state of the codebase.
        std::unique_ptr<core::GlobalState> indexedState;
    };

    /** Conservatively reruns entire pipeline without caching any trees. Returns 'true' if committed, 'false' if
     * canceled. */
    SlowPathResult runSlowPath(LSPFileUpdates updates, std::unique_ptr<KeyValueStore> kvstore, WorkerPool &workers,
                               std::shared_ptr<core::ErrorFlusher> errorFlusher, SlowPathMode mode);

    /** Runs incremental typechecking on the provided updates. Returns the final list of files typechecked. */
    std::vector<core::FileRef> runFastPath(LSPFileUpdates &updates, WorkerPool &workers,
                                           std::shared_ptr<core::ErrorFlusher> errorFlusher,
                                           bool isNoopUpdateForRetypecheck) const;

    /** Commits the given file updates to LSPTypechecker. Does not send diagnostics. */
    void commitFileUpdates(LSPFileUpdates &updates, bool couldBeCanceled);

    /** Deep copy all entries in `indexed` that contain ASTs, except for those with IDs in the ignore set. Returns true
     * on success, false if the operation was canceled. */
    bool copyIndexed(WorkerPool &workers, const UnorderedSet<int> &ignore, std::vector<ast::ParsedFile> &out) const;

public:
    LSPTypechecker(std::shared_ptr<const LSPConfiguration> config,
                   std::shared_ptr<core::lsp::PreemptionTaskManager> preemptionTaskManager);
    ~LSPTypechecker();

    /**
     * Conducts the first typechecking pass of the session, and initializes `gs` and `index`
     * variables. Must be called before typecheck and other functions work.
     *
     * Writes all diagnostic messages to LSPOutput.
     */
    void initialize(TaskQueue &queue, std::unique_ptr<core::GlobalState> gs, std::unique_ptr<KeyValueStore> kvstore,
                    WorkerPool &workers, const LSPConfiguration &currentConfig);

    /**
     * Typechecks the given input. Returns 'true' if the updates were committed, or 'false' if typechecking was
     * canceled. Distributes work across the given worker pool.
     */
    bool typecheck(LSPFileUpdates updates, WorkerPool &workers,
                   std::vector<std::unique_ptr<Timer>> diagnosticLatencyTimers);

    /**
     * Re-typechecks the provided files to re-produce error messages.
     */
    std::vector<std::unique_ptr<core::Error>> retypecheck(std::vector<core::FileRef> frefs, WorkerPool &workers) const;

    /** Runs the provided query against the given files, and returns matches. */
    LSPQueryResult query(const core::lsp::Query &q, const std::vector<core::FileRef> &filesForQuery,
                         WorkerPool &workers) const;

    /**
     * Returns the parsed file for the given file, up to the desugar pass, and local vars.
     *
     * This is never cached, which means that the file will be re-parsed from scratch.
     * This is slower than getting the indexed tree (everything before namer), so if
     * You can use the indexed tree that will be more performant. Certain IDE actions
     * need particularly fine-grained fidelity in the AST (precludes rewriter).
     */
    ast::ExpressionPtr getLocalVarTrees(core::FileRef fref) const;
    /**
     * Returns the most up-to-date indexed tree for the file ref. The tree will either come from this->indexed if it has
     * not been modified since the LSP process has been initialized, or from this->indexedFinalGS if it has been.
     */
    const ast::ParsedFile &getIndexed(core::FileRef fref) const;

    /**
     * Returns copies of the indexed trees that have been run through the incremental resolver.
     */
    std::vector<ast::ParsedFile> getResolved(absl::Span<const core::FileRef> frefs, WorkerPool &workers) const;

    /**
     * Returns a copy of the indexed tree that has been run through the incremental resolver.
     */
    ast::ParsedFile getResolved(core::FileRef fref, WorkerPool &workers) const;

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
     * (For tests only) Set a flag that forces the slow path to block indefinitely after saving undo state. Setting
     * this flag to `false` will immediately unblock any currently blocked slow paths.
     */
    void setSlowPathBlocked(bool blocked);

    /**
     * Exposes very limited mutability to typechecker's global state in order to support the client changing
     * options (such as highlighting untyped code) without doing a full restart of Sorbet.
     */
    void updateGsFromOptions(const DidChangeConfigurationParams &options) const;

    /**
     * Get an LSPFileUpdates containing the latest versions of the given files. It's a "no-op" file update because it
     * doesn't actually change anything.
     */
    LSPFileUpdates getNoopUpdate(std::vector<core::FileRef> frefs) const;
};

/**
 * Provides lambdas with a set of operations that they are allowed to do with the LSPTypechecker.
 */
class LSPTypecheckerDelegate final {
    LSPTypechecker &typechecker;

    TaskQueue &queue;

    /** The WorkerPool on which work will be performed. If the task is multithreaded, the pool will contain multiple
     * worker threads. */
    WorkerPool &workers;

public:
    /**
     * Creates a new delegate that runs LSPTypechecker operations on the WorkerPool threads.
     */
    LSPTypecheckerDelegate(TaskQueue &queue, WorkerPool &workers, LSPTypechecker &typechecker);

    // Delete copy constructor / assignment.
    LSPTypecheckerDelegate(LSPTypecheckerDelegate &) = delete;
    LSPTypecheckerDelegate(const LSPTypecheckerDelegate &) = delete;
    LSPTypecheckerDelegate &operator=(LSPTypecheckerDelegate &&) = delete;
    LSPTypecheckerDelegate &operator=(const LSPTypecheckerDelegate &) = delete;

    virtual ~LSPTypecheckerDelegate() = default;

    void initialize(InitializedTask &task, std::unique_ptr<core::GlobalState> gs,
                    std::unique_ptr<KeyValueStore> kvstore, const LSPConfiguration &currentConfig);

    void resumeTaskQueue(InitializedTask &task);

    void typecheckOnFastPath(LSPFileUpdates updates, std::vector<std::unique_ptr<Timer>> diagnosticLatencyTimers);
    std::vector<std::unique_ptr<core::Error>> retypecheck(std::vector<core::FileRef> frefs) const;
    LSPQueryResult query(const core::lsp::Query &q, const std::vector<core::FileRef> &filesForQuery) const;
    const ast::ParsedFile &getIndexed(core::FileRef fref) const;
    std::vector<ast::ParsedFile> getResolved(absl::Span<const core::FileRef> frefs) const;
    ast::ParsedFile getResolved(core::FileRef fref) const;
    ast::ExpressionPtr getLocalVarTrees(core::FileRef fref) const;

    const core::GlobalState &state() const;

    void updateGsFromOptions(const DidChangeConfigurationParams &options) const;
    LSPFileUpdates getNoopUpdate(std::vector<core::FileRef> frefs) const;
};
} // namespace sorbet::realmain::lsp
#endif
