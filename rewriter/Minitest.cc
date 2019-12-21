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
    vector<unique_ptr<ast::Expression>> movedConstants;

public:
    unique_ptr<ast::Expression> createConstAssign(ast::Assign &asgn) {
        auto loc = asgn.loc;
        auto unsafeNil = ast::MK::Unsafe(loc, ast::MK::Nil(loc));
        if (auto send = ast::cast_tree<ast::Send>(asgn.rhs.get())) {
            if (send->fun == core::Names::let() && send->args.size() == 2) {
                auto rhs = ast::MK::Let(loc, move(unsafeNil), send->args[1]->deepCopy());
                return ast::MK::Assign(asgn.loc, move(asgn.lhs), move(rhs));
            }
        }

        return ast::MK::Assign(asgn.loc, move(asgn.lhs), move(unsafeNil));
    }

    unique_ptr<ast::Expression> postTransformAssign(core::MutableContext ctx, unique_ptr<ast::Assign> asgn) {
        if (auto cnst = ast::cast_tree<ast::UnresolvedConstantLit>(asgn->lhs.get())) {
            if (ast::isa_tree<ast::UnresolvedConstantLit>(asgn->rhs.get())) {
                movedConstants.emplace_back(move(asgn));
                return ast::MK::EmptyTree();
            }
            auto name = ast::MK::Symbol(cnst->loc, cnst->cnst);

            // if the constant is already in a T.let, preserve it, otherwise decay it to unsafe
            movedConstants.emplace_back(createConstAssign(*asgn));

            auto module = ast::MK::Constant(asgn->loc, core::Symbols::Module());
            auto const_set = ctx.state.enterNameUTF8("const_set");
            return ast::MK::Send2(asgn->loc, move(module), const_set, move(name), move(asgn->rhs));
        }

        return asgn;
    }

    // classdefs define new constants, so we always move those if they're the "top-level" classdef (i.e. if we have
    // nested classdefs, we should only move the outermost one)
    unique_ptr<ast::ClassDef> preTransformClassDef(core::MutableContext ctx, unique_ptr<ast::ClassDef> classDef) {
        classDepth++;
        return classDef;
    }

    unique_ptr<ast::Expression> postTransformClassDef(core::MutableContext ctx, unique_ptr<ast::ClassDef> classDef) {
        classDepth--;
        if (classDepth == 0) {
            movedConstants.emplace_back(move(classDef));
            return ast::MK::EmptyTree();
        }
        return classDef;
    }

    // we move sends if they are other minitest `describe` blocks, as those end up being classes anyway: consequently,
    // we treat those the same way we treat classes
    unique_ptr<ast::Send> preTransformSend(core::MutableContext ctx, unique_ptr<ast::Send> send) {
        if (send->recv->isSelfReference() && send->args.size() == 1 && send->fun == core::Names::describe()) {
            classDepth++;
        }
        return send;
    }

    unique_ptr<ast::Expression> postTransformSend(core::MutableContext ctx, unique_ptr<ast::Send> send) {
        if (send->recv->isSelfReference() && send->args.size() == 1 && send->fun == core::Names::describe()) {
            classDepth--;
            if (classDepth == 0) {
                movedConstants.emplace_back(move(send));
                return ast::MK::EmptyTree();
            }
        }
        return send;
    }

    vector<unique_ptr<ast::Expression>> getMovedConstants() {
        return move(movedConstants);
    }

    unique_ptr<ast::Expression> addConstantsToExpression(core::Loc loc, unique_ptr<ast::Expression> expr) {
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

unique_ptr<ast::Expression> addSigVoid(unique_ptr<ast::Expression> expr) {
    return ast::MK::InsSeq1(expr->loc, ast::MK::SigVoid(expr->loc, ast::MK::Hash0(expr->loc)), std::move(expr));
}
} // namespace

unique_ptr<ast::Expression> recurse(core::MutableContext ctx, unique_ptr<ast::Expression> body,
                                    optional<ast::Expression *> context);

