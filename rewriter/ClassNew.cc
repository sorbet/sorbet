#include "rewriter/ClassNew.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/Context.h"
#include "core/Names.h"
#include "core/core.h"
#include "rewriter/rewriter.h"

using namespace std;

namespace sorbet::rewriter {

// Is this expression a synthetic `T.bind` call we added from the `ClassNew` rewriter on sends?
bool isRewrittenBind(ast::ExpressionPtr &expr) {
    auto send = ast::cast_tree<ast::Send>(expr);
    if (send == nullptr) {
        return false;
    }

    if (send->fun != core::Names::bind()) {
        return false;
    }

    return send->flags.isRewriterSynthesized;
}

// Is this expression a synthetic `T.cast` call we added from the `ClassNew` rewriter on sends?
bool isRewrittenCast(ast::ExpressionPtr &expr) {
    auto send = ast::cast_tree<ast::Send>(expr);
    if (send == nullptr) {
        return false;
    }

    if (send->fun != core::Names::cast()) {
        return false;
    }

    return send->flags.isRewriterSynthesized;
}

vector<ast::ExpressionPtr> ClassNew::run(core::MutableContext ctx, ast::Assign *asgn) {
    vector<ast::ExpressionPtr> empty;

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

    auto rhs = asgn->rhs.deepCopy();

    if (isRewrittenCast(asgn->rhs)) {
        // This is a synthetic cast we added from the `ClassNew` rewriter, we want to rewrite the value of the cast.
        rhs = ast::cast_tree<ast::Send>(rhs)->getPosArg(0).deepCopy();
    }

    auto send = ast::cast_tree<ast::Send>(rhs);
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
                if (isRewrittenBind(stat)) {
                    // Remove synthetic `T.bind` statements we added during the ClassNew rewriter on sends.
                    continue;
                }
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

    auto loc = asgn->loc;

    vector<ast::ExpressionPtr> stats;
    stats.emplace_back(ast::MK::Class(loc, loc, std::move(asgn->lhs), std::move(ancestors), std::move(body)));
    return stats;
}

ast::ExpressionPtr ClassNew::run(core::MutableContext ctx, ast::Send *send) {
    auto recv = ast::cast_tree<ast::UnresolvedConstantLit>(send->recv);
    if (recv == nullptr) {
        return nullptr;
    }

    if (!ast::isa_tree<ast::EmptyTree>(recv->scope) || recv->cnst != core::Names::Constants::Class() ||
        send->fun != core::Names::new_()) {
        return nullptr;
    }

    auto argc = send->numPosArgs();
    if (argc > 1 || send->hasKwArgs()) {
        return nullptr;
    }

    if (argc == 1) {
        auto parent = ast::cast_tree<ast::UnresolvedConstantLit>(send->getPosArg(0));
        if (parent == nullptr) {
            return nullptr;
        }
    }

    auto *block = send->block();
    if (block == nullptr) {
        return nullptr;
    }

    bool hasParent = false;
    ast::ExpressionPtr type;

    if (argc == 0) {
        type = ast::MK::Constant(send->loc, core::Symbols::Class());
    } else {
        hasParent = true;
        auto target = send->getPosArg(0).deepCopy();
        type = ast::MK::ClassOf(send->loc, std::move(target));
    }

    auto bind = ast::MK::Bind(send->loc, ast::MK::Self(send->loc), type.deepCopy());

    // Mark the bind as synthetic so we can spot it from the `ClassNew` rewriter on assigns and remove it from the tree.
    ast::cast_tree<ast::Send>(bind)->flags.isRewriterSynthesized = true;

    ast::InsSeq::STATS_store blockStats;
    blockStats.emplace_back(std::move(bind));

    if (auto insSeq = ast::cast_tree<ast::InsSeq>(block->body)) {
        for (auto &stat : insSeq->stats) {
            blockStats.emplace_back(std::move(stat));
        }
        block->body = ast::MK::InsSeq(block->loc, std::move(blockStats), std::move(insSeq->expr));
    } else {
        block->body = ast::MK::InsSeq(block->loc, std::move(blockStats), std::move(block->body));
    }

    if (ast::cast_tree<ast::Send>(type)) {
        // The class inherits from someting else than `Class`, we need to wrap the call into a synthetic `T.cast`.
        auto tree = ast::MK::Cast(send->loc, send->deepCopy(), type.deepCopy());
        auto cast = ast::cast_tree<ast::Send>(tree);
        // Mark the cast as synthetic so we can spot it from the `ClassNew` rewriter on assigns and remove it.
        cast->flags.isRewriterSynthesized = true;
        return cast->deepCopy();
    }

    return send->deepCopy();
}

}; // namespace sorbet::rewriter
