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
    uint32_t classDepth = 0;
    vector<ast::ExpressionPtr> movedConstants = {};

public:
    ast::ExpressionPtr createConstAssign(ast::Assign &asgn) {
        auto loc = asgn.loc;
        auto raiseUnimplemented = ast::MK::RaiseUnimplemented(loc);
        if (auto send = ast::cast_tree<ast::Send>(asgn.rhs)) {
            if (send->fun == core::Names::let() && send->numPosArgs() == 2) {
                auto rhs = ast::MK::Let(loc, move(raiseUnimplemented), send->getPosArg(1).deepCopy());
                return ast::MK::Assign(asgn.loc, move(asgn.lhs), move(rhs));
            }
        }

        return ast::MK::Assign(asgn.loc, move(asgn.lhs), move(raiseUnimplemented));
    }

    ast::ExpressionPtr postTransformAssign(core::MutableContext ctx, ast::ExpressionPtr tree) {
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
            return ast::MK::Send2(asgn->loc, move(module), core::Names::constSet(), asgn->loc.copyWithZeroLength(),
                                  move(name), move(asgn->rhs));
        }

        return tree;
    }

    // classdefs define new constants, so we always move those if they're the "top-level" classdef (i.e. if we have
    // nested classdefs, we should only move the outermost one)
    ast::ExpressionPtr preTransformClassDef(core::MutableContext ctx, ast::ExpressionPtr classDef) {
        classDepth++;
        return classDef;
    }

    ast::ExpressionPtr postTransformClassDef(core::MutableContext ctx, ast::ExpressionPtr classDef) {
        classDepth--;
        if (classDepth == 0) {
            movedConstants.emplace_back(move(classDef));
            return ast::MK::EmptyTree();
        }
        return classDef;
    }

    // we move sends if they are other minitest `describe` blocks, as those end up being classes anyway: consequently,
    // we treat those the same way we treat classes
    ast::ExpressionPtr preTransformSend(core::MutableContext ctx, ast::ExpressionPtr tree) {
        auto *send = ast::cast_tree<ast::Send>(tree);
        if (send->recv.isSelfReference() && send->numPosArgs() == 1 && send->fun == core::Names::describe()) {
            classDepth++;
        }
        return tree;
    }

    ast::ExpressionPtr postTransformSend(core::MutableContext ctx, ast::ExpressionPtr tree) {
        auto *send = ast::cast_tree<ast::Send>(tree);
        if (send->recv.isSelfReference() && send->numPosArgs() == 1 && send->fun == core::Names::describe()) {
            classDepth--;
            if (classDepth == 0) {
                movedConstants.emplace_back(move(tree));
                return ast::MK::EmptyTree();
            }
        }
        return tree;
    }

    vector<ast::ExpressionPtr> getMovedConstants() {
        return move(movedConstants);
    }

    ast::ExpressionPtr addConstantsToExpression(core::LocOffsets loc, ast::ExpressionPtr expr) {
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

ast::ExpressionPtr addSigVoid(ast::ExpressionPtr expr) {
    return ast::MK::InsSeq1(expr.loc(), ast::MK::SigVoid(expr.loc(), {}), std::move(expr));
}
} // namespace

enum class RunSingleLevel {
    Toplevel,
    Interior,
};

ast::ExpressionPtr recurse(core::MutableContext ctx, bool isClass, ast::ExpressionPtr body);

ast::ExpressionPtr prepareBody(core::MutableContext ctx, bool isClass, ast::ExpressionPtr body) {
    body = recurse(ctx, isClass, std::move(body));

    if (auto bodySeq = ast::cast_tree<ast::InsSeq>(body)) {
        for (auto &exp : bodySeq->stats) {
            exp = recurse(ctx, isClass, std::move(exp));
        }

        bodySeq->expr = recurse(ctx, isClass, std::move(bodySeq->expr));
    }
    return body;
}

string to_s(core::Context ctx, ast::ExpressionPtr &arg) {
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
    return arg.toString(ctx);
}

