#include "rewriter/Rails.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/Context.h"
#include "core/Names.h"
#include "core/core.h"
#include "core/errors/rewriter.h"
#include "rewriter/rewriter.h"

using namespace std;

namespace sorbet::rewriter {

void Rails::run(core::MutableContext ctx, ast::ClassDef *cdef) {
    if (cdef->ancestors.size() != 1) {
        return;
    }
    auto send = ast::cast_tree<ast::Send>(cdef->ancestors[0]);
    if (!send) {
        return;
    }
    if (send->fun != core::Names::squareBrackets()) {
        return;
    }
    auto name = ast::cast_tree<ast::UnresolvedConstantLit>(send->recv);
    if (!name) {
        return;
    }
    if (name->cnst != core::Names::Constants::Migration()) {
        return;
    }
    auto name2 = ast::cast_tree<ast::UnresolvedConstantLit>(name->scope);
    if (!name2) {
        return;
    }
    if (name2->cnst != core::Names::Constants::ActiveRecord()) {
        return;
    }
    if (send->args.size() != 1 && !send->hasKwArgs()) {
        return;
    }
    auto arg = ast::cast_tree<ast::Literal>(send->args[0]);
    if (!arg) {
        return;
    }
    auto value = core::cast_type<core::LiteralType>(arg->value.get());
    if (value->literalKind != core::LiteralType::LiteralTypeKind::Float) {
        return;
    }
    char version[5];
    snprintf(version, sizeof(version), "V%.1f", value->floatval);
    absl::c_replace(version, '.', '_');

    cdef->ancestors.emplace_back(ast::MK::UnresolvedConstant(
        arg->loc, ast::MK::UnresolvedConstant(arg->loc, std::move(send->recv), core::Names::Constants::Compatibility()),
        ctx.state.enterNameConstant(version)));
    cdef->ancestors.erase(cdef->ancestors.begin(), cdef->ancestors.begin() + 1);
}

}; // namespace sorbet::rewriter
