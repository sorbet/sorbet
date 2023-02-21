#include "rewriter/Initializable.h"
#include "ast/Helpers.h"
#include "core/errors/rewriter.h"

using namespace std;
namespace sorbet::rewriter {

vector<ast::ExpressionPtr> Initializable::run(core::MutableContext ctx, bool isClass, ast::Send *send) {
    vector<ast::ExpressionPtr> empty;

    if (send->fun != core::Names::declareInitializable()) {
        return empty;
    }

    // TODO(jez) More validation? e.g. recv is self, no args, block
    // TODO(jez) support for bounds?

    if (isClass) {
        if (auto e = ctx.beginError(send->loc, core::errors::Rewriter::InitializableInClass)) {
            e.setHeader("Nope, can't do that");
        }
        return empty;
    }

    auto zeroLoc = send->loc.copyWithZeroLength();
    vector<ast::ExpressionPtr> result;
    auto lhs = ast::MK::UnresolvedConstant(zeroLoc, ast::MK::EmptyTree(), core::Names::Constants::AttachedClass());
    auto rhs = ast::MK::Send0(send->loc, ast::MK::Self(send->recv.loc()), core::Names::typeMember(), send->funLoc);
    result.emplace_back(ast::MK::Assign(send->loc, move(lhs), move(rhs)));
    result.emplace_back(send->deepCopy());
    return result;
}

} // namespace sorbet::rewriter
