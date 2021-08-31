#include "rewriter/ChainedSig.h"
#include "ast/Helpers.h"
#include "ast/treemap/treemap.h"
#include "core/errors/infer.h"
#include <iostream>

using namespace std;
namespace sorbet::rewriter {

struct ChainedSigWalk {
    vector<ast::Send *> incompleteSigs;

    ast::ExpressionPtr postTransformSend(core::MutableContext ctx, ast::ExpressionPtr tree) {
        auto *send = ast::cast_tree<ast::Send>(tree);

        // When we receive a `sig` send, we don't know ahead of time of whether it's a chained sig
        // or an incomplete `sig` without a block. Save it for later, so that we can add errors
        // to sigs missing block parameters
        if (send->fun == core::Names::sig()) {
            if (send->block == nullptr) {
                incompleteSigs.push_back(send);
                return tree;
            } else if (ctx.state.ruby3KeywordArgs) {
                if (auto e = ctx.beginError(send->loc, core::errors::Infer::BlockNotPassed)) {
                    e.setHeader("This signature uses the old syntax. Please use the new syntax");

                    auto loc = core::Loc(ctx.file, send->loc);
                    string original = loc.source(ctx).value();
                    auto *block = ast::cast_tree<ast::Block>(send->block);
                    string replacement = buildReplacement(ctx, original, ast::cast_tree<ast::Send>(block->body));

                    e.addAutocorrect(
                        core::AutocorrectSuggestion{fmt::format("Replace `{}` with `{}`", original, replacement),
                                                    {core::AutocorrectSuggestion::Edit{loc, replacement}}});
                }
            }
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

    string buildReplacement(core::MutableContext ctx, string original, ast::Send *sigBody) {
        string result, statementAfter = "}", search = "";

        // Correcting final is easier, since it is outside of the block, we just need to substitute
        // everything up to the first curly brace
        if (original.find(":final") != string::npos) {
            result = original.replace(0, original.find("{") + 1, "sig.final {");
            return result;
        }

        // Find the statement of interest and save the statement after it so that we can have accurate length
        // calculation
        do {
            if (sigBody->fun == core::Names::abstract() || sigBody->fun == core::Names::override_() ||
                sigBody->fun == core::Names::overridable()) {
                search = sigBody->fun.toString(ctx);
            } else {
                statementAfter = sigBody->fun.toString(ctx);
                sigBody = ast::cast_tree<ast::Send>(sigBody->recv);
            }
        } while (sigBody && search.empty());

        unsigned long searchStart = original.find(search);
        unsigned long searchLength = original.find(statementAfter, searchStart) - searchStart;

        result = original.replace(searchStart, searchLength, "");
        result = result.replace(0, result.find("{") + 1, fmt::format("sig.{} {}", search, "{"));

        return result;
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
