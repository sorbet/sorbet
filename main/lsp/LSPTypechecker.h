#ifndef RUBY_TYPER_LSP_LSPTYPECHECKER_H
#define RUBY_TYPER_LSP_LSPTYPECHECKER_H

#include "ast/ast.h"
#include "common/concurrency/WorkerPool.h"
#include "common/kvstore/KeyValueStore.h"
#include "core/ErrorQueue.h"
#include "core/NameHash.h"
#include "core/core.h"
#include "main/lsp/LSPConfiguration.h"
#include "main/lsp/LSPOutput.h"

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
    // If update took the slow path, contains a new global state that should be used moving forward.
    std::optional<std::unique_ptr<core::GlobalState>> newGS;

    TypecheckRun(std::vector<std::unique_ptr<core::Error>> errors = {},
                 std::vector<core::FileRef> filesTypechecked = {}, LSPFileUpdates updates = {},
                 bool tookFastPath = false, std::optional<std::unique_ptr<core::GlobalState>> newGS = std::nullopt);

    // Make a canceled TypecheckRun.
    static TypecheckRun makeCanceled();
};

class LSPTypechecker;

/**
 * Provides lambdas with a set of operations that they are allowed to do with the LSPTypechecker.
 */
class LSPTypecheckerOps final {
private:
    friend class LSPTypechecker;
    LSPTypechecker &typechecker;

    LSPTypecheckerOps(LSPTypechecker &typechecker, const core::GlobalState &gs);
    ~LSPTypecheckerOps() = default;

public:
    const core::GlobalState &gs;

    /** Runs the provided query against the given files, and returns matches. */
    LSPQueryResult query(const core::lsp::Query &q, const std::vector<core::FileRef> &filesForQuery) const;

    /** Typechecks the provided files on the fast path. */
    TypecheckRun fastPathTypecheck(LSPFileUpdates updates, LSPOutput &output) const;

    const ast::ParsedFile &getIndex(core::FileRef fref) const;

    const std::vector<core::FileHash> &getFileHashes() const;
};

/**
 * Handles typechecking and other queries. Can either operate in async mode (in which it runs on a dedicated thread)
 * or sync mode.
 */
class LSPTypechecker final {
    friend class LSPTypecheckerOps;

    /** Protects lambda and wasShutdown. */
    absl::Mutex mtx;
    /** Contains a lambda that needs to be run to completion next. */
    std::optional<std::function<void()>> lambda GUARDED_BY(mtx);
    /** If 'true', then the typechecker has shut down. */
    bool wasShutdown GUARDED_BY(mtx);
    /** Active global state instance used for typechecking. Mutable because fast path/query updates replace it, but are
     * innocuous. */
    mutable std::unique_ptr<core::GlobalState> gs;
    /** Trees that have been indexed (with initialGS) and can be reused between different runs */
    std::vector<ast::ParsedFile> indexed;
    /** Trees that have been indexed (with finalGS) and can be reused between different runs */
    UnorderedMap<int, ast::ParsedFile> indexedFinalGS;
    /** Hashes of global states obtained by resolving every file in isolation. Used for fastpath. */
    std::vector<core::FileHash> globalStateHashes;
    /** List of files that have had errors in last run*/
    std::vector<core::FileRef> filesThatHaveErrors;
    /** Contains the ID of the thread responsible for typechecking. */
    std::thread::id typecheckerThreadId;
    std::unique_ptr<KeyValueStore> kvstore; // always null for now.

    std::shared_ptr<spd::logger> logger;
    WorkerPool &workers;
    LSPConfiguration config;

    /** Conservatively reruns entire pipeline without caching any trees. If canceled, returns a TypecheckRun containing
     * the previous global state. */
    TypecheckRun runSlowPath(LSPFileUpdates updates, bool cancelable, LSPOutput &output) const;
    /** Runs typechecking on the provided updates. */
    TypecheckRun runTypechecking(LSPFileUpdates updates, LSPOutput &output) const;

    LSPQueryResult query(const core::lsp::Query &q, const std::vector<core::FileRef> &filesForQuery) const;

    /**
     * Runs the provided function on the typechecker thread. Blocks until complete.
     */
    void runOnTypecheckerThread(std::function<void()> lambda);

    void pushDiagnostics(TypecheckRun run, LSPOutput &output);

    /** Officially 'commits' the output of a `TypecheckRun` by updating the relevant state on LSPLoop and, if specified,
     * sending diagnostics to the editor. */
    void commitTypecheckRun(TypecheckRun run, LSPOutput &output);

public:
    LSPTypechecker(const std::shared_ptr<spd::logger> &logger, WorkerPool &workers, LSPConfiguration config);
    ~LSPTypechecker() = default;

    /**
     * Conducts the first typechecking pass of the session, and initializes `gs`, `index`, and `globalStatehashes`
     * variables. Must be called before typecheck and other functions work. This method always blocks and runs to
     * completion.
     *
     * Writes all diagnostic messages to LSPOutput.
     */
    void initialize(LSPFileUpdates updates, LSPOutput &output);

    /**
     * Typecheck the given file updates, and write any diagnostics to output. Returns 'true' if typecheck completed,
     * 'false' if it was canceled.
     *
     * This function currently always blocks, but it will eventually become asynchronous when using a dedicated
     * typechecking thread.
     */
    bool typecheck(LSPFileUpdates updates, LSPOutput &output);

    /**
     * Runs lambda with exclusive access to GlobalState. lambda runs on typechecker thread, but this method blocks
     * until the lambda runs.
     */
    void enterCriticalSection(std::function<void(const LSPTypecheckerOps &)> lambda);

    void shutdown();

    std::unique_ptr<core::GlobalState> destroyAndReturnGlobalState();

    /** Runs the typechecker in a new thread. */
    std::unique_ptr<Joinable> runTypechecker();
};

} // namespace sorbet::realmain::lsp
#endif
