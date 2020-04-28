#ifndef RUBY_TYPER_LSP_UNDOSTATE_H
#define RUBY_TYPER_LSP_UNDOSTATE_H

#include "ast/ast.h"
#include "core/NameHash.h"
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
    UnorderedMap<int, ast::ParsedFile> evictedIndexed;
    // Stores the index trees stored in `gs` that were evicted because the slow path operation replaced `gs`.
    UnorderedMap<int, ast::ParsedFile> evictedIndexedFinalGS;
    // Stores the list of files that had errors before the slow path began.
    std::vector<core::FileRef> evictedFilesThatHaveErrors;

public:
    UndoState(std::unique_ptr<core::GlobalState> evictedGs, UnorderedMap<int, ast::ParsedFile> evictedIndexedFinalGS);

    /**
     * Records that the given items were evicted from LSPTypechecker following a typecheck run.
     */
    void recordEvictedState(ast::ParsedFile evictedIndexTree);

    /**
     * Undoes the slow path changes represented by this class.
     */
    void restore(std::unique_ptr<core::GlobalState> &gs, std::vector<ast::ParsedFile> &indexed,
                 UnorderedMap<int, ast::ParsedFile> &indexedFinalGS);
};

} // namespace sorbet::realmain::lsp
#endif
