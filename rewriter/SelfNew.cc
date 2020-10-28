#include "rewriter/SelfNew.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/core.h"

namespace sorbet::rewriter {

ast::TreePtr SelfNew::run(core::MutableContext ctx, ast::Send *send) {
    if (send->fun != core::Names::new_() || !send->recv->isSelfReference()) {
        return nullptr;
    }

    ast::Send::ARGS_store args;

    args.emplace_back(std::move(send->recv));

    for (auto &arg : send->args) {
        args.emplace_back(std::move(arg));
    }

    auto numPosArgs = send->numPosArgs + 1;
    return ast::MK::SelfNew(send->loc, numPosArgs, std::move(args), send->flags, std::move(send->block));
}

} // namespace sorbet::rewriter
