#include "rewriter/Minitest.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "ast/treemap/treemap.h"
#include "core/Context.h"
#include "core/Names.h"
#include "core/core.h"
#include "core/errors/rewriter.h"
#include "rewriter/rewriter.h"

using namespace std;

namespace sorbet::rewriter {

namespace {
class ConstantMover {
    u4 classDepth = 0;
    vector<ast::TreePtr> movedConstants = {};

public:
    ast::TreePtr createConstAssign(ast::Assign &asgn) {
        auto loc = asgn.loc;
        auto raiseUnimplemented = ast::MK::RaiseUnimplemented(loc);
        if (auto send = ast::cast_tree<ast::Send>(asgn.rhs)) {
            if (send->fun == core::Names::let() && send->args.size() == 2) {
                auto rhs = ast::MK::Let(loc, move(raiseUnimplemented), send->args[1]->deepCopy());
                return ast::MK::Assign(asgn.loc, move(asgn.lhs), move(rhs));
            }
        }

        return ast::MK::Assign(asgn.loc, move(asgn.lhs), move(raiseUnimplemented));
    }

    ast::TreePtr postTransformAssign(core::MutableContext ctx, ast::TreePtr tree) {
        auto *asgn = ast::cast_tree<ast::Assign>(tree);
        if (auto *cnst = ast::cast_tree<ast::UnresolvedConstantLit>(asgn->lhs)) {
            if (ast::isa_tree<ast::UnresolvedConstantLit>(asgn->rhs)) {
                movedConstants.emplace_back(move(tree));
                return ast::MK::EmptyTree();
            }
            auto name = ast::MK::Symbol(cnst->loc, cnst->cnst);

            // if the constant is already in a T.let, preserve it, otherwise decay it to unsafe
            movedConstants.emplace_back(createConstAssign(*asgn));

            auto module = ast::MK::Constant(asgn->loc, core::Symbols::Module());
            return ast::MK::Send2(asgn->loc, move(module), core::Names::constSet(), move(name), move(asgn->rhs));
        }

        return tree;
    }

    // classdefs define new constants, so we always move those if they're the "top-level" classdef (i.e. if we have
    // nested classdefs, we should only move the outermost one)
    ast::TreePtr preTransformClassDef(core::MutableContext ctx, ast::TreePtr classDef) {
        classDepth++;
        return classDef;
    }

    ast::TreePtr postTransformClassDef(core::MutableContext ctx, ast::TreePtr classDef) {
        classDepth--;
        if (classDepth == 0) {
            movedConstants.emplace_back(move(classDef));
            return ast::MK::EmptyTree();
        }
        return classDef;
    }

    // we move sends if they are other minitest `describe` blocks, as those end up being classes anyway: consequently,
    // we treat those the same way we treat classes
    ast::TreePtr preTransformSend(core::MutableContext ctx, ast::TreePtr tree) {
        auto *send = ast::cast_tree<ast::Send>(tree);
        if (send->recv->isSelfReference() && send->args.size() == 1 && send->fun == core::Names::describe()) {
            classDepth++;
        }
        return tree;
    }

    ast::TreePtr postTransformSend(core::MutableContext ctx, ast::TreePtr tree) {
        auto *send = ast::cast_tree<ast::Send>(tree);
        if (send->recv->isSelfReference() && send->args.size() == 1 && send->fun == core::Names::describe()) {
            classDepth--;
            if (classDepth == 0) {
                movedConstants.emplace_back(move(tree));
                return ast::MK::EmptyTree();
            }
        }
        return tree;
    }

    vector<ast::TreePtr> getMovedConstants() {
        return move(movedConstants);
    }

