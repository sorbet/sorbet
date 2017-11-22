#include <algorithm>

#include "../ast.h"
#include "Desugar.h"
#include "ast/ast.h"
#include "common/common.h"

namespace ruby_typer {
namespace ast {
namespace desugar {

using namespace std;

namespace {

unique_ptr<Expression> node2TreeImpl(core::Context ctx, unique_ptr<parser::Node> &what);

unique_ptr<Expression> mkSend(core::Loc loc, unique_ptr<Expression> &recv, core::NameRef fun, Send::ARGS_store &args,
                              u4 flags = 0) {
    auto send = make_unique<Send>(loc, move(recv), fun, args);
    send->flags = flags;
    return move(send);
}
unique_ptr<Expression> mkSend(core::Loc loc, unique_ptr<Expression> &&recv, core::NameRef fun, Send::ARGS_store &args,
                              u4 flags = 0) {
    return mkSend(loc, recv, fun, args, flags);
}

unique_ptr<Expression> mkSend1(core::Loc loc, unique_ptr<Expression> &recv, core::NameRef fun,
                               unique_ptr<Expression> &arg1) {
    Send::ARGS_store nargs;
    nargs.emplace_back(move(arg1));
    return make_unique<Send>(loc, move(recv), fun, nargs);
}

unique_ptr<Expression> mkSend1(core::Loc loc, unique_ptr<Expression> &&recv, core::NameRef fun,
                               unique_ptr<Expression> &&arg1) {
    return mkSend1(loc, recv, fun, arg1);
}

unique_ptr<Expression> mkSend1(core::Loc loc, unique_ptr<Expression> &recv, core::NameRef fun,
                               unique_ptr<Expression> &&arg1) {
    return mkSend1(loc, recv, fun, arg1);
}

unique_ptr<Expression> mkSend2(core::Loc loc, unique_ptr<Expression> &recv, core::NameRef fun,
                               unique_ptr<Expression> &arg1, unique_ptr<Expression> &arg2) {
    Send::ARGS_store nargs;
    nargs.emplace_back(move(arg1));
    nargs.emplace_back(move(arg2));
    return make_unique<Send>(loc, move(recv), fun, nargs);
}

unique_ptr<Expression> mkSend3(core::Loc loc, unique_ptr<Expression> &recv, core::NameRef fun,
                               unique_ptr<Expression> &arg1, unique_ptr<Expression> &arg2,
                               unique_ptr<Expression> &arg3) {
    Send::ARGS_store nargs;
    nargs.emplace_back(move(arg1));
    nargs.emplace_back(move(arg2));
    nargs.emplace_back(move(arg3));
    return make_unique<Send>(loc, move(recv), fun, nargs);
}

unique_ptr<Expression> mkSend0(core::Loc loc, unique_ptr<Expression> &recv, core::NameRef fun) {
    Send::ARGS_store nargs;
    return make_unique<Send>(loc, move(recv), fun, nargs);
}

unique_ptr<Expression> mkSend0(core::Loc loc, unique_ptr<Expression> &&recv, core::NameRef fun) {
    return mkSend0(loc, recv, fun);
}

unique_ptr<Expression> mkIdent(core::Loc loc, core::SymbolRef symbol) {
    return make_unique<Ident>(loc, symbol);
}

unique_ptr<Expression> mkLocal(core::Loc loc, core::NameRef name) {
    return make_unique<UnresolvedIdent>(loc, UnresolvedIdent::Local, name);
}

unique_ptr<Expression> cpRef(core::Loc loc, Reference &name) {
    if (UnresolvedIdent *nm = cast_tree<UnresolvedIdent>(&name))
        return make_unique<UnresolvedIdent>(loc, nm->kind, nm->name);
    if (Ident *id = cast_tree<Ident>(&name))
        return make_unique<Ident>(loc, id->symbol);
    Error::notImplemented();
}

unique_ptr<Expression> mkAssign(core::Loc loc, unique_ptr<Expression> &lhs, unique_ptr<Expression> &rhs) {
    return make_unique<Assign>(loc, move(lhs), move(rhs));
}

unique_ptr<Expression> mkAssign(core::Loc loc, core::NameRef name, unique_ptr<Expression> &rhs) {
    auto id = mkLocal(loc, name);
    return mkAssign(loc, id, rhs);
}

unique_ptr<Expression> mkAssign(core::Loc loc, core::NameRef name, unique_ptr<Expression> &&rhs) {
    return mkAssign(loc, name, rhs);
}

unique_ptr<Expression> mkIf(core::Loc loc, unique_ptr<Expression> &cond, unique_ptr<Expression> &thenp,
                            unique_ptr<Expression> &elsep) {
    return make_unique<If>(loc, move(cond), move(thenp), move(elsep));
}

unique_ptr<Expression> mkIf(core::Loc loc, unique_ptr<Expression> &&cond, unique_ptr<Expression> &&thenp,
                            unique_ptr<Expression> &&elsep) {
    return mkIf(loc, cond, thenp, elsep);
}

unique_ptr<Expression> mkEmptyTree(core::Loc loc) {
    return make_unique<EmptyTree>(loc);
}

unique_ptr<Expression> mkInsSeq(core::Loc loc, InsSeq::STATS_store &stats, unique_ptr<Expression> &&expr) {
    return make_unique<InsSeq>(loc, stats, move(expr));
}

unique_ptr<Expression> mkInsSeq1(core::Loc loc, unique_ptr<Expression> stat, unique_ptr<Expression> &&expr) {
    InsSeq::STATS_store stats;
    stats.emplace_back(move(stat));
    return make_unique<InsSeq>(loc, stats, move(expr));
}

unique_ptr<Expression> mkTrue(core::Loc loc) {
    return make_unique<BoolLit>(loc, true);
}

unique_ptr<Expression> mkFalse(core::Loc loc) {
    return make_unique<BoolLit>(loc, false);
}

pair<MethodDef::ARGS_store, unique_ptr<Expression>> desugarArgsAndBody(core::Context ctx, core::Loc loc,
                                                                       unique_ptr<parser::Node> &argnode,
                                                                       unique_ptr<parser::Node> &bodynode) {
    MethodDef::ARGS_store args;
    InsSeq::STATS_store destructures;

    if (auto *oargs = parser::cast_node<parser::Args>(argnode.get())) {
        args.reserve(oargs->args.size());
        for (auto &arg : oargs->args) {
            if (parser::Mlhs *lhs = parser::cast_node<parser::Mlhs>(arg.get())) {
                core::NameRef temporary =
                    ctx.state.freshNameUnique(core::UniqueNameKind::Desugar, core::Names::destructureArg());
                args.emplace_back(make_unique<UnresolvedIdent>(arg->loc, UnresolvedIdent::Local, temporary));
                unique_ptr<parser::Node> lvarNode = make_unique<parser::LVar>(arg->loc, temporary);
                unique_ptr<parser::Node> destructure = make_unique<parser::Masgn>(arg->loc, move(arg), move(lvarNode));
                destructures.emplace_back(node2TreeImpl(ctx, destructure));
            } else {
                args.emplace_back(node2TreeImpl(ctx, arg));
            }
        }
    } else if (argnode.get() == nullptr) {
        // do nothing
    } else {
        Error::notImplemented();
    }

    auto body = node2TreeImpl(ctx, bodynode);
    if (destructures.size() > 0) {
        core::Loc bodyLoc = body->loc;
        if (bodyLoc.is_none())
            bodyLoc = loc;
        body = make_unique<InsSeq>(loc, destructures, move(body));
    }

    return make_pair(move(args), move(body));
}

unique_ptr<Expression> desugarDString(core::Context ctx, core::Loc loc, parser::NodeVec &nodes) {
    if (nodes.empty()) {
        return make_unique<StringLit>(loc, core::Names::empty());
    }
    auto it = nodes.begin();
    auto end = nodes.end();
    unique_ptr<Expression> res;
    unique_ptr<Expression> first = node2TreeImpl(ctx, *it);
    if (cast_tree<StringLit>(first.get()) == nullptr) {
        res = mkSend0(loc, first, core::Names::to_s());
    } else {
        res = move(first);
    }
    ++it;
    for (; it != end; ++it) {
        auto &stat = *it;
        unique_ptr<Expression> narg = node2TreeImpl(ctx, stat);
        if (cast_tree<StringLit>(narg.get()) == nullptr) {
            narg = mkSend0(loc, narg, core::Names::to_s());
        }
        auto n = mkSend1(loc, move(res), core::Names::concat(), move(narg));
        res.reset(n.release());
    };
    return res;
}

unique_ptr<MethodDef> buildMethod(core::Context ctx, core::Loc loc, core::NameRef name,
                                  unique_ptr<parser::Node> &argnode, unique_ptr<parser::Node> &body) {
    auto argsAndBody = desugarArgsAndBody(ctx, loc, argnode, body);
    return make_unique<MethodDef>(loc, ctx.state.defn_todo(), name, argsAndBody.first, move(argsAndBody.second), false);
}

bool locReported = false;

unique_ptr<Expression> node2TreeImpl(core::Context ctx, unique_ptr<parser::Node> &what) {
    try {
        if (what.get() == nullptr) {
            return make_unique<EmptyTree>(core::Loc::none(0));
        }
        if (what->loc.is_none()) {
            DEBUG_ONLY(Error::check(false, "parse-tree node has no location: ", what->toString(ctx)));
        }
        unique_ptr<Expression> result;
        typecase(
            what.get(),
            [&](parser::And *a) {
                auto lhs = node2TreeImpl(ctx, a->left);
                if (auto i = cast_tree<Reference>(lhs.get())) {
                    auto cond = cpRef(what->loc, *i);
                    auto iff = mkIf(what->loc, move(cond), node2TreeImpl(ctx, a->right), move(lhs));
                    result.swap(iff);
                } else {
                    core::NameRef tempName =
                        ctx.state.freshNameUnique(core::UniqueNameKind::Desugar, core::Names::andAnd());
                    auto temp = mkAssign(what->loc, tempName, lhs);

                    auto iff = mkIf(what->loc, mkLocal(what->loc, tempName), node2TreeImpl(ctx, a->right),
                                    mkLocal(what->loc, tempName));
                    auto wrapped = mkInsSeq1(what->loc, move(temp), move(iff));

                    result.swap(wrapped);
                }
            },
            [&](parser::Or *a) {
                auto lhs = node2TreeImpl(ctx, a->left);
                if (auto i = cast_tree<Reference>(lhs.get())) {
                    auto cond = cpRef(what->loc, *i);
                    auto iff = mkIf(what->loc, move(cond), move(lhs), node2TreeImpl(ctx, a->right));
                    result.swap(iff);
                } else {
                    core::NameRef tempName =
                        ctx.state.freshNameUnique(core::UniqueNameKind::Desugar, core::Names::orOr());
                    auto temp = mkAssign(what->loc, tempName, lhs);

                    auto iff = mkIf(what->loc, mkLocal(what->loc, tempName), mkLocal(what->loc, tempName),
                                    node2TreeImpl(ctx, a->right));
                    auto wrapped = mkInsSeq1(what->loc, move(temp), move(iff));

                    result.swap(wrapped);
                }
            },
            [&](parser::AndAsgn *a) {
                auto recv = node2TreeImpl(ctx, a->left);
                auto arg = node2TreeImpl(ctx, a->right);
                if (auto s = cast_tree<Send>(recv.get())) {
                    InsSeq::STATS_store stats;
                    core::NameRef tempRecv = ctx.state.freshNameUnique(core::UniqueNameKind::Desugar, s->fun);
                    stats.emplace_back(mkAssign(what->loc, tempRecv, move(s->recv)));
                    Send::ARGS_store readArgs;
                    Send::ARGS_store assgnArgs;
                    for (auto &arg : s->args) {
                        core::Loc loc = arg->loc;
                        core::NameRef name = ctx.state.freshNameUnique(core::UniqueNameKind::Desugar, s->fun);
                        stats.emplace_back(mkAssign(loc, name, move(arg)));
                        readArgs.emplace_back(mkLocal(loc, name));
                        assgnArgs.emplace_back(mkLocal(loc, name));
                    }
                    assgnArgs.emplace_back(move(arg));
                    auto cond = mkSend(what->loc, mkLocal(what->loc, tempRecv), s->fun, readArgs, s->flags);
                    core::NameRef tempResult = ctx.state.freshNameUnique(core::UniqueNameKind::Desugar, s->fun);
                    stats.emplace_back(mkAssign(what->loc, tempResult, move(cond)));

                    auto body = mkSend(what->loc, mkLocal(what->loc, tempRecv), s->fun.addEq(ctx), assgnArgs, s->flags);
                    auto elsep = mkLocal(what->loc, tempResult);
                    auto iff = mkIf(what->loc, mkLocal(what->loc, tempResult), move(body), move(elsep));
                    auto wrapped = mkInsSeq(what->loc, stats, move(iff));
                    result.swap(wrapped);
                } else if (auto i = cast_tree<Reference>(recv.get())) {
                    auto cond = cpRef(what->loc, *i);
                    auto body = mkAssign(what->loc, recv, arg);
                    auto elsep = cpRef(what->loc, *i);
                    auto iff = mkIf(what->loc, cond, body, elsep);
                    result.swap(iff);
                } else {
                    Error::notImplemented();
                }
            },
            [&](parser::OrAsgn *a) {
                auto recv = node2TreeImpl(ctx, a->left);
                auto arg = node2TreeImpl(ctx, a->right);
                if (auto s = cast_tree<Send>(recv.get())) {
                    InsSeq::STATS_store stats;
                    core::NameRef tempRecv = ctx.state.freshNameUnique(core::UniqueNameKind::Desugar, s->fun);
                    stats.emplace_back(mkAssign(what->loc, tempRecv, move(s->recv)));
                    Send::ARGS_store readArgs;
                    Send::ARGS_store assgnArgs;
                    for (auto &arg : s->args) {
                        core::Loc loc = arg->loc;
                        core::NameRef name = ctx.state.freshNameUnique(core::UniqueNameKind::Desugar, s->fun);
                        stats.emplace_back(mkAssign(loc, name, move(arg)));
                        readArgs.emplace_back(mkLocal(loc, name));
                        assgnArgs.emplace_back(mkLocal(loc, name));
                    }
                    assgnArgs.emplace_back(move(arg));
                    auto cond = mkSend(what->loc, mkLocal(what->loc, tempRecv), s->fun, readArgs, s->flags);
                    core::NameRef tempResult = ctx.state.freshNameUnique(core::UniqueNameKind::Desugar, s->fun);
                    stats.emplace_back(mkAssign(what->loc, tempResult, move(cond)));

                    auto elsep =
                        mkSend(what->loc, mkLocal(what->loc, tempRecv), s->fun.addEq(ctx), assgnArgs, s->flags);
                    auto body = mkLocal(what->loc, tempResult);
                    auto iff = mkIf(what->loc, mkLocal(what->loc, tempResult), move(body), move(elsep));
                    auto wrapped = mkInsSeq(what->loc, stats, move(iff));
                    result.swap(wrapped);
                } else if (auto i = cast_tree<Reference>(recv.get())) {
                    auto cond = cpRef(what->loc, *i);
                    auto body = mkAssign(what->loc, recv, arg);
                    auto elsep = cpRef(what->loc, *i);
                    auto iff = mkIf(what->loc, cond, elsep, body);
                    result.swap(iff);

                } else {
                    Error::notImplemented();
                }
            },
            [&](parser::Send *a) {
                u4 flags = 0;
                auto rec = node2TreeImpl(ctx, a->receiver);
                if (cast_tree<EmptyTree>(rec.get()) != nullptr) {
                    rec = make_unique<Self>(what->loc, ctx.state.defn_todo());
                    flags |= Send::PRIVATE_OK;
                }
                Send::ARGS_store args;
                args.reserve(a->args.size());
                for (auto &stat : a->args) {
                    args.emplace_back(node2TreeImpl(ctx, stat));
                };

                auto send = mkSend(what->loc, rec, a->method, args, flags);
                result.swap(send);
            },
            [&](parser::CSend *a) {
                core::NameRef tempRecv =
                    ctx.state.freshNameUnique(core::UniqueNameKind::Desugar, core::Names::assignTemp());
                core::Loc recvLoc = a->receiver->loc;

                // NOTE(nelhage): We actually desugar into a call to `nil?`. If an
                // object has overridden `nil?`, this technically will not match
                // Ruby's behavior.

                auto assgn = mkAssign(recvLoc, tempRecv, node2TreeImpl(ctx, a->receiver));
                auto cond = mkSend0(a->loc, mkLocal(recvLoc, tempRecv), core::Names::nil_p());

                unique_ptr<parser::Node> sendNode = make_unique<parser::Send>(
                    a->loc, make_unique<parser::LVar>(recvLoc, tempRecv), a->method, move(a->args));
                auto send = node2TreeImpl(ctx, sendNode);

                unique_ptr<Expression> nil = make_unique<Nil>(a->loc);
                auto iff = mkIf(a->loc, cond, nil, send);
                InsSeq::STATS_store stats;
                stats.emplace_back(move(assgn));
                auto res = mkInsSeq(a->loc, stats, move(iff));
                result.swap(res);
            },
            [&](parser::Self *a) {
                unique_ptr<Expression> self = make_unique<Self>(what->loc, ctx.state.defn_todo());
                result.swap(self);
            },
            [&](parser::DString *a) {
                unique_ptr<Expression> res = desugarDString(ctx, a->loc, a->nodes);
                result.swap(res);
            },
            [&](parser::Symbol *a) {
                unique_ptr<Expression> res = make_unique<ast::SymbolLit>(what->loc, a->val);
                result.swap(res);
            },
            [&](parser::String *a) {
                unique_ptr<Expression> res = make_unique<StringLit>(what->loc, a->val);
                result.swap(res);
            },
            [&](parser::FileLiteral *a) {
                unique_ptr<Expression> res = make_unique<StringLit>(what->loc, core::Names::currentFile());
                result.swap(res);
            },
            [&](parser::Const *a) {
                auto scope = node2TreeImpl(ctx, a->scope);
                unique_ptr<Expression> res = make_unique<ConstantLit>(what->loc, move(scope), a->name);
                result.swap(res);
            },
            [&](parser::ConstLhs *a) {
                auto scope = node2TreeImpl(ctx, a->scope);
                unique_ptr<Expression> res = make_unique<ConstantLit>(what->loc, move(scope), a->name);
                result.swap(res);
            },
            [&](parser::Cbase *a) {
                unique_ptr<Expression> res = mkIdent(what->loc, core::GlobalState::defn_root());
                result.swap(res);
            },
            [&](parser::Begin *a) {
                if (a->stmts.size() > 0) {
                    InsSeq::STATS_store stats;
                    stats.reserve(a->stmts.size() - 1);
                    auto end = a->stmts.end();
                    --end;
                    for (auto it = a->stmts.begin(); it != end; ++it) {
                        auto &stat = *it;
                        stats.emplace_back(node2TreeImpl(ctx, stat));
                    };
                    auto &last = a->stmts.back();
                    auto expr = node2TreeImpl(ctx, last);
                    auto block = mkInsSeq(what->loc, stats, move(expr));
                    result.swap(block);
                } else {
                    unique_ptr<Expression> res = mkEmptyTree(what->loc);
                    result.swap(res);
                }
            },
            [&](parser::Kwbegin *a) {
                if (a->stmts.size() > 0) {
                    InsSeq::STATS_store stats;
                    stats.reserve(a->stmts.size() - 1);
                    auto end = a->stmts.end();
                    --end;
                    for (auto it = a->stmts.begin(); it != end; ++it) {
                        auto &stat = *it;
                        stats.emplace_back(node2TreeImpl(ctx, stat));
                    };
                    auto &last = a->stmts.back();
                    auto expr = node2TreeImpl(ctx, last);
                    auto block = mkInsSeq(what->loc, stats, move(expr));
                    result.swap(block);
                } else {
                    unique_ptr<Expression> res = mkEmptyTree(what->loc);
                    result.swap(res);
                }
            },
            [&](parser::Module *module) {
                ClassDef::RHS_store body;
                if (auto *a = parser::cast_node<parser::Begin>(module->body.get())) {
                    body.reserve(a->stmts.size());
                    for (auto &stat : a->stmts) {
                        body.emplace_back(node2TreeImpl(ctx, stat));
                    };
                } else {
                    body.emplace_back(node2TreeImpl(ctx, module->body));
                }
                ClassDef::ANCESTORS_store ancestors;
                unique_ptr<Expression> res =
                    make_unique<ClassDef>(what->loc, ctx.state.defn_todo(), node2TreeImpl(ctx, module->name), ancestors,
                                          body, ClassDefKind::Module);
                result.swap(res);
            },
            [&](parser::Class *claz) {
                ClassDef::RHS_store body;
                if (auto *a = parser::cast_node<parser::Begin>(claz->body.get())) {
                    body.reserve(a->stmts.size());
                    for (auto &stat : a->stmts) {
                        body.emplace_back(node2TreeImpl(ctx, stat));
                    };
                } else {
                    body.emplace_back(node2TreeImpl(ctx, claz->body));
                }
                ClassDef::ANCESTORS_store ancestors;
                if (claz->superclass == nullptr) {
                    ancestors.emplace_back(make_unique<Ident>(claz->loc, ctx.state.defn_todo()));
                } else {
                    ancestors.emplace_back(node2TreeImpl(ctx, claz->superclass));
                }

                unique_ptr<Expression> res =
                    make_unique<ClassDef>(what->loc, ctx.state.defn_todo(), node2TreeImpl(ctx, claz->name), ancestors,
                                          body, ClassDefKind::Class);
                result.swap(res);
            },
            [&](parser::Arg *arg) {
                unique_ptr<Expression> res = make_unique<UnresolvedIdent>(what->loc, UnresolvedIdent::Local, arg->name);
                result.swap(res);
            },
            [&](parser::Restarg *arg) {
                unique_ptr<Expression> res = make_unique<RestArg>(
                    what->loc, make_unique<UnresolvedIdent>(what->loc, UnresolvedIdent::Local, arg->name));
                result.swap(res);
            },
            [&](parser::Kwrestarg *arg) {
                unique_ptr<Expression> res = make_unique<RestArg>(
                    what->loc, make_unique<KeywordArg>(what->loc, make_unique<UnresolvedIdent>(
                                                                      what->loc, UnresolvedIdent::Local, arg->name)));
                result.swap(res);
            },
            [&](parser::Kwarg *arg) {
                unique_ptr<Expression> res = make_unique<KeywordArg>(
                    what->loc, make_unique<UnresolvedIdent>(what->loc, UnresolvedIdent::Local, arg->name));
                result.swap(res);
            },
            [&](parser::Blockarg *arg) {
                unique_ptr<Expression> res = make_unique<BlockArg>(
                    what->loc, make_unique<UnresolvedIdent>(what->loc, UnresolvedIdent::Local, arg->name));
                result.swap(res);
            },
            [&](parser::Kwoptarg *arg) {
                unique_ptr<Expression> res = make_unique<OptionalArg>(
                    what->loc,
                    make_unique<KeywordArg>(what->loc,
                                            make_unique<UnresolvedIdent>(what->loc, UnresolvedIdent::Local, arg->name)),
                    node2TreeImpl(ctx, arg->default_));
                result.swap(res);
            },
            [&](parser::Optarg *arg) {
                unique_ptr<Expression> res = make_unique<OptionalArg>(
                    what->loc, make_unique<UnresolvedIdent>(what->loc, UnresolvedIdent::Local, arg->name),
                    node2TreeImpl(ctx, arg->default_));
                result.swap(res);
            },
            [&](parser::Shadowarg *arg) {
                unique_ptr<Expression> res = make_unique<ShadowArg>(
                    what->loc, make_unique<UnresolvedIdent>(what->loc, UnresolvedIdent::Local, arg->name));
                result.swap(res);
            },
            [&](parser::DefMethod *method) {
                unique_ptr<Expression> res = buildMethod(ctx, what->loc, method->name, method->args, method->body);
                result.swap(res);
            },
            [&](parser::DefS *method) {
                parser::Self *self = parser::cast_node<parser::Self>(method->singleton.get());
                if (self == nullptr) {
                    ctx.state.errors.error(method->loc, core::ErrorClass::InvalidSingletonDef,
                                           "`def EXPRESSION.method' is only supported for `def self.method'");
                    unique_ptr<Expression> res = make_unique<EmptyTree>(what->loc);
                    result.swap(res);
                    return;
                }
                unique_ptr<MethodDef> meth = buildMethod(ctx, what->loc, method->name, method->args, method->body);
                meth->isSelf = true;
                unique_ptr<Expression> res(meth.release());
                result.swap(res);
            },
            [&](parser::Block *block) {
                auto recv = node2TreeImpl(ctx, block->send);
                Send *send;
                unique_ptr<Expression> res;
                if ((send = cast_tree<Send>(recv.get())) != nullptr) {
                    res.swap(recv);
                } else {
                    // This must have been a csend; That will have been desugared
                    // into an insseq with an If in the expression.
                    res.swap(recv);
                    InsSeq *is = cast_tree<InsSeq>(res.get());
                    Error::check(is != nullptr);
                    If *iff = cast_tree<If>(is->expr.get());
                    Error::check(iff != nullptr);
                    send = cast_tree<Send>(iff->elsep.get());
                    Error::check(send != nullptr);
                }
                auto argsAndBody = desugarArgsAndBody(ctx, block->loc, block->args, block->body);

                send->block = make_unique<Block>(what->loc, argsAndBody.first, move(argsAndBody.second));
                result.swap(res);
            },
            [&](parser::While *wl) {
                auto cond = node2TreeImpl(ctx, wl->cond);
                auto body = node2TreeImpl(ctx, wl->body);
                unique_ptr<Expression> res = make_unique<While>(what->loc, move(cond), move(body));
                result.swap(res);
            },
            [&](parser::WhilePost *wl) {
                auto cond = node2TreeImpl(ctx, wl->cond);
                auto body = node2TreeImpl(ctx, wl->body);
                unique_ptr<Expression> res = make_unique<While>(what->loc, move(cond), move(body));
                result.swap(res);
            },
            [&](parser::Until *wl) {
                auto cond = mkSend0(what->loc, node2TreeImpl(ctx, wl->cond), core::Names::bang());
                auto body = node2TreeImpl(ctx, wl->body);
                unique_ptr<Expression> res = make_unique<While>(what->loc, move(cond), move(body));
                result.swap(res);
            },
            [&](parser::UntilPost *wl) {
                auto cond = mkSend0(what->loc, node2TreeImpl(ctx, wl->cond), core::Names::bang());
                auto body = node2TreeImpl(ctx, wl->body);
                unique_ptr<Expression> res = make_unique<While>(what->loc, move(cond), move(body));
                result.swap(res);
            },
            [&](parser::Nil *wl) {
                unique_ptr<Expression> res = make_unique<Nil>(what->loc);
                result.swap(res);
            },
            [&](parser::IVar *var) {
                unique_ptr<Expression> res =
                    make_unique<UnresolvedIdent>(what->loc, UnresolvedIdent::Instance, var->name);
                result.swap(res);
            },
            [&](parser::LVar *var) {
                unique_ptr<Expression> res = make_unique<UnresolvedIdent>(what->loc, UnresolvedIdent::Local, var->name);
                result.swap(res);
            },
            [&](parser::GVar *var) {
                unique_ptr<Expression> res =
                    make_unique<UnresolvedIdent>(what->loc, UnresolvedIdent::Global, var->name);
                result.swap(res);
            },
            [&](parser::CVar *var) {
                unique_ptr<Expression> res = make_unique<UnresolvedIdent>(what->loc, UnresolvedIdent::Class, var->name);
                result.swap(res);
            },
            [&](parser::LVarLhs *var) {
                unique_ptr<Expression> res = make_unique<UnresolvedIdent>(what->loc, UnresolvedIdent::Local, var->name);
                result.swap(res);
            },
            [&](parser::GVarLhs *var) {
                unique_ptr<Expression> res =
                    make_unique<UnresolvedIdent>(what->loc, UnresolvedIdent::Global, var->name);
                result.swap(res);
            },
            [&](parser::CVarLhs *var) {
                unique_ptr<Expression> res = make_unique<UnresolvedIdent>(what->loc, UnresolvedIdent::Class, var->name);
                result.swap(res);
            },
            [&](parser::IVarLhs *var) {
                unique_ptr<Expression> res =
                    make_unique<UnresolvedIdent>(what->loc, UnresolvedIdent::Instance, var->name);
                result.swap(res);
            },
            [&](parser::Assign *asgn) {
                auto lhs = node2TreeImpl(ctx, asgn->lhs);
                auto rhs = node2TreeImpl(ctx, asgn->rhs);
                auto res = mkAssign(what->loc, lhs, rhs);
                result.swap(res);
            },
            [&](parser::Super *super) {
                Send::ARGS_store args;
                args.reserve(super->args.size());
                for (auto &stat : super->args) {
                    args.emplace_back(node2TreeImpl(ctx, stat));
                };

                unique_ptr<Expression> res = make_unique<Send>(
                    what->loc, make_unique<Self>(what->loc, ctx.state.defn_todo()), core::Names::super(), args);
                result.swap(res);
            },
            [&](parser::ZSuper *zuper) {
                Send::ARGS_store args;
                args.emplace_back(make_unique<ZSuperArgs>(zuper->loc));
                unique_ptr<Expression> res = make_unique<Send>(
                    what->loc, make_unique<Self>(what->loc, ctx.state.defn_todo()), core::Names::super(), args);
                result.swap(res);
            },
            [&](parser::Integer *integer) {
                int64_t val;
                try {
                    val = stol(integer->val);
                } catch (std::out_of_range &) {
                    val = 0;
                    ctx.state.errors.error(integer->loc, core::ErrorClass::IntegerOutOfRange,
                                           "Unsupported large integer literal: {}", integer->val);
                } catch (std::invalid_argument &) {
                    val = 0;
                    ctx.state.errors.error(integer->loc, core::ErrorClass::IntegerOutOfRange,
                                           "Unsupported integer literal: {}", integer->val);
                }

                unique_ptr<Expression> res = make_unique<IntLit>(what->loc, val);
                result.swap(res);
            },
            [&](parser::Float *floatNode) {
                double val;
                try {
                    val = stod(floatNode->val);
                    if (isinf(val)) {
                        val = std::numeric_limits<double>::quiet_NaN();
                        ctx.state.errors.error(floatNode->loc, core::ErrorClass::FloatOutOfRange,
                                               "Unsupported large float literal: {}", floatNode->val);
                    }
                } catch (std::out_of_range &) {
                    val = std::numeric_limits<double>::quiet_NaN();
                    ctx.state.errors.error(floatNode->loc, core::ErrorClass::FloatOutOfRange,
                                           "Unsupported large float literal: {}", floatNode->val);
                } catch (std::invalid_argument &) {
                    val = std::numeric_limits<double>::quiet_NaN();
                    ctx.state.errors.error(floatNode->loc, core::ErrorClass::FloatOutOfRange,
                                           "Unsupported float literal: {}", floatNode->val);
                }

                unique_ptr<Expression> res = make_unique<FloatLit>(what->loc, val);
                result.swap(res);
            },
            [&](parser::Array *array) {
                Array::ENTRY_store elems;
                elems.reserve(array->elts.size());
                for (auto &stat : array->elts) {
                    elems.emplace_back(node2TreeImpl(ctx, stat));
                };

                unique_ptr<Expression> res = make_unique<Array>(what->loc, elems);
                result.swap(res);
            },
            [&](parser::Hash *hash) {
                Hash::ENTRY_store keys;
                Hash::ENTRY_store values;
                keys.reserve(hash->pairs.size());   // overapproximation in case there are KwSpats
                values.reserve(hash->pairs.size()); // overapproximation in case there are KwSpats
                unique_ptr<Expression> lastMerge;

                for (auto &pairAsExpression : hash->pairs) {
                    parser::Pair *pair = parser::cast_node<parser::Pair>(pairAsExpression.get());
                    if (pair != nullptr) {
                        auto key = node2TreeImpl(ctx, pair->key);
                        auto value = node2TreeImpl(ctx, pair->value);
                        keys.emplace_back(move(key));
                        values.emplace_back(move(value));
                    } else {
                        parser::Kwsplat *splat = parser::cast_node<parser::Kwsplat>(pairAsExpression.get());
                        Error::check(splat);

                        // Desguar
                        //   {a: 'a', **x, remaining}
                        // into
                        //   {a: 'a'}.merge(x).merge(remaining)
                        if (keys.size() == 0) {
                            if (lastMerge != nullptr) {
                                lastMerge = mkSend1(what->loc, lastMerge, core::Names::merge(),
                                                    node2TreeImpl(ctx, splat->expr));
                            } else {
                                lastMerge = node2TreeImpl(ctx, splat->expr);
                            }
                        } else {
                            unique_ptr<Expression> current = make_unique<Hash>(what->loc, keys, values);
                            keys.clear();
                            values.clear();
                            if (lastMerge != nullptr) {
                                lastMerge = mkSend1(what->loc, lastMerge, core::Names::merge(), current);
                            } else {
                                lastMerge = move(current);
                            }
                            lastMerge =
                                mkSend1(what->loc, lastMerge, core::Names::merge(), node2TreeImpl(ctx, splat->expr));
                        }
                    }
                };

                unique_ptr<Expression> res;
                if (keys.size() == 0) {
                    if (lastMerge != nullptr) {
                        res = move(lastMerge);
                    } else {
                        // Empty array
                        res = make_unique<Hash>(what->loc, keys, values);
                    }
                } else {
                    res = make_unique<Hash>(what->loc, keys, values);
                    if (lastMerge != nullptr) {
                        res = mkSend1(what->loc, lastMerge, core::Names::merge(), res);
                    }
                }

                result.swap(res);
            },
            [&](parser::IRange *ret) {
                core::NameRef range_name = core::GlobalState::defn_Range().info(ctx).name;
                unique_ptr<Expression> range = make_unique<ConstantLit>(what->loc, mkEmptyTree(what->loc), range_name);
                auto from = node2TreeImpl(ctx, ret->from);
                auto to = node2TreeImpl(ctx, ret->to);
                auto send = mkSend2(what->loc, range, core::Names::new_(), from, to);
                result.swap(send);
            },
            [&](parser::ERange *ret) {
                unique_ptr<Expression> range = mkIdent(what->loc, core::GlobalState::defn_Range());
                auto from = node2TreeImpl(ctx, ret->from);
                auto to = node2TreeImpl(ctx, ret->to);
                auto true_ = mkTrue(what->loc);
                auto send = mkSend3(what->loc, range, core::Names::new_(), from, to, true_);
                result.swap(send);
            },
            [&](parser::Regexp *regexpNode) {
                unique_ptr<Expression> regexp = mkIdent(what->loc, core::GlobalState::defn_Regexp());
                auto regex = desugarDString(ctx, what->loc, regexpNode->regex);
                auto optsNode = node2TreeImpl(ctx, regexpNode->opts);
                auto optString = cast_tree<StringLit>(optsNode.get());
                Error::check(optString != nullptr);
                unique_ptr<Expression> send;
                if (optString->value == core::Names::empty()) {
                    send = mkSend1(what->loc, regexp, core::Names::new_(), regex);
                } else {
                    send = mkSend2(what->loc, regexp, core::Names::new_(), regex, optsNode);
                }
                result.swap(send);
            },
            [&](parser::Regopt *a) {
                unique_ptr<Expression> res = make_unique<StringLit>(what->loc, a->opts);
                result.swap(res);
            },
            [&](parser::Return *ret) {
                if (ret->exprs.size() > 1) {
                    Array::ENTRY_store elems;
                    elems.reserve(ret->exprs.size());
                    for (auto &stat : ret->exprs) {
                        elems.emplace_back(node2TreeImpl(ctx, stat));
                    };
                    unique_ptr<Expression> arr = make_unique<Array>(what->loc, elems);
                    unique_ptr<Expression> res = make_unique<Return>(what->loc, move(arr));
                    result.swap(res);
                } else if (ret->exprs.size() == 1) {
                    unique_ptr<Expression> res = make_unique<Return>(what->loc, node2TreeImpl(ctx, ret->exprs[0]));
                    result.swap(res);
                } else {
                    unique_ptr<Expression> res = make_unique<Return>(what->loc, mkEmptyTree(what->loc));
                    result.swap(res);
                }
            },
            [&](parser::Break *ret) {
                if (ret->exprs.size() > 1) {
                    Array::ENTRY_store elems;
                    elems.reserve(ret->exprs.size());
                    for (auto &stat : ret->exprs) {
                        elems.emplace_back(node2TreeImpl(ctx, stat));
                    };
                    unique_ptr<Expression> arr = make_unique<Array>(what->loc, elems);
                    unique_ptr<Expression> res = make_unique<Break>(what->loc, move(arr));
                    result.swap(res);
                } else if (ret->exprs.size() == 1) {
                    unique_ptr<Expression> res = make_unique<Break>(what->loc, node2TreeImpl(ctx, ret->exprs[0]));
                    result.swap(res);
                } else {
                    unique_ptr<Expression> res = make_unique<Break>(what->loc, mkEmptyTree(what->loc));
                    result.swap(res);
                }
            },
            [&](parser::Next *ret) {
                if (ret->exprs.size() > 1) {
                    Array::ENTRY_store elems;
                    elems.reserve(ret->exprs.size());
                    for (auto &stat : ret->exprs) {
                        elems.emplace_back(node2TreeImpl(ctx, stat));
                    };
                    unique_ptr<Expression> arr = make_unique<Array>(what->loc, elems);
                    unique_ptr<Expression> res = make_unique<Next>(what->loc, move(arr));
                    result.swap(res);
                } else if (ret->exprs.size() == 1) {
                    unique_ptr<Expression> res = make_unique<Next>(what->loc, node2TreeImpl(ctx, ret->exprs[0]));
                    result.swap(res);
                } else {
                    unique_ptr<Expression> res = make_unique<Next>(what->loc, mkEmptyTree(what->loc));
                    result.swap(res);
                }
            },
            [&](parser::Yield *ret) {
                if (ret->exprs.size() > 1) {
                    Array::ENTRY_store elems;
                    elems.reserve(ret->exprs.size());
                    for (auto &stat : ret->exprs) {
                        elems.emplace_back(node2TreeImpl(ctx, stat));
                    };
                    unique_ptr<Expression> arr = make_unique<Array>(what->loc, elems);
                    unique_ptr<Expression> res = make_unique<Yield>(what->loc, move(arr));
                    result.swap(res);
                } else if (ret->exprs.size() == 1) {
                    unique_ptr<Expression> res = make_unique<Yield>(what->loc, node2TreeImpl(ctx, ret->exprs[0]));
                    result.swap(res);
                } else {
                    unique_ptr<Expression> res = make_unique<Yield>(what->loc, mkEmptyTree(what->loc));
                    result.swap(res);
                }
            },
            [&](parser::Rescue *rescue) {
                Rescue::RESCUE_CASE_store cases;
                for (auto &node : rescue->rescue) {
                    unique_ptr<Expression> rescueCaseExpr = node2TreeImpl(ctx, node);
                    auto rescueCase = cast_tree<ast::RescueCase>(rescueCaseExpr.get());
                    DEBUG_ONLY(Error::check(rescueCase != nullptr));
                    cases.emplace_back(rescueCase);
                    rescueCaseExpr.release();
                }
                unique_ptr<Expression> res = make_unique<Rescue>(what->loc, node2TreeImpl(ctx, rescue->body), cases,
                                                                 node2TreeImpl(ctx, rescue->else_));
                result.swap(res);
            },
            [&](parser::Resbody *resbody) {
                RescueCase::EXCEPTION_store exceptions;
                auto exceptionsExpr = node2TreeImpl(ctx, resbody->exception);
                if (cast_tree<EmptyTree>(exceptionsExpr.get()) != nullptr) {
                    // No exceptions captured
                } else {
                    auto exceptionsArray = cast_tree<ast::Array>(exceptionsExpr.get());
                    DEBUG_ONLY(Error::check(exceptionsArray != nullptr));

                    for (auto &elem : exceptionsArray->elems) {
                        exceptions.emplace_back(move(elem));
                    }
                }

                unique_ptr<Expression> res = make_unique<RescueCase>(
                    what->loc, exceptions, node2TreeImpl(ctx, resbody->var), node2TreeImpl(ctx, resbody->body));
                result.swap(res);
            },
            [&](parser::If *a) {
                auto cond = node2TreeImpl(ctx, a->condition);
                auto thenp = node2TreeImpl(ctx, a->then_);
                auto elsep = node2TreeImpl(ctx, a->else_);
                auto iff = mkIf(what->loc, cond, thenp, elsep);
                result.swap(iff);
            },
            [&](parser::Masgn *masgn) {
                parser::Mlhs *lhs = parser::cast_node<parser::Mlhs>(masgn->lhs.get());
                Error::check(lhs != nullptr);
                core::NameRef tempName =
                    ctx.state.freshNameUnique(core::UniqueNameKind::Desugar, core::Names::assignTemp());
                InsSeq::STATS_store stats;
                stats.emplace_back(mkAssign(what->loc, tempName, node2TreeImpl(ctx, masgn->rhs)));
                int i = 0;
                for (auto &c : lhs->exprs) {
                    unique_ptr<Expression> lh = node2TreeImpl(ctx, c);
                    if (ast::Send *snd = cast_tree<ast::Send>(lh.get())) {
                        Error::check(snd->args.size() == 0);
                        unique_ptr<Expression> getElement =
                            mkSend1(what->loc, mkLocal(what->loc, tempName), core::Names::squareBrackets(),
                                    make_unique<IntLit>(what->loc, i));
                        snd->args.emplace_back(move(getElement));
                        stats.emplace_back(move(lh));
                    } else if (cast_tree<ast::Reference>(lh.get()) != nullptr ||
                               cast_tree<ast::ConstantLit>(lh.get()) != nullptr) {
                        auto access = mkSend1(what->loc, mkLocal(what->loc, tempName), core::Names::squareBrackets(),
                                              make_unique<IntLit>(what->loc, i));
                        unique_ptr<Expression> assign = mkAssign(what->loc, lh, access);
                        stats.emplace_back(move(assign));
                    } else if (ast::NotSupported *snd = cast_tree<ast::NotSupported>(lh.get())) {
                        stats.emplace_back(move(lh));
                    } else {
                        Error::notImplemented();
                    }
                    i++;
                }
                unique_ptr<Expression> res = mkInsSeq(what->loc, stats, mkLocal(what->loc, tempName));
                result.swap(res);
            },
            [&](parser::True *t) {
                auto res = mkTrue(what->loc);
                result.swap(res);
            },
            [&](parser::False *t) {
                auto res = mkFalse(what->loc);
                result.swap(res);
            },
            [&](parser::Case *c) {
                unique_ptr<Expression> assign;
                core::NameRef temp(0);
                core::Loc cloc;

                if (c->condition != nullptr) {
                    cloc = c->condition->loc;
                    temp = ctx.state.freshNameUnique(core::UniqueNameKind::Desugar, core::Names::assignTemp());
                    assign = mkAssign(cloc, temp, node2TreeImpl(ctx, c->condition));
                }
                unique_ptr<Expression> res = node2TreeImpl(ctx, c->else_);
                for (auto it = c->whens.rbegin(); it != c->whens.rend(); ++it) {
                    auto when = parser::cast_node<parser::When>(it->get());
                    Error::check(when != nullptr);
                    unique_ptr<Expression> cond;
                    for (auto &cnode : when->patterns) {
                        auto ctree = node2TreeImpl(ctx, cnode);
                        unique_ptr<Expression> test;
                        if (temp.exists()) {
                            auto local = mkLocal(cloc, temp);
                            test = mkSend1(cnode->loc, local, core::Names::tripleEq(), ctree);
                        } else {
                            test.swap(ctree);
                        }
                        if (cond == nullptr) {
                            cond.swap(test);
                        } else {
                            cond = make_unique<If>(test->loc, move(test), mkTrue(test->loc), move(cond));
                        }
                    }
                    res = make_unique<If>(when->loc, move(cond), node2TreeImpl(ctx, when->body), move(res));
                }
                if (assign != nullptr) {
                    res = mkInsSeq1(c->loc, move(assign), move(res));
                }
                result.swap(res);
            },
            [&](parser::Node *a) {
                ctx.state.errors.error(what->loc, core::ErrorClass::UnsupportedNode, "Unsupported node type {}",
                                       a->nodeName());
                result.reset(new NotSupported(what->loc, a->nodeName()));
            });
        Error::check(result.get());
        return result;
    } catch (...) {
        if (!locReported) {
            locReported = true;
            ctx.state.errors.error(what->loc, core::ErrorClass::Internal,
                                   "Failed to process tree (backtrace is above)");
        }
        throw;
    }
}
} // namespace

unique_ptr<Expression> node2Tree(core::Context ctx, unique_ptr<parser::Node> &what) {
    try {
        auto result = node2TreeImpl(ctx, what);
        auto verifiedResult = Verifier::run(ctx, move(result));
        return verifiedResult;
    } catch (...) {
        locReported = false;
        throw;
    }
}
} // namespace desugar
} // namespace ast
} // namespace ruby_typer
