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

const ast::Send *findParams(ast::ExpressionPtr *send) {
    ast::Send *sig = ASTUtil::castSig(*send);
    if (sig == nullptr) {
        return nullptr;
    }

    auto *block = sig->block();
    if (block == nullptr) {
        return nullptr;
    }

    auto *bodyBlock = ast::cast_tree<ast::Send>(block->body);

    while (bodyBlock && bodyBlock->fun != core::Names::params()) {
        bodyBlock = ast::cast_tree<ast::Send>(bodyBlock->recv);
    }

    return bodyBlock;
}

optional<const ast::Send *> getInitialize(const core::GlobalState &gs, ast::Send *send) {
    if (!send->hasBlock()) {
        return nullopt;
    }

    auto block = send->block();

    if (auto *insSeq = ast::cast_tree<ast::InsSeq>(block->body)) {
        ast::ExpressionPtr *prevStat = nullptr;
        for (auto &&stat : insSeq->stats) {
            auto methodDef = ast::cast_tree<ast::MethodDef>(stat);

            if (methodDef && methodDef->name == core::Names::initialize()) {
                return findParams(prevStat);
            }

            prevStat = &stat;
        }

        // the last expression of the block is stored separately as expr
        auto methodDef = ast::cast_tree<ast::MethodDef>(insSeq->expr);
        if (methodDef && methodDef->name == core::Names::initialize()) {
            return findParams(prevStat);
        }
    }

    return nullopt;
}

ast::ExpressionPtr getMemberType(core::MutableContext ctx, const ast::Send *params, core::NameRef name,
                                 core::LocOffsets loc) {
    if (params) {
        for (int i = 0; i < params->numKwArgs(); i++) {
            auto key = ast::cast_tree<ast::Literal>(params->getKwKey(i))->asName();

            if (key.toString(ctx) == name.toString(ctx)) {
                return params->getKwValue(i).deepCopy();
            }
        }
    }

    return ast::MK::Constant(loc, core::Symbols::BasicObject());
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

    auto initialize = getInitialize(ctx, send);
    const ast::Send *initializeSigParams = nullptr;
    if (initialize.has_value()) {
        initializeSigParams = initialize.value();
    }

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

        auto memberType = getMemberType(ctx, initializeSigParams, name, symLoc);

        sigArgs.emplace_back(ast::MK::Symbol(symLoc, name));
        sigArgs.emplace_back(ASTUtil::dupType(memberType));

        auto argName = ast::MK::Local(symLoc, name);
        newArgs.emplace_back(ast::MK::OptionalArg(symLoc, move(argName), ast::MK::Nil(symLoc)));

        body.emplace_back(ast::MK::Sig(symLoc, {}, ASTUtil::dupType(memberType)));
        body.emplace_back(ast::MK::SyntheticMethod0(symLoc, symLoc, name, ast::MK::RaiseUnimplemented(loc)));
    }

    if (!initialize.has_value()) {
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
