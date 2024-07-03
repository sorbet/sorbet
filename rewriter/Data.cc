#include "rewriter/Data.h"
#include "absl/strings/match.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/Context.h"
#include "core/Names.h"
#include "core/core.h"
#include "core/errors/rewriter.h"
#include "rewriter/Util.h"
#include "rewriter/rewriter.h"

using namespace std;

namespace sorbet::rewriter {

namespace {

bool isMissingInitialize(const core::GlobalState &gs, const ast::Send *send) {
    if (!send->hasBlock()) {
        return true;
    }

    auto block = send->block();

    if (auto *insSeq = ast::cast_tree<ast::InsSeq>(block->body)) {
        auto methodDef = ast::cast_tree<ast::MethodDef>(insSeq->expr);

        if (methodDef && methodDef->name == core::Names::initialize()) {
            return false;
        }

        for (auto &&stat : insSeq->stats) {
            methodDef = ast::cast_tree<ast::MethodDef>(stat);

            if (methodDef && methodDef->name == core::Names::initialize()) {
                return false;
            }
        }
    }

    return true;
}

} // namespace

vector<ast::ExpressionPtr> Data::run(core::MutableContext ctx, ast::Assign *asgn) {
    vector<ast::ExpressionPtr> empty;

    if (ctx.state.runningUnderAutogen) {
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

    if (!ast::MK::isRootScope(recv->scope) || recv->cnst != core::Names::Constants::Data() ||
        send->fun != core::Names::define() || !send->hasPosArgs()) {
        return empty;
    }

    auto loc = asgn->loc;

    ast::MethodDef::ARGS_store newArgs;
    ast::Send::ARGS_store sigArgs;
    ast::ClassDef::RHS_store body;

    for (int i = 0; i < send->numPosArgs(); i++) {
        auto *sym = ast::cast_tree<ast::Literal>(send->getPosArg(i));
        if (!sym || !sym->isName()) {
            return empty;
        }
        core::NameRef name = sym->asName();
        auto symLoc = sym->loc;
        auto strname = name.shortName(ctx);
        if (!strname.empty() && strname.back() == '=') {
            if (auto e = ctx.beginError(symLoc, core::errors::Rewriter::InvalidStructMember)) {
                e.setHeader("Data member `{}` cannot end with an equal", strname);
            }
        }

        if (symLoc.exists() && ctx.locAt(symLoc).adjustLen(ctx, 0, 1).source(ctx) == ":") {
            symLoc = ctx.locAt(symLoc).adjust(ctx, 1, 0).offsets();
        }

        sigArgs.emplace_back(ast::MK::Symbol(symLoc, name));
        sigArgs.emplace_back(ast::MK::Constant(symLoc, core::Symbols::BasicObject()));

        auto argName = ast::MK::ResolvedLocal(symLoc, name);
        newArgs.emplace_back(ast::MK::OptionalArg(symLoc, move(argName), ast::MK::Nil(symLoc)));

        body.emplace_back(ast::MK::SyntheticMethod0(symLoc, symLoc, name, ast::MK::RaiseUnimplemented(loc)));
    }

    if (isMissingInitialize(ctx, send)) {
        body.emplace_back(ast::MK::SigVoid(loc, std::move(sigArgs)));
        body.emplace_back(ast::MK::SyntheticMethod(loc, loc, core::Names::initialize(), std::move(newArgs),
                                                   ast::MK::RaiseUnimplemented(loc)));
    }

    if (auto *block = send->block()) {
        // Steal the trees, because the run is going to remove the original send node from the tree anyway.
        if (auto *insSeq = ast::cast_tree<ast::InsSeq>(block->body)) {
            for (auto &&stat : insSeq->stats) {
                body.emplace_back(move(stat));
            }
            body.emplace_back(move(insSeq->expr));
        } else {
            body.emplace_back(move(block->body));
        }

        // NOTE: the code in this block _STEALS_ trees. No _return empty_'s should go after it
    }

    ast::ClassDef::ANCESTORS_store ancestors;
    ancestors.emplace_back(ast::MK::UnresolvedConstant(loc, ast::MK::Constant(loc, core::Symbols::root()),
                                                       core::Names::Constants::Data()));

    vector<ast::ExpressionPtr> stats;
    stats.emplace_back(ast::MK::Class(loc, loc, std::move(asgn->lhs), std::move(ancestors), std::move(body)));
    return stats;
}

}; // namespace sorbet::rewriter
