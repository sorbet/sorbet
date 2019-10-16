#include "dsl/OpusEnum.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/Context.h"
#include "core/Names.h"
#include "core/core.h"
#include "core/errors/dsl.h"
#include "dsl/dsl.h"

using namespace std;

namespace sorbet::dsl {

namespace {

enum class FromWhere {
    Inside,
    Outside,
};

bool isOpusEnum(core::MutableContext ctx, ast::ClassDef *klass) {
    if (klass->kind != ast::Class || klass->ancestors.empty()) {
        return false;
    }
    auto *cnst = ast::cast_tree<ast::UnresolvedConstantLit>(klass->ancestors.front().get());
    if (cnst == nullptr) {
        return false;
    }
    if (cnst->cnst != core::Names::Constants::Enum()) {
        return false;
    }
    auto *scope = ast::cast_tree<ast::UnresolvedConstantLit>(cnst->scope.get());
    if (scope == nullptr) {
        return false;
    }
    if (scope->cnst != core::Names::Constants::Opus()) {
        return false;
    }
    if (ast::isa_tree<ast::EmptyTree>(scope->scope.get())) {
        return true;
    }
    auto *id = ast::cast_tree<ast::ConstantLit>(scope->scope.get());
    if (id == nullptr) {
        return false;
    }
    return id->symbol == core::Symbols::root();
}

ast::Send *asEnumsDo(ast::Expression *stat) {
    auto *send = ast::cast_tree<ast::Send>(stat);

    if (send != nullptr && send->block != nullptr && send->fun == core::Names::enums()) {
        return send;
    } else {
        return nullptr;
    }
}

vector<unique_ptr<ast::Expression>> badConst(core::MutableContext ctx, core::Loc headerLoc, core::Loc line1Loc) {
    if (auto e = ctx.state.beginError(headerLoc, core::errors::DSL::OpusEnumConstNotEnumValue)) {
        e.setHeader("All constants defined on an `{}` must be unique instances of the enum", "Opus::Enum");
        e.addErrorLine(line1Loc, "Enclosing definition here");
    }
    return {};
}

vector<unique_ptr<ast::Expression>> processStat(core::MutableContext ctx, ast::ClassDef *klass, ast::Expression *stat,
                                                FromWhere fromWhere) {
    auto *asgn = ast::cast_tree<ast::Assign>(stat);
    if (asgn == nullptr) {
        return {};
    }

    auto *lhs = ast::cast_tree<ast::UnresolvedConstantLit>(asgn->lhs.get());
    if (lhs == nullptr) {
        return {};
    }

    auto *rhs = ast::cast_tree<ast::Send>(asgn->rhs.get());
    if (rhs == nullptr) {
        return badConst(ctx, stat->loc, klass->loc);
    }

    if (rhs->fun != core::Names::new_() && rhs->fun != core::Names::let()) {
        return badConst(ctx, stat->loc, klass->loc);
    }

    if (rhs->fun == core::Names::new_() && !rhs->recv->isSelfReference()) {
        return badConst(ctx, stat->loc, klass->loc);
    }

    if (rhs->fun == core::Names::let()) {
        auto recv = ast::cast_tree<ast::UnresolvedConstantLit>(rhs->recv.get());
        if (recv == nullptr) {
            return badConst(ctx, stat->loc, klass->loc);
        }

        if (rhs->args.size() != 2) {
            return badConst(ctx, stat->loc, klass->loc);
        }

        auto arg0 = ast::cast_tree<ast::Send>(rhs->args[0].get());
        if (arg0 == nullptr) {
            return badConst(ctx, stat->loc, klass->loc);
        }

        if (!(arg0->fun == core::Names::new_() && arg0->recv->isSelfReference())) {
            return badConst(ctx, stat->loc, klass->loc);
        }
    }

    // By this point, we have something that looks like
    //
    //   A = new | T.let(new, ...)
    //
    // So we're good to process this thing as a new Opus::Enum value.

    if (fromWhere != FromWhere::Inside) {
        if (auto e = ctx.state.beginError(stat->loc, core::errors::DSL::OpusEnumOutsideEnumsDo)) {
            e.setHeader("Definition of enum value `{}` must be within the `{}` block for this `{}`",
                        lhs->cnst.show(ctx), "enums do", "Opus::Enum");
            e.addErrorLine(klass->declLoc, "Enclosing definition here");
        }
    }

    auto name = ctx.state.enterNameConstant(ctx.state.freshNameUnique(core::UniqueNameKind::OpusEnum, lhs->cnst, 1));
    auto classCnst = ast::MK::UnresolvedConstant(lhs->loc, ast::MK::EmptyTree(), name);
    ast::ClassDef::ANCESTORS_store parent;
    parent.emplace_back(klass->name->deepCopy());
    ast::ClassDef::RHS_store classRhs;
    classRhs.emplace_back(ast::MK::Send1(stat->loc, ast::MK::Self(stat->loc), core::Names::include(),
                                         ast::MK::Constant(stat->loc, core::Symbols::Singleton())));
    classRhs.emplace_back(ast::MK::Send0(stat->loc, ast::MK::Self(stat->loc), core::Names::declareFinal()));
    auto classDef = ast::MK::Class(stat->loc, stat->loc, classCnst->deepCopy(), std::move(parent), std::move(classRhs),
                                   ast::ClassDefKind::Class);

    auto singletonAsgn =
        ast::MK::Assign(stat->loc, std::move(asgn->lhs),
                        ast::MK::Send2(stat->loc, ast::MK::Constant(stat->loc, core::Symbols::T()), core::Names::let(),
                                       ast::MK::Send0(stat->loc, classCnst->deepCopy(), core::Names::instance()),
                                       std::move(classCnst)));

    vector<unique_ptr<ast::Expression>> result;
    result.emplace_back(std::move(classDef));
    result.emplace_back(std::move(singletonAsgn));
    return result;
}

void collectNewStats(core::MutableContext ctx, ast::ClassDef *klass, unique_ptr<ast::Expression> stat,
                     FromWhere fromWhere) {
    auto newStats = processStat(ctx, klass, stat.get(), fromWhere);
    if (newStats.empty()) {
        klass->rhs.emplace_back(std::move(stat));
    } else {
        for (auto &newStat : newStats) {
            klass->rhs.emplace_back(std::move(newStat));
        }
    }
}

} // namespace

void OpusEnum::patchDSL(core::MutableContext ctx, ast::ClassDef *klass) {
    if (ctx.state.runningUnderAutogen) {
        return;
    }

    if (!isOpusEnum(ctx, klass)) {
        return;
    }

    auto oldRHS = std::move(klass->rhs);
    klass->rhs.clear();
    klass->rhs.reserve(oldRHS.size());
    auto loc = klass->declLoc;
    klass->rhs.emplace_back(ast::MK::Send1(loc, ast::MK::Self(loc), core::Names::extend(),
                                           ast::MK::Constant(loc, core::Symbols::T_Helpers())));
    klass->rhs.emplace_back(ast::MK::Send0(loc, ast::MK::Self(loc), core::Names::declareAbstract()));
    klass->rhs.emplace_back(ast::MK::Send0(loc, ast::MK::Self(loc), core::Names::declareSealed()));
    for (auto &stat : oldRHS) {
        // TODO(jez) Clean this up when there is only one way to define enum variants
        if (auto enumsDo = asEnumsDo(stat.get())) {
            if (auto insSeq = ast::cast_tree<ast::InsSeq>(enumsDo->block->body.get())) {
                for (auto &stat : insSeq->stats) {
                    collectNewStats(ctx, klass, std::move(stat), FromWhere::Inside);
                }
                collectNewStats(ctx, klass, std::move(insSeq->expr), FromWhere::Inside);
            } else {
                collectNewStats(ctx, klass, std::move(enumsDo->block->body), FromWhere::Inside);
            }
        } else {
            collectNewStats(ctx, klass, std::move(stat), FromWhere::Outside);
        }
    }
}
}; // namespace sorbet::dsl
