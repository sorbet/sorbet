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
    if (ast::isa_tree<ast::EmptyTree>(scope->scope)) {
        return true;
    }
    auto *id = ast::cast_tree<ast::ConstantLit>(scope->scope);
    if (id == nullptr) {
        return false;
    }
    return id->symbol == core::Symbols::root();
}

ast::Send *asEnumsDo(ast::TreePtr &stat) {
    auto *send = ast::cast_tree<ast::Send>(stat);

    if (send != nullptr && send->block != nullptr && send->fun == core::Names::enums()) {
        return send;
    } else {
        return nullptr;
    }
}

vector<ast::TreePtr> badConst(core::MutableContext ctx, core::LocOffsets headerLoc, core::LocOffsets line1Loc) {
    if (auto e = ctx.beginError(headerLoc, core::errors::Rewriter::TEnumConstNotEnumValue)) {
        e.setHeader("All constants defined on an `{}` must be unique instances of the enum", "T::Enum");
        e.addErrorLine(core::Loc(ctx.file, line1Loc), "Enclosing definition here");
    }
    return {};
}

vector<ast::TreePtr> processStat(core::MutableContext ctx, ast::ClassDef *klass, ast::TreePtr &stat,
                                 FromWhere fromWhere) {
    auto *asgn = ast::cast_tree<ast::Assign>(stat);
    if (asgn == nullptr) {
        return {};
    }

    auto *lhs = ast::cast_tree<ast::UnresolvedConstantLit>(asgn->lhs);
    if (lhs == nullptr) {
        return {};
    }

    auto *rhs = ast::cast_tree<ast::Send>(asgn->rhs);
    if (rhs == nullptr) {
        return badConst(ctx, stat->loc, klass->loc);
    }

    if (rhs->fun != core::Names::selfNew() && rhs->fun != core::Names::let()) {
        return badConst(ctx, stat->loc, klass->loc);
    }

    if (rhs->fun == core::Names::selfNew() && !ast::MK::isMagicClass(rhs->recv)) {
        return badConst(ctx, stat->loc, klass->loc);
    }

    if (rhs->fun == core::Names::let()) {
        auto recv = ast::cast_tree<ast::UnresolvedConstantLit>(rhs->recv);
        if (recv == nullptr) {
            return badConst(ctx, stat->loc, klass->loc);
        }

        if (rhs->args.size() != 2) {
            return badConst(ctx, stat->loc, klass->loc);
        }

        auto arg0 = ast::cast_tree<ast::Send>(rhs->args[0]);
        if (arg0 == nullptr) {
            return badConst(ctx, stat->loc, klass->loc);
        }

        if (!ast::MK::isSelfNew(arg0)) {
            return badConst(ctx, stat->loc, klass->loc);
        }
    }

    // By this point, we have something that looks like
    //
    //   A = Magic.<self-new>(self) | T.let(Magic.<self-new>(self), ...)
    //
    // So we're good to process this thing as a new T::Enum value.

    if (fromWhere != FromWhere::Inside) {
        if (auto e = ctx.beginError(stat->loc, core::errors::Rewriter::TEnumOutsideEnumsDo)) {
            e.setHeader("Definition of enum value `{}` must be within the `{}` block for this `{}`",
                        lhs->cnst.show(ctx), "enums do", "T::Enum");
            e.addErrorLine(klass->declLoc, "Enclosing definition here");
        }
    }

    auto name = ctx.state.enterNameConstant(ctx.state.freshNameUnique(core::UniqueNameKind::TEnum, lhs->cnst, 1));
    auto classCnst = ast::MK::UnresolvedConstant(lhs->loc, ast::MK::EmptyTree(), name);
    ast::ClassDef::ANCESTORS_store parent;
    parent.emplace_back(klass->name->deepCopy());
    ast::ClassDef::RHS_store classRhs;
    classRhs.emplace_back(ast::MK::Send1(stat->loc, ast::MK::Self(stat->loc), core::Names::include(),
                                         ast::MK::Constant(stat->loc, core::Symbols::Singleton())));
    classRhs.emplace_back(ast::MK::Send0(stat->loc, ast::MK::Self(stat->loc), core::Names::declareFinal()));
    auto classDef = ast::MK::Class(stat->loc, core::Loc(ctx.file, stat->loc), classCnst->deepCopy(), std::move(parent),
                                   std::move(classRhs));

    auto singletonAsgn =
        ast::MK::Assign(stat->loc, std::move(asgn->lhs),
                        ast::MK::Send2(stat->loc, ast::MK::Constant(stat->loc, core::Symbols::T()), core::Names::let(),
                                       ast::MK::Send0(stat->loc, classCnst->deepCopy(), core::Names::instance()),
                                       std::move(classCnst)));

    vector<ast::TreePtr> result;
    result.emplace_back(std::move(classDef));
    result.emplace_back(std::move(asgn->rhs));
    result.emplace_back(std::move(singletonAsgn));
    return result;
}

void collectNewStats(core::MutableContext ctx, ast::ClassDef *klass, ast::TreePtr stat, FromWhere fromWhere) {
    auto newStats = processStat(ctx, klass, stat, fromWhere);
    if (newStats.empty()) {
        klass->rhs.emplace_back(std::move(stat));
    } else {
        for (auto &newStat : newStats) {
            klass->rhs.emplace_back(std::move(newStat));
        }
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
    klass->rhs.emplace_back(ast::MK::Send1(loc.offsets(), ast::MK::Self(loc.offsets()), core::Names::extend(),
                                           ast::MK::Constant(loc.offsets(), core::Symbols::T_Helpers())));
    klass->rhs.emplace_back(
        ast::MK::Send0(loc.offsets(), ast::MK::Self(loc.offsets()), core::Names::declareAbstract()));
    klass->rhs.emplace_back(ast::MK::Send0(loc.offsets(), ast::MK::Self(loc.offsets()), core::Names::declareSealed()));
    for (auto &stat : oldRHS) {
        if (auto enumsDo = asEnumsDo(stat)) {
            auto &block = ast::ref_tree<ast::Block>(enumsDo->block);
            if (auto insSeq = ast::cast_tree<ast::InsSeq>(block.body)) {
                for (auto &stat : insSeq->stats) {
                    collectNewStats(ctx, klass, std::move(stat), FromWhere::Inside);
                }
                collectNewStats(ctx, klass, std::move(insSeq->expr), FromWhere::Inside);
            } else {
                collectNewStats(ctx, klass, std::move(block.body), FromWhere::Inside);
            }
        } else {
            collectNewStats(ctx, klass, std::move(stat), FromWhere::Outside);
        }
    }
}
}; // namespace sorbet::rewriter
