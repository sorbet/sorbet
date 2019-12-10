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

    auto bodySeq = ast::cast_tree<ast::InsSeq>(body.get());
    if (bodySeq) {
        for (auto &exp : bodySeq->stats) {
            exp = recurse(ctx, std::move(exp), context);
        }

        bodySeq->expr = recurse(ctx, std::move(bodySeq->expr), context);
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

unique_ptr<ast::Expression> makeContext(core::MutableContext ctx, unique_ptr<ast::Expression> blockArg,
                                        unique_ptr<ast::Expression> &as, optional<ast::Expression *> context) {
    ENFORCE(ast::isa_tree<ast::Array>(as.get()));
    auto collect = ctx.state.enterNameUTF8("collect");
    auto enumToList = ast::MK::Send0(as->loc, as->deepCopy(), collect);
    auto firstOfList = ast::MK::Send0(as->loc, move(enumToList), core::Names::first());
    auto mustOfList =
        ast::MK::Send1(as->loc, ast::MK::UnresolvedConstant(as->loc, ast::MK::EmptyTree(), core::Names::Constants::T()),
                       core::Names::must(), move(firstOfList));
    auto assn = ast::MK::Assign(as->loc, move(blockArg), move(mustOfList));
    if (auto &c = context) {
        ast::InsSeq::STATS_store ins;
        ins.emplace_back((*c)->deepCopy());
        assn = ast::MK::InsSeq(as->loc, std::move(ins), std::move(assn));
    }

    return assn;
}

unique_ptr<ast::Expression> runSingle(core::MutableContext ctx, ast::Send *send, optional<ast::Expression *> context) {
    if (send->block == nullptr) {
        return nullptr;
    }

    if (!send->recv->isSelfReference()) {
        if (send->fun == core::Names::each() && send->args.empty() && ast::isa_tree<ast::Array>(send->recv.get()) &&
            send->block != nullptr && send->block->args.size() == 1) {
            auto assn = makeContext(ctx, send->block->args.front()->deepCopy(), send->recv, context);

            ast::Send::ARGS_store noArgs;
            return ast::MK::Send(send->loc, std::move(send->recv), send->fun, std::move(noArgs), send->flags,
                                 ast::MK::Block(send->block->loc,
                                                prepareBody(ctx, std::move(send->block->body), assn.get()),
                                                std::move(send->block->args)));
        }
        return nullptr;
    }

    if (send->args.empty() && (send->fun == core::Names::before() || send->fun == core::Names::after())) {
        auto name = send->fun == core::Names::after() ? core::Names::afterAngles() : core::Names::initialize();
        ConstantMover constantMover;
        send->block->body = ast::TreeMap::apply(ctx, constantMover, move(send->block->body));
        unique_ptr<ast::Expression> body = std::move(send->block->body);
        if (auto &c = context) {
            ast::InsSeq::STATS_store ins;
            ins.emplace_back((*c)->deepCopy());
            body = ast::MK::InsSeq(send->loc, std::move(ins), std::move(body));
        }
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

        unique_ptr<ast::Expression> body = std::move(send->block->body);
        if (auto &c = context) {
            ast::InsSeq::STATS_store ins;
            ins.emplace_back((*c)->deepCopy());
            if (const auto &other_ins = ast::cast_tree<ast::InsSeq>(body.get())) {
                for (auto &i : other_ins->stats) {
                    ins.emplace_back(move(i));
                }
                body = ast::MK::InsSeq(body->loc, std::move(ins), std::move(other_ins->expr));
            } else {
                body = ast::MK::InsSeq(send->loc, std::move(ins), std::move(body));
            }
        }

        rhs.emplace_back(prepareBody(ctx, std::move(body), context));
        auto name = ast::MK::UnresolvedConstant(arg->loc, ast::MK::EmptyTree(),
                                                ctx.state.enterNameConstant("<describe '" + argString + "'>"));
        return ast::MK::Class(send->loc, send->loc, std::move(name), std::move(ancestors), std::move(rhs));
    } else if (send->fun == core::Names::it()) {
        ConstantMover constantMover;
        send->block->body = ast::TreeMap::apply(ctx, constantMover, move(send->block->body));
        auto name = ctx.state.enterNameUTF8("<it '" + argString + "'>");
        unique_ptr<ast::Expression> body = std::move(send->block->body);
        if (auto &c = context) {
            ast::InsSeq::STATS_store ins;
            ins.emplace_back((*c)->deepCopy());
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
