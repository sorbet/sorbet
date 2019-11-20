#ifndef RUBY_TYPER_LSP_LSPTYPECHECKER_H
#define RUBY_TYPER_LSP_LSPTYPECHECKER_H

#include "ast/ast.h"
#include "common/concurrency/WorkerPool.h"
#include "common/kvstore/KeyValueStore.h"
#include "core/ErrorQueue.h"
#include "core/NameHash.h"
#include "core/core.h"
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
    // If update took the slow path, contains a new global state that should be used moving forward.
    std::optional<std::unique_ptr<core::GlobalState>> newGS;

    TypecheckRun(std::vector<std::unique_ptr<core::Error>> errors = {},
                 std::vector<core::FileRef> filesTypechecked = {}, LSPFileUpdates updates = {},
                 bool tookFastPath = false, std::optional<std::unique_ptr<core::GlobalState>> newGS = std::nullopt);

    // Make a canceled TypecheckRun.
    static TypecheckRun makeCanceled();
};

/**
 * Provides lambdas with a set of operations that they are allowed to do with the LSPTypechecker.
 */
class LSPTypechecker final {
    /** Contains the ID of the thread responsible for typechecking. */
    std::thread::id typecheckerThreadId;
    /** GlobalState used for typechecking. Mutable because typechecking routines, even when not changing the GlobalState
     * instance, actively consume and replace GlobalState. */
    mutable std::unique_ptr<core::GlobalState> gs;
    /** Trees that have been indexed (with initialGS) and can be reused between different runs */
    std::vector<ast::ParsedFile> indexed;
    /** Trees that have been indexed (with finalGS) and can be reused between different runs */
    UnorderedMap<int, ast::ParsedFile> indexedFinalGS;
    /** Hashes of global states obtained by resolving every file in isolation. Used for fastpath. */
    std::vector<core::FileHash> globalStateHashes;
    /** List of files that have had errors in last run*/
    std::vector<core::FileRef> filesThatHaveErrors;
    std::unique_ptr<KeyValueStore> kvstore; // always null for now.

    std::shared_ptr<const LSPConfiguration> config;

    /** Conservatively reruns entire pipeline without caching any trees. If canceled, returns a TypecheckRun containing
     * the previous global state. */
    TypecheckRun runSlowPath(LSPFileUpdates updates, bool cancelable) const;
    /** Runs typechecking on the provided updates. */
    TypecheckRun runTypechecking(LSPFileUpdates updates) const;

    /**
     * Sends diagnostics from a typecheck run to the client.
     */
    void pushDiagnostics(TypecheckRun run);

    /** Officially 'commits' the output of a `TypecheckRun` by updating the relevant state on LSPLoop and, if specified,
     * sending diagnostics to the editor. */
    void commitTypecheckRun(TypecheckRun run);

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
     * canceled.
     */
    bool typecheck(LSPFileUpdates updates);

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
};

} // namespace sorbet::realmain::lsp
#endif
