#include "rewriter/AssertNil.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/core.h"

namespace sorbet::rewriter {

namespace {

bool isAssignable(const ast::ExpressionPtr &expr) {
    if (auto ident = ast::cast_tree<ast::UnresolvedIdent>(expr)) {
        // Local, instance, and class variables are assignable
        return ident->kind == ast::UnresolvedIdent::Kind::Local ||
               ident->kind == ast::UnresolvedIdent::Kind::Instance ||
               ident->kind == ast::UnresolvedIdent::Kind::Class;
    }

    if (ast::isa_tree<ast::Local>(expr)) {
        // Local variables (after namer pass) are assignable
        return true;
    }

    // Constants, method calls, literals, etc. are not assignable
    return false;
}

} // namespace

ast::ExpressionPtr AssertNil::run(core::MutableContext ctx, ast::Send *send) {
    if (send->fun != core::Names::assertNil() &&
        send->fun != core::Names::assertNotNil() &&
        send->fun != core::Names::refuteNil()) {
        return nullptr;
    }

    if (!send->recv.isSelfReference()) {
        return nullptr;
    }

    if (send->numPosArgs() == 0 || send->numPosArgs() > 2) {
        return nullptr;
    }

    auto loc = send->loc;
    auto &arg = send->getPosArg(0);

    if (!isAssignable(arg)) {
        return nullptr;
    }

    auto nilCheck = ast::MK::Send0(loc, arg.deepCopy(), core::Names::nil_p(), loc);
    auto raiseCall = ast::MK::Send0(loc, ast::MK::Self(loc), core::Names::raise(), loc);

    if (send->fun == core::Names::assertNil()) {
        nilCheck = ast::MK::Send0(loc, std::move(nilCheck), core::Names::bang(), loc);
    }

    return ast::MK::If(loc, std::move(nilCheck), std::move(raiseCall), ast::MK::EmptyTree());
}

} // namespace sorbet::rewriter
