#ifndef RUBY_TYPER_LSP_UNDOSTATE_H
#define RUBY_TYPER_LSP_UNDOSTATE_H

#include "ast/ast.h"
#include "core/FileHash.h"
#include "core/core.h"
#include "core/packages/Stratum.h"

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

    // The saved file-to-stratum mapping from the previous slow path.
    std::vector<core::packages::Stratum> fileToStratum;

    // The id of the last stratum in the previous slow path.
    const core::packages::Stratum lastStratum;

    // The workspaceFiles vector as it was when the slow path started. The slow path appends new files to the live
    // vector and then permutes it in place (partitionPackageFiles), so rolling back by truncating to the old size
    // would erase whatever ended up at the tail rather than the new files; restoring the snapshot is order-independent.
    std::vector<core::FileRef> savedWorkspaceFiles;

public:
    // Epoch of the running slow path
    const uint32_t epoch;

    UndoState(std::unique_ptr<core::GlobalState> evictedGs, UnorderedMap<int, ast::ParsedFile> evictedIndexedFinalGS,
              std::vector<core::packages::Stratum> fileToStratum, core::packages::Stratum lastStratum,
              const std::vector<core::FileRef> &workspaceFiles, uint32_t epoch);

    /**
     * Undoes the slow path changes represented by this class.
     */
    void restore(std::unique_ptr<core::GlobalState> &gs, UnorderedMap<int, ast::ParsedFile> &indexedFinalGS,
                 std::vector<core::packages::Stratum> &fileToStratum, core::packages::Stratum &lastStratum,
                 std::vector<core::FileRef> &workspaceFiles);

    /**
     * Retrieves the evicted global state.
     */
    const std::unique_ptr<core::GlobalState> &getEvictedGs();
};

} // namespace sorbet::realmain::lsp
#endif