// This returns `true` for expressions which can be moved from class to method scope without changing their meaning, and
// `false` otherwise. This mostly encompasses literals (arrays, hashes, basic literals), constants, and sends that only
// involve the other things described.
bool canMoveIntoMethodDef(const ast::ExpressionPtr &exp) {
    if (ast::isa_tree<ast::Literal>(exp)) {
        return true;
    } else if (auto *list = ast::cast_tree<ast::Array>(exp)) {
        return absl::c_all_of(list->elems, [](auto &elem) { return canMoveIntoMethodDef(elem); });
    } else if (auto *hash = ast::cast_tree<ast::Hash>(exp)) {
        return absl::c_all_of(hash->keys, [](auto &elem) { return canMoveIntoMethodDef(elem); }) &&
               absl::c_all_of(hash->values, [](auto &elem) { return canMoveIntoMethodDef(elem); });
    } else if (auto *send = ast::cast_tree<ast::Send>(exp)) {
        if (!canMoveIntoMethodDef(send->recv)) {
            return false;
        }
        for (auto &arg : send->nonBlockArgs()) {
            if (!canMoveIntoMethodDef(arg)) {
                return false;
            }
        }

        return true;
    } else if (ast::isa_tree<ast::UnresolvedConstantLit>(exp)) {
        return true;
    }
    return false;
}

// if the thing can be moved into a method def, then the thing we iterate over can be copied into the body of the
// method, and otherwise we replace it with a synthesized 'nil'
ast::ExpressionPtr getIteratee(ast::ExpressionPtr &exp) {
    if (canMoveIntoMethodDef(exp)) {
        return exp.deepCopy();
    } else {
        return ast::MK::RaiseUnimplemented(exp.loc());
    }
}

// this applies to each statement contained within a `test_each`: if it's an `it`-block, then convert it appropriately,
// otherwise flag an error about it
ast::ExpressionPtr runUnderEach(core::MutableContext ctx, core::NameRef eachName,
                                const ast::InsSeq::STATS_store &destructuringStmts, ast::ExpressionPtr stmt,
                                ast::MethodDef::ARGS_store &args, ast::ExpressionPtr &iteratee) {
    // this statement must be a send
    if (auto *send = ast::cast_tree<ast::Send>(stmt)) {
        // the send must be a call to `it` with a single argument (the test name) and a block with no arguments
        if (send->fun == core::Names::it() && send->numPosArgs() == 1 && send->hasBlock() &&
            send->block()->args.size() == 0) {
            // we use this for the name of our test
            auto argString = to_s(ctx, send->getPosArg(0));
            auto name = ctx.state.enterNameUTF8("<it '" + argString + "'>");

            // pull constants out of the block
            ConstantMover constantMover;
            ast::ExpressionPtr body = move(send->block()->body);
            body = ast::TreeMap::apply(ctx, constantMover, move(body));

            // pull the arg and the iteratee in and synthesize `iterate.each { |arg| body }`
            ast::MethodDef::ARGS_store new_args;
            for (auto &arg : args) {
                new_args.emplace_back(arg.deepCopy());
            }

            // add the destructuring statements to the block if they're present
            if (!destructuringStmts.empty()) {
                ast::InsSeq::STATS_store stmts;
                for (auto &stmt : destructuringStmts) {
                    stmts.emplace_back(stmt.deepCopy());
                }
                body = ast::MK::InsSeq(body.loc(), std::move(stmts), std::move(body));
            }

            auto blk = ast::MK::Block(send->loc, move(body), std::move(new_args));
            auto each = ast::MK::Send0Block(send->loc, iteratee.deepCopy(), core::Names::each(),
                                            send->loc.copyWithZeroLength(), move(blk));
            // put that into a method def named the appropriate thing
            auto method = addSigVoid(ast::MK::SyntheticMethod0(send->loc, send->loc, move(name), move(each)));
            // add back any moved constants
            return constantMover.addConstantsToExpression(send->loc, move(method));
        }
    }

    // if any of the above tests were not satisfied, then mark this statement as being invalid here
    if (auto e = ctx.beginError(stmt.loc(), core::errors::Rewriter::BadTestEach)) {
        e.setHeader("Only valid `{}`-blocks can appear within `{}`", "it", eachName.show(ctx));
    }

    return stmt;
}

bool isDestructuringArg(core::GlobalState &gs, const ast::MethodDef::ARGS_store &args, const ast::ExpressionPtr &expr) {
    auto *local = ast::cast_tree<ast::UnresolvedIdent>(expr);
    if (local == nullptr || local->kind != ast::UnresolvedIdent::Kind::Local) {
        return false;
    }

    auto name = local->name;
    if (name.kind() != core::NameKind::UNIQUE || name.dataUnique(gs)->original != core::Names::destructureArg()) {
        return false;
    }

    return absl::c_find_if(args, [name](auto &argExpr) {
               auto *arg = ast::cast_tree<ast::UnresolvedIdent>(argExpr);
               return arg && arg->name == name;
           }) != args.end();
}

