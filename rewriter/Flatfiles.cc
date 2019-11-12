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
void Flatfiles::run(core::MutableContext ctx, ast::ClassDef *klass) {
    if (klass->kind != ast::Class || klass->ancestors.empty()) {
        return;
    }
    auto *cnst = ast::cast_tree<ast::UnresolvedConstantLit>(klass->ancestors.front().get());
    if (cnst == nullptr) {
        return;
    }
    auto record = ctx.state.enterNameConstant("Record");
    if (cnst->cnst != record) {
        return;
    }

    auto from = ctx.state.enterNameUTF8("from");
    vector<unique_ptr<ast::Expression>> methods;
    for (auto &stat : klass->rhs) {
        if (auto send = ast::cast_tree<ast::Send>(stat.get())) {
            if (send->fun != from || !send->recv->isSelfReference() || send->args.size() < 2) {
                continue;
            }
            auto &propName = send->args[1];
            auto propLit = ast::cast_tree<ast::Literal>(propName.get());
            if (!propLit) {
                continue;
            }
            auto sig = ast::MK::Sig0(send->loc, ast::MK::Untyped(send->loc));
            auto method = ast::MK::Method0(send->loc, send->loc, propLit->asSymbol(ctx), ast::MK::Nil(send->loc));
            methods.emplace_back(move(sig));
            methods.emplace_back(move(method));
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
