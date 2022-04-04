#ifndef SORBET_RESOLVER_RESOLVER_H
#define SORBET_RESOLVER_RESOLVER_H

#include "ast/ast.h"
#include "common/concurrency/WorkerPool.h"
#include <memory>

namespace sorbet::resolver {

class Resolver final {
public:
    static ast::ParsedFilesOrCancelled run(core::GlobalState &gs, std::vector<ast::ParsedFile> trees,
                                           WorkerPool &workers);
    Resolver() = delete;

    /**
     * Only runs tree passes, used for incremental changes that do not affect global state. Assumes
     * that `run` was called on a tree that contains same definitions before (LSP uses heuristics
     * that should only have false negatives to find this)
     *
     * Has two versions: one that requires a mutable GlobalState, and one that does a "best effort"
     * mode with a constant GlobalState (for, e.g., serving stale LSP requests)
     *
     * These two versions are explicitly instantiated in resolver.cc
     */
    template <typename StateType>
    static ast::ParsedFilesOrCancelled runIncremental(StateType &gs, std::vector<ast::ParsedFile> trees);

    // used by autogen only
    static std::vector<ast::ParsedFile> runConstantResolution(core::GlobalState &gs, std::vector<ast::ParsedFile> trees,
                                                              WorkerPool &workers);

private:
    static void finalizeAncestors(core::GlobalState &gs);
    static void finalizeSymbols(core::GlobalState &gs);
    static void sanityCheck(const core::GlobalState &gs, std::vector<ast::ParsedFile> &trees);
};

} // namespace sorbet::resolver

#endif
