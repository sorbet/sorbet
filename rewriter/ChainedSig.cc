#include "rewriter/ChainedSig.h"
#include "ast/Helpers.h"
#include "ast/treemap/treemap.h"
#include "core/errors/rewriter.h"

using namespace std;
namespace sorbet::rewriter {

struct ChainedSigWalk {
    // The previousSigSend is used to allow us to add errors to `sig` with missing block wihtout accidentally adding it
    // to a chained sig. E.g.:
    // `sig` -> error no block
    // `sig.final {}` -> no error. Need to prevent adding the error when we receive `sig` by itself
    ast::Send *previousSigSend;

    ast::ExpressionPtr preTransformSend(core::MutableContext ctx, ast::ExpressionPtr tree) {
        auto *send = ast::cast_tree<ast::Send>(tree);
        ast::Send *sigSend = ast::cast_tree<ast::Send>(send->recv);

        // Make sure the first receiver is sig. E.g.: `sig.abstract {}`, `sig.override.final {}`
        if (send->fun != core::Names::sig() && (sigSend == nullptr || !firstReceiverIsSig(sigSend))) {
            return tree;
        }

        // Return early if we identify things like `params` being invoked on `sig`
        if (invalidChainedStatement(ctx, send)) {
            return tree;
        }

        if (send->fun == core::Names::sig() && !send->hasBlock() && send != previousSigSend) {
            if (auto e = ctx.beginError(send->loc, core::errors::Rewriter::InvalidChainedSig)) {
                e.setHeader("Signature declarations expect a block");
                e.addErrorNote("Complete the signature by adding a block declaration: sig `{}`", "{ ... }");
            }

            return tree;
        }

        // Return early unless it's one of the sends we are interested in
        if (send->fun != core::Names::abstract() && send->fun != core::Names::final_() &&
            send->fun != core::Names::override_() && send->fun != core::Names::overridable()) {
            return tree;
        }

        // This is still an invocation where `sig` is the first receiver, but there is still no block. We need to pop
        // the last entry for incomplete sigs and add the current send to account for multiple methods chained on sig
        //
        // E.g.: `sig.override.final {}`
        //
        // If we never find a send where the first receiver is a `sig` and the last invocation has the declaration
        // block, then we add the error later on
        if (!send->hasBlock()) {
            if (auto e = ctx.beginError(send->loc, core::errors::Rewriter::InvalidChainedSig)) {
                e.setHeader("Signature declarations expect a block");
                e.addErrorNote("Complete the signature by adding a block declaration: sig `{}`", "{ ... }");
            }

            return tree;
        }

        // For all other cases, we have to re-write the block
        // E.g.: `sig.abstract { void }` -> `sig { abstract.void }`
        auto *block = send->block();
        auto blockBody = ast::cast_tree<ast::Send>(block->body);

        // If the blockBody is not a send, then we have a sequence of instructions inside the signature block
        if (blockBody == nullptr) {
            if (auto e = ctx.beginError(block->loc, core::errors::Rewriter::InvalidChainedSig)) {
                e.setHeader("Malformed signature: cannot have multiple instructions inside a signature block");
            }

            return tree;
        }

        return buildReplacement(ctx, send, blockBody);
    }

    bool firstReceiverIsSig(ast::Send *send) {
        do {
            if (send->fun == core::Names::sig()) {
                this->previousSigSend = send;
                return true;
            }
            send = ast::cast_tree<ast::Send>(send->recv);
        } while (send);

        return false;
    }

    void checkDuplicates(core::MutableContext ctx, ast::Send *sigBlock, core::NameRef fun) {
        while (sigBlock) {
            if (sigBlock->fun == fun) {
                if (auto e = ctx.beginError(sigBlock->loc, core::errors::Rewriter::InvalidChainedSig)) {
                    e.setHeader("Duplicate invocation of `{}` in signature declaration", fun.toString(ctx));
                }
            }

            sigBlock = ast::cast_tree<ast::Send>(sigBlock->recv);
        }
    }

    bool invalidChainedStatement(core::MutableContext ctx, ast::Send *send) {
        if (send->fun == core::Names::params() || send->fun == core::Names::returns() ||
            send->fun == core::Names::void_() || send->fun == core::Names::bind() ||
            send->fun == core::Names::checked() || send->fun == core::Names::onFailure() ||
            send->fun == core::Names::typeParameters()) {
            if (auto e = ctx.beginError(send->loc, core::errors::Rewriter::InvalidChainedSig)) {
                e.setHeader("Cannot use `{}` outside of a sig block", send->fun.toString(ctx));
            }

            return true;
        }

        return false;
    }

    ast::ExpressionPtr buildReplacement(core::MutableContext ctx, ast::Send *send, ast::Send *blockBody) {
        ast::Send::ARGS_store args;
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
        ast::ExpressionPtr newBlockReceiver = std::move(blockBody->recv);
        do {
            if (sendCopy->fun != core::Names::final_()) {
                if (sendCopy->hasBlock()) {
                    // Drop the block argument before moving the arguments.
                    sendCopy->args.pop_back();
                }
                newBlockReceiver = ast::MK::Send(sendCopy->loc, std::move(newBlockReceiver), sendCopy->fun,
                                                 sendCopy->numPosArgs, std::move(sendCopy->args));
            } else {
                isFinal = true;
            }

            sendCopy = ast::cast_tree<ast::Send>(sendCopy->recv);

            // If a previous receiver has a block, then we need to add an error to prevent signatures like this:
            // `sig.final {}.override{}
            if (sendCopy->hasBlock()) {
                if (auto e = ctx.beginError(send->loc, core::errors::Rewriter::InvalidChainedSig)) {
                    e.setHeader("Cannot add more signature statements after the declaration block");
                }
            }
        } while (sendCopy && sendCopy->fun != core::Names::sig());

        // If the signature is final, we need the `:final` positional argument
        if (isFinal) {
            args.emplace_back(ast::MK::Symbol(sendCopy->loc, core::Names::final_()));
        }

        // Create the new body, composed of the chained statements that were moved inside and the original block
        auto newBody = ast::MK::Send(send->loc, std::move(newBlockReceiver), blockBody->fun, blockBody->numPosArgs,
                                     std::move(blockBody->args));

        u2 numPosArgs = args.size();
        args.emplace_back(ast::MK::Block0(send->block()->loc, std::move(newBody)));

        ast::Send::Flags flags;
        flags.hasBlock = true;

        return ast::MK::Send(send->loc, std::move(sendCopy->recv), core::Names::sig(), numPosArgs, std::move(args),
                             flags);
    }
};

ast::ExpressionPtr ChainedSig::run(core::MutableContext &ctx, ast::ExpressionPtr tree) {
    ChainedSigWalk walker;
    return ast::TreeMap::apply(ctx, walker, std::move(tree));
}
} // namespace sorbet::rewriter
