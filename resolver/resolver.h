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
     *
     * These two versions are explicitly instantiated in resolver.cc
     */
    static ast::ParsedFilesOrCancelled runIncremental(core::GlobalState &gs, std::vector<ast::ParsedFile> trees,
                                                      bool ranIncrementalNamer, WorkerPool &workers);

    // used by autogen only
    static std::vector<ast::ParsedFile> runConstantResolution(core::GlobalState &gs, std::vector<ast::ParsedFile> trees,
                                                              WorkerPool &workers);

    static void finalizeSymbols(core::GlobalState &gs);

private:
    static void finalizeAncestors(core::GlobalState &gs);
};

} // namespace sorbet::resolver

#endif
