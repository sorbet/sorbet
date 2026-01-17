#include "rewriter/ClassNew.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/Context.h"
#include "core/Names.h"
#include "core/core.h"
#include "rewriter/rewriter.h"
#include "rewriter/util/Util.h"

using namespace std;

namespace sorbet::rewriter {

// Is this expression a synthetic `T.bind` call we added from the `ClassNew` rewriter on sends?
bool isRewrittenBind(ast::ExpressionPtr &expr) {
    auto cast = ast::cast_tree<ast::Cast>(expr);
    if (cast == nullptr) {
        return false;
    }

    return cast->cast == core::Names::syntheticBind();
}

vector<ast::ExpressionPtr> ClassNew::run(core::MutableContext ctx, ast::Assign *asgn) {
    vector<ast::ExpressionPtr> empty;

    if (ctx.state.cacheSensitiveOptions.runningUnderAutogen) {
        // This is not safe to run under autogen, because we'd be outputting
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

    if (!ASTUtil::isRootScopedSyntacticConstant(send->recv, {core::Names::Constants::Class()})) {
        return empty;
    }

    if (send->fun != core::Names::new_()) {
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
    if (block != nullptr && block->params.size() == 1) {
        auto blockParam = move(block->params[0]);
        body.emplace_back(ast::MK::Assign(blockParam.loc(), move(blockParam), asgn->lhs.deepCopy()));
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
    auto declLoc = block != nullptr ? core::LocOffsets(asgn->loc.beginPos(), block->loc.beginPos()) : loc;

    vector<ast::ExpressionPtr> stats;
    stats.emplace_back(ast::MK::Class(loc, declLoc, std::move(asgn->lhs), std::move(ancestors), std::move(body)));
    return stats;
}

bool ClassNew::run(core::MutableContext ctx, ast::Send *send) {
    if (!ASTUtil::isRootScopedSyntacticConstant(send->recv, {core::Names::Constants::Class()})) {
        return false;
    }

    if (send->fun != core::Names::new_()) {
        return false;
    }

    auto argc = send->numPosArgs();
    if (argc > 1 || send->hasKwArgs()) {
        return false;
    }

    if (argc == 1 && !ast::isa_tree<ast::UnresolvedConstantLit>(send->getPosArg(0))) {
        return false;
    }

    auto *block = send->block();
    if (block == nullptr) {
        return false;
    }

    ast::ExpressionPtr type;

    if (argc == 0) {
        auto zeroLoc = send->loc.copyWithZeroLength();
        type =
            ast::MK::Send1(send->loc, ast::MK::Constant(send->recv.loc(), core::Symbols::T_Class()),
                           core::Names::squareBrackets(), zeroLoc, ast::MK::Constant(zeroLoc, core::Symbols::Object()));
    } else {
        auto target = send->getPosArg(0).deepCopy();
        type = ast::MK::ClassOf(send->loc, std::move(target));
    }

    auto bind = ast::MK::SyntheticBind(send->loc, ast::MK::Self(send->loc), std::move(type));

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

    return true;
}

}; // namespace sorbet::rewriter
