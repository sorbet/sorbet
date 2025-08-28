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

bool isDescribeOrSimilar(core::NameRef method) {
    return method == core::Names::describe() || method == core::Names::context() ||
           method == core::Names::xdescribe() || method == core::Names::xcontext();
}

bool isSharedExamples(core::NameRef method) {
    return method == core::Names::sharedExamples() || method == core::Names::sharedContext();
}

class ConstantMover {
    uint32_t classDepth = 0;
    vector<ast::ExpressionPtr> movedConstants = {};

public:
    ast::ExpressionPtr createConstAssign(ast::Assign &asgn) {
        auto loc = asgn.loc;
        auto raiseUnimplemented = ast::MK::RaiseUnimplemented(loc);
        if (auto cast = ast::cast_tree<ast::Cast>(asgn.rhs)) {
            if (cast->cast == core::Names::let()) {
                auto rhs = ast::MK::Let(loc, move(raiseUnimplemented), cast->typeExpr.deepCopy());
                return ast::MK::Assign(asgn.loc, move(asgn.lhs), move(rhs));
            }
        }

        return ast::MK::Assign(asgn.loc, move(asgn.lhs), move(raiseUnimplemented));
    }

    void postTransformAssign(core::MutableContext ctx, ast::ExpressionPtr &tree) {
        auto asgn = ast::cast_tree<ast::Assign>(tree);
        if (auto cnst = ast::cast_tree<ast::UnresolvedConstantLit>(asgn->lhs)) {
            if (ast::isa_tree<ast::UnresolvedConstantLit>(asgn->rhs)) {
                movedConstants.emplace_back(move(tree));
                tree = ast::MK::EmptyTree();
                return;
            }
            auto name = ast::MK::Symbol(cnst->loc, cnst->cnst);

            // if the constant is already in a T.let, preserve it, otherwise decay it to unsafe
            movedConstants.emplace_back(createConstAssign(*asgn));

            auto module = ast::MK::Constant(asgn->loc, core::Symbols::Module());
            tree = ast::MK::Send2(asgn->loc, move(module), core::Names::constSet(), asgn->loc.copyWithZeroLength(),
                                  move(name), move(asgn->rhs));
            return;
        }
    }

    // classdefs define new constants, so we always move those if they're the "top-level" classdef (i.e. if we have
    // nested classdefs, we should only move the outermost one)
    void preTransformClassDef(core::MutableContext ctx, ast::ExpressionPtr &classDef) {
        classDepth++;
    }

    void postTransformClassDef(core::MutableContext ctx, ast::ExpressionPtr &classDef) {
        classDepth--;
        if (classDepth == 0) {
            movedConstants.emplace_back(move(classDef));
            classDef = ast::MK::EmptyTree();
        }
    }

    // we move sends if they are other minitest `describe` blocks, as those end up being classes anyway: consequently,
    // we treat those the same way we treat classes
    void preTransformSend(core::MutableContext ctx, ast::ExpressionPtr &tree) {
        auto send = ast::cast_tree<ast::Send>(tree);
        if (send->recv.isSelfReference() && send->numPosArgs() == 1 &&
            (isDescribeOrSimilar(send->fun) || isSharedExamples(send->fun))) {
            classDepth++;
        }
    }

    void postTransformSend(core::MutableContext ctx, ast::ExpressionPtr &tree) {
        auto send = ast::cast_tree<ast::Send>(tree);
        if (send->recv.isSelfReference() && send->numPosArgs() == 1 &&
            (isDescribeOrSimilar(send->fun) || isSharedExamples(send->fun))) {
            classDepth--;
            if (classDepth == 0) {
                movedConstants.emplace_back(move(tree));
                tree = ast::MK::EmptyTree();
                return;
            }
        }
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
    core::LocOffsets declLoc;
    if (auto mdef = ast::cast_tree<ast::MethodDef>(expr)) {
        declLoc = mdef->declLoc;
    } else {
        ENFORCE(false, "Added a sig to something that wasn't a method def");
        declLoc = expr.loc();
    }
    return ast::MK::InsSeq1(expr.loc(), ast::MK::SigVoid(declLoc, {}), std::move(expr));
}

core::LocOffsets declLocForSendWithBlock(const ast::Send &send) {
    return send.loc.copyWithZeroLength().join(send.block()->loc.copyWithZeroLength());
}

// Helper function to create shared_examples_module constant reference with local scope only
ast::ExpressionPtr makeSharedExamplesModuleConstantLocal(core::MutableContext ctx, core::LocOffsets loc,
                                                         const string &argString) {
    return ast::MK::UnresolvedConstant(loc, ast::MK::EmptyTree(),
                                       ctx.state.enterNameConstant("<shared_examples_module '" + argString + "'>"));
}

} // namespace

