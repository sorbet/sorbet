#include "rewriter/SelfNew.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/core.h"

namespace sorbet::rewriter {

namespace {
ast::ExpressionPtr convertSelfNew(core::MutableContext ctx, ast::Send *send) {
    auto numPosArgs = send->numPosArgs();
    ast::Send::ARGS_store args;

    args.emplace_back(std::move(send->recv));

    for (auto i = 0; i < numPosArgs; ++i) {
        args.emplace_back(std::move(send->getPosArg(i)));
    }

    const auto numKwArgs = send->numKwArgs();
    for (auto i = 0; i < numKwArgs; ++i) {
        args.emplace_back(std::move(send->getKwKey(i)));
        args.emplace_back(std::move(send->getKwValue(i)));
    }

    if (send->hasKwSplat()) {
        args.emplace_back(std::move(*send->kwSplat()));
    }

    if (send->hasBlock()) {
        args.emplace_back(std::move(*send->rawBlock()));
    }

    return ast::MK::SelfNew(send->loc, numPosArgs + 1, std::move(args), send->flags);
}

bool isSelfNewCallWithSplat(core::MutableContext ctx, ast::Send *send) {
    if (send->fun != core::Names::callWithSplat()) {
        return false;
    }

    if (!send->getPosArg(0).isSelfReference()) {
        return false;
    }

    auto *lit = ast::cast_tree<ast::Literal>(send->getPosArg(1));
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
    auto flags = send->flags;
    ast::Send::ARGS_store args = std::move(send->rawArgs());
    auto magic = ast::MK::Constant(send->loc, core::Symbols::Magic());
    args[0] = std::move(magic);
    args[1] = ast::MK::Symbol(send->loc, core::Names::selfNew());

    return ast::MK::Send(send->loc, std::move(send->recv), send->fun, send->numPosArgs(), std::move(args), flags);
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
