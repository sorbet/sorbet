#include "rewriter/SigRewriter.h"
#include "ast/Helpers.h"
#include "ast/treemap/treemap.h"

using namespace std;
namespace sorbet::rewriter {

// Rewrite all sig usage into explicit uses of `T::Sig::WithoutRuntime.sig`, and
// mark them as being dsl synthesized.
void SigRewriter::run(core::MutableContext &ctx, ast::ClassDef *klass) {
    for (auto &rhs : klass->rhs) {
        if (auto send = ast::cast_tree<ast::Send>(rhs.get())) {
            if (send->fun == core::Names::sig() && send->recv->isSelfReference()) {
                send->recv = ast::MK::Constant(send->loc, core::Symbols::T_Sig_WithoutRuntime());
                send->flags.isRewriterSynthesized = true;
            }
        }
    }
}

} // namespace sorbet::rewriter