ast::ExpressionPtr recurse(core::MutableContext ctx, bool isClass, ast::ExpressionPtr body, bool insideDescribe);

ast::ExpressionPtr prepareBody(core::MutableContext ctx, bool isClass, ast::ExpressionPtr body, bool insideDescribe) {
    body = recurse(ctx, isClass, std::move(body), insideDescribe);

    if (auto bodySeq = ast::cast_tree<ast::InsSeq>(body)) {
        for (auto &exp : bodySeq->stats) {
            exp = recurse(ctx, isClass, std::move(exp), insideDescribe);
        }

        bodySeq->expr = recurse(ctx, isClass, std::move(bodySeq->expr), insideDescribe);
    }
    return body;
}

// Namer only looks at ancestors at the ClassDef top-level. If a `describe` block has ancestor items
// at the top level inside the InsSeq of the Block body, that should count as a an ancestor in namer.
// But if we just plop the whole InsSeq as a single element inside the the ClassDef rhs, they won't
// be at the top level anymore.
ast::ClassDef::RHS_store flattenDescribeBody(ast::ExpressionPtr body) {
    ast::ClassDef::RHS_store rhs;
    if (auto bodySeq = ast::cast_tree<ast::InsSeq>(body)) {
        absl::c_move(bodySeq->stats, back_inserter(rhs));
        rhs.emplace_back(move(bodySeq->expr));
    } else {
        rhs.emplace_back(move(body));
    }

    return rhs;
}

