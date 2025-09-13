#include "rewriter/Data.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/Context.h"
#include "core/Names.h"
#include "core/core.h"
#include "core/errors/rewriter.h"
#include "rewriter/util/Util.h"

using namespace std;

namespace sorbet::rewriter {

namespace {

bool isMissingInitialize(const core::GlobalState &gs, const ast::Send *send) {
    if (!send->hasBlock()) {
        return true;
    }

    auto block = send->block();

    if (auto insSeq = ast::cast_tree<ast::InsSeq>(block->body)) {
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

    if (ctx.state.cacheSensitiveOptions.runningUnderAutogen) {
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

    if (!ASTUtil::isRootScopedSyntacticConstant(send->recv, {core::Names::Constants::Data()})) {
        return empty;
    }

    if (send->fun != core::Names::define() || send->hasKwArgs() || send->hasKwSplat()) {
        return empty;
    }

    auto loc = asgn->loc;

    ast::MethodDef::PARAMS_store newArgs;
    ast::Send::ARGS_store sigArgs;
    ast::ClassDef::RHS_store body;

    if (auto dup = ASTUtil::findDuplicateArg(ctx, send)) {
        if (auto e = ctx.beginIndexerError(dup->secondLoc, core::errors::Rewriter::InvalidStructMember)) {
            e.setHeader("Duplicate member `{}` in Data definition", dup->name.show(ctx));
            e.addErrorLine(ctx.locAt(dup->firstLoc), "First occurrence of `{}` in Data definition",
                           dup->name.show(ctx));
        }
        return empty;
    }

    for (auto &arg : send->posArgs()) {
        auto sym = ast::cast_tree<ast::Literal>(arg);
        if (!sym || !sym->isName()) {
            return empty;
        }
        core::NameRef name = sym->asName();
        auto symLoc = sym->loc;
        auto strname = name.shortName(ctx);
        if (!strname.empty() && strname.back() == '=') {
            if (auto e = ctx.beginIndexerError(symLoc, core::errors::Rewriter::InvalidStructMember)) {
                e.setHeader("Data member `{}` cannot end with an equal", strname);
            }
        }

        if (symLoc.exists() && ctx.locAt(symLoc).adjustLen(ctx, 0, 1).source(ctx) == ":") {
            symLoc = ctx.locAt(symLoc).adjust(ctx, 1, 0).offsets();
        }

        sigArgs.emplace_back(ast::MK::Symbol(symLoc, name));
        sigArgs.emplace_back(ast::MK::Constant(symLoc, core::Symbols::BasicObject()));

        auto argName = ast::MK::Local(symLoc, name);
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
        if (auto insSeq = ast::cast_tree<ast::InsSeq>(block->body)) {
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
