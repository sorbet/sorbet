#include "rewriter/ClassNew.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/Context.h"
#include "core/Names.h"
#include "core/core.h"
#include "rewriter/rewriter.h"

using namespace std;

namespace sorbet::rewriter {

vector<ast::ExpressionPtr> ClassNew::run(core::MutableContext ctx, ast::Assign *asgn) {
    vector<ast::ExpressionPtr> empty;
    auto loc = asgn->loc;

    if (ctx.state.runningUnderAutogen) {
        // This is not safe to run under autogen, because we'd be outputing
        // autoloader files that predeclare the class and cause "warning:
        // already initialized constant" errors
        return empty;
    }

    auto lhs = ast::cast_tree<ast::UnresolvedConstantLit>(asgn->lhs);
    if (lhs == nullptr) {
        return empty;
    }

    auto send = ast::cast_tree<ast::Send>(asgn->rhs);
    if (send == nullptr) {
        return empty;
    }

    auto recv = ast::cast_tree<ast::UnresolvedConstantLit>(send->recv);
    if (recv == nullptr) {
        return empty;
    }

    if (!ast::isa_tree<ast::EmptyTree>(recv->scope) || recv->cnst != core::Names::Constants::Class() ||
        send->fun != core::Names::new_()) {
        return empty;
    }

    auto argc = send->numPosArgs();
    if (argc > 1 || send->hasKwArgs()) {
        return empty;
    }

    if (argc == 1 && !ast::isa_tree<ast::UnresolvedConstantLit>(send->getPosArg(0))) {
        return empty;
    }

    ast::ClassDef::RHS_store body;

    auto *block = send->block();
    if (block != nullptr && block->args.size() == 1) {
        auto blockArg = move(block->args[0]);
        body.emplace_back(ast::MK::Assign(blockArg.loc(), move(blockArg), asgn->lhs.deepCopy()));
    }

    if (block != nullptr) {
        // Steal the trees, because the run is going to remove the original send node from the tree anyway.
        if (auto insSeq = ast::cast_tree<ast::InsSeq>(block->body)) {
            for (auto &&stat : insSeq->stats) {
                body.emplace_back(move(stat));
            }
            body.emplace_back(move(insSeq->expr));
        } else {
            body.emplace_back(move(block->body));
        }
    }

    ast::ClassDef::ANCESTORS_store ancestors;
    if (argc == 1) {
        ancestors.emplace_back(move(send->getPosArg(0)));
    } else {
        ancestors.emplace_back(ast::MK::Constant(send->loc, core::Symbols::todo()));
    }

    vector<ast::ExpressionPtr> stats;
    stats.emplace_back(ast::MK::Class(loc, loc, std::move(asgn->lhs), std::move(ancestors), std::move(body)));
    return stats;
}

}; // namespace sorbet::rewriter