// When given code that looks like
//
//     test_each(pairs) do |(x, y)|
//       # ...
//     end
//
// Sorbet desugars it to essentially
//
//     test_each(pairs) do |<destructureArg$1|
//       x = <destructureArg$1>[0]
//       y = <destructureArg$1>[1]
//
//       # ...
//     end
//
// which would otherwise defeat the "Only valid it-blocks can appear within test_each" error message.
//
// Because this case is so common, we have special handling to detect "contains only valid it-blocks
// plus desugared destruturing assignments."
bool isDestructuringInsSeq(core::GlobalState &gs, const ast::MethodDef::ARGS_store &args, ast::InsSeq *body) {
    return absl::c_all_of(body->stats, [&gs, &args](auto &stat) {
        auto *insSeq = ast::cast_tree<ast::InsSeq>(stat);
        if (insSeq == nullptr) {
            return false;
        }

        auto *assign = ast::cast_tree<ast::Assign>(insSeq->stats.front());
        return assign && isDestructuringArg(gs, args, assign->rhs);
    });
}

// this just walks the body of a `test_each` and tries to transform every statement
ast::ExpressionPtr prepareTestEachBody(core::MutableContext ctx, core::NameRef eachName, ast::ExpressionPtr body,
                                       ast::MethodDef::ARGS_store &args, ast::InsSeq::STATS_store destructuringStmts,
                                       ast::ExpressionPtr &iteratee) {
    if (auto *bodySeq = ast::cast_tree<ast::InsSeq>(body)) {
        if (isDestructuringInsSeq(ctx, args, bodySeq)) {
            ENFORCE(destructuringStmts.empty(), "Nested destructuring statements");
            destructuringStmts.reserve(bodySeq->stats.size());
            std::move(bodySeq->stats.begin(), bodySeq->stats.end(), std::back_inserter(destructuringStmts));
            return prepareTestEachBody(ctx, eachName, std::move(bodySeq->expr), args, std::move(destructuringStmts),
                                       iteratee);
        }

        for (auto &exp : bodySeq->stats) {
            exp = runUnderEach(ctx, eachName, destructuringStmts, std::move(exp), args, iteratee);
        }

        bodySeq->expr = runUnderEach(ctx, eachName, destructuringStmts, std::move(bodySeq->expr), args, iteratee);
    } else {
        body = runUnderEach(ctx, eachName, destructuringStmts, std::move(body), args, iteratee);
    }

    return body;
}

