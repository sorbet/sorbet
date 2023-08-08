#include "rewriter/TEnum.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/Context.h"
#include "core/Names.h"
#include "core/core.h"
#include "core/errors/rewriter.h"
#include "rewriter/rewriter.h"

using namespace std;

namespace sorbet::rewriter {

namespace {

enum class FromWhere {
    Inside,
    Outside,
};

bool isTEnum(core::MutableContext ctx, ast::ClassDef *klass) {
    if (klass->kind != ast::ClassDef::Kind::Class || klass->ancestors.empty()) {
        return false;
    }
    auto *cnst = ast::cast_tree<ast::UnresolvedConstantLit>(klass->ancestors.front());
    if (cnst == nullptr) {
        return false;
    }
    if (cnst->cnst != core::Names::Constants::Enum()) {
        return false;
    }
    auto *scope = ast::cast_tree<ast::UnresolvedConstantLit>(cnst->scope);
    if (scope == nullptr) {
        return false;
    }
    if (scope->cnst != core::Names::Constants::T()) {
        return false;
    }
    return ast::MK::isRootScope(scope->scope);
}

ast::Send *asEnumsDo(ast::ExpressionPtr &stat) {
    auto *send = ast::cast_tree<ast::Send>(stat);

    if (send != nullptr && send->hasBlock() && send->fun == core::Names::enums()) {
        return send;
    } else {
        return nullptr;
    }
}

void badConst(core::MutableContext ctx, core::LocOffsets headerLoc, core::LocOffsets line1Loc) {
    if (auto e = ctx.beginError(headerLoc, core::errors::Rewriter::BadTEnumSyntax)) {
        e.setHeader("All constants defined on an `{}` must be unique instances of the enum", "T::Enum");
        e.addErrorLine(ctx.locAt(line1Loc), "Enclosing definition here");
    }
}

ast::Send *findSelfNew(ast::ExpressionPtr &assignRhs) {
    auto *rhs = ast::cast_tree<ast::Send>(assignRhs);
    if (rhs != nullptr) {
        if (rhs->fun != core::Names::new_() && rhs->fun != core::Names::let()) {
            return nullptr;
        }

        if (rhs->fun == core::Names::new_() && !rhs->recv.isSelfReference()) {
            return nullptr;
        }

        auto *magicSelfNew = rhs;
        if (rhs->fun == core::Names::let()) {
            auto recv = ast::cast_tree<ast::UnresolvedConstantLit>(rhs->recv);
            if (recv == nullptr) {
                return nullptr;
            }

            if (rhs->numPosArgs() != 2) {
                return nullptr;
            }

            auto arg0 = ast::cast_tree<ast::Send>(rhs->getPosArg(0));
            if (arg0 == nullptr) {
                return nullptr;
            }

            if (!ast::MK::isSelfNew(arg0)) {
                return nullptr;
            }
            magicSelfNew = arg0;
        }
        return magicSelfNew;
    }

    auto *cast = ast::cast_tree<ast::Cast>(assignRhs);
    if (cast == nullptr) {
        return nullptr;
    }

    return findSelfNew(cast->arg);
}

struct ProcessStatResult {
    vector<ast::ExpressionPtr> stats;
    core::TypePtr type;
};

std::optional<ProcessStatResult> processStat(core::MutableContext ctx, ast::ClassDef *klass, ast::ExpressionPtr &stat,
                                             FromWhere fromWhere) {
    auto *asgn = ast::cast_tree<ast::Assign>(stat);
    if (asgn == nullptr) {
        return {};
    }

    auto *lhs = ast::cast_tree<ast::UnresolvedConstantLit>(asgn->lhs);
    if (lhs == nullptr) {
        return {};
    }

    auto *selfNew = findSelfNew(asgn->rhs);
    if (selfNew == nullptr) {
        badConst(ctx, stat.loc(), klass->loc);
        return {};
    }

    // By this point, we have something that looks like
    //
    //   A = <self>.new(...) | T.let(<self>.new(...))
    //
    // So we're good to process this thing as a new T::Enum value.

    core::TypePtr serializeType = core::Types::untypedUntracked();

    if (selfNew->numPosArgs() == 0 && selfNew->onlyPosArgs()) {
        serializeType = core::Types::String();
    } else if (selfNew->numPosArgs() == 1) {
        if (auto *selfNewArg = ast::cast_tree<ast::Literal>(selfNew->getPosArg(0))) {
            // If the enum has exactly one variant that has a literal passed (ex. "a"),
            // then its type will be String("a"), but we want a ClassType as the return type.
            serializeType = core::Types::dropLiteral(ctx, selfNewArg->value);
        }
    }

    if (fromWhere != FromWhere::Inside) {
        if (auto e = ctx.beginError(stat.loc(), core::errors::Rewriter::BadTEnumSyntax)) {
            e.setHeader("Definition of enum value `{}` must be within the `{}` block for this `{}`",
                        lhs->cnst.show(ctx), "enums do", "T::Enum");
            e.addErrorLine(ctx.locAt(klass->declLoc), "Enclosing definition here");
        }
    }

    auto statLocZero = stat.loc().copyWithZeroLength();
    auto name = ctx.state.enterNameConstant(ctx.state.freshNameUnique(core::UniqueNameKind::TEnum, lhs->cnst, 1));
    auto classCnst = ast::MK::UnresolvedConstant(statLocZero, ast::MK::EmptyTree(), name);
    ast::ClassDef::ANCESTORS_store parent;
    parent.emplace_back(klass->name.deepCopy());
    ast::ClassDef::RHS_store classRhs;
    auto classDef =
        ast::MK::Class(statLocZero, statLocZero, classCnst.deepCopy(), std::move(parent), std::move(classRhs));

    ast::Send::Flags flags = {};
    flags.isPrivateOk = true;
    auto singletonAsgn = ast::MK::Assign(
        statLocZero, std::move(asgn->lhs),
        ast::make_expression<ast::Cast>(statLocZero, core::Types::todo(),
                                        selfNew->withNewBody(selfNew->loc, classCnst.deepCopy(), core::Names::new_()),
                                        core::Names::uncheckedLet(), std::move(classCnst)));
    vector<ast::ExpressionPtr> result;
    result.emplace_back(std::move(classDef));
    result.emplace_back(std::move(singletonAsgn));

    return {{std::move(result), serializeType}};
}

core::TypePtr collectNewStats(core::MutableContext ctx, ast::ClassDef *klass, ast::ExpressionPtr stat,
                              FromWhere fromWhere, vector<ast::ExpressionPtr> &into) {
    auto result = processStat(ctx, klass, stat, fromWhere);
    if (result) {
        auto [newStats, type] = std::move(*result);
        for (auto &newStat : newStats) {
            into.emplace_back(std::move(newStat));
        }
        return type;
    } else {
        into.emplace_back(std::move(stat));
        return core::Types::bottom();
    }
}

} // namespace

void TEnum::run(core::MutableContext ctx, ast::ClassDef *klass) {
    if (ctx.state.runningUnderAutogen) {
        return;
    }

    if (!isTEnum(ctx, klass)) {
        return;
    }

    auto oldRHS = std::move(klass->rhs);
    klass->rhs.clear();
    klass->rhs.reserve(oldRHS.size());
    auto loc = klass->declLoc;
    auto locZero = loc.copyWithZeroLength();
    klass->rhs.emplace_back(ast::MK::Send1(loc, ast::MK::Self(loc), core::Names::extend(), locZero,
                                           ast::MK::Constant(loc, core::Symbols::T_Helpers())));
    klass->rhs.emplace_back(ast::MK::Send0(loc, ast::MK::Self(loc), core::Names::declareAbstract(), locZero));
    klass->rhs.emplace_back(ast::MK::Send0(loc, ast::MK::Self(loc), core::Names::declareSealed(), locZero));
    core::TypePtr serializeReturnType = core::Types::bottom();
    for (auto &stat : oldRHS) {
        if (auto enumsDo = asEnumsDo(stat)) {
            auto *block = enumsDo->block();
            vector<ast::ExpressionPtr> newStats;
            if (auto insSeq = ast::cast_tree<ast::InsSeq>(block->body)) {
                for (auto &stat : insSeq->stats) {
                    auto type = collectNewStats(ctx, klass, std::move(stat), FromWhere::Inside, newStats);
                    serializeReturnType = core::Types::any(ctx, serializeReturnType, type);
                }
                auto type = collectNewStats(ctx, klass, std::move(insSeq->expr), FromWhere::Inside, newStats);
                serializeReturnType = core::Types::any(ctx, serializeReturnType, type);
            } else {
                auto type = collectNewStats(ctx, klass, std::move(block->body), FromWhere::Inside, newStats);
                serializeReturnType = core::Types::any(ctx, serializeReturnType, type);
            }

            ast::InsSeq::STATS_store insSeqStats;
            for (auto &newStat : newStats) {
                insSeqStats.emplace_back(std::move(newStat));
            }

            block->body = ast::MK::InsSeq(block->loc, std::move(insSeqStats), ast::MK::Nil(block->loc));
            klass->rhs.emplace_back(std::move(stat));
        } else {
            vector<ast::ExpressionPtr> newStats;
            collectNewStats(ctx, klass, std::move(stat), FromWhere::Outside, newStats);
            for (auto &newStat : newStats) {
                klass->rhs.emplace_back(std::move(newStat));
            }
        }
    }
    if (core::isa_type<core::ClassType>(serializeReturnType) && !serializeReturnType.isUntyped() &&
        !serializeReturnType.isBottom()) {
        auto serializeReturnTypeClass = core::cast_type_nonnull<core::ClassType>(serializeReturnType);
        ast::ExpressionPtr return_type_ast = ast::MK::Constant(klass->declLoc, serializeReturnTypeClass.symbol);
        auto sig = ast::MK::Sig0(klass->declLoc, std::move(return_type_ast));
        auto method = ast::MK::SyntheticMethod0(klass->loc, klass->declLoc, core::Names::serialize(),
                                                ast::MK::RaiseTypedUnimplemented(klass->declLoc));
        ast::Send::ARGS_store nargs;
        ast::Send::Flags flags;
        flags.isPrivateOk = true;
        auto visibility = ast::MK::Send(klass->declLoc, ast::MK::Self(klass->declLoc), core::Names::public_(),
                                        klass->declLoc, 0, std::move(nargs), flags);

        klass->rhs.emplace_back(std::move(visibility));
        klass->rhs.emplace_back(std::move(sig));
        klass->rhs.emplace_back(std::move(method));
    }
}
}; // namespace sorbet::rewriter
