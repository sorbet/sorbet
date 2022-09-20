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
    /** GlobalState used for typechecking. */
    std::unique_ptr<core::GlobalState> gs;
    /** Trees that have been indexed (with initialGS) and can be reused between different runs */
    std::vector<ast::ParsedFile> indexed;
    /** Trees that have been indexed (with finalGS) and can be reused between different runs */
    UnorderedMap<int, ast::ParsedFile> indexedFinalGS;

    /** Used to guard access to `cancellationUndoState` and specifically the stale
     * `GlobalState` residing inside of it. */
    absl::Mutex cancellationUndoStateRWLock;
    /** Set only when typechecking is happening on the slow path. Contains all of the state needed to restore
     * LSPTypechecker to its pre-slow-path state. Can be null, which indicates that no slow path is currently running */
    std::unique_ptr<UndoState> cancellationUndoState ABSL_GUARDED_BY(cancellationUndoStateRWLock);

    std::shared_ptr<const LSPConfiguration> config;
    /** Used to preempt running slow paths. */
    std::shared_ptr<core::lsp::PreemptionTaskManager> preemptManager;
    /** Used for assertions. Indicates if `initialize` has been run. */
    bool initialized = false;

    std::shared_ptr<ErrorReporter> errorReporter;

    /** Used in tests to force the slow path to block just after cancellation state has been set. */
    bool slowPathBlocked ABSL_GUARDED_BY(slowPathBlockedMutex) = false;
    mutable absl::Mutex slowPathBlockedMutex;

    /** Conservatively reruns entire pipeline without caching any trees. Returns 'true' if committed, 'false' if
     * canceled. */
    bool runSlowPath(LSPFileUpdates updates, WorkerPool &workers, bool cancelable,
                     std::shared_ptr<core::ErrorFlusher> errorFlusher);

    /** Runs incremental typechecking on the provided updates. Returns the final list of files typechecked. */
    std::vector<core::FileRef> runFastPath(LSPFileUpdates &updates, WorkerPool &workers,
                                           std::shared_ptr<core::ErrorFlusher> errorFlusher) const;

    /** Commits the given file updates to LSPTypechecker. Does not send diagnostics. */
    void commitFileUpdates(LSPFileUpdates &updates, bool couldBeCanceled);

    /**
     * Get an LSPFileUpdates containing the latest versions of the given files. It's a "no-op" file update because it
     * doesn't actually change anything.
     */
    LSPFileUpdates getNoopUpdate(std::vector<core::FileRef> frefs) const;

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
                    WorkerPool &workers);

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
     * Returns the parsed file for the given file, up to the index passes (does not include resolver passes).
     */
    const ast::ParsedFile &getIndexed(core::FileRef fref) const;

    /**
     * Returns the parsed files for the given files, including resolver.
     */
    std::vector<ast::ParsedFile> getResolved(const std::vector<core::FileRef> &frefs) const;

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
     * Tries to run the function on the stale undo state, acquiring a lock.
     */
    bool tryRunOnStaleState(std::function<void(UndoState &)> func);

    /**
     * (For tests only) Set a flag that forces the slow path to block indefinitely after saving undo state. Setting
     * this flag to `false` will immediately unblock any currently blocked slow paths.
     */
    void setSlowPathBlocked(bool blocked);

    /** (For tests only) Checks if the `setSlowPathBlocked` flag is set. */
    bool isSlowPathBlocked() const;
};

/**
 * Provides lambdas with a set of operations that they are allowed to do with the LSPTypechecker.
 */
class LSPTypecheckerInterface {
public:
    /**
     * Special case handling for the initialized task, which coordinates global state initialization with the
     * typechecker and indexer threads.
     */
    virtual void initialize(InitializedTask &task, std::unique_ptr<core::GlobalState> gs,
                            std::unique_ptr<KeyValueStore> kvstore) = 0;

    /**
     * Resume processing of the task queue
     */
    virtual void resumeTaskQueue(InitializedTask &task) = 0;

    /**
     * Typechecks the given input on the fast path. The edit *must* be a fast path edit!
     */
    virtual void typecheckOnFastPath(LSPFileUpdates updates,
                                     std::vector<std::unique_ptr<Timer>> diagnosticLatencyTimers) = 0;

