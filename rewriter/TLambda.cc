#include "rewriter/TLambda.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "core/Context.h"
#include "core/Names.h"
#include "core/core.h"
#include "core/errors/rewriter.h"
#include "rewriter/rewriter.h"

using namespace std;

namespace sorbet::rewriter {

bool TLambda::run(core::MutableContext ctx, ast::Send *send) {
    if (ctx.state.runningUnderAutogen) {
        return false;
    }

    // Double-check that this method is called `lambda`
    if (send->fun != core::Names::lambda()) {
        return false;
    }

    // ensure that the receiver is `T` or `::T`
    auto recv = ast::cast_tree<ast::UnresolvedConstantLit>(send->recv);
    if (!recv || !ast::MK::isRootScope(recv->scope)) {
        return false;
    }

    if (recv->cnst != core::Names::Constants::T()) {
        return false;
    }

    // ensure that it's got a block at all
    auto block = send->block();
    if (!block) {
        return false;
    }

    // we now know it's got a block; start assembling a fake proc we'll replace it with

    // We've got to do two things with the arguments: first, turn them into positional args for the block arguments;
    // second give each one a `T.let` at the beginning of the proc to give it a type. We'll walk over the kwargs and do both simultaneously.
    ast::InsSeq::STATS_store newBody;
    ast::MethodDef::ARGS_store newArgs;

    // we'll have to loop over the keyword args twice
    for (auto &arg : block->args) {
        auto optArg = ast::cast_tree<ast::OptionalArg>(arg);
        if (!optArg) {
            // TODO: only allow kwargs with defaults! This is an error!
            return false;
        }
        auto kwarg = ast::cast_tree<ast::KeywordArg>(optArg->expr);
        if (!kwarg) {
            return false;
        }

        auto keyword = ast::cast_tree<ast::UnresolvedIdent>(kwarg->expr);
        ENFORCE(keyword);

        // first, make a new positional arg for the block
        newArgs.emplace_back(ast::MK::Local(arg.loc(), keyword->name));

        // Then, add a `T.let` to type the body
        auto asst = ast::MK::Let(arg.loc(), ast::MK::Local(arg.loc(), keyword->name), move(optArg->default_));
        newBody.emplace_back(ast::MK::Assign(arg.loc(), ast::MK::Local(arg.loc(), keyword->name), move(asst)));
    }

    // the new block
    auto newBlock = ast::MK::Block(block->loc, ast::MK::InsSeq(block->loc, std::move(newBody), move(block->body)), std::move(newArgs));

    // pass that to `Proc.new`
    auto procConst = ast::MK::UnresolvedConstant(send->loc, ast::MK::EmptyTree(), core::Names::Constants::Proc());

    // transform the existing send from `T.lambda` into `Proc.new`:
    send->clearArgs();
    send->recv = move(procConst);
    send->fun = core::Names::new_();
    send->setBlock(move(newBlock));

    return true;
}

}; // namespace sorbet::rewriter
