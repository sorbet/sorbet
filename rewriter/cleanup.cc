#include "rewriter/cleanup.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "ast/treemap/treemap.h"
#include "core/core.h"

using namespace std;

namespace sorbet::rewriter {

struct CleanupWalk {
    unique_ptr<ast::Expression> postTransformInsSeq(core::Context ctx, unique_ptr<ast::InsSeq> insSeq) {
        ast::InsSeq::STATS_store newStore;
        for (auto &m : insSeq->stats) {
            if (!ast::isa_tree<ast::EmptyTree>(m.get())) {
                newStore.emplace_back(move(m));
            }
        }
        if (newStore.empty()) {
            return ast::MK::EmptyTree();
        }
        insSeq->stats = std::move(newStore);
        return insSeq;
    }

    unique_ptr<ast::Expression> postTransformClassDef(core::Context ctx, unique_ptr<ast::ClassDef> classDef) {
        ast::ClassDef::RHS_store newStore;
        for (auto &m : classDef->rhs) {
            if (!(ast::isa_tree<ast::EmptyTree>(m.get()) || ast::isa_tree<ast::Literal>(m.get()))) {
                newStore.emplace_back(move(m));
            }
        }
        classDef->rhs = std::move(newStore);
        return classDef;
    }
};

unique_ptr<ast::Expression> Cleanup::run(core::Context ctx, unique_ptr<ast::Expression> tree) {
    CleanupWalk cleanup;
    return ast::TreeMap::apply(ctx, cleanup, std::move(tree));
}

} // namespace sorbet::rewriter
