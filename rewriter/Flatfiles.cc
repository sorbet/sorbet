#include "rewriter/Flatfiles.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/Context.h"
#include "core/Names.h"
#include "core/core.h"
#include "core/errors/rewriter.h"
#include "rewriter/rewriter.h"

using namespace std;

namespace sorbet::rewriter {
optional<core::NameRef> getFieldName(core::MutableContext ctx, ast::Send& send) {
    if (auto propLit = ast::cast_tree<ast::Literal>(send.args.front().get())) {
        if (propLit->isSymbol(ctx)) {
            return propLit->asSymbol(ctx);
        }
    }
    if (send.args.size() >= 1) {
        if (auto propLit = ast::cast_tree<ast::Literal>(send.args[1].get())) {
            if (propLit->isSymbol(ctx)) {
                return propLit->asSymbol(ctx);
            }
        }
    }
    return nullopt;
}

void Flatfiles::run(core::MutableContext ctx, ast::ClassDef *klass) {
    if (klass->kind != ast::Class || klass->ancestors.empty()) {
        return;
    }
    auto from = ctx.state.enterNameUTF8("from");
    auto field = ctx.state.enterNameUTF8("field");
    auto pattern = ctx.state.enterNameUTF8("pattern");
    vector<unique_ptr<ast::Expression>> methods;
    for (auto &stat : klass->rhs) {
        if (auto send = ast::cast_tree<ast::Send>(stat.get())) {
            if ((send->fun != from && send->fun != pattern && send->fun != field) || !send->recv->isSelfReference() || send->args.size() < 1) {
                continue;
            }
            auto name = getFieldName(ctx, *send);
            if (!name) {
                continue;
            }

            methods.emplace_back(ast::MK::Sig0(send->loc, ast::MK::Untyped(send->loc)));
            methods.emplace_back(ast::MK::Method0(send->loc, send->loc, *name, ast::MK::Nil(send->loc)));
            auto var = ast::MK::Local(send->loc, core::Names::arg0());
            auto setName = name->addEq(ctx);
            methods.emplace_back(ast::MK::Sig1(send->loc, ast::MK::Symbol(send->loc, core::Names::arg0()), ast::MK::Untyped(send->loc), ast::MK::Untyped(send->loc)));
            methods.emplace_back(ast::MK::Method1(send->loc, send->loc, setName, move(var), ast::MK::Nil(send->loc)));
        }
    }
    if (methods.empty()) {
        klass = nullptr;
    }

    for (auto &m : methods) {
        klass->rhs.emplace_back(move(m));
    }
}
}; // namespace sorbet::rewriter