    /**
     * Re-typechecks the provided files to re-produce error messages.
     */
    virtual std::vector<std::unique_ptr<core::Error>> retypecheck(std::vector<core::FileRef> frefs) const = 0;

    /** Runs the provided query against the given files, and returns matches. */
    virtual LSPQueryResult query(const core::lsp::Query &q, const std::vector<core::FileRef> &filesForQuery) const = 0;

    /**
     * Returns the parsed file for the given file, up to the index passes (does not include resolver passes).
     */
    virtual const ast::ParsedFile &getIndexed(core::FileRef fref) const = 0;

    /**
     * Returns the parsed files for the given files, including resolver.
     */
    virtual std::vector<ast::ParsedFile> getResolved(const std::vector<core::FileRef> &frefs) const = 0;

    /**
     * Returns the currently active GlobalState.
     */
    virtual const core::GlobalState &state() const = 0;

    /**
     * Returns `true` if the delegate is processing stale state.
     *
     * TODO(aprocter): We may eventually delurk this because we would ideally like the difference
     * between the interface for stale and fresh state to be opaque, with no special-case logic
     * inside tasks when running on stale state.
     */
    virtual bool isStale() const {
        return false;
    }
};

class LSPStaleTypechecker final : public LSPTypecheckerInterface {
    std::shared_ptr<const LSPConfiguration> config;
    UndoState &undoState;
    // Using an WorkerPool with size 0 for all typechecker operations causes the work to run on the
    // current thread (usually: the indexer thread).
    std::unique_ptr<WorkerPool> emptyWorkers;

public:
    LSPStaleTypechecker(std::shared_ptr<const LSPConfiguration> config, UndoState &undoState);

    // Delete copy constructor / assignment.
    LSPStaleTypechecker(LSPStaleTypechecker &) = delete;
    LSPStaleTypechecker(const LSPStaleTypechecker &) = delete;
    LSPStaleTypechecker &operator=(LSPStaleTypechecker &&) = delete;
    LSPStaleTypechecker &operator=(const LSPStaleTypechecker &) = delete;

    virtual ~LSPStaleTypechecker() = default;

    void initialize(InitializedTask &task, std::unique_ptr<core::GlobalState> gs,
                    std::unique_ptr<KeyValueStore> kvstore) override;

    void resumeTaskQueue(InitializedTask &task) override;

    void typecheckOnFastPath(LSPFileUpdates updates,
                             std::vector<std::unique_ptr<Timer>> diagnosticLatencyTimers) override;

    std::vector<std::unique_ptr<core::Error>> retypecheck(std::vector<core::FileRef> frefs) const override;

    LSPQueryResult query(const core::lsp::Query &q, const std::vector<core::FileRef> &filesForQuery) const override;

    const ast::ParsedFile &getIndexed(core::FileRef fref) const override;

    std::vector<ast::ParsedFile> getResolved(const std::vector<core::FileRef> &frefs) const override;

    const core::GlobalState &state() const override;

    bool isStale() const override {
        return true;
    }
};

/**
 * An implementation of LSPTypecheckerInterface used for tasks running on the latest GlobalState, in the typechecking
 * thread.
 */
class LSPTypecheckerDelegate final : public LSPTypecheckerInterface {
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
                    std::unique_ptr<KeyValueStore> kvstore) override;

    void resumeTaskQueue(InitializedTask &task) override;

    void typecheckOnFastPath(LSPFileUpdates updates,
                             std::vector<std::unique_ptr<Timer>> diagnosticLatencyTimers) override;
    std::vector<std::unique_ptr<core::Error>> retypecheck(std::vector<core::FileRef> frefs) const override;
    LSPQueryResult query(const core::lsp::Query &q, const std::vector<core::FileRef> &filesForQuery) const override;
    const ast::ParsedFile &getIndexed(core::FileRef fref) const override;
    std::vector<ast::ParsedFile> getResolved(const std::vector<core::FileRef> &frefs) const override;
    const core::GlobalState &state() const override;
};
} // namespace sorbet::realmain::lsp
#endif
