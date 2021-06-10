#include "rewriter/ChainedSig.h"
#include "ast/Helpers.h"
#include "ast/treemap/treemap.h"
#include "core/errors/rewriter.h"

using namespace std;
namespace sorbet::rewriter {

struct ChainedSigWalk {
    vector<ast::Send *> incompleteSigs;

    ast::ExpressionPtr postTransformSend(core::MutableContext ctx, ast::ExpressionPtr tree) {
        auto *send = ast::cast_tree<ast::Send>(tree);
        ast::Send *sigSend;

        // Emit an error if using invalid type syntax in a chained sig context e.g.: sig.params(...) {}
        if (send->fun == core::Names::params() || send->fun == core::Names::returns() ||
            send->fun == core::Names::void_() || send->fun == core::Names::bind() ||
            send->fun == core::Names::checked() || send->fun == core::Names::onFailure() ||
            send->fun == core::Names::typeParameters()) {
            sigSend = ast::cast_tree<ast::Send>(send->recv);

            if (sigSend != nullptr && sigSend->fun == core::Names::sig()) {
                if (auto e = ctx.beginError(send->loc, core::errors::Rewriter::InvalidChainedSig)) {
                    e.setHeader("Cannot use `{}` outside of a sig block", send->fun.toString(ctx));
                }

                // We already know this signature is invalid, so no need to emit two errors for it
                if (incompleteSigs.back()->loc == sigSend->loc) {
                    incompleteSigs.pop_back();
                }
            }

            return tree;
        }

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

        sigSend = ast::cast_tree<ast::Send>(send->recv);

        // Make sure the first receiver is sig. E.g.: `sig.abstract {}`, `sig.override.final {}`
        if (sigSend == nullptr || !firstReceiverIsSig(sigSend)) {
            return tree;
        }

        // This is still an invocation where `sig` is the first receiver, but there is still no block. We need to pop
        // the last entry for incomplete sigs and add the current send to account for multiple methods chained on sig
        //
        // E.g.: `sig.override.final {}`
        //
        // If we never find a send where the first receiver is a `sig` and the last invocation has the declaration
        // block, then we add the error later on
        if (send->block == nullptr) {
            if (incompleteSigs.back()->loc == sigSend->loc) {
                incompleteSigs.pop_back();
                incompleteSigs.push_back(send);
            }

            // Add an error if the previous part of the signature already declared a block
            // E.g.: sig.final {}.override
            if (sigSend->block != nullptr) {
                if (auto e = ctx.beginError(send->loc, core::errors::Rewriter::InvalidChainedSig)) {
                    e.setHeader("Cannot add more signature statements after the declaration block");
                }
            }

            return tree;
        }

        // If someone writes something like `sig.final {}.override {}`, then we would try to pop incomplete sigs twice
        if (incompleteSigs.empty()) {
            if (auto e = ctx.beginError(send->loc, core::errors::Rewriter::InvalidChainedSig)) {
                e.setHeader("Cannot chain two blocks in a single `{}`", "sig");
            }

            return tree;
        }

        // This is the continuation of the previous `sig` send by chaining other methods
        // remove it from the incomplete sigs, so that we don't add errors to it
        if (incompleteSigs.back()->loc == sigSend->loc) {
            incompleteSigs.pop_back();
        }

        ast::Send::ARGS_store args;

        // For all other cases, we have to re-write the block
        // E.g.: `sig.abstract { void }` -> `sig { abstract.void }`
        auto *block = ast::cast_tree<ast::Block>(send->block);
        auto blockBody = ast::cast_tree<ast::Send>(block->body);
        ast::ExpressionPtr newBlockReceiver;

        // If the blockBody is not a send, then we have a sequence of instructions inside the signature block
        if (blockBody == nullptr) {
            if (auto e = ctx.beginError(block->loc, core::errors::Rewriter::InvalidChainedSig)) {
                e.setHeader("Malformed signature: cannot have multiple instructions inside a signature block");
            }

            return tree;
        }

        auto treeCopy = send->deepCopy();
        auto sendCopy = ast::cast_tree<ast::Send>(treeCopy);

        // Go through each part of the chained sig and make sure it's not duplicated inside the block
        // E.g.: `sig.abstract { abstract.void }`
        while (sendCopy && sendCopy->fun != core::Names::sig()) {
            checkDuplicates(ctx, blockBody, sendCopy->fun);

            sendCopy = ast::cast_tree<ast::Send>(sendCopy->recv);
        }

        sendCopy = ast::cast_tree<ast::Send>(treeCopy);
        bool isFinal = false;

        // Create a new receiver for the block to move the statements chained on sig inside. Also, finds out whether the
        // signature is final or not
        newBlockReceiver = std::move(blockBody->recv);
        do {
            if (sendCopy->fun != core::Names::final_()) {
                newBlockReceiver = ast::MK::Send(sendCopy->loc, std::move(newBlockReceiver), sendCopy->fun,
                                                 sendCopy->numPosArgs, std::move(sendCopy->args));
            } else {
                isFinal = true;
            }

            sendCopy = ast::cast_tree<ast::Send>(sendCopy->recv);
        } while (sendCopy && sendCopy->fun != core::Names::sig());

        // If the signature is final, we need the `:final` positional argument
        if (isFinal) {
            args.emplace_back(ast::MK::Symbol(sendCopy->loc, core::Names::final_()));
        }

        // Create the new body, composed of the chained statements that were moved inside and the original block
        auto newBody = ast::MK::Send(send->loc, std::move(newBlockReceiver), blockBody->fun, blockBody->numPosArgs,
                                     std::move(blockBody->args));

        return ast::MK::Send(send->loc, std::move(sendCopy->recv), core::Names::sig(), args.size(), std::move(args), {},
                             ast::MK::Block0(send->block.loc(), std::move(newBody)));
    }

    bool firstReceiverIsSig(ast::Send *send) {
        do {
            if (send->fun == core::Names::sig()) {
                return true;
            }
            send = ast::cast_tree<ast::Send>(send->recv);
        } while (send);

        return false;
    }

    // For all remaining incompleteSigs, emit an error of missing block parameter
    void emitIncompleteSigErrors(core::MutableContext ctx) {
        for (ast::Send *sig : incompleteSigs) {
            if (auto e = ctx.beginError(sig->loc, core::errors::Rewriter::InvalidChainedSig)) {
                e.setHeader("Signature declarations expect a block");
                e.addErrorNote("Complete the signature by adding a block declaration: sig `{}`", "{ ... }");
            }
        }
    }

    void checkDuplicates(core::MutableContext ctx, ast::Send *sigBlock, core::NameRef fun) {
        auto expr = sigBlock->deepCopy();
        ast::Send *sigStatement = ast::cast_tree<ast::Send>(expr);

        while (sigStatement) {
            if (sigStatement->fun == fun) {
                if (auto e = ctx.beginError(sigStatement->loc, core::errors::Rewriter::InvalidChainedSig)) {
                    e.setHeader("Duplicate invocation of `{}` in signature declaration", fun.toString(ctx));
                }
            }

            sigStatement = ast::cast_tree<ast::Send>(sigStatement->recv);
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
