#include "rewriter/SigRewriter.h"
#include "ast/Helpers.h"
#include "ast/treemap/treemap.h"

using namespace std;
namespace sorbet::rewriter {

namespace {

bool isTSigWithoutRuntime(ast::TreePtr &expr) {
    if (auto *cnst = ast::cast_tree<ast::ConstantLit>(expr)) {
        return cnst->symbol == core::Symbols::T_Sig_WithoutRuntime();
    } else {
        auto *withoutRuntime = ast::cast_tree<ast::UnresolvedConstantLit>(expr);
        if (withoutRuntime == nullptr || withoutRuntime->cnst != core::Names::Constants::WithoutRuntime()) {
            return false;
        }
        auto *sig = ast::cast_tree<ast::UnresolvedConstantLit>(withoutRuntime->scope);
        if (sig == nullptr || sig->cnst != core::Names::Constants::Sig()) {
            return false;
        }
        auto *t = ast::cast_tree<ast::UnresolvedConstantLit>(sig->scope);
        return t != nullptr && t->cnst == core::Names::Constants::T() && ast::MK::isRootScope(t->scope);
    }
}

} // namespace

// Rewrite all sig usage into uses of `Sorbet::Private::Static.<sig>`, and mark them as being dsl synthesized.
bool SigRewriter::run(core::MutableContext &ctx, ast::Send *send) {
    if (send->fun != core::Names::sig()) {
        return false;
    }

    if (!(send->recv->isSelfReference() || isTSigWithoutRuntime(send->recv))) {
        return false;
    }

    if (!(send->args.size() == 0 || send->args.size() == 1)) {
        return false;
    }

    if (send->block == nullptr) {
        return false;
    }

    // Keep track of old receiver at this point so that we can report whether a method called
    // sig with the right arity even existed at this point.
    auto oldRecv = std::move(send->recv);
    send->numPosArgs += 1;
    send->recv = ast::MK::Constant(send->loc, core::Symbols::Sorbet_Private_Static());
    send->args.emplace(send->args.begin(), std::move(oldRecv));
    return true;
}

} // namespace sorbet::rewriter
