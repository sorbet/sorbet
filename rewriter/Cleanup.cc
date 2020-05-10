#include "rewriter/Cleanup.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "ast/treemap/treemap.h"
#include "core/core.h"

using namespace std;

namespace sorbet::rewriter {

// This pass gets rid of some unnecessary nodes that are likely to have gotten created in the course of the rewriter
// pass, specifically by removing EmptyTree nodes in places where they can be safely removed (i.e. as part of longer
// sequences of expressions where they are not a return value)
struct CleanupWalk {
    ast::TreePtr postTransformInsSeq(core::Context ctx, ast::TreePtr tree) {
        auto &insSeq = ast::ref_tree<ast::InsSeq>(tree);

        ast::InsSeq::STATS_store newStore;
        for (auto &m : insSeq.stats) {
            if (!ast::isa_tree<ast::EmptyTree>(m)) {
                newStore.emplace_back(move(m));
            }
        }
        if (newStore.empty()) {
            return move(insSeq.expr);
        }
        insSeq.stats = std::move(newStore);
        return tree;
    }

    ast::TreePtr postTransformClassDef(core::Context ctx, ast::TreePtr tree) {
        auto &classDef = ast::ref_tree<ast::ClassDef>(tree);

        ast::ClassDef::RHS_store newStore;
        for (auto &m : classDef.rhs) {
            if (!ast::isa_tree<ast::EmptyTree>(m)) {
                newStore.emplace_back(move(m));
            }
        }
        classDef.rhs = std::move(newStore);
        return tree;
    }
};

ast::TreePtr Cleanup::run(core::Context ctx, ast::TreePtr tree) {
    CleanupWalk cleanup;
    return ast::TreeMap::apply(ctx, cleanup, std::move(tree));
}

} // namespace sorbet::rewriter
