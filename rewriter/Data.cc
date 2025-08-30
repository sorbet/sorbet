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

const ast::Send *findParams(const ast::ExpressionPtr *send) {
    auto *sig = ASTUtil::castSig(*send);
    if (sig == nullptr) {
        return nullptr;
    }

    auto *block = sig->block();
    if (block == nullptr) {
        return nullptr;
    }

    auto bodyBlock = ast::cast_tree<ast::Send>(block->body);

    while (bodyBlock && bodyBlock->fun != core::Names::params()) {
        bodyBlock = ast::cast_tree<ast::Send>(bodyBlock->recv);
    }

    return bodyBlock;
}

optional<pair<const ast::MethodDef *, const ast::Send *>> getInitialize(const ast::Send *send) {
    if (!send->hasBlock()) {
        return nullopt;
    }

    auto block = send->block();

    if (auto insSeq = ast::cast_tree<ast::InsSeq>(block->body)) {
        const ast::ExpressionPtr *prevStat = nullptr;
        for (auto &stat : insSeq->stats) {
            auto methodDef = ast::cast_tree<ast::MethodDef>(stat);

            if (methodDef && methodDef->name == core::Names::initialize()) {
                return {{methodDef, findParams(prevStat)}};
            }

            prevStat = &stat;
        }

        // the last expression of the block is stored separately as expr
        auto methodDef = ast::cast_tree<ast::MethodDef>(insSeq->expr);
        if (methodDef && methodDef->name == core::Names::initialize()) {
            return {{methodDef, findParams(prevStat)}};
        }
    } else if (auto methodDef = ast::cast_tree<ast::MethodDef>(block->body)) {
        if (methodDef && methodDef->name == core::Names::initialize()) {
            return {{methodDef, nullptr}};
        }
    }

    return nullopt;
}

bool canCreateTypedAccessors(const core::GlobalState &gs, const ast::MethodDef *initialize) {
    auto send = ast::cast_tree<ast::Send>(initialize->rhs);
    if (!send)
        return false;
    if (send->fun != core::Names::untypedSuper())
        return false;
    if (send->numPosArgs() == 1 && ast::isa_tree<ast::ZSuperArgs>(send->getPosArg(0)))
        return true;

    return false;
}

ast::ExpressionPtr getMemberType(core::MutableContext ctx, const ast::Send *params, core::NameRef name,
                                 core::LocOffsets loc) {
    if (params != nullptr) {
        for (int i = 0; i < params->numKwArgs(); i++) {
            auto key = ast::cast_tree<ast::Literal>(params->getKwKey(i))->asName();

            if (key.toString(ctx) == name.toString(ctx)) {
                return params->getKwValue(i).deepCopy();
            }
        }
    }

    return ast::MK::Untyped(loc);
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

    if (auto dup = ASTUtil::findDuplicateArg(ctx, send)) {
        if (auto e = ctx.beginIndexerError(dup->secondLoc, core::errors::Rewriter::InvalidStructMember)) {
            e.setHeader("Duplicate member `{}` in Data definition", dup->name.show(ctx));
            e.addErrorLine(ctx.locAt(dup->firstLoc), "First occurrence of `{}` in Data definition",
                           dup->name.show(ctx));
        }
        return empty;
    }

    auto initialize = getInitialize(send);
    bool initializeHasSig = false;
    const ast::Send *reliableSigParams = nullptr;
    if (initialize.has_value()) {
        auto methodDef = initialize->first;

        initializeHasSig = !!initialize->second;
        if (initializeHasSig && canCreateTypedAccessors(ctx, methodDef)) {
            reliableSigParams = initialize->second;
        }
    }

    auto loc = asgn->loc;
    ast::Send::ARGS_store newSigArgs;
    ast::MethodDef::ARGS_store newArgs;
    ast::Send::ARGS_store initializeSigArgs;
    ast::MethodDef::ARGS_store initializeArgs;
    ast::ClassDef::RHS_store body;
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

        auto memberType = getMemberType(ctx, reliableSigParams, name, symLoc);
        body.emplace_back(ast::MK::Sig0(symLoc, ASTUtil::dupType(memberType)));
        body.emplace_back(ast::MK::SyntheticMethod0(symLoc, symLoc, name, ast::MK::RaiseUnimplemented(loc)));

        if (!(initialize.has_value() && initializeHasSig)) {
            newSigArgs.emplace_back(ast::MK::Symbol(symLoc, name));
            newSigArgs.emplace_back(ASTUtil::dupType(memberType));
            auto argName = ast::MK::Local(symLoc, name);
            newArgs.emplace_back(ast::MK::OptionalArg(symLoc, move(argName), ast::MK::Nil(symLoc)));
        }

        if (initialize.has_value()) {
            initializeArgs.emplace_back(ast::MK::KeywordArg(symLoc, name));
            initializeSigArgs.emplace_back(ast::MK::Symbol(symLoc, name));
            initializeSigArgs.emplace_back(ASTUtil::dupType(memberType));
        }
    }

    if (newArgs.size() > 0) {
        body.emplace_back(ast::MK::Sig(loc, move(newSigArgs), ast::MK::AttachedClass(loc)));
        ast::MethodDef::Flags flags;
        flags.isSelfMethod = true;
        body.emplace_back(ast::MK::SyntheticMethod(loc, loc, core::Names::new_(), move(newArgs),
                                                   ast::MK::RaiseUnimplemented(loc), flags));
    }

    if (initializeArgs.size() > 0) {
        body.emplace_back(ast::MK::SigVoid(loc, move(initializeSigArgs)));
        body.emplace_back(ast::MK::SyntheticMethod(loc, loc, core::Names::initialize(), move(initializeArgs),
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
