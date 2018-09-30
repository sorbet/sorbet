#ifndef SORBET_RESOLVER_RESOLVER_H
#define SORBET_RESOLVER_RESOLVER_H

#include "ast/ast.h"
#include <memory>

namespace sorbet::resolver {

class Resolver final {
public:
    static std::vector<std::unique_ptr<ast::Expression>> run(core::MutableContext ctx,
                                                             std::vector<std::unique_ptr<ast::Expression>> trees);
    Resolver() = delete;

    /** Only runs tree passes, used for incremental changes that do not affect global state. Assumes that `run` was
     * called on a tree that contains same definitions before (LSP uses heuristics that should only have false negatives
     * to find this) */
    static std::vector<std::unique_ptr<ast::Expression>>
    runTreePasses(core::MutableContext ctx, std::vector<std::unique_ptr<ast::Expression>> trees);

    static std::vector<std::unique_ptr<ast::Expression>>
    runConstantResolution(core::MutableContext ctx, std::vector<std::unique_ptr<ast::Expression>> trees);

private:
    static void finalizeAncestors(core::GlobalState &gs);
    static void finalizeResolution(core::GlobalState &gs);
    static std::vector<std::unique_ptr<ast::Expression>>
    resolveSigs(core::MutableContext ctx, std::vector<std::unique_ptr<ast::Expression>> trees);
    static void sanityCheck(core::MutableContext ctx, std::vector<std::unique_ptr<ast::Expression>> &trees);
};

} // namespace sorbet::resolver

#endif
