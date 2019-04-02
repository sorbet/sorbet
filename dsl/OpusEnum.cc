#include "dsl/OpusEnum.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/Context.h"
#include "core/Names.h"
#include "core/core.h"
#include "dsl/dsl.h"

using namespace std;

namespace sorbet::dsl {

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

void OpusEnum::patchDSL(core::MutableContext ctx, ast::ClassDef *klass) {
    if (ctx.state.runningUnderAutogen) {
        // TODO(jez) Verify whether this DSL pass is safe to run in for autogen
        return;
    }

    if (!isOpusEnum(ctx, klass)) {
        return;
    }

    for (auto &stat : klass->rhs) {
        auto *asgn = ast::cast_tree<ast::Assign>(stat.get());
        if (asgn == nullptr) {
            continue;
        }

        auto *rhs = ast::cast_tree<ast::Send>(asgn->rhs.get());
        if (rhs == nullptr) {
            continue;
        }

        if (!(rhs->fun == core::Names::new_() && rhs->recv->isSelfReference())) {
            continue;
        }

        auto loc = asgn->rhs->loc;
        auto T = ast::MK::Constant(loc, core::Symbols::T());
        asgn->rhs = ast::MK::Send2(loc, move(T), core::Names::let(), move(asgn->rhs), ast::MK::Self(loc));
    }
}

}; // namespace sorbet::dsl
