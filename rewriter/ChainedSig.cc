#include "rewriter/ChainedSig.h"
#include "ast/Helpers.h"
#include "ast/treemap/treemap.h"
#include "core/errors/infer.h"

using namespace std;
namespace sorbet::rewriter {

struct ChainedSigWalk {
    vector<ast::Send *> incompleteSigs;

    ast::ExpressionPtr postTransformSend(core::MutableContext ctx, ast::ExpressionPtr tree) {
        auto *send = ast::cast_tree<ast::Send>(tree);

        // When we receive a `sig` send, we don't know ahead of time of whether it's a chained sig
        // or an incomplete `sig` without a block. Save it for later, so that we can add errors
        // to sigs missing block parameters
        if (send->fun == core::Names::sig() && send->block == nullptr) {
            incompleteSigs.push_back(send);
            return tree;
        }

        // Return early unless it's one of the sends we are interested in
        if (send->fun != core::Names::abstract() && send->fun != core::Names::final_() &&
            send->fun != core::Names::override_() && send->fun != core::Names::overridable()) {
            return tree;
        }

        if (send->block == nullptr) {
            return tree;
        }

        // Make sure the receiver is sig. E.g.: `sig.abstract {}`
        auto sigSend = ast::cast_tree<ast::Send>(send->recv);

        if (sigSend->fun != core::Names::sig()) {
            return tree;
        }

        // This is the continuation of the previous `sig` send by chaining other methods
        // remove it from the incomplete sigs, so that we don't add errors to it
        if (incompleteSigs.back()->loc == sigSend->loc) {
            incompleteSigs.pop_back();
        }

        ast::Send::ARGS_store args;

        // If the function is final, we need to add it as a positional argument, but
        // we don't need to re-write the block. E.g.: `sig.final {...}` -> `sig(:final) {...}`
        if (send->fun == core::Names::final_()) {
            args.emplace_back(ast::MK::Symbol(sigSend->loc, core::Names::final_()));

            return ast::MK::Send(send->loc, std::move(sigSend->recv), core::Names::sig(), 0, std::move(args), {},
                                 std::move(send->block));
        }

        // For all other cases, we have to re-write the block
        // E.g.: `sig.abstract { void }` -> `sig { abstract.void }`
        auto *block = ast::cast_tree<ast::Block>(send->block);
        auto blockBody = ast::cast_tree<ast::Send>(block->body);
        ast::ExpressionPtr body =
            ast::MK::Send(send->loc, std::move(blockBody->recv), send->fun, send->numPosArgs, std::move(send->args));

        auto newBody = ast::MK::Send(send->loc, std::move(body), blockBody->fun, blockBody->numPosArgs,
                                     std::move(blockBody->args));

        return ast::MK::Send(send->loc, std::move(sigSend->recv), core::Names::sig(), 0, std::move(args), {},
                             ast::MK::Block0(send->block.loc(), std::move(newBody)));
    }

    // For all remaining incompleteSigs, emit an error of missing block parameter
    void emitIncompleteSigErrors(core::MutableContext ctx) {
        for (ast::Send *sig : incompleteSigs) {
            if (auto e = ctx.beginError(sig->loc, core::errors::Infer::BlockNotPassed)) {
                e.setHeader("{} requires a block parameter, but no block was passed", "sig");
            }
        }
    }
};

ast::ExpressionPtr ChainedSig::run(core::MutableContext &ctx, ast::ExpressionPtr tree) {
    ChainedSigWalk walker;
    auto ast = ast::TreeMap::apply(ctx, walker, std::move(tree));

    walker.emitIncompleteSigErrors(ctx);
    return ast;
}
} // namespace sorbet::rewriter
