#ifndef RUBY_TYPER_LSP_LSPFILEUPDATES_H
#define RUBY_TYPER_LSP_LSPFILEUPDATES_H

#include "ast/ast.h"
#include "common/common.h"
#include "core/FileHash.h"
#include "main/lsp/LSPConfiguration.h"

namespace sorbet::realmain::lsp {
/**
 * Encapsulates an update to LSP's file state in a compact form.
 */
class LSPFileUpdates final {
public:
    // This specific update contains edits with the given epoch
    uint32_t epoch = 0;
    // The total number of edits that this update represents. Used for stats and assertions.
    uint32_t editCount = 0;
    // The total number of edits in this update that are already committed & had diagnostics sent out (via preemption).
    // Used for stats and assertions.
    uint32_t committedEditCount = 0;

    std::vector<std::shared_ptr<core::File>> updatedFiles;

    TypecheckingPath typecheckingPath = TypecheckingPath::Slow;

    // Indicates whether or not the incremental namer should be used on the fast path.
    bool fastPathUseIncrementalNamer = false;

    // The paths of any additional files implicated in a fast path edit. This vector stores path
    // strings because FileRef IDs might be different in the GlobalState used on the indexing thread
    // and the typechecking thread, but the `LSPFileUpdates` structure must be valid to transfer
    // ownership from one thread to the other.
    std::vector<std::string> fastPathExtraFiles;

    // Indicates that this update contains a new file. Is a hack for determining if combining two updates can take the
    // fast path.
    bool hasNewFiles = false;
    // If true, this update caused a slow path to be canceled.
    bool canceledSlowPath = false;
    // Updated on typechecking thread. Contains indexes processed with typechecking global state.
    std::vector<ast::ParsedFile> updatedFinalGSFileIndexes;
    // (Used in tests) Ensures that a slow path typecheck on these updates waits until it gets cancelled.
    bool cancellationExpected = false;
    // (Used in tests) Ensures that a slow path typecheck waits until this number of preemption occurs before finishing.
    int preemptionsExpected = 0;

    /**
     * Merges the given (and older) LSPFileUpdates object into this LSPFileUpdates object.
     *
     * Resets `getTypecheckingPath`.
     */
    void mergeOlder(const LSPFileUpdates &older);

    /**
     * Returns a copy of this LSPFileUpdates object. Does not handle deepCopying `updatedGS`.
     */
    LSPFileUpdates copy() const;

    LSPFileUpdates() = default;
    LSPFileUpdates(LSPFileUpdates &&other) = default;
    LSPFileUpdates &operator=(LSPFileUpdates &&other) = default;

    LSPFileUpdates(const LSPFileUpdates &other) = delete;
    LSPFileUpdates &operator=(const LSPFileUpdates &other) = delete;

    struct FastPathFilesToTypecheckResult {
        // The number of files that would be checked in the fast path.
        size_t totalChanged = 0;

        // True when we should use the incremental namer, which happens if a symbol name changed and we brought in
        // additional related files to check.
        bool useIncrementalNamer = false;

        // Extra files that need to be typechecked because the file mentions the name of one of the changed symbols.
        std::vector<std::string> extraFiles;
    };

    // It would be nice to have this accept `...<const core::File>...`
    // As written, it's possible to mutate the files, but that's not intentional.
    static FastPathFilesToTypecheckResult
    fastPathFilesToTypecheck(const core::GlobalState &gs, const LSPConfiguration &config,
                             const std::vector<std::shared_ptr<core::File>> &updatedFiles);

    // Overload because sometimes we have to look up the old file's hash in GlobalState (when
    // running on the typechecker thread), and sometimes we have to look it up in evictedFiles (when
    // running on the indexer thread).
    static FastPathFilesToTypecheckResult
    fastPathFilesToTypecheck(const core::GlobalState &gs, const LSPConfiguration &config,
                             const std::vector<std::shared_ptr<core::File>> &updatedFiles,
                             const UnorderedMap<core::FileRef, std::shared_ptr<core::File>> &evictedFiles);
};
} // namespace sorbet::realmain::lsp

#endif
