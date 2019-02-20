#ifndef SORBET_RESOLVER_RESOLVER_H
#define SORBET_RESOLVER_RESOLVER_H

#include "ast/ast.h"
#include <memory>

namespace sorbet::resolver {

class Resolver final {
public:
    static std::vector<ast::ParsedFile> run(core::MutableContext ctx, std::vector<ast::ParsedFile> trees);
    Resolver() = delete;

    /** Only runs tree passes, used for incremental changes that do not affect global state. Assumes that `run` was
     * called on a tree that contains same definitions before (LSP uses heuristics that should only have false negatives
     * to find this) */
    static std::vector<ast::ParsedFile> runTreePasses(core::MutableContext ctx, std::vector<ast::ParsedFile> trees);

    static std::vector<ast::ParsedFile> runConstantResolution(core::MutableContext ctx,
                                                              std::vector<ast::ParsedFile> trees);

private:
    static void finalizeAncestors(core::GlobalState &gs);
    static void finalizeSymbols(core::GlobalState &gs);
    static std::vector<ast::ParsedFile> resolveSigs(core::MutableContext ctx, std::vector<ast::ParsedFile> trees);
    static std::vector<ast::ParsedFile> resolveMixesInClassMethods(core::MutableContext ctx,
                                                                   std::vector<ast::ParsedFile> trees);
    static void validateSymbols(core::GlobalState &gs);
    static void sanityCheck(core::MutableContext ctx, std::vector<ast::ParsedFile> &trees);
};

} // namespace sorbet::resolver

#endif
