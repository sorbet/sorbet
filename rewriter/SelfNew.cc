#include "rewriter/SelfNew.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/core.h"

namespace sorbet::rewriter {

namespace {
ast::ExpressionPtr convertSelfNew(core::MutableContext ctx, ast::Send *send) {
    ast::Send::ARGS_store args;

    args.emplace_back(std::move(send->recv));

    for (auto &arg : send->args) {
        args.emplace_back(std::move(arg));
    }

    auto numPosArgs = send->numPosArgs + 1;
    return ast::MK::SelfNew(send->loc, numPosArgs, std::move(args), send->flags);
}

bool isSelfNewCallWithSplat(core::MutableContext ctx, ast::Send *send) {
    if (send->fun != core::Names::callWithSplat()) {
        return false;
    }

    if (!send->args[0].isSelfReference()) {
        return false;
    }

    auto *lit = ast::cast_tree<ast::Literal>(send->args[1]);
    if (lit == nullptr) {
        return false;
    }

    if (!core::isa_type<core::LiteralType>(lit->value)) {
        return false;
    }

    const auto &litType = core::cast_type_nonnull<core::LiteralType>(lit->value);
    if (litType.literalKind != core::LiteralType::LiteralTypeKind::Symbol) {
        return false;
    }

    if (litType.asName(ctx) != core::Names::new_()) {
        return false;
    }

    return true;
}

ast::ExpressionPtr convertSelfNewCallWithSplat(core::MutableContext ctx, ast::Send *send) {
    ast::Send::ARGS_store args;

    for (auto &arg : send->args) {
        args.emplace_back(std::move(arg));
    }

    auto magic = ast::MK::Constant(send->loc, core::Symbols::Magic());
    args[0] = std::move(magic);
    args[1] = ast::MK::Symbol(send->loc, core::Names::selfNew());

    return ast::MK::Send(send->loc, std::move(send->recv), send->fun, send->numPosArgs, std::move(args), send->flags);
}
} // namespace

ast::ExpressionPtr SelfNew::run(core::MutableContext ctx, ast::Send *send) {
    if (send->fun == core::Names::new_() && send->recv.isSelfReference()) {
        return convertSelfNew(ctx, send);
    }

    if (isSelfNewCallWithSplat(ctx, send)) {
        return convertSelfNewCallWithSplat(ctx, send);
    }

    return nullptr;
}

} // namespace sorbet::rewriter