unique_ptr<ast::Expression> prepareBody(core::MutableContext ctx, unique_ptr<ast::Expression> body,
                                        optional<ast::Expression *> context) {
    body = recurse(ctx, std::move(body), context);

    if (auto bodySeq = ast::cast_tree<ast::InsSeq>(body.get())) {
        for (auto &exp : bodySeq->stats) {
            exp = recurse(ctx, std::move(exp), context);
        }

        bodySeq->expr = recurse(ctx, std::move(bodySeq->expr), context);
    }
    return body;
}

void checkItBlock(core::MutableContext ctx, ast::Expression *exp) {
    auto send = ast::cast_tree<ast::Send>(exp);
    if (send) {
        if (send->fun == core::Names::it() && send->args.size() == 1 && send->block != nullptr) {
            return;
        }
    }

    if (auto e = ctx.state.beginError(exp->loc, core::errors::Rewriter::NonItInTestEach)) {
        e.setHeader("Only `{}` blocks are allowed inside `{}`", "it", "test_each");
    }
}

unique_ptr<ast::Expression> prepareTestEachBody(core::MutableContext ctx, unique_ptr<ast::Expression> body,
                                                optional<ast::Expression *> context) {
    auto bodySeq = ast::cast_tree<ast::InsSeq>(body.get());
    if (bodySeq) {
        for (auto &exp : bodySeq->stats) {
            checkItBlock(ctx, exp.get());
            exp = recurse(ctx, std::move(exp), context);
        }

        checkItBlock(ctx, bodySeq->expr.get());
        bodySeq->expr = recurse(ctx, std::move(bodySeq->expr), context);
    } else {
        checkItBlock(ctx, body.get());
        body = recurse(ctx, std::move(body), context);
    }
    return body;
}

string to_s(core::Context ctx, unique_ptr<ast::Expression> &arg) {
    auto argLit = ast::cast_tree<ast::Literal>(arg.get());
    string argString;
    if (argLit != nullptr) {
        if (argLit->isString(ctx)) {
            return argLit->asString(ctx).show(ctx);
        } else if (argLit->isSymbol(ctx)) {
            return argLit->asSymbol(ctx).show(ctx);
        }
    }
    auto argConstant = ast::cast_tree<ast::UnresolvedConstantLit>(arg.get());
    if (argConstant != nullptr) {
        return argConstant->cnst.show(ctx);
    }
    return arg->toString(ctx);
}

// The 'context' here is any expressions which are put at the top of the 'it' blocks. If we've looked at an instance of
// `test_each`, then we need to somehow pull the variable being introduced by the `each` into the body of the method. We
// do that by taking
//
//   test_each(expr) do |var|
//
// and turning it into an assignment like
//
//   var = T.must(expr.collect.first)
//
// which will have both the correct type and the correct name. (The reason for the `collect` is that if `expr` is an
// array literal, then Sorbet would treat it as a tuple type, and `first` would be the first thing in the tuple,
// i.e. `[1,true].first` has the type `Integer`, but we're using this to stand in for an arbitrary member of the
// collection, not actually the first one: what we want is something with the type `T.any(Integer,TrueClass)` here. The
// `.collect` will give us an array instead of a tuple, then `first` will give us a nilable element, and then the `must`
// will remove the nilable.
unique_ptr<ast::Expression> makeContext(core::MutableContext ctx, unique_ptr<ast::Expression> blockArg,
                                        unique_ptr<ast::Expression> &as, optional<ast::Expression *> context) {
    ENFORCE(ast::isa_tree<ast::Array>(as.get()) || ast::isa_tree<ast::UnresolvedConstantLit>(as.get()));
    auto enumToList = ast::MK::Send0(as->loc, as->deepCopy(), core::Names::collect());
    auto firstOfList = ast::MK::Send0(as->loc, move(enumToList), core::Names::first());
    auto mustOfList =
        ast::MK::Send1(as->loc, ast::MK::UnresolvedConstant(as->loc, ast::MK::EmptyTree(), core::Names::Constants::T()),
                       core::Names::must(), move(firstOfList));
    auto assn = ast::MK::Assign(as->loc, move(blockArg), move(mustOfList));
    if (context.has_value()) {
        ast::InsSeq::STATS_store ins;
        ins.emplace_back(context.value()->deepCopy());
        assn = ast::MK::InsSeq(as->loc, std::move(ins), std::move(assn));
    }

    return assn;
}

