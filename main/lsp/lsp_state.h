#include "ast/ast.h"
#include "core/core.h"

class WorkerPool;

namespace spdlog {
class logger;
}

namespace sorbet {
class KeyValueStore;
}

namespace sorbet::realmain::options {
struct Options;
}

namespace sorbet::realmain::lsp {

struct TypecheckRun {
    std::vector<std::unique_ptr<core::Error>> errors;
    std::vector<core::FileRef> filesTypechecked;
    std::vector<std::unique_ptr<core::lsp::QueryResponse>> responses;
    // The global state, post-typechecking.
    std::unique_ptr<core::GlobalState> gs;
};

class LSPState final {
private:
    /** Trees that have been indexed and can be reused between different runs */
    std::vector<ast::ParsedFile> indexed GUARDED_BY(mtx);
    /** Hashes of global states obtained by resolving every file in isolation. Used for fastpath. */
    std::vector<unsigned int> globalStateHashes GUARDED_BY(mtx);
    /**
     * `initialGS` is used for indexing. It accumulates a huge nametable of all global things,
     * and is updated as global things are added/removed/updated. It is never discarded.
     *
     * Typechecking is never run on `initialGS` directly. Instead, LSPLoop clones `initialGS` and runs type checking on
     * the clone. This clone is what LSPLoop returns within a `TypecheckRun`.
     */
    std::unique_ptr<core::GlobalState> initialGS GUARDED_BY(mtx);
    std::unique_ptr<KeyValueStore> kvstore; // always null for now.
    WorkerPool &workers;
    /** Concrete error queue shared by all global states */
    std::shared_ptr<core::ErrorQueue> errorQueue GUARDED_BY(mtx);
    /** The set of files currently open in the user's editor. */
    UnorderedSet<std::string> openFiles GUARDED_BY(mtx);

    std::shared_ptr<spdlog::logger> logger;

    /** If true, LSPLoop will skip configatron during type checking */
    const bool skipConfigatron;
    /** If true, all queries will hit the slow path. */
    const bool disableFastPath;

public:
    /**
     * Mutex that must be held during all operations on LSPState and all memory transitively accessible from LSPState.
     */
    absl::Mutex mtx;
    const options::Options &opts;

    LSPState(std::unique_ptr<core::GlobalState> gs, const std::shared_ptr<spdlog::logger> &logger,
             const options::Options &opts, WorkerPool &workers, bool skipConfigatron, bool disableFastPath);

    /** Invalidate all currently cached trees and re-index them from file system.
     * This runs code that is not considered performance critical and this is expected to be slow */
    TypecheckRun reIndexFromFileSystem() EXCLUSIVE_LOCKS_REQUIRED(mtx);

    std::vector<unsigned int> computeStateHashes(const std::vector<std::shared_ptr<core::File>> &files);

    core::FileRef updateFile(const std::shared_ptr<core::File> &file) EXCLUSIVE_LOCKS_REQUIRED(mtx);

    /** Conservatively rerun entire pipeline without caching any trees */
    TypecheckRun runSlowPath(const std::vector<std::shared_ptr<core::File>> &changedFiles)
        EXCLUSIVE_LOCKS_REQUIRED(mtx);
    /** Apply conservative heuristics to see if we can run a fast path, if not, bail out and run slowPath */
    TypecheckRun tryFastPath(std::unique_ptr<core::GlobalState> gs,
                             std::vector<std::shared_ptr<core::File>> &changedFiles, bool allFiles = false)
        EXCLUSIVE_LOCKS_REQUIRED(mtx);

    TypecheckRun runLSPQuery(std::unique_ptr<core::GlobalState> gs, const core::lsp::Query &q,
                             std::vector<std::shared_ptr<core::File>> &changedFiles, bool allFiles = false)
        EXCLUSIVE_LOCKS_REQUIRED(mtx);

    core::FileRef findFileByPath(std::string_view path) EXCLUSIVE_LOCKS_REQUIRED(mtx);
    std::optional<std::string> getFileContents(core::FileRef fref) EXCLUSIVE_LOCKS_REQUIRED(mtx);

    void openFile(std::string_view path) EXCLUSIVE_LOCKS_REQUIRED(mtx);
    void closeFile(std::string_view path) EXCLUSIVE_LOCKS_REQUIRED(mtx);
    bool isFileOpen(std::string_view path) EXCLUSIVE_LOCKS_REQUIRED(mtx);
};
} // namespace sorbet::realmain::lsp