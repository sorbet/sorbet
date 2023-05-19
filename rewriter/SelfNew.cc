#include "rewriter/SelfNew.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/core.h"

namespace sorbet::rewriter {

namespace {
ast::ExpressionPtr convertSelfNew(core::MutableContext ctx, ast::Send *send) {
    ast::Send::ARGS_store args;

    args.emplace_back(std::move(send->recv));

    for (auto &arg : send->nonBlockArgs()) {
        args.emplace_back(std::move(arg));
    }

    if (auto *block = send->rawBlock()) {
        args.emplace_back(std::move(*block));
    }

    return ast::MK::SelfNew(send->loc, send->funLoc, send->numPosArgs() + 1, std::move(args), send->flags);
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

    if (!core::isa_type<core::NamedLiteralType>(lit->value)) {
        return false;
    }

    const auto &litType = core::cast_type_nonnull<core::NamedLiteralType>(lit->value);
    if (litType.literalKind != core::NamedLiteralType::LiteralTypeKind::Symbol) {
        return false;
    }

    if (litType.asName() != core::Names::new_()) {
        return false;
    }

    return true;
}

ast::ExpressionPtr convertSelfNewCallWithSplat(core::MutableContext ctx, ast::Send *send) {
    auto magic = ast::MK::Magic(send->loc);
    auto &arg0 = send->getPosArg(0);
    arg0 = std::move(magic);

    auto &arg1 = send->getPosArg(1);
    arg1 = ast::MK::Symbol(send->loc, core::Names::selfNew());

    // The original expression is now properly mutated, but we need to return an expression not a Send.
    return send->withNewBody(send->loc, std::move(send->recv), send->fun);
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