string to_s(core::Context ctx, ast::ExpressionPtr &arg) {
    auto argLit = ast::cast_tree<ast::Literal>(arg);
    if (argLit != nullptr && argLit->isName()) {
        return argLit->asName().show(ctx);
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
    } else if (auto list = ast::cast_tree<ast::Array>(exp)) {
        return absl::c_all_of(list->elems, [](auto &elem) { return canMoveIntoMethodDef(elem); });
    } else if (auto hash = ast::cast_tree<ast::Hash>(exp)) {
        return absl::c_all_of(hash->keys, [](auto &elem) { return canMoveIntoMethodDef(elem); }) &&
               absl::c_all_of(hash->values, [](auto &elem) { return canMoveIntoMethodDef(elem); });
    } else if (auto send = ast::cast_tree<ast::Send>(exp)) {
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

bool isRSpec(const ast::ExpressionPtr &recv) {
    auto cnst = ast::cast_tree<ast::UnresolvedConstantLit>(recv);
    if (cnst == nullptr) {
        return false;
    }

    return cnst->cnst == core::Names::Constants::RSpec();
}

ast::ExpressionPtr prepareTestEachBody(core::MutableContext ctx, core::NameRef eachName, ast::ExpressionPtr body,
                                       const ast::MethodDef::ARGS_store &args,
                                       absl::Span<const ast::ExpressionPtr> destructuringStmts,
                                       ast::ExpressionPtr &iteratee, bool insideDescribe);

// this applies to each statement contained within a `test_each`: if it's an `it`-block, then convert it appropriately,
// otherwise flag an error about it
ast::ExpressionPtr runUnderEach(core::MutableContext ctx, core::NameRef eachName,
                                absl::Span<const ast::ExpressionPtr> destructuringStmts, ast::ExpressionPtr stmt,
                                const ast::MethodDef::ARGS_store &args, ast::ExpressionPtr &iteratee,
                                bool insideDescribe) {
    // this statement must be a send
    if (auto send = ast::cast_tree<ast::Send>(stmt)) {
        auto correctBlockArity = send->hasBlock() && send->block()->args.size() == 0;

        // Check for different types of test blocks
        auto isItOrXit = (send->fun == core::Names::it() || send->fun == core::Names::xit());
        auto isIts = (send->fun == core::Names::its());
        auto isBeforeOrAfter = (send->fun == core::Names::before() || send->fun == core::Names::after());

        auto itOrXitWithValidArgs = isItOrXit && (send->numPosArgs() == 0 || send->numPosArgs() == 1);
        auto itsWithValidArgs = isIts && send->numPosArgs() == 1;
        auto beforeOrAfterWithValidArgs = isBeforeOrAfter && send->numPosArgs() == 0;

        auto isValidTestBlock = (itOrXitWithValidArgs || itsWithValidArgs) && correctBlockArity;
        auto isValidSetupTeardown = beforeOrAfterWithValidArgs && correctBlockArity;

        if (isValidTestBlock || isValidSetupTeardown) {
            core::NameRef name;
            if (send->fun == core::Names::before()) {
                name = core::Names::beforeAngles();
            } else if (send->fun == core::Names::after()) {
                name = core::Names::afterAngles();
            } else {
                // we use this for the name of our test
                if (send->numPosArgs() == 1) {
                    auto argString = to_s(ctx, send->getPosArg(0));
                    if (send->fun == core::Names::its()) {
                        name = ctx.state.enterNameUTF8("<its " + argString + ">");
                    } else if (send->fun == core::Names::xit()) {
                        name = ctx.state.enterNameUTF8("<xit '" + argString + "'>");
                    } else {
                        name = ctx.state.enterNameUTF8("<it '" + argString + "'>");
                    }
                } else {
                    // no argument provided, use static anonymous names
                    if (send->fun == core::Names::xit()) {
                        name = core::Names::anonymousXit();
                    } else {
                        name = core::Names::anonymousIt();
                    }
                }
            }

            // pull constants out of the block
            ConstantMover constantMover;
            ast::ExpressionPtr body = move(send->block()->body);

            // we don't need to make a new body if the original one was empty
            if (!ast::isa_tree<ast::EmptyTree>(body)) {
                ast::TreeWalk::apply(ctx, constantMover, body);

                // add the destructuring statements to the block if they're present
                if (!destructuringStmts.empty()) {
                    ast::InsSeq::STATS_store stmts;
                    for (auto &stmt : destructuringStmts) {
                        stmts.emplace_back(stmt.deepCopy());
                    }
                    body = ast::MK::InsSeq(body.loc(), std::move(stmts), std::move(body));
                }
            }

            // pull the arg and the iteratee in and synthesize `iterate.each { |arg| body }`
            ast::MethodDef::ARGS_store new_args;
            for (auto &arg : args) {
                new_args.emplace_back(arg.deepCopy());
            }

            auto blk = ast::MK::Block(send->block()->loc, move(body), std::move(new_args));
            auto each = ast::MK::Send0Block(send->loc, iteratee.deepCopy(), core::Names::each(),
                                            send->loc.copyWithZeroLength(), move(blk));
            // put that into a method def named the appropriate thing
            auto declLoc = declLocForSendWithBlock(*send);
            auto method = addSigVoid(ast::MK::SyntheticMethod0(send->loc, declLoc, move(name), move(each)));
            // add back any moved constants
            return constantMover.addConstantsToExpression(send->loc, move(method));
        } else if ((isDescribeOrSimilar(send->fun) || isSharedExamples(send->fun)) && send->numPosArgs() == 1 &&
                   correctBlockArity) {
            return prepareTestEachBody(ctx, eachName, std::move(send->block()->body), args, destructuringStmts,
                                       iteratee,
                                       /* insideDescribe */ true);
        } else if (insideDescribe && send->fun == core::Names::itBehavesLike() && send->numPosArgs() == 1) {
            // Handle it_behaves_like within test_each blocks
            auto &arg = send->getPosArg(0);
            auto argString = to_s(ctx, arg);
            // Create an include statement that includes the shared_examples companion module
            auto moduleName = makeSharedExamplesModuleConstantLocal(ctx, arg.loc(), argString);
            // Create an include statement to include the shared_examples companion module
            return ast::MK::Send1(send->loc, ast::MK::Self(send->loc), core::Names::include(), send->funLoc,
                                  std::move(moduleName));
        } else if (insideDescribe &&
                   ((send->fun == core::Names::let() && send->numPosArgs() == 1) ||
                    (send->fun == core::Names::letBang() && send->numPosArgs() == 1) ||
                    (send->fun == core::Names::subject() && send->numPosArgs() <= 1)) &&
                   correctBlockArity && (send->numPosArgs() == 0 || ast::isa_tree<ast::Literal>(send->getPosArg(0)))) {
            core::NameRef methodName;
            core::LocOffsets declLoc;

            if (send->numPosArgs() == 1) {
                auto argLiteral = ast::cast_tree_nonnull<ast::Literal>(send->getPosArg(0));
                if (argLiteral.isName()) {
                    declLoc = send->loc.copyWithZeroLength().join(argLiteral.loc);
                    methodName = argLiteral.asName();
                } else {
                    return nullptr;
                }
            } else {
                declLoc = send->loc.copyWithZeroLength().join(send->funLoc);
                methodName = core::Names::subject();
            }

            // Handle let blocks the same way as it blocks - give them access to test_each arguments
            ConstantMover constantMover;
            ast::ExpressionPtr body = move(send->block()->body);

            if (!ast::isa_tree<ast::EmptyTree>(body)) {
                ast::TreeWalk::apply(ctx, constantMover, body);

                // add the destructuring statements to the block if they're present
                if (!destructuringStmts.empty()) {
                    ast::InsSeq::STATS_store stmts;
                    for (auto &stmt : destructuringStmts) {
                        stmts.emplace_back(stmt.deepCopy());
                    }
                    body = ast::MK::InsSeq(body.loc(), std::move(stmts), std::move(body));
                }
            }

            ast::MethodDef::ARGS_store new_args;
            for (auto &arg : args) {
                new_args.emplace_back(arg.deepCopy());
            }

            ast::ExpressionPtr method = ast::MK::SyntheticMethod0(send->loc, declLoc, methodName, move(body));
            return constantMover.addConstantsToExpression(send->loc, move(method));
        }
    }

    // if any of the above tests were not satisfied, then mark this statement as being invalid here
    if (auto e = ctx.beginIndexerError(stmt.loc(), core::errors::Rewriter::BadTestEach)) {
        e.setHeader("Only valid `{}`, `{}`, `{}`, and `{}` blocks can appear within `{}`", "it", "before", "after",
                    "describe", eachName.show(ctx));
        e.addErrorNote("For other things, like constant and variable assignments,"
                       "    hoist them to constants or methods defined outside the `{}` block.",
                       eachName.show(ctx));
    }

    return stmt;
}

bool isDestructuringArg(core::GlobalState &gs, const ast::MethodDef::ARGS_store &args, const ast::ExpressionPtr &expr) {
    auto local = ast::cast_tree<ast::UnresolvedIdent>(expr);
    if (local == nullptr || local->kind != ast::UnresolvedIdent::Kind::Local) {
        return false;
    }

    auto name = local->name;
    if (name.kind() != core::NameKind::UNIQUE || name.dataUnique(gs)->original != core::Names::destructureArg()) {
        return false;
    }

    return absl::c_find_if(args, [name](auto &argExpr) {
               auto arg = ast::cast_tree<ast::UnresolvedIdent>(argExpr);
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
        auto insSeq = ast::cast_tree<ast::InsSeq>(stat);
        if (insSeq == nullptr) {
            return false;
        }

        auto assign = ast::cast_tree<ast::Assign>(insSeq->stats.front());
        return assign && isDestructuringArg(gs, args, assign->rhs);
    });
}

// this just walks the body of a `test_each` and tries to transform every statement
ast::ExpressionPtr prepareTestEachBody(core::MutableContext ctx, core::NameRef eachName, ast::ExpressionPtr body,
                                       const ast::MethodDef::ARGS_store &args,
                                       absl::Span<const ast::ExpressionPtr> destructuringStmts,
                                       ast::ExpressionPtr &iteratee, bool insideDescribe) {
    if (auto bodySeq = ast::cast_tree<ast::InsSeq>(body)) {
        if (isDestructuringInsSeq(ctx, args, bodySeq)) {
            ENFORCE(destructuringStmts.empty(), "Nested destructuring statements");
            return prepareTestEachBody(ctx, eachName, std::move(bodySeq->expr), args, absl::MakeSpan(bodySeq->stats),
                                       iteratee, insideDescribe);
        }

        for (auto &exp : bodySeq->stats) {
            exp = runUnderEach(ctx, eachName, destructuringStmts, std::move(exp), args, iteratee, insideDescribe);
        }

        bodySeq->expr =
            runUnderEach(ctx, eachName, destructuringStmts, std::move(bodySeq->expr), args, iteratee, insideDescribe);
    } else {
        body = runUnderEach(ctx, eachName, destructuringStmts, std::move(body), args, iteratee, insideDescribe);
    }

    return body;
}

ast::ExpressionPtr runSingle(core::MutableContext ctx, bool isClass, ast::Send *send, bool insideDescribe) {
    // Handle include_context first (1+ pos args + optional kwargs/block)
    if (send->fun == core::Names::includeContext() && send->numPosArgs() >= 1) {
        auto &arg = send->getPosArg(0);
        auto argString = to_s(ctx, arg);
        // Create an include statement that includes the shared_examples companion module
        auto moduleName = makeSharedExamplesModuleConstantLocal(ctx, arg.loc(), argString);
        // Create an include statement to include the shared_examples companion module
        return ast::MK::Send1(send->loc, ast::MK::Self(send->loc), core::Names::include(), send->funLoc,
                              std::move(moduleName));
    }

    if (!send->hasBlock()) {
        return nullptr;
    }

    auto *block = send->block();

    if (!send->recv.isSelfReference() && !((isDescribeOrSimilar(send->fun) || isSharedExamples(send->fun) ||
                                            send->fun == core::Names::includeContext()) &&
                                           isRSpec(send->recv))) {
        return nullptr;
    }

    if ((send->fun == core::Names::testEach() || send->fun == core::Names::testEachHash()) && send->numPosArgs() == 1) {
        if ((send->fun == core::Names::testEach() && block->args.size() < 1) ||
            (send->fun == core::Names::testEachHash() && block->args.size() != 2)) {
            if (auto e = ctx.beginIndexerError(block->loc, core::errors::Rewriter::BadTestEach)) {
                e.setHeader("Wrong number of parameters for `{}` block: expected `{}`, got `{}`", send->fun.show(ctx),
                            send->fun == core::Names::testEach() ? "at least 1" : "2", block->args.size());
            }
            return nullptr;
        }
        // if this has the form `test_each(expr) { |arg | .. }`, then start by trying to convert `expr` into a thing we
        // can freely copy into methoddef scope
        auto iteratee = getIteratee(send->getPosArg(0));
        // and then reconstruct the send but with a modified body
        auto body =
            prepareTestEachBody(ctx, send->fun, std::move(block->body), block->args, {}, iteratee, insideDescribe);
        return ast::MK::Send(send->loc, ast::MK::Self(send->recv.loc()), send->fun, send->funLoc, 1,
                             ast::MK::SendArgs(move(send->getPosArg(0)),
                                               ast::MK::Block(block->loc, std::move(body), std::move(block->args))),
                             send->flags);
    }

    if (send->fun == core::Names::testEachHash() && send->numKwArgs() > 0) {
        auto errLoc = send->getKwKey(0).loc().join(send->getKwValue(send->numKwArgs() - 1).loc());
        if (auto e = ctx.beginIndexerError(errLoc, core::errors::Rewriter::BadTestEach)) {
            e.setHeader("`{}` expects a single `{}` argument, not keyword args", "test_each_hash", "Hash");
            if (send->numPosArgs() == 0 && errLoc.exists()) {
                auto replaceLoc = ctx.locAt(errLoc);
                e.replaceWith("Wrap with curly braces", replaceLoc, "{{{}}}", replaceLoc.source(ctx).value());
            }
        }
    }

    if (send->numPosArgs() == 0 && (send->fun == core::Names::before() || send->fun == core::Names::after())) {
        auto name = send->fun == core::Names::after() ? core::Names::afterAngles() : core::Names::beforeAngles();
        ConstantMover constantMover;
        ast::TreeWalk::apply(ctx, constantMover, block->body);
        auto declLoc = declLocForSendWithBlock(*send);
        auto method = addSigVoid(ast::MK::SyntheticMethod0(
            send->loc, declLoc, name, prepareBody(ctx, isClass, std::move(block->body), insideDescribe)));
        return constantMover.addConstantsToExpression(send->loc, move(method));
    }

    if (send->numPosArgs() == 0 && send->fun == core::Names::subject()) {
        auto declLoc = send->funLoc;
        return ast::MK::SyntheticMethod0(send->loc, declLoc, core::Names::subject(), std::move(block->body));
    }

    // Handle include_context with block (1+ pos args + optional kwargs/block)
    if (send->fun == core::Names::includeContext() && send->numPosArgs() >= 1) {
        auto &arg = send->getPosArg(0);
        auto argString = to_s(ctx, arg);
        // Create an include statement that includes the filtered shared_examples module
        // Use proper scoping - if we're inside a describe block, look locally; otherwise use root
        auto moduleName = makeSharedExamplesModuleConstantLocal(ctx, arg.loc(), argString);
        // Create an include statement to include the filtered shared_examples module
        return ast::MK::Send1(send->loc, ast::MK::Self(send->loc), core::Names::include(), send->funLoc,
                              std::move(moduleName));
    }

    // Handle it/xit blocks with 0 arguments separately
    if (send->fun == core::Names::it() || send->fun == core::Names::xit()) {
        if (send->numPosArgs() != 0 && send->numPosArgs() != 1) {
            return nullptr;
        }

        ConstantMover constantMover;
        ast::TreeWalk::apply(ctx, constantMover, block->body);

        core::NameRef name;
        if (send->numPosArgs() == 1) {
            auto argString = to_s(ctx, send->getPosArg(0));
            if (send->fun == core::Names::xit()) {
                name = ctx.state.enterNameUTF8("<xit '" + argString + "'>");
            } else {
                name = ctx.state.enterNameUTF8("<it '" + argString + "'>");
            }
        } else {
            // no argument provided, use a deterministic unique name based on location
            auto pos = send->loc.beginPos();
            if (send->fun == core::Names::xit()) {
                name = ctx.state.enterNameUTF8("<anonymous_xit_" + to_string(pos) + ">");
            } else {
                name = ctx.state.enterNameUTF8("<anonymous_it_" + to_string(pos) + ">");
            }
        }

        const bool bodyIsClass = false;
        auto declLoc = declLocForSendWithBlock(*send);
        auto method = addSigVoid(
            ast::MK::SyntheticMethod0(send->loc, declLoc, std::move(name),
                                      prepareBody(ctx, bodyIsClass, std::move(block->body), insideDescribe)));

        // Only add the argument copy if there's an argument
        if (send->numPosArgs() == 1) {
            method = ast::MK::InsSeq1(send->loc, send->getPosArg(0).deepCopy(), move(method));
        }
        return constantMover.addConstantsToExpression(send->loc, move(method));
    }

    if (send->numPosArgs() != 1) {
        return nullptr;
    }
    auto &arg = send->getPosArg(0);

    if (isDescribeOrSimilar(send->fun)) {
        auto argString = to_s(ctx, arg);
        ast::ClassDef::ANCESTORS_store ancestors;

        if (send->recv.isSelfReference()) {
            // First ancestor is the superclass
            if (isClass) {
                ancestors.emplace_back(ast::MK::Self(arg.loc()));
            } else {
                // Avoid subclassing self when it's a module, as that will produce an error.
                ancestors.emplace_back(ast::MK::Constant(arg.loc(), core::Symbols::todo()));

                // Note: For cases like `module M; describe '' {}; end`, minitest does not treat `M` as
                // an ancestor of the dynamically-created class. Instead, it treats `Minitest::Spec` as
                // an ancestor, which we're choosing not to model so that this rewriter pass works for
                // RSpec specs too. This means users might have to add extra `include` lines in their
                // describe bodies to convince Sorbet what's available, but at least it won't say that
                // Minitest::Spec is an ancestor for RSpec tests.
            }
        } else {
            ENFORCE(isRSpec(send->recv));
            auto exampleGroup = ast::MK::EmptyTree();
            exampleGroup =
                ast::MK::UnresolvedConstant(send->recv.loc(), move(exampleGroup), core::Names::Constants::RSpec());
            exampleGroup =
                ast::MK::UnresolvedConstant(send->recv.loc(), move(exampleGroup), core::Names::Constants::Core());
            exampleGroup = ast::MK::UnresolvedConstant(send->recv.loc(), move(exampleGroup),
                                                       core::Names::Constants::ExampleGroup());
            ancestors.emplace_back(move(exampleGroup));
        }

        const bool bodyIsClass = true;
        auto rhs = prepareBody(ctx, bodyIsClass, std::move(block->body), /* insideDescribe */ true);

        auto name = ast::MK::UnresolvedConstant(arg.loc(), ast::MK::EmptyTree(),
                                                ctx.state.enterNameConstant("<describe '" + argString + "'>"));
        auto declLoc = declLocForSendWithBlock(*send);
        return ast::MK::Class(send->loc, declLoc, std::move(name), std::move(ancestors),
                              flattenDescribeBody(move(rhs)));
    } else if (isSharedExamples(send->fun)) {
        auto argString = to_s(ctx, arg);
        const bool bodyIsClass = true; // Process body as class context to get proper RSpec context
        auto rhs = prepareBody(ctx, bodyIsClass, std::move(block->body), /* insideDescribe */ true);

        // Create a class for shared_examples that inherits from the same parent as the current describe block
        // Use proper scoping - if we're inside a describe block, nest under it; otherwise use root
        auto name = ast::MK::UnresolvedConstant(
            arg.loc(), insideDescribe ? ast::MK::EmptyTree() : ast::MK::Constant(arg.loc(), core::Symbols::root()),
            ctx.state.enterNameConstant("<shared_examples '" + argString + "'>"));
        auto declLoc = declLocForSendWithBlock(*send);
        ast::ClassDef::ANCESTORS_store ancestors;

        // shared_examples should inherit from the same parent as the current describe block
        // This gives them access to RSpec methods and context
        if (send->recv.isSelfReference()) {
            if (isClass) {
                // Inherit from Self's parent (same as what a describe block would inherit from)
                ancestors.emplace_back(ast::MK::Self(arg.loc()));
            } else {
                ancestors.emplace_back(ast::MK::Constant(arg.loc(), core::Symbols::todo()));
            }
        } else {
            ENFORCE(isRSpec(send->recv));
            auto exampleGroup = ast::MK::EmptyTree();
            exampleGroup =
                ast::MK::UnresolvedConstant(send->recv.loc(), move(exampleGroup), core::Names::Constants::RSpec());
            exampleGroup =
                ast::MK::UnresolvedConstant(send->recv.loc(), move(exampleGroup), core::Names::Constants::Core());
            exampleGroup = ast::MK::UnresolvedConstant(send->recv.loc(), move(exampleGroup),
                                                       core::Names::Constants::ExampleGroup());
            ancestors.emplace_back(move(exampleGroup));
        }

        // Create the class version (contains everything)
        auto classResult = ast::MK::Class(send->loc, declLoc, name.deepCopy(), std::move(ancestors),
                                          flattenDescribeBody(rhs.deepCopy()));

        // Create a filtered module version that contains only includable methods (let, etc.)
        auto moduleName = makeSharedExamplesModuleConstantLocal(ctx, arg.loc(), argString);

        // Filter the RHS to include only methods that should be includable
        ast::ClassDef::RHS_store filteredModuleRhs;
        auto flattenedRhs = flattenDescribeBody(move(rhs));

        // Add extend T::Helpers at the beginning
        auto tHelpers = ast::MK::EmptyTree();
        tHelpers = ast::MK::UnresolvedConstant(arg.loc(), move(tHelpers), core::Names::Constants::T());
        tHelpers = ast::MK::UnresolvedConstant(arg.loc(), move(tHelpers), core::Names::Constants::Helpers());
        auto extendStatement = ast::MK::Send1(send->loc, ast::MK::Self(send->loc), core::Names::extend(), send->funLoc,
                                              std::move(tHelpers));
        filteredModuleRhs.emplace_back(std::move(extendStatement));

        // Add requires_ancestor { RSpec::Core::ExampleGroup }
        if (!send->recv.isSelfReference() && isRSpec(send->recv)) {
            // Add requires_ancestor when we have explicit RSpec receiver (RSpec context)
            auto rspecExampleGroup = ast::MK::EmptyTree();
            rspecExampleGroup =
                ast::MK::UnresolvedConstant(arg.loc(), move(rspecExampleGroup), core::Names::Constants::RSpec());
            rspecExampleGroup =
                ast::MK::UnresolvedConstant(arg.loc(), move(rspecExampleGroup), core::Names::Constants::Core());
            rspecExampleGroup =
                ast::MK::UnresolvedConstant(arg.loc(), move(rspecExampleGroup), core::Names::Constants::ExampleGroup());

            // Create a block that returns the RSpec::Core::ExampleGroup constant
            auto blockBody = std::move(rspecExampleGroup);
            ast::MethodDef::ARGS_store blockArgs;
            auto block = ast::MK::Block(send->loc, std::move(blockBody), std::move(blockArgs));

            auto requiresAncestorStatement = ast::MK::Send0Block(
                send->loc, ast::MK::Self(send->loc), core::Names::requiresAncestor(), send->loc, std::move(block));
            filteredModuleRhs.emplace_back(std::move(requiresAncestorStatement));
        }

        for (auto &expr : flattenedRhs) {
            // Only include method definitions (let, subject, etc.) in the module
            // Skip describe blocks and other non-includable constructs
            if (auto method = ast::cast_tree<ast::MethodDef>(expr)) {
                filteredModuleRhs.emplace_back(expr.deepCopy());
            }
        }

        ast::ClassDef::ANCESTORS_store moduleAncestors;
        auto moduleResult = ast::MK::Module(send->loc, declLoc, std::move(moduleName), std::move(moduleAncestors),
                                            std::move(filteredModuleRhs));

        // Return both the class and filtered module in a sequence
        ast::InsSeq::STATS_store statements;
        statements.emplace_back(std::move(classResult));
        return ast::MK::InsSeq(send->loc, std::move(statements), std::move(moduleResult));
    } else if (send->fun == core::Names::its()) {
        auto argString = to_s(ctx, arg);
        ConstantMover constantMover;
        ast::TreeWalk::apply(ctx, constantMover, block->body);

        auto name = ctx.state.enterNameUTF8("<its " + argString + ">");

        const bool bodyIsClass = false;
        auto declLoc = declLocForSendWithBlock(*send);
        auto method = addSigVoid(
            ast::MK::SyntheticMethod0(send->loc, declLoc, std::move(name),
                                      prepareBody(ctx, bodyIsClass, std::move(block->body), insideDescribe)));
        method = ast::MK::InsSeq1(send->loc, send->getPosArg(0).deepCopy(), move(method));
        return constantMover.addConstantsToExpression(send->loc, move(method));
    } else if (insideDescribe &&
               ((send->fun == core::Names::let() || send->fun == core::Names::letBang() ||
                 send->fun == core::Names::subject())) &&
               ast::isa_tree<ast::Literal>(arg)) {
        auto argLiteral = ast::cast_tree_nonnull<ast::Literal>(arg);
        if (!argLiteral.isName()) {
            return nullptr;
        }

        auto declLoc = send->loc.copyWithZeroLength().join(argLiteral.loc);
        auto methodName = argLiteral.asName();
        return ast::MK::SyntheticMethod0(send->loc, declLoc, methodName, std::move(block->body));
    } else if (insideDescribe && (send->fun == core::Names::expect() || send->fun == core::Names::change()) &&
               send->hasBlock()) {
        // Handle expect { ... } and change { ... } blocks
        // Process the block body in the current context so it has access to the same scope as the it block
        const bool bodyIsClass = false;
        auto processedBlockBody = prepareBody(ctx, bodyIsClass, std::move(send->block()->body), insideDescribe);

        // Create a new block with the processed body
        auto newBlock =
            ast::MK::Block(send->block()->loc, std::move(processedBlockBody), std::move(send->block()->args));

        // Create args store with positional arguments
        ast::Send::ARGS_store args;
        for (uint16_t i = 0; i < send->numPosArgs(); i++) {
            args.emplace_back(send->getPosArg(i).deepCopy());
        }

        // Add the processed block
        args.emplace_back(std::move(newBlock));

        // Create new send with block flag
        ast::Send::Flags flags = send->flags;
        flags.hasBlock = true;

        return ast::MK::Send(send->loc, send->recv.deepCopy(), send->fun, send->funLoc, send->numPosArgs(),
                             std::move(args), flags);
    }

    return nullptr;
}

ast::ExpressionPtr recurse(core::MutableContext ctx, bool isClass, ast::ExpressionPtr body, bool insideDescribe) {
    auto bodySend = ast::cast_tree<ast::Send>(body);
    if (bodySend) {
        auto change = runSingle(ctx, isClass, bodySend, insideDescribe);
        if (change) {
            return change;
        }
    }
    return body;
}

vector<ast::ExpressionPtr> Minitest::run(core::MutableContext ctx, bool isClass, ast::Send *send) {
    vector<ast::ExpressionPtr> stats;
    if (ctx.state.cacheSensitiveOptions.runningUnderAutogen) {
        return stats;
    }

    auto insideDescribe = false;
    auto exp = runSingle(ctx, isClass, send, insideDescribe);
    if (exp != nullptr) {
        stats.emplace_back(std::move(exp));
    }
    return stats;
}

}; // namespace sorbet::rewriter
