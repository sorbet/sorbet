#include "rewriter/HasAttachedClass.h"
#include "ast/Helpers.h"
#include "core/errors/rewriter.h"

using namespace std;
namespace sorbet::rewriter {

vector<ast::ExpressionPtr> HasAttachedClass::run(core::MutableContext ctx, bool isClass, ast::Send *send) {
    vector<ast::ExpressionPtr> empty;

    if (send->fun != core::Names::declareHasAttachedClass() || !send->recv.isSelfReference()) {
        return empty;
    }

    if (isClass) {
        // TODO(jez) Figure out some way to allow `initializeable!` to only be used in `Class` (or
        // maybe `Class` + children of `Class`)
        // if (auto e = ctx.beginError(send->loc, core::errors::Rewriter::HasAttachedClassInClass)) {
        //     e.setHeader("`{}` can only be used inside a `{}`, not a `{}`",
        //                 core::Names::declareHasAttachedClass().show(ctx), "module", "class");
        // }
        return empty;
    }

    if (send->numPosArgs() > 0) {
        const auto &arg0 = send->posArgs()[0];
        if (const auto *lit = ast::cast_tree<ast::Literal>(arg0)) {
            if (lit->isSymbol() && lit->asSymbol() == core::Names::contravariant()) {
                if (auto e = ctx.beginError(arg0.loc(), core::errors::Rewriter::ContravariantHasAttachedClass)) {
                    e.setHeader("`{}` cannot be declared `{}`, only invariant or `{}`",
                                core::Names::declareHasAttachedClass().show(ctx), ":in", ":out");
                    e.replaceWith("Convert to covariant", ctx.locAt(arg0.loc()), "{}", ":out");
                }
            }
        }
    }

    auto zeroLoc = send->loc.copyWithZeroLength();
    vector<ast::ExpressionPtr> result;
    auto lhs = ast::MK::UnresolvedConstant(zeroLoc, ast::MK::EmptyTree(), core::Names::Constants::AttachedClass());

    // `has_attached_class!` and `type_member` have the same arity, so let's just dup the Send and
    // change the fun so that everything gets passed through. The rest of the pipeline will validate
    // that any args passed here are correct or not.
    //
    // (If there are other popular Ruby DSLs using `has_attached_class!`, we can consider being more
    // conservative in this rewrite by only applying it if the args look correct, but being
    // over-eager like this makes it more obvious when the user did something like make a typo
    // which would have prevented the rewriter from firing.)
    auto rhs = send->deepCopy();
    auto &rhsSend = ast::cast_tree_nonnull<ast::Send>(rhs);
    rhsSend.fun = core::Names::typeMember();

    // Need the call to `type_member` in an `Assign` node specicially, because all the downstream
    // logic in namer and resolver expects type members to be declared via `Assign` nodes.
    result.emplace_back(ast::MK::Assign(send->loc, move(lhs), move(rhs)));

    // Keep the call to `has_attached_class!` in the tree so that it still gets type checked.
    // If this proves to be a problem, we can either: change the `Assign` to look like
    //
    //   <AttachedClass> = has_attached_class!(...) {...}
    //
    // or we can just drop the call to `has_attached_class!`, or we can try to do some loc munging to
    // fix any problems that arise.
    result.emplace_back(send->deepCopy());
    return result;
}

} // namespace sorbet::rewriter
