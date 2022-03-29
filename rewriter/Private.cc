#include "rewriter/Private.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/Context.h"
#include "core/Names.h"
#include "core/core.h"
#include "core/errors/rewriter.h"
#include "rewriter/rewriter.h"

using namespace std;

namespace sorbet::rewriter {

vector<ast::ExpressionPtr> Private::run(core::MutableContext ctx, ast::Send *send) {
    vector<ast::ExpressionPtr> empty;

    if (send->numPosArgs() != 1) {
        return empty;
    }

    auto mdef = ast::cast_tree<ast::MethodDef>(send->getPosArg(0));
    if (mdef == nullptr) {
        return empty;
    }

    if (send->fun == core::Names::private_() && mdef->flags.isSelfMethod) {
        if (auto e = ctx.beginError(send->loc, core::errors::Rewriter::PrivateMethodMismatch)) {
            e.setHeader("Use `{}` to define private class methods", "private_class_method");
            auto replacementLoc = ctx.locAt(send->funLoc);
            e.replaceWith("Replace with `private_class_method`", replacementLoc, "private_class_method");
        }
    } else if (send->fun == core::Names::privateClassMethod() && !mdef->flags.isSelfMethod) {
        if (auto e = ctx.beginError(send->loc, core::errors::Rewriter::PrivateMethodMismatch)) {
            e.setHeader("Use `{}` to define private instance methods", "private");
            auto replacementLoc = ctx.locAt(send->funLoc);
            e.replaceWith("Replace with `private`", replacementLoc, "private");
        }
    } else if (send->fun == core::Names::packagePrivate() && mdef->flags.isSelfMethod) {
        if (auto e = ctx.beginError(send->loc, core::errors::Rewriter::PrivateMethodMismatch)) {
            e.setHeader("Use `{}` to define package-private class methods", "package_private_class_method");
            auto replacementLoc = ctx.locAt(send->funLoc);
            e.replaceWith("Replace with `package_private_class_method`", replacementLoc,
                          "package_private_class_method");
        }
    } else if (send->fun == core::Names::packagePrivateClassMethod() && !mdef->flags.isSelfMethod) {
        if (auto e = ctx.beginError(send->loc, core::errors::Rewriter::PrivateMethodMismatch)) {
            e.setHeader("Use `{}` to define package-private instance methods", "package_private");
            auto replacementLoc = ctx.locAt(send->funLoc);
            e.replaceWith("Replace with `package_private`", replacementLoc, "package_private");
        }
    }

    return empty;
}

}; // namespace sorbet::rewriter
