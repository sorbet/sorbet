#include "rewriter/ChainedSig.h"
#include "ast/Helpers.h"
#include "ast/treemap/treemap.h"
#include "core/errors/rewriter.h"

using namespace std;
namespace sorbet::rewriter {

struct ChainedSigWalk {
    // The previousSigSend is used to allow us to add errors to `sig` with missing block without accidentally adding it
    // to a chained sig. E.g.:
    // `sig` -> error no block
    // `sig.final {}` -> no error. Need to prevent adding the error when we receive `sig` by itself
    ast::Send *previousSigSend;

    void preTransformSend(core::MutableContext ctx, ast::ExpressionPtr &tree) {
        auto *send = ast::cast_tree<ast::Send>(tree);
        ast::Send *sigSend = ast::cast_tree<ast::Send>(send->recv);

        // Return early unless the `send` chain begins with a `sig` invocation
        // E.g.: `sig.abstract {}`,`sig.override.final {}`
        if (send->fun != core::Names::sig() && (sigSend == nullptr || !firstSendIsSig(sigSend))) {
            return;
        }

        // Return early if we identify things like `params` being invoked on `sig`
        if (invalidChainedStatement(ctx, send)) {
            return;
        }

        if (send->fun == core::Names::sig() && !send->hasBlock() && send != previousSigSend) {
            if (auto e = ctx.beginError(send->loc, core::errors::Rewriter::InvalidChainedSig)) {
                e.setHeader("Signature declarations expect a block");
                e.addErrorNote("Complete the signature by adding a block declaration: sig `{}`", "{ ... }");
            }

            return;
        }

        // Return early unless it's one of the sends we are interested in
        if (send->fun != core::Names::abstract() && send->fun != core::Names::final_() &&
            send->fun != core::Names::override_() && send->fun != core::Names::overridable()) {
            return;
        }

        if (!send->hasBlock()) {
            if (auto e = ctx.beginError(send->funLoc, core::errors::Rewriter::InvalidChainedSig)) {
                e.setHeader("Signature declarations expect a block");
                e.addErrorNote("Complete the signature by adding a block declaration: sig `{}`", "{ ... }");
            }

            return;
        }

        // For all other cases, we have to re-write the block
        // E.g.: `sig.abstract { void }` -> `sig { abstract.void }`
        auto *block = send->block();
        auto blockBody = ast::cast_tree<ast::Send>(block->body);

        // If the blockBody is not a send, then we have a sequence of expressions inside the signature block
        if (blockBody == nullptr) {
            if (auto e = ctx.beginError(block->loc, core::errors::Rewriter::InvalidChainedSig)) {
                e.setHeader("Malformed signature: cannot have multiple statements inside a signature block");
            }

            return;
        }

        tree = buildReplacement(ctx, send, blockBody);
    }

    bool firstSendIsSig(ast::Send *send) {
        do {
            if (send->fun == core::Names::sig()) {
                this->previousSigSend = send;
                return true;
            }
            send = ast::cast_tree<ast::Send>(send->recv);
        } while (send);

        return false;
    }

    void checkDuplicates(core::MutableContext ctx, ast::Send *sendFromSigBlock, core::NameRef sendCopyFun,
                         core::LocOffsets sendCopyFunLoc) {
        while (sendFromSigBlock) {
            if (sendFromSigBlock->fun == sendCopyFun) {
                if (auto e = ctx.beginError(sendFromSigBlock->loc, core::errors::Rewriter::InvalidChainedSig)) {
                    e.setHeader("Duplicate invocation of `{}` in signature declaration", sendCopyFun.toString(ctx));
                    e.addErrorLine(ctx.locAt(sendCopyFunLoc), "Initial invocation made here");
                }
            }

            sendFromSigBlock = ast::cast_tree<ast::Send>(sendFromSigBlock->recv);
        }
    }

    bool invalidChainedStatement(core::MutableContext ctx, ast::Send *send) {
        if (send->fun == core::Names::params() || send->fun == core::Names::returns() ||
            send->fun == core::Names::void_() || send->fun == core::Names::bind() ||
            send->fun == core::Names::checked() || send->fun == core::Names::onFailure() ||
            send->fun == core::Names::typeParameters()) {
            if (auto e = ctx.beginError(send->funLoc, core::errors::Rewriter::InvalidChainedSig)) {
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
        auto sendCopyIter = sendCopy;
        while (sendCopyIter && sendCopyIter->fun != core::Names::sig()) {
            checkDuplicates(ctx, blockBody, sendCopyIter->fun, sendCopyIter->funLoc);

            sendCopyIter = ast::cast_tree<ast::Send>(sendCopyIter->recv);
        }

        bool isFinal = false;

        // Create a new receiver for the block to move the statements chained on sig inside. Also, finds out whether the
        // signature is final or not
        ast::ExpressionPtr newBlockReceiver = std::move(blockBody->recv);
        do {
            if (sendCopy->fun != core::Names::final_()) {
                if (sendCopy->hasBlock()) {
                    // Drop the block argument before moving the arguments.
                    sendCopy->setBlock(nullptr);
                }
                newBlockReceiver = sendCopy->withNewBody(sendCopy->loc, std::move(newBlockReceiver), sendCopy->fun);
            } else {
                isFinal = true;
            }

            sendCopy = ast::cast_tree<ast::Send>(sendCopy->recv);

            // If a previous receiver has a block, then we need to add an error to prevent signatures like this:
            // `sig.final {}.override{}
            if (sendCopy->hasBlock()) {
                if (auto e = ctx.beginError(send->funLoc, core::errors::Rewriter::InvalidChainedSig)) {
                    e.setHeader("Cannot add more signature statements after the declaration block");
                    e.addErrorLine(ctx.locAt(sendCopy->block()->loc), "Initial statement defined here");
                }
            }
        } while (sendCopy && sendCopy->fun != core::Names::sig());

        // If the signature is final, we need the `:final` positional argument
        if (isFinal) {
            args.emplace_back(ast::MK::Symbol(sendCopy->loc, core::Names::final_()));
        }

        // Create the new body, composed of the chained statements that were moved inside and the original block
        auto newBody = blockBody->withNewBody(send->loc, std::move(newBlockReceiver), blockBody->fun);
        ast::cast_tree_nonnull<ast::Send>(newBody).setBlock(nullptr);

        auto rv = ast::MK::Send(send->loc, std::move(sendCopy->recv), core::Names::sig(), send->funLoc, args.size(),
                                std::move(args));
        ast::cast_tree_nonnull<ast::Send>(rv).setBlock(ast::MK::Block0(send->block()->loc, std::move(newBody)));
        return rv;
    }
};

ast::ExpressionPtr ChainedSig::run(core::MutableContext &ctx, ast::ExpressionPtr tree) {
    ChainedSigWalk walker;
    ast::TreeWalk::apply(ctx, walker, tree);
    return tree;
}
} // namespace sorbet::rewriter
