#include "rewriter/Cleanup.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "ast/treemap/treemap.h"
#include "core/core.h"

using namespace std;

namespace sorbet::rewriter {

// This pass gets rid of some unnecessary nodes that are likely to have gotten created in the course of the rewriter
// pass, specifically by removing EmptyTree nodes and `nil` nodes in places where they can be safely
// removed (i.e. as part of longer sequences of expressions where they are not a return value)
struct CleanupWalk {
    void postTransformInsSeq(core::Context ctx, ast::ExpressionPtr &tree) {
        auto &insSeq = ast::cast_tree_nonnull<ast::InsSeq>(tree);

        ast::InsSeq::STATS_store newStore;
        for (auto &m : insSeq.stats) {
            if (ast::isa_tree<ast::EmptyTree>(m)) {
                continue;
            }

            if (ast::isa_tree<ast::Literal>(m)) {
                auto lit = ast::cast_tree_nonnull<ast::Literal>(m);
                if (lit.isNil(ctx)) {
                    continue;
                }
            }

            newStore.emplace_back(move(m));
        }
        if (newStore.empty()) {
            tree = move(insSeq.expr);
            return;
        }
        insSeq.stats = std::move(newStore);
    }

    void postTransformClassDef(core::Context ctx, ast::ExpressionPtr &tree) {
        auto &classDef = ast::cast_tree_nonnull<ast::ClassDef>(tree);

        ast::ClassDef::RHS_store newStore;
        for (auto &m : classDef.rhs) {
            if (ast::isa_tree<ast::EmptyTree>(m)) {
                continue;
            }

            if (ast::isa_tree<ast::Literal>(m)) {
                auto lit = ast::cast_tree_nonnull<ast::Literal>(m);
                if (lit.isNil(ctx)) {
                    continue;
                }
            }

            newStore.emplace_back(move(m));
        }
        classDef.rhs = std::move(newStore);
    }
};

ast::ExpressionPtr Cleanup::run(core::Context ctx, ast::ExpressionPtr tree) {
    CleanupWalk cleanup;
    ast::TreeWalk::apply(ctx, cleanup, tree);
    return tree;
}

} // namespace sorbet::rewriter