unique_ptr<ast::Expression> runSingle(core::MutableContext ctx, ast::Send *send, optional<ast::Expression *> context) {
    if (send->block == nullptr) {
        return nullptr;
    }

    if (!send->recv->isSelfReference()) {
        return nullptr;
    }

    if (send->fun == core::Names::testEach() && send->args.size() == 1 && send->block != nullptr) {
        auto &expr = send->args.front();
        if (!ast::isa_tree<ast::Array>(expr.get()) && !ast::isa_tree<ast::UnresolvedConstantLit>(expr.get())) {
            if (auto e = ctx.state.beginError(expr->loc, core::errors::Rewriter::NonConstantTestEach)) {
                e.setHeader("`{}` can only be used with constants or array literals", "test_each");
            }

            return nullptr;
        }

        // if this is a test_each, build a binding for the variable used in iteration
        auto assn = makeContext(ctx, send->block->args.front()->deepCopy(), expr, context);

        ast::Send::ARGS_store args;
        args.emplace_back(move(expr));
        // reconstruct the send but with a modified body, making sure we pass the constructed assignment in
        return ast::MK::Send(send->loc, ast::MK::Self(send->loc), send->fun, std::move(args), send->flags,
                             ast::MK::Block(send->block->loc,
                                            prepareTestEachBody(ctx, std::move(send->block->body), assn.get()),
                                            std::move(send->block->args)));
    }

    if (send->args.empty() && (send->fun == core::Names::before() || send->fun == core::Names::after())) {
        auto name = send->fun == core::Names::after() ? core::Names::afterAngles() : core::Names::initialize();
        ConstantMover constantMover;
        send->block->body = ast::TreeMap::apply(ctx, constantMover, move(send->block->body));
        unique_ptr<ast::Expression> body = std::move(send->block->body);
        auto method =
            addSigVoid(ast::MK::Method0(send->loc, send->loc, name, prepareBody(ctx, std::move(body), context),
                                        ast::MethodDef::RewriterSynthesized));
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

        rhs.emplace_back(prepareBody(ctx, std::move(send->block->body), context));
        auto name = ast::MK::UnresolvedConstant(arg->loc, ast::MK::EmptyTree(),
                                                ctx.state.enterNameConstant("<describe '" + argString + "'>"));
        return ast::MK::Class(send->loc, send->loc, std::move(name), std::move(ancestors), std::move(rhs));
    } else if (send->fun == core::Names::it()) {
        ConstantMover constantMover;
        send->block->body = ast::TreeMap::apply(ctx, constantMover, move(send->block->body));
        auto name = ctx.state.enterNameUTF8("<it '" + argString + "'>");
        unique_ptr<ast::Expression> body = std::move(send->block->body);
        // if we have context, then it means we've got a binding to add to the top of 'it'-blocks here: add it to the
        // front of the body so we re-introduce that variable into scope
        if (context.has_value()) {
            ast::InsSeq::STATS_store ins;
            ins.emplace_back(context.value()->deepCopy());
            body = ast::MK::InsSeq(send->loc, std::move(ins), std::move(body));
        }
        auto method = addSigVoid(ast::MK::Method0(send->loc, send->loc, std::move(name),
                                                  prepareBody(ctx, std::move(body), context),
                                                  ast::MethodDef::RewriterSynthesized));
        method = ast::MK::InsSeq1(send->loc, send->args.front()->deepCopy(), move(method));
        return constantMover.addConstantsToExpression(send->loc, move(method));
    }

    return nullptr;
}

unique_ptr<ast::Expression> recurse(core::MutableContext ctx, unique_ptr<ast::Expression> body,
                                    optional<ast::Expression *> context) {
    auto bodySend = ast::cast_tree<ast::Send>(body.get());
    if (bodySend) {
        auto change = runSingle(ctx, bodySend, context);
        if (change) {
            return change;
        }
    }
    return body;
}

vector<unique_ptr<ast::Expression>> Minitest::run(core::MutableContext ctx, ast::Send *send) {
    vector<unique_ptr<ast::Expression>> stats;
    if (ctx.state.runningUnderAutogen) {
        return stats;
    }

    auto exp = runSingle(ctx, send, nullopt);
    if (exp != nullptr) {
        stats.emplace_back(std::move(exp));
    }
    return stats;
}

}; // namespace sorbet::rewriter
