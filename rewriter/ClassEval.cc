#include "rewriter/ClassEval.h"
#include "absl/algorithm/container.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/Context.h"
#include "core/Names.h"
#include "core/core.h"
#include "rewriter/rewriter.h"
#include "rewriter/util/Util.h"

using namespace std;

namespace sorbet::rewriter {

namespace {

// A `def`, or a visibility-wrapped def like `private def`.
bool definesMethod(const ast::ExpressionPtr &stat) {
    if (ast::isa_tree<ast::MethodDef>(stat)) {
        return true;
    }
    if (auto send = ast::cast_tree<ast::Send>(stat)) {
        for (auto &arg : send->posArgs()) {
            if (ast::isa_tree<ast::MethodDef>(arg)) {
                return true;
            }
        }
    }
    return false;
}

// A bare `private`/`protected`/`public`, which changes the default visibility of the defs
// that follow it. These must move together with the defs they modify.
bool isBareVisibilityModifier(const ast::ExpressionPtr &stat) {
    auto send = ast::cast_tree<ast::Send>(stat);
    if (send == nullptr || send->hasPosArgs() || send->hasKwArgs() || send->hasBlock()) {
        return false;
    }
    if (!send->recv.isSelfReference()) {
        return false;
    }
    return send->fun == core::Names::private_() || send->fun == core::Names::protected_() ||
           send->fun == core::Names::public_();
}

} // namespace

vector<ast::ExpressionPtr> ClassEval::run(core::MutableContext ctx, ast::Send *send) {
    vector<ast::ExpressionPtr> empty;

    if (ctx.state.cacheSensitiveOptions.runningUnderAutogen) {
        // Not safe to run under autogen for the same reason as ClassNew: the synthesized
        // class definition would end up predeclared in autoloader files.
        return empty;
    }

    if (send->fun != core::Names::classEval() && send->fun != core::Names::classExec()) {
        return empty;
    }

    if (!ast::isa_tree<ast::UnresolvedConstantLit>(send->recv)) {
        return empty;
    }

    // A literal block is required; this also rules out the string form of class_eval.
    auto *block = send->block();
    if (block == nullptr) {
        return empty;
    }

    // Only act when the block actually defines methods; anything else (including blocks on
    // receivers that may well be modules, like `Enumerable.class_eval { |mod| puts mod }`)
    // is left untouched.
    bool anyMethodDef = false;
    if (auto insSeq = ast::cast_tree<ast::InsSeq>(block->body)) {
        anyMethodDef = definesMethod(insSeq->expr) || absl::c_any_of(insSeq->stats, definesMethod);
    } else {
        anyMethodDef = definesMethod(block->body);
    }
    if (!anyMethodDef) {
        return empty;
    }

    // Partition the block's statements: defs move into the synthesized class body together
    // with their sigs and any bare visibility modifiers (in original relative order, since
    // visibility applies to subsequent defs); everything else stays in the block, keeping
    // its closure semantics (locals captured from the enclosing scope keep working). Note
    // that `def` is a scope gate even inside `class_eval` blocks in Ruby, so moving the
    // defs out cannot sever any local variable capture.
    ast::ClassDef::RHS_store hoisted;
    ast::InsSeq::STATS_store residual;
    vector<ast::ExpressionPtr> pendingSigs;

    auto processStat = [&](ast::ExpressionPtr &stat) {
        // Sigs are buffered until we know whether the statement they annotate is a def
        // (hoist them together, so the resolver still sees them adjacent) or not.
        if (ASTUtil::castSig(stat) != nullptr) {
            pendingSigs.emplace_back(move(stat));
            return;
        }

        if (definesMethod(stat)) {
            absl::c_move(pendingSigs, back_inserter(hoisted));
            pendingSigs.clear();
            hoisted.emplace_back(move(stat));
            return;
        }

        absl::c_move(pendingSigs, back_inserter(residual));
        pendingSigs.clear();

        if (isBareVisibilityModifier(stat)) {
            hoisted.emplace_back(move(stat));
        } else {
            residual.emplace_back(move(stat));
        }
    };

    if (auto insSeq = ast::cast_tree<ast::InsSeq>(block->body)) {
        for (auto &stat : insSeq->stats) {
            processStat(stat);
        }
        processStat(insSeq->expr);
    } else {
        processStat(block->body);
    }
    absl::c_move(pendingSigs, back_inserter(residual));
    pendingSigs.clear();

    // Rebuild the block around the statements that stayed behind. The send itself is left
    // in place; the synthesized class definition is inserted after it.
    if (residual.empty()) {
        block->body = ast::MK::EmptyTree();
    } else {
        auto expr = move(residual.back());
        residual.pop_back();
        block->body = ast::MK::InsSeq(block->loc, move(residual), move(expr));
    }

    ast::ClassDef::ANCESTORS_store ancestors;
    ancestors.emplace_back(ast::MK::Constant(send->loc, core::Symbols::todo()));

    auto declLoc = core::LocOffsets(send->loc.beginPos(), block->loc.beginPos());

    vector<ast::ExpressionPtr> stats;
    stats.emplace_back(
        ast::MK::Class(send->loc, declLoc, send->recv.deepCopy(), std::move(ancestors), std::move(hoisted)));
    return stats;
}

}; // namespace sorbet::rewriter
