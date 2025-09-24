#include "rewriter/RequiresAncestorKernel.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/Names.h"
#include "core/core.h"

using namespace std;

namespace sorbet::rewriter {

namespace {

bool hasAnyRequiresAncestorCall(core::MutableContext ctx, ast::ClassDef *klass) {
    return absl::c_any_of(klass->rhs, [](const auto &stat) {
        auto send = ast::cast_tree<ast::Send>(stat);
        return send && send->fun == core::Names::requiresAncestor() && send->recv.isSelfReference() &&
               send->nonBlockArgs().empty() && send->hasBlock();
    });
}

} // namespace

void RequiresAncestorKernel::run(core::MutableContext ctx, ast::ClassDef *klass) {
    if (klass->kind != ast::ClassDef::Kind::Module) {
        return;
    }

    // If the module is named Kernel and has no scope, don't add Kernel
    auto id = ast::cast_tree<ast::UnresolvedConstantLit>(klass->name);
    if (id && id->cnst == core::Names::Constants::Kernel() && ast::MK::isRootScope(id->scope)) {
        return;
    }

    if (hasAnyRequiresAncestorCall(ctx, klass)) {
        return;
    }

    auto loc = klass->loc;
    auto locZero = loc.copyWithZeroLength();

    // Add extend T::Helpers
    klass->rhs.emplace_back(ast::MK::Send1(loc, ast::MK::Self(loc), core::Names::extend(), locZero,
                                           ast::MK::Constant(loc, core::Symbols::T_Helpers())));

    // Add requires_ancestor { Kernel }
    auto kernelConstant = ast::MK::Constant(loc, core::Symbols::Kernel());
    auto block = ast::MK::Block0(loc, std::move(kernelConstant));
    auto requiresAncestorSend =
        ast::MK::Send0Block(loc, ast::MK::Self(loc), core::Names::requiresAncestor(), loc, std::move(block));

    klass->rhs.emplace_back(std::move(requiresAncestorSend));
}

} // namespace sorbet::rewriter
