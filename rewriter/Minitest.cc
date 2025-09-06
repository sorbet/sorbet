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
        if (send->recv.isSelfReference() && send->numPosArgs() == 1 && send->fun == core::Names::describe()) {
            classDepth++;
        }
    }

    void postTransformSend(core::MutableContext ctx, ast::ExpressionPtr &tree) {
        auto send = ast::cast_tree<ast::Send>(tree);
        if (send->recv.isSelfReference() && send->numPosArgs() == 1 && send->fun == core::Names::describe()) {
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

ast::ExpressionPtr addSigVoid(core::Context ctx, ast::ExpressionPtr expr) {
    if (ctx.file.data(ctx).strictLevel < core::StrictLevel::Strict) {
        // Only add a dummy sig if it would be required (because the file is `# typed: strict`).
        // This is to save memory (and possibly also typechecking runtime).
        //
        // Another alternative if this approach becomes problematic would be to set some sort of
        // flag on MethodDef that says to suppress the "missing sig" error, or to implicitly assume
        // a sig of `sig { void }` or something.
        //
        // For example, in a world where all tests are `# typed: strict`, this approach saves no memory.
        return expr;
    }

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
    string argString;
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

optional<pair<core::NameRef, core::LocOffsets>> getLetNameAndDeclLoc(const ast::Send &send) {
    if (send.numPosArgs() != 1) {
        return nullopt;
    }

    auto &arg = send.getPosArg(0);
    if (!ast::isa_tree<ast::Literal>(arg)) {
        return nullopt;
    }
    auto argLiteral = ast::cast_tree_nonnull<ast::Literal>(arg);
    if (!argLiteral.isName()) {
        return nullopt;
    }

    auto declLoc = send.loc.copyWithZeroLength().join(argLiteral.loc);
    auto methodName = argLiteral.asName();
    return pair{methodName, declLoc};
}

ast::ExpressionPtr prepareTestEachBody(core::MutableContext ctx, core::NameRef eachName, ast::ExpressionPtr body,
                                       const ast::MethodDef::ARGS_store &args,
                                       absl::Span<const ast::ExpressionPtr> destructuringStmts,
                                       ast::ExpressionPtr &iteratee, bool insideDescribe);

ast::ExpressionPtr invalidUnderTestEach(core::MutableContext ctx, core::NameRef eachName, ast::ExpressionPtr stmt) {
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

// this applies to each statement contained within a `test_each`: if it's an `it`-block, then convert it appropriately,
// otherwise flag an error about it
ast::ExpressionPtr runUnderEach(core::MutableContext ctx, core::NameRef eachName,
                                absl::Span<const ast::ExpressionPtr> destructuringStmts, ast::ExpressionPtr stmt,
                                const ast::MethodDef::ARGS_store &args, ast::ExpressionPtr &iteratee,
                                bool insideDescribe) {
    // this statement must be a send
    auto send = ast::cast_tree<ast::Send>(stmt);
    if (send == nullptr) {
        return invalidUnderTestEach(ctx, eachName, move(stmt));
    }

    if (send->hasBlock() && send->block()->args.size() != 0) {
        return invalidUnderTestEach(ctx, eachName, move(stmt));
    }

    // the send must be a call to `it` with a single argument (the test name) and a block with no arguments
    if ((send->fun == core::Names::it() && send->numPosArgs() == 1) ||
        ((send->fun == core::Names::before() || send->fun == core::Names::after()) && send->numPosArgs() == 0)) {
        core::NameRef name;
        if (send->fun == core::Names::before()) {
            name = core::Names::beforeAngles();
        } else if (send->fun == core::Names::after()) {
            name = core::Names::afterAngles();
        } else {
            // we use this for the name of our test
            auto argString = to_s(ctx, send->getPosArg(0));
            name = ctx.state.enterNameUTF8("<it '" + argString + "'>");
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
        auto method = addSigVoid(ctx, ast::MK::SyntheticMethod0(send->loc, declLoc, move(name), move(each)));
        // add back any moved constants
        return constantMover.addConstantsToExpression(send->loc, move(method));
    } else if (send->fun == core::Names::describe() && send->numPosArgs() == 1) {
        return prepareTestEachBody(ctx, eachName, std::move(send->block()->body), args, destructuringStmts, iteratee,
                                   /* insideDescribe */ true);
    } else if (insideDescribe && send->fun == core::Names::let() && send->numPosArgs() == 1 &&
               ast::isa_tree<ast::Literal>(send->getPosArg(0))) {
        auto maybeDecl = getLetNameAndDeclLoc(*send);
        if (maybeDecl.has_value()) {
            auto [methodName, declLoc] = maybeDecl.value();

            ConstantMover constantMover;
            auto body = move(send->block()->body);
            ast::TreeWalk::apply(ctx, constantMover, body);

            auto method = ast::MK::SyntheticMethod0(send->loc, declLoc, methodName, move(body));
            return constantMover.addConstantsToExpression(send->loc, move(method));
        }
    }

    return invalidUnderTestEach(ctx, eachName, move(stmt));
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
    if (!send->hasBlock()) {
        return nullptr;
    }

    auto *block = send->block();

    if (!send->recv.isSelfReference()) {
        return nullptr;
    }

    switch (send->fun.rawId()) {
        case core::Names::testEach().rawId():
        case core::Names::testEachHash().rawId(): {
            if (send->numPosArgs() != 1) {
                if (send->fun == core::Names::testEachHash() && send->numKwArgs() > 0) {
                    auto errLoc = send->getKwKey(0).loc().join(send->getKwValue(send->numKwArgs() - 1).loc());
                    if (auto e = ctx.beginIndexerError(errLoc, core::errors::Rewriter::BadTestEach)) {
                        e.setHeader("`{}` expects a single `{}` argument, not keyword args", "test_each_hash", "Hash");
                        if (send->numPosArgs() == 0 && errLoc.exists()) {
                            auto replaceLoc = ctx.locAt(errLoc);
                            e.replaceWith("Wrap with curly braces", replaceLoc, "{{{}}}",
                                          replaceLoc.source(ctx).value());
                        }
                    }
                }

                return nullptr;
            }

            if ((send->fun == core::Names::testEach() && block->args.size() < 1) ||
                (send->fun == core::Names::testEachHash() && block->args.size() != 2)) {
                if (auto e = ctx.beginIndexerError(block->loc, core::errors::Rewriter::BadTestEach)) {
                    e.setHeader("Wrong number of parameters for `{}` block: expected `{}`, got `{}`",
                                send->fun.show(ctx), send->fun == core::Names::testEach() ? "at least 1" : "2",
                                block->args.size());
                }
                return nullptr;
            }

            // if this has the form `test_each(expr) { |arg | .. }`, then start by trying to convert `expr` into a thing
            // we can freely copy into methoddef scope
            auto iteratee = getIteratee(send->getPosArg(0));
            // and then reconstruct the send but with a modified body
            auto body =
                prepareTestEachBody(ctx, send->fun, std::move(block->body), block->args, {}, iteratee, insideDescribe);
            return ast::MK::Send(send->loc, ast::MK::Self(send->recv.loc()), send->fun, send->funLoc, 1,
                                 ast::MK::SendArgs(move(send->getPosArg(0)),
                                                   ast::MK::Block(block->loc, std::move(body), std::move(block->args))),
                                 send->flags);
        }

        case core::Names::after().rawId():
        case core::Names::before().rawId(): {
            if (send->numPosArgs() != 0) {
                return nullptr;
            }

            auto name = send->fun == core::Names::after() ? core::Names::afterAngles() : core::Names::beforeAngles();
            ConstantMover constantMover;
            ast::TreeWalk::apply(ctx, constantMover, block->body);
            auto declLoc = declLocForSendWithBlock(*send);
            auto method = addSigVoid(
                ctx, ast::MK::SyntheticMethod0(send->loc, declLoc, name,
                                               prepareBody(ctx, isClass, std::move(block->body), insideDescribe)));
            return constantMover.addConstantsToExpression(send->loc, move(method));
        }

        case core::Names::describe().rawId(): {
            if (send->numPosArgs() != 1) {
                return nullptr;
            }
            auto &arg = send->getPosArg(0);
            auto argString = to_s(ctx, arg);
            ast::ClassDef::ANCESTORS_store ancestors;

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

            const bool bodyIsClass = true;
            auto rhs = prepareBody(ctx, bodyIsClass, std::move(block->body), /* insideDescribe */ true);

            auto name = ast::MK::UnresolvedConstant(arg.loc(), ast::MK::EmptyTree(),
                                                    ctx.state.enterNameConstant("<describe '" + argString + "'>"));
            auto declLoc = declLocForSendWithBlock(*send);
            return ast::MK::Class(send->loc, declLoc, std::move(name), std::move(ancestors),
                                  flattenDescribeBody(move(rhs)));
        }

        case core::Names::it().rawId(): {
            if (send->numPosArgs() != 1) {
                return nullptr;
            }
            auto &arg = send->getPosArg(0);

            auto argString = to_s(ctx, arg);
            ConstantMover constantMover;
            ast::TreeWalk::apply(ctx, constantMover, block->body);
            auto name = ctx.state.enterNameUTF8("<it '" + argString + "'>");
            const bool bodyIsClass = false;
            auto declLoc = declLocForSendWithBlock(*send);
            auto method =
                ast::MK::SyntheticMethod0(send->loc, declLoc, std::move(name),
                                          prepareBody(ctx, bodyIsClass, std::move(block->body), insideDescribe));
            // This prevents the `RuntimeMethodDefinition` from getting generated. For these `it`-block
            // defined methods, we don't actually need to care about the RuntimeMethodDefinition, and
            // omitting it saves memory.
            ast::cast_tree_nonnull<ast::MethodDef>(method).flags.discardDef = true;
            method = addSigVoid(ctx, move(method));
            if (!ast::isa_tree<ast::Literal>(arg)) {
                method = ast::MK::InsSeq1(send->loc, send->getPosArg(0).deepCopy(), move(method));
            }
            return constantMover.addConstantsToExpression(send->loc, move(method));
        }

        case core::Names::let().rawId(): {
            if (!insideDescribe) {
                return nullptr;
            }

            ConstantMover constantMover;
            ast::TreeWalk::apply(ctx, constantMover, block->body);

            auto maybeDecl = getLetNameAndDeclLoc(*send);
            if (!maybeDecl.has_value()) {
                return nullptr;
            }

            auto [methodName, declLoc] = maybeDecl.value();
            auto method = ast::MK::SyntheticMethod0(send->loc, declLoc, methodName, std::move(block->body));
            return constantMover.addConstantsToExpression(send->loc, move(method));
        }
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