ast::ExpressionPtr runSingle(core::MutableContext ctx, bool isClass, ast::Send *send, RunSingleLevel level) {
    if (!send->hasBlock()) {
        return nullptr;
    }

    auto *block = send->block();

    if (!send->recv.isSelfReference()) {
        // It's pretty common for people new (or old!) to Sorbet to use `each` in tests
        // and be extremely puzzled that Sorbet produces errors about their tests,
        // especially because the test works at runtime.  Try to detect when people are
        // using `each` and hint that they should be doing something else.
        //
        // We only check for this when we're recursing into describe blocks, because
        // that's usually where the problems come up.
        if (level == RunSingleLevel::Interior && send->fun == core::Names::each() && send->numNonBlockArgs() == 0 &&
            !block->args.empty()) {
            auto &blockBody = block->body;
            ast::Send *blockSend = nullptr;
            if (auto *seq = ast::cast_tree<ast::InsSeq>(blockBody)) {
                blockSend = ast::cast_tree<ast::Send>(seq->expr);
            } else {
                blockSend = ast::cast_tree<ast::Send>(blockBody);
            }
            if (blockSend != nullptr && blockSend->fun == core::Names::it()) {
                ENFORCE(!send->argsLoc().exists());
                auto sendLoc = send->recv.loc().join(send->funLoc);
                if (auto e = ctx.beginError(sendLoc, core::errors::Rewriter::UseTestEachNotEach)) {
                    e.setHeader("`{}` cannot be used to write table-driven tests with Sorbet", "each");
                    e.replaceWith("Use `test_each`", ctx.locAt(sendLoc), "test_each({})", send->recv.toString(ctx));

                    ENFORCE(send->numNonBlockArgs() == 0);
                    // Drop the block body so the user doesn't get mysterious errors about
                    // certain functions not existing on our synthesized classes for
                    // describe blocks.
                    return ast::MK::Send(
                        send->loc, send->recv.deepCopy(), send->fun, send->funLoc, send->numNonBlockArgs(),
                        ast::MK::SendArgs(ast::MK::Block(block->loc, ast::MK::EmptyTree(), std::move(block->args))),
                        send->flags);
                }
            }
        }
        return nullptr;
    }

    if ((send->fun == core::Names::testEach() || send->fun == core::Names::testEachHash()) && send->numPosArgs() == 1) {
        if ((send->fun == core::Names::testEach() && block->args.size() < 1) ||
            (send->fun == core::Names::testEachHash() && block->args.size() != 2)) {
            if (auto e = ctx.beginError(block->loc, core::errors::Rewriter::BadTestEach)) {
                e.setHeader("Wrong number of parameters for `{}` block: expected `{}`, got `{}`", send->fun.show(ctx),
                            send->fun == core::Names::testEach() ? "at least 1" : "2", block->args.size());
            }
            return nullptr;
        }
        // if this has the form `test_each(expr) { |arg | .. }`, then start by trying to convert `expr` into a thing we
        // can freely copy into methoddef scope
        auto iteratee = getIteratee(send->getPosArg(0));
        // and then reconstruct the send but with a modified body
        return ast::MK::Send(
            send->loc, ast::MK::Self(send->loc), send->fun, send->funLoc, 1,
            ast::MK::SendArgs(
                move(send->getPosArg(0)),
                ast::MK::Block(block->loc,
                               prepareTestEachBody(ctx, send->fun, std::move(block->body), block->args, {}, iteratee),
                               std::move(block->args))),
            send->flags);
    }

    if (send->numPosArgs() == 0 && (send->fun == core::Names::before() || send->fun == core::Names::after())) {
        auto name = send->fun == core::Names::after() ? core::Names::afterAngles() : core::Names::initialize();
        ConstantMover constantMover;
        block->body = ast::TreeMap::apply(ctx, constantMover, move(block->body));
        auto method = addSigVoid(
            ast::MK::SyntheticMethod0(send->loc, send->loc, name, prepareBody(ctx, isClass, std::move(block->body))));
        return constantMover.addConstantsToExpression(send->loc, move(method));
    }

    if (send->numPosArgs() != 1) {
        return nullptr;
    }
    auto &arg = send->getPosArg(0);
    auto argString = to_s(ctx, arg);

    if (send->fun == core::Names::describe()) {
        ast::ClassDef::ANCESTORS_store ancestors;

        // Avoid subclassing the containing context when it's a module, as that will produce an error in typed: false
        // files
        if (isClass) {
            ancestors.emplace_back(ast::MK::Self(arg.loc()));
        }

        ast::ClassDef::RHS_store rhs;
        const bool bodyIsClass = true;
        rhs.emplace_back(prepareBody(ctx, bodyIsClass, std::move(block->body)));
        auto name = ast::MK::UnresolvedConstant(arg.loc(), ast::MK::EmptyTree(),
                                                ctx.state.enterNameConstant("<describe '" + argString + "'>"));
        return ast::MK::Class(send->loc, send->loc, std::move(name), std::move(ancestors), std::move(rhs));
    } else if (send->fun == core::Names::it()) {
        ConstantMover constantMover;
        block->body = ast::TreeMap::apply(ctx, constantMover, move(block->body));
        auto name = ctx.state.enterNameUTF8("<it '" + argString + "'>");
        const bool bodyIsClass = false;
        auto method = addSigVoid(ast::MK::SyntheticMethod0(send->loc, send->loc, std::move(name),
                                                           prepareBody(ctx, bodyIsClass, std::move(block->body))));
        method = ast::MK::InsSeq1(send->loc, send->getPosArg(0).deepCopy(), move(method));
        return constantMover.addConstantsToExpression(send->loc, move(method));
    }

    return nullptr;
}

ast::ExpressionPtr recurse(core::MutableContext ctx, bool isClass, ast::ExpressionPtr body) {
    auto bodySend = ast::cast_tree<ast::Send>(body);
    if (bodySend) {
        auto change = runSingle(ctx, isClass, bodySend, RunSingleLevel::Interior);
        if (change) {
            return change;
        }
    }
    return body;
}

vector<ast::ExpressionPtr> Minitest::run(core::MutableContext ctx, bool isClass, ast::Send *send) {
    vector<ast::ExpressionPtr> stats;
    if (ctx.state.runningUnderAutogen) {
        return stats;
    }

    auto exp = runSingle(ctx, isClass, send, RunSingleLevel::Toplevel);
    if (exp != nullptr) {
        stats.emplace_back(std::move(exp));
    }
    return stats;
}

}; // namespace sorbet::rewriter