    ast::TreePtr addConstantsToExpression(core::LocOffsets loc, ast::TreePtr expr) {
        auto consts = getMovedConstants();

        if (consts.empty()) {
            return expr;
        } else {
            ast::InsSeq::STATS_store stats;

            for (auto &m : consts) {
                stats.emplace_back(move(m));
            }

            return ast::MK::InsSeq(loc, std::move(stats), move(expr));
        }
    }
};

ast::TreePtr addSigVoid(ast::TreePtr expr) {
    return ast::MK::InsSeq1(expr->loc, ast::MK::SigVoid(expr->loc, ast::MK::Hash0(expr->loc)), std::move(expr));
}
} // namespace

ast::TreePtr recurse(core::MutableContext ctx, ast::TreePtr body);

ast::TreePtr prepareBody(core::MutableContext ctx, ast::TreePtr body) {
    body = recurse(ctx, std::move(body));

    if (auto bodySeq = ast::cast_tree<ast::InsSeq>(body)) {
        for (auto &exp : bodySeq->stats) {
            exp = recurse(ctx, std::move(exp));
        }

        bodySeq->expr = recurse(ctx, std::move(bodySeq->expr));
    }
    return body;
}

string to_s(core::Context ctx, ast::TreePtr &arg) {
    auto argLit = ast::cast_tree<ast::Literal>(arg);
    string argString;
    if (argLit != nullptr) {
        if (argLit->isString(ctx)) {
            return argLit->asString(ctx).show(ctx);
        } else if (argLit->isSymbol(ctx)) {
            return argLit->asSymbol(ctx).show(ctx);
        }
    }
    auto argConstant = ast::cast_tree<ast::UnresolvedConstantLit>(arg);
    if (argConstant != nullptr) {
        return argConstant->cnst.show(ctx);
    }
    return arg->toString(ctx);
}

// This returns `true` for expressions which can be moved from class to method scope without changing their meaning, and
// `false` otherwise. This mostly encompasses literals (arrays, hashes, basic literals), constants, and sends that only
// involve the other things described.
bool canMoveIntoMethodDef(const ast::TreePtr &exp) {
    if (ast::isa_tree<ast::Literal>(exp)) {
        return true;
    } else if (auto *list = ast::cast_tree_const<ast::Array>(exp)) {
        return absl::c_all_of(list->elems, [](auto &elem) { return canMoveIntoMethodDef(elem); });
    } else if (auto *hash = ast::cast_tree_const<ast::Hash>(exp)) {
        return absl::c_all_of(hash->keys, [](auto &elem) { return canMoveIntoMethodDef(elem); }) &&
               absl::c_all_of(hash->values, [](auto &elem) { return canMoveIntoMethodDef(elem); });
    } else if (auto *send = ast::cast_tree_const<ast::Send>(exp)) {
        return canMoveIntoMethodDef(send->recv) &&
               absl::c_all_of(send->args, [](auto &elem) { return canMoveIntoMethodDef(elem); });
    } else if (ast::isa_tree<ast::UnresolvedConstantLit>(exp)) {
        return true;
    }
    return false;
}

// if the thing can be moved into a method def, then the thing we iterate over can be copied into the body of the
// method, and otherwise we replace it with a synthesized 'nil'
ast::TreePtr getIteratee(ast::TreePtr &exp) {
    if (canMoveIntoMethodDef(exp)) {
        return exp->deepCopy();
    } else {
        return ast::MK::RaiseUnimplemented(exp->loc);
    }
}

// this applies to each statement contained within a `test_each`: if it's an `it`-block, then convert it appropriately,
// otherwise flag an error about it
ast::TreePtr runUnderEach(core::MutableContext ctx, core::NameRef eachName, ast::TreePtr stmt,
                          ast::MethodDef::ARGS_store &args, ast::TreePtr &iteratee) {
    // this statement must be a send
    if (auto *send = ast::cast_tree<ast::Send>(stmt)) {
        // the send must be a call to `it` with a single argument (the test name) and a block with no arguments
        if (send->fun == core::Names::it() && send->args.size() == 1 && send->block != nullptr &&
            ast::cast_tree<ast::Block>(send->block)->args.size() == 0) {
            // we use this for the name of our test
            auto argString = to_s(ctx, send->args.front());
            auto name = ctx.state.enterNameUTF8("<it '" + argString + "'>");

            // pull constants out of the block
            ConstantMover constantMover;
            ast::TreePtr body = move(ast::cast_tree<ast::Block>(send->block)->body);
            body = ast::TreeMap::apply(ctx, constantMover, move(body));

            // pull the arg and the iteratee in and synthesize `iterate.each { |arg| body }`
            ast::MethodDef::ARGS_store new_args;
            for (auto &arg : args) {
                new_args.emplace_back(arg->deepCopy());
            }
            auto blk = ast::MK::Block(send->loc, move(body), std::move(new_args));
            auto each = ast::MK::Send0Block(send->loc, iteratee->deepCopy(), core::Names::each(), move(blk));
            // put that into a method def named the appropriate thing
            auto method = addSigVoid(
                ast::MK::SyntheticMethod0(send->loc, core::Loc(ctx.file, send->loc), move(name), move(each)));
            // add back any moved constants
            return constantMover.addConstantsToExpression(send->loc, move(method));
        }
    }
    // if any of the above tests were not satisfied, then mark this statement as being invalid here
    if (auto e = ctx.beginError(stmt->loc, core::errors::Rewriter::BadTestEach)) {
        e.setHeader("Only valid `{}`-blocks can appear within `{}`", "it", eachName.show(ctx));
    }

    return stmt;
}

// this just walks the body of a `test_each` and tries to transform every statement
ast::TreePtr prepareTestEachBody(core::MutableContext ctx, core::NameRef eachName, ast::TreePtr body,
                                 ast::MethodDef::ARGS_store &args, ast::TreePtr &iteratee) {
    auto *bodySeq = ast::cast_tree<ast::InsSeq>(body);
    if (bodySeq) {
        for (auto &exp : bodySeq->stats) {
            exp = runUnderEach(ctx, eachName, std::move(exp), args, iteratee);
        }

        bodySeq->expr = runUnderEach(ctx, eachName, std::move(bodySeq->expr), args, iteratee);
    } else {
        body = runUnderEach(ctx, eachName, std::move(body), args, iteratee);
    }

    return body;
}

ast::TreePtr runSingle(core::MutableContext ctx, ast::Send *send) {
    if (send->block == nullptr) {
        return nullptr;
    }

    auto *block = ast::cast_tree<ast::Block>(send->block);

    if (!send->recv->isSelfReference()) {
        return nullptr;
    }

    if ((send->fun == core::Names::testEach() || send->fun == core::Names::testEachHash()) && send->args.size() == 1) {
        if ((send->fun == core::Names::testEach() && block->args.size() != 1) ||
            (send->fun == core::Names::testEachHash() && block->args.size() != 2)) {
            if (auto e = ctx.beginError(send->block->loc, core::errors::Rewriter::BadTestEach)) {
                e.setHeader("Wrong number of parameters for `{}` block: expected `{}`, got `{}`", send->fun.show(ctx),
                            1, block->args.size());
            }
            return nullptr;
        }
        // if this has the form `test_each(expr) { |arg | .. }`, then start by trying to convert `expr` into a thing we
        // can freely copy into methoddef scope
        auto iteratee = getIteratee(send->args.front());
        // and then reconstruct the send but with a modified body
        ast::Send::ARGS_store args;
        args.emplace_back(move(send->args.front()));
        return ast::MK::Send(
            send->loc, ast::MK::Self(send->loc), send->fun, std::move(args), send->flags,
            ast::MK::Block(send->block->loc,
                           prepareTestEachBody(ctx, send->fun, std::move(block->body), block->args, iteratee),
                           std::move(block->args)));
    }

    if (send->args.empty() && (send->fun == core::Names::before() || send->fun == core::Names::after())) {
        auto name = send->fun == core::Names::after() ? core::Names::afterAngles() : core::Names::initialize();
        ConstantMover constantMover;
        block->body = ast::TreeMap::apply(ctx, constantMover, move(block->body));
        auto method = addSigVoid(ast::MK::SyntheticMethod0(send->loc, core::Loc(ctx.file, send->loc), name,
                                                           prepareBody(ctx, std::move(block->body))));
        return constantMover.addConstantsToExpression(send->loc, move(method));
    }

    if (send->args.size() != 1) {
        return nullptr;
    }
    auto &arg = send->args[0];
    auto argString = to_s(ctx, arg);

    if (send->fun == core::Names::describe()) {
        ast::ClassDef::ANCESTORS_store ancestors;
        ancestors.emplace_back(ast::MK::Self(arg->loc));
        ast::ClassDef::RHS_store rhs;
        rhs.emplace_back(prepareBody(ctx, std::move(block->body)));
        auto name = ast::MK::UnresolvedConstant(arg->loc, ast::MK::EmptyTree(),
                                                ctx.state.enterNameConstant("<describe '" + argString + "'>"));
        return ast::MK::Class(send->loc, core::Loc(ctx.file, send->loc), std::move(name), std::move(ancestors),
                              std::move(rhs));
    } else if (send->fun == core::Names::it()) {
        ConstantMover constantMover;
        block->body = ast::TreeMap::apply(ctx, constantMover, move(block->body));
        auto name = ctx.state.enterNameUTF8("<it '" + argString + "'>");
        auto method = addSigVoid(ast::MK::SyntheticMethod0(send->loc, core::Loc(ctx.file, send->loc), std::move(name),
                                                           prepareBody(ctx, std::move(block->body))));
        method = ast::MK::InsSeq1(send->loc, send->args.front()->deepCopy(), move(method));
        return constantMover.addConstantsToExpression(send->loc, move(method));
    }

    return nullptr;
}

ast::TreePtr recurse(core::MutableContext ctx, ast::TreePtr body) {
    auto bodySend = ast::cast_tree<ast::Send>(body);
    if (bodySend) {
        auto change = runSingle(ctx, bodySend);
        if (change) {
            return change;
        }
    }
    return body;
}

vector<ast::TreePtr> Minitest::run(core::MutableContext ctx, ast::Send *send) {
    vector<ast::TreePtr> stats;
    if (ctx.state.runningUnderAutogen) {
        return stats;
    }

    auto exp = runSingle(ctx, send);
    if (exp != nullptr) {
        stats.emplace_back(std::move(exp));
    }
    return stats;
}

}; // namespace sorbet::rewriter
