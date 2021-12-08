#ifndef RUBY_TYPER_LSP_LSPFILEUPDATES_H
#define RUBY_TYPER_LSP_LSPFILEUPDATES_H

#include "ast/ast.h"
#include "common/common.h"

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
    std::vector<ast::ParsedFile> updatedFileIndexes;

    bool canTakeFastPath = false;
    // Indicates that this update contains a new file. Is a hack for determining if combining two updates can take the
    // fast path.
    bool hasNewFiles = false;
    // If true, this update caused a slow path to be canceled.
    bool canceledSlowPath = false;
    // Updated on typechecking thread. Contains indexes processed with typechecking global state.
    std::vector<ast::ParsedFile> updatedFinalGSFileIndexes;
    // (Optional) Updated global state object to use to typecheck this update.
    std::optional<std::unique_ptr<core::GlobalState>> updatedGS;
    // (Used in tests) Ensures that a slow path typecheck on these updates waits until it gets cancelled.
    bool cancellationExpected = false;
    // (Used in tests) Ensures that a slow path typecheck waits until this number of preemption occurs before finishing.
    int preemptionsExpected = 0;

    /**
     * Merges the given (and older) LSPFileUpdates object into this LSPFileUpdates object.
     *
     * Resets `fastPathDecision`.
     */
    void mergeOlder(const LSPFileUpdates &older);

    /**
     * Returns a copy of this LSPFileUpdates object. Does not handle deepCopying `updatedGS`.
     */
    LSPFileUpdates copy() const;
};
} // namespace sorbet::realmain::lsp

#endif