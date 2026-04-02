#ifndef RUBY_TYPER_LSP_UNDOSTATE_H
#define RUBY_TYPER_LSP_UNDOSTATE_H

#include "ast/ast.h"
#include "core/FileHash.h"
#include "core/core.h"

namespace sorbet::realmain::lsp {
class LSPConfiguration;
/**
 * Contains the LSPTypechecker state that is needed to cancel a running slow path operation and any subsequent fast
 * path operations that have preempted it.
 */
class UndoState final {
    // Stores the pre-slow-path global state.
    std::unique_ptr<core::GlobalState> evictedGs;

    // Stores index trees containing data stored in `gs` that have been evicted during the slow path operation.
    UnorderedMap<int, ast::ParsedFile> evictedIndexedFinalGS;

    // The size of the workspaceFiles vector when the slow path started. Tracked so that we can roll back additions to
    // the vector from new files added in the canceled edit.
    const size_t initialWorkspaceFilesSize;

public:
    // Epoch of the running slow path
    const uint32_t epoch;

    UndoState(std::unique_ptr<core::GlobalState> evictedGs, UnorderedMap<int, ast::ParsedFile> evictedIndexedFinalGS,
              const std::vector<core::FileRef> &workspaceFiles, uint32_t epoch);

    /**
     * Undoes the slow path changes represented by this class.
     */
    void restore(std::unique_ptr<core::GlobalState> &gs, UnorderedMap<int, ast::ParsedFile> &indexedFinalGS,
                 std::vector<core::FileRef> &workspaceFiles);

    /**
     * Retrieves the evicted global state.
     */
    const std::unique_ptr<core::GlobalState> &getEvictedGs();
};

} // namespace sorbet::realmain::lsp
#endif
