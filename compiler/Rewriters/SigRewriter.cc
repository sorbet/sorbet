#include "compiler/Rewriters/SigRewriter.h"
#include "ast/Helpers.h"
#include "ast/treemap/treemap.h"
#include "compiler/Names/Names.h"

using namespace std;
namespace sorbet::compiler {

class SigRewriterWalker {
    friend class SigRewriter;

public:
    unique_ptr<ast::Expression> postTransformSend(core::MutableContext ctx, unique_ptr<ast::Send> send) {
        if (send->fun != core::Names::sig()) {
            return send;
        }
        if (!send->recv->isSelfReference()) {
            return send;
        }

        auto newSigExpr = ast::MK::Send0(send->loc, ast::MK::Constant(send->loc, core::Symbols::T_Sig_WithoutRuntime()),
                                         core::Names::sig());
        auto newSig = ast::cast_tree<ast::Send>(newSigExpr.get());
        newSig->block = move(send->block);
        newSig->flags.isRewriterSynthesized = true;
        return newSigExpr;
    }

private:
    SigRewriterWalker() = default;
};

void SigRewriter::run(core::MutableContext &ctx, ast::ClassDef *klass) {
    SigRewriterWalker sigRewriterWalker;
    unique_ptr<ast::ClassDef> uniqueClass(klass);
    auto ret = ast::TreeMap::apply(ctx, sigRewriterWalker, std::move(uniqueClass));
    klass = static_cast<ast::ClassDef *>(ret.release());
    ENFORCE(klass);
}

} // namespace sorbet::compiler
