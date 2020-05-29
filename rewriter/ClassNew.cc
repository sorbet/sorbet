#include "rewriter/ClassNew.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/Context.h"
#include "core/Names.h"
#include "core/core.h"
#include "rewriter/rewriter.h"

using namespace std;

namespace sorbet::rewriter {

vector<unique_ptr<ast::Expression>> ClassNew::run(core::MutableContext ctx, ast::Assign *asgn) {
    vector<unique_ptr<ast::Expression>> empty;
    auto loc = asgn->loc;

    if (ctx.state.runningUnderAutogen) {
        // This is not safe to run under autogen, because we'd be outputing
        // autoloader files that predeclare the class and cause "warning:
        // already initialized constant" errors
        return empty;
    }

    auto lhs = ast::cast_tree<ast::UnresolvedConstantLit>(asgn->lhs.get());
    if (lhs == nullptr) {
        return empty;
    }

    auto send = ast::cast_tree<ast::Send>(asgn->rhs.get());
    if (send == nullptr) {
        return empty;
    }

    auto recv = ast::cast_tree<ast::UnresolvedConstantLit>(send->recv.get());
    if (recv == nullptr) {
        return empty;
    }

    if (!ast::isa_tree<ast::EmptyTree>(recv->scope.get()) || recv->cnst != core::Names::Constants::Class() ||
        send->fun != core::Names::new_()) {
        return empty;
    }

    auto argc = send->args.size();
    if (argc > 1) {
        return empty;
    }

    if (argc == 1 && !ast::isa_tree<ast::UnresolvedConstantLit>(send->args[0].get())) {
        return empty;
    }

    ast::ClassDef::RHS_store body;

    if (send->block != nullptr && send->block->args.size() == 1) {
        auto blockArg = move(send->block->args[0]);
        body.emplace_back(ast::MK::Assign(blockArg->loc, move(blockArg), asgn->lhs->deepCopy()));
    }

    if (send->block != nullptr) {
        // Steal the trees, because the run is going to remove the original send node from the tree anyway.
        if (auto insSeq = ast::cast_tree<ast::InsSeq>(send->block->body.get())) {
            for (auto &&stat : insSeq->stats) {
                body.emplace_back(move(stat));
            }
            body.emplace_back(move(insSeq->expr));
        } else {
            body.emplace_back(move(send->block->body));
        }
    }

    ast::ClassDef::ANCESTORS_store ancestors;
    if (argc == 1) {
        ancestors.emplace_back(move(send->args[0]));
    } else {
        ancestors.emplace_back(ast::MK::Constant(send->loc, core::Symbols::todo()));
    }

    vector<unique_ptr<ast::Expression>> stats;
    stats.emplace_back(
        ast::MK::Class(loc, core::Loc(ctx.file, loc), std::move(asgn->lhs), std::move(ancestors), std::move(body)));
    return stats;
}

}; // namespace sorbet::rewriter
