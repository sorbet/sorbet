#include <algorithm>

#include "../ast.h"
#include "Desugar.h"
#include "ast/ast.h"
#include "ast/verifier/verifier.h"
#include "common/common.h"

namespace ruby_typer {
namespace ast {
namespace desugar {

using namespace std;

namespace {

unique_ptr<Expression> node2TreeImpl(core::Context ctx, unique_ptr<parser::Node> &what);

unique_ptr<Expression> mkSend(core::Loc loc, unique_ptr<Expression> recv, core::NameRef fun, Send::ARGS_store args,
                              u4 flags = 0, unique_ptr<Block> blk = nullptr) {
    auto send = make_unique<Send>(loc, move(recv), fun, move(args), move(blk));
    send->flags = flags;
    return move(send);
}

unique_ptr<Expression> mkSend1(core::Loc loc, unique_ptr<Expression> recv, core::NameRef fun,
                               unique_ptr<Expression> arg1) {
    Send::ARGS_store nargs;
    nargs.emplace_back(move(arg1));
    return make_unique<Send>(loc, move(recv), move(fun), move(nargs));
}

unique_ptr<Expression> mkSend2(core::Loc loc, unique_ptr<Expression> recv, core::NameRef fun,
                               unique_ptr<Expression> arg1, unique_ptr<Expression> arg2) {
    Send::ARGS_store nargs;
    nargs.emplace_back(move(arg1));
    nargs.emplace_back(move(arg2));
    return make_unique<Send>(loc, move(recv), move(fun), move(nargs));
}

unique_ptr<Expression> mkSend3(core::Loc loc, unique_ptr<Expression> recv, core::NameRef fun,
                               unique_ptr<Expression> arg1, unique_ptr<Expression> arg2, unique_ptr<Expression> arg3) {
    Send::ARGS_store nargs;
    nargs.emplace_back(move(arg1));
    nargs.emplace_back(move(arg2));
    nargs.emplace_back(move(arg3));
    return make_unique<Send>(loc, move(recv), move(fun), move(nargs));
}

unique_ptr<Expression> mkSend0(core::Loc loc, unique_ptr<Expression> recv, core::NameRef fun) {
    Send::ARGS_store nargs;
    return make_unique<Send>(loc, move(recv), move(fun), move(nargs));
}

unique_ptr<Expression> mkIdent(core::Loc loc, core::SymbolRef symbol) {
    return make_unique<Ident>(loc, symbol);
}

unique_ptr<Expression> mkLocal(core::Loc loc, core::NameRef name) {
    return make_unique<UnresolvedIdent>(loc, UnresolvedIdent::Local, name);
}

unique_ptr<Expression> cpRef(core::Loc loc, Reference &name) {
    if (UnresolvedIdent *nm = cast_tree<UnresolvedIdent>(&name)) {
        return make_unique<UnresolvedIdent>(loc, nm->kind, nm->name);
    }
    if (Ident *id = cast_tree<Ident>(&name)) {
        return make_unique<Ident>(loc, id->symbol);
    }
    Error::notImplemented();
}

unique_ptr<Expression> mkAssign(core::Loc loc, unique_ptr<Expression> lhs, unique_ptr<Expression> rhs) {
    if (auto *s = cast_tree<ast::Send>(lhs.get())) {
        s->args.emplace_back(move(rhs));
        return lhs;
    }

    return make_unique<Assign>(loc, move(lhs), move(rhs));
}

unique_ptr<Expression> mkAssign(core::Loc loc, core::NameRef name, unique_ptr<Expression> rhs) {
    return mkAssign(loc, mkLocal(loc, name), move(rhs));
}

unique_ptr<Expression> mkIf(core::Loc loc, unique_ptr<Expression> cond, unique_ptr<Expression> thenp,
                            unique_ptr<Expression> elsep) {
    return make_unique<If>(loc, move(cond), move(thenp), move(elsep));
}

unique_ptr<Expression> mkEmptyTree(core::Loc loc) {
    return make_unique<EmptyTree>(loc);
}

unique_ptr<Expression> mkSelf(core::Loc loc) {
    return make_unique<Self>(loc, core::GlobalState::defn_todo());
}

unique_ptr<Expression> mkInsSeq(core::Loc loc, InsSeq::STATS_store stats, unique_ptr<Expression> expr) {
    return make_unique<InsSeq>(loc, move(stats), move(expr));
}

unique_ptr<Expression> mkSplat(core::Loc loc, unique_ptr<Expression> arg) {
    auto to_a = mkSend0(loc, move(arg), core::Names::to_a());
    return mkSend1(loc, mkIdent(loc, core::GlobalState::defn_Magic()), core::Names::splat(), move(to_a));
}

unique_ptr<Expression> mkInsSeq1(core::Loc loc, unique_ptr<Expression> stat, unique_ptr<Expression> expr) {
    InsSeq::STATS_store stats;
    stats.emplace_back(move(stat));
    return make_unique<InsSeq>(loc, move(stats), move(expr));
}

unique_ptr<Expression> mkTrue(core::Loc loc) {
    return make_unique<BoolLit>(loc, true);
}

unique_ptr<Expression> mkFalse(core::Loc loc) {
    return make_unique<BoolLit>(loc, false);
}

unique_ptr<Expression> mkConstant(core::Loc loc, unique_ptr<Expression> scope, core::NameRef name) {
    return make_unique<ConstantLit>(loc, move(scope), name);
}

unique_ptr<Expression> mkInt(core::Loc loc, int64_t val) {
    return make_unique<IntLit>(loc, val);
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
    if (!destructures.empty()) {
        core::Loc bodyLoc = body->loc;
        if (bodyLoc.is_none()) {
            bodyLoc = loc;
        }
        body = make_unique<InsSeq>(loc, move(destructures), move(body));
    }

    return make_pair(move(args), move(body));
}

unique_ptr<Expression> desugarDString(core::Context ctx, core::Loc loc, parser::NodeVec nodes) {
    if (nodes.empty()) {
        return make_unique<StringLit>(loc, core::Names::empty());
    }
    auto it = nodes.begin();
    auto end = nodes.end();
    unique_ptr<Expression> res;
    unique_ptr<Expression> first = node2TreeImpl(ctx, *it);
    if (cast_tree<StringLit>(first.get()) == nullptr) {
        res = mkSend0(loc, move(first), core::Names::to_s());
    } else {
        res = move(first);
    }
    ++it;
    for (; it != end; ++it) {
        auto &stat = *it;
        unique_ptr<Expression> narg = node2TreeImpl(ctx, stat);
        if (cast_tree<StringLit>(narg.get()) == nullptr) {
            narg = mkSend0(loc, move(narg), core::Names::to_s());
        }
        auto n = mkSend1(loc, move(res), core::Names::concat(), move(narg));
        res.reset(n.release());
    };
    return res;
}

unique_ptr<MethodDef> buildMethod(core::Context ctx, core::Loc loc, core::NameRef name,
                                  unique_ptr<parser::Node> &argnode, unique_ptr<parser::Node> &body) {
    auto argsAndBody = desugarArgsAndBody(ctx, loc, argnode, body);
    return make_unique<MethodDef>(loc, ctx.state.defn_todo(), name, move(argsAndBody.first), move(argsAndBody.second),
                                  false);
}

unique_ptr<Block> node2Proc(core::Context ctx, unique_ptr<parser::Node> node) {
    if (node == nullptr)
        return nullptr;

    auto expr = node2TreeImpl(ctx, node);
    core::Loc loc = expr->loc;
    core::NameRef temp = ctx.state.freshNameUnique(core::UniqueNameKind::Desugar, core::Names::blockPassTemp());

    if (auto sym = cast_tree<SymbolLit>(expr.get())) {
        // &:foo => {|temp| temp.foo() }
        MethodDef::ARGS_store args;
        args.emplace_back(mkLocal(loc, temp));
        unique_ptr<Expression> recv = mkLocal(loc, temp);
        unique_ptr<Expression> body = mkSend0(loc, move(recv), sym->name);
        return make_unique<Block>(loc, move(args), move(body));
    }

    // &foo => {|*args| foo.to_proc.call(*args) }
    auto proc = mkSend0(loc, move(expr), core::Names::to_proc());
    MethodDef::ARGS_store args;
    unique_ptr<Expression> rest =
        make_unique<RestArg>(loc, unique_ptr<Reference>(cast_tree<Reference>(mkLocal(loc, temp).release())));
    args.emplace_back(move(rest));
    unique_ptr<Expression> body = mkSend1(loc, move(proc), core::Names::call(), mkSplat(loc, mkLocal(loc, temp)));
    return make_unique<Block>(loc, move(args), move(body));
}

// Desugar a multi-assignment
//
// TODO(nelhage): Known incompletenesses:
//
//  - If the array is too small, and there are elements after a splat, we read
//    from the back of the array instead of padding with nil.
//
//     e.g. in `*b,c = x`, we assign `c = x[-1]`, even if x has only a single
//     element
//  - If `rhs` is not an array, we index into it anyways, instead of
//    `nil`-padding.
unique_ptr<Expression> desugarMlhs(core::Context ctx, core::Loc loc, parser::Mlhs *lhs, unique_ptr<Expression> rhs) {
    InsSeq::STATS_store stats;

    core::NameRef tempName = ctx.state.freshNameUnique(core::UniqueNameKind::Desugar, core::Names::assignTemp());
    stats.emplace_back(mkAssign(loc, tempName, move(rhs)));

    int i = 0;
    bool didSplat = false;

    for (auto &c : lhs->exprs) {
        if (auto *splat = parser::cast_node<parser::SplatLhs>(c.get())) {
            Error::check(!didSplat);
            didSplat = true;

            unique_ptr<Expression> lh = node2TreeImpl(ctx, splat->var);

            int left = i;
            int right = lhs->exprs.size() - left - 1;
            if (cast_tree<EmptyTree>(lh.get()) == nullptr) {
                auto exclusive = mkTrue(lh->loc);
                if (right == 0) {
                    right = 1;
                    exclusive = mkFalse(lh->loc);
                }
                auto index =
                    mkSend3(lh->loc, make_unique<Ident>(lh->loc, core::GlobalState::defn_Range()), core::Names::new_(),
                            mkInt(lh->loc, left), mkInt(lh->loc, -right), move(exclusive));
                stats.emplace_back(mkAssign(lh->loc, move(lh),
                                            mkSend1(loc, mkLocal(loc, tempName), core::Names::slice(), move(index))));
            }
            i = -right;
        } else {
            auto val = mkSend1(loc, mkLocal(loc, tempName), core::Names::squareBrackets(), mkInt(loc, i));

            if (auto *mlhs = parser::cast_node<parser::Mlhs>(c.get())) {
                stats.emplace_back(desugarMlhs(ctx, mlhs->loc, mlhs, move(val)));
            } else {
                unique_ptr<Expression> lh = node2TreeImpl(ctx, c);
                stats.emplace_back(mkAssign(lh->loc, move(lh), move(val)));
            }

            i++;
        }
    }

    return mkInsSeq(loc, move(stats), mkLocal(loc, tempName));
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
            [&](parser::And *and_) {
                auto lhs = node2TreeImpl(ctx, and_->left);
                if (auto i = cast_tree<Reference>(lhs.get())) {
                    auto cond = cpRef(what->loc, *i);
                    auto iff = mkIf(what->loc, move(cond), node2TreeImpl(ctx, and_->right), move(lhs));
                    result.swap(iff);
                } else {
                    core::NameRef tempName =
                        ctx.state.freshNameUnique(core::UniqueNameKind::Desugar, core::Names::andAnd());
                    auto temp = mkAssign(what->loc, tempName, move(lhs));

                    auto iff = mkIf(what->loc, mkLocal(what->loc, tempName), node2TreeImpl(ctx, and_->right),
                                    mkLocal(what->loc, tempName));
                    auto wrapped = mkInsSeq1(what->loc, move(temp), move(iff));

                    result.swap(wrapped);
                }
            },
            [&](parser::Or *or_) {
                auto lhs = node2TreeImpl(ctx, or_->left);
                if (auto i = cast_tree<Reference>(lhs.get())) {
                    auto cond = cpRef(what->loc, *i);
                    auto iff = mkIf(what->loc, move(cond), move(lhs), node2TreeImpl(ctx, or_->right));
                    result.swap(iff);
                } else {
                    core::NameRef tempName =
                        ctx.state.freshNameUnique(core::UniqueNameKind::Desugar, core::Names::orOr());
                    auto temp = mkAssign(what->loc, tempName, move(lhs));

                    auto iff = mkIf(what->loc, mkLocal(what->loc, tempName), mkLocal(what->loc, tempName),
                                    node2TreeImpl(ctx, or_->right));
                    auto wrapped = mkInsSeq1(what->loc, move(temp), move(iff));

                    result.swap(wrapped);
                }
            },
            [&](parser::AndAsgn *andAsgn) {
                auto recv = node2TreeImpl(ctx, andAsgn->left);
                auto arg = node2TreeImpl(ctx, andAsgn->right);
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
                    auto cond = mkSend(what->loc, mkLocal(what->loc, tempRecv), s->fun, move(readArgs), s->flags);
                    core::NameRef tempResult = ctx.state.freshNameUnique(core::UniqueNameKind::Desugar, s->fun);
                    stats.emplace_back(mkAssign(what->loc, tempResult, move(cond)));

                    auto body =
                        mkSend(what->loc, mkLocal(what->loc, tempRecv), s->fun.addEq(ctx), move(assgnArgs), s->flags);
                    auto elsep = mkLocal(what->loc, tempResult);
                    auto iff = mkIf(what->loc, mkLocal(what->loc, tempResult), move(body), move(elsep));
                    auto wrapped = mkInsSeq(what->loc, move(stats), move(iff));
                    result.swap(wrapped);
                } else if (auto i = cast_tree<Reference>(recv.get())) {
                    auto cond = cpRef(what->loc, *i);
                    auto body = mkAssign(what->loc, move(recv), move(arg));
                    auto elsep = cpRef(what->loc, *i);
                    auto iff = mkIf(what->loc, move(cond), move(body), move(elsep));
                    result.swap(iff);
                } else {
                    Error::notImplemented();
                }
            },
            [&](parser::OrAsgn *orAsgn) {
                auto recv = node2TreeImpl(ctx, orAsgn->left);
                auto arg = node2TreeImpl(ctx, orAsgn->right);
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
                    auto cond = mkSend(what->loc, mkLocal(what->loc, tempRecv), s->fun, move(readArgs), s->flags);
                    core::NameRef tempResult = ctx.state.freshNameUnique(core::UniqueNameKind::Desugar, s->fun);
                    stats.emplace_back(mkAssign(what->loc, tempResult, move(cond)));

                    auto elsep =
                        mkSend(what->loc, mkLocal(what->loc, tempRecv), s->fun.addEq(ctx), move(assgnArgs), s->flags);
                    auto body = mkLocal(what->loc, tempResult);
                    auto iff = mkIf(what->loc, mkLocal(what->loc, tempResult), move(body), move(elsep));
                    auto wrapped = mkInsSeq(what->loc, move(stats), move(iff));
                    result.swap(wrapped);
                } else if (auto i = cast_tree<Reference>(recv.get())) {
                    auto cond = cpRef(what->loc, *i);
                    auto body = mkAssign(what->loc, move(recv), move(arg));
                    auto elsep = cpRef(what->loc, *i);
                    auto iff = mkIf(what->loc, move(cond), move(elsep), move(body));
                    result.swap(iff);

                } else {
                    Error::notImplemented();
                }
            },
            [&](parser::OpAsgn *opAsgn) {
                auto recv = node2TreeImpl(ctx, opAsgn->left);
                auto rhs = node2TreeImpl(ctx, opAsgn->right);
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
                    auto prevValue = mkSend(what->loc, mkLocal(what->loc, tempRecv), s->fun, move(readArgs), s->flags);
                    auto newValue = mkSend1(what->loc, move(prevValue), opAsgn->op, move(rhs));
                    assgnArgs.emplace_back(move(newValue));

                    auto res =
                        mkSend(what->loc, mkLocal(what->loc, tempRecv), s->fun.addEq(ctx), move(assgnArgs), s->flags);
                    auto wrapped = mkInsSeq(what->loc, move(stats), move(res));
                    result.swap(wrapped);
                } else if (auto i = cast_tree<Reference>(recv.get())) {
                    auto lhs = cpRef(what->loc, *i);
                    auto send = mkSend1(what->loc, move(recv), opAsgn->op, move(rhs));
                    auto res = mkAssign(what->loc, move(lhs), move(send));
                    result.swap(res);
                } else {
                    Error::notImplemented();
                }
            },
            [&](parser::Send *send) {
                u4 flags = 0;
                auto rec = node2TreeImpl(ctx, send->receiver);
                if (cast_tree<EmptyTree>(rec.get()) != nullptr) {
                    rec = mkSelf(what->loc);
                    flags |= Send::PRIVATE_OK;
                }
                Send::ARGS_store args;
                unique_ptr<parser::Node> block;
                args.reserve(send->args.size());
                for (auto &stat : send->args) {
                    if (auto bp = parser::cast_node<parser::BlockPass>(stat.get())) {
                        Error::check(block == nullptr);
                        block = move(bp->block);
                    } else {
                        args.emplace_back(node2TreeImpl(ctx, stat));
                    }
                };

                auto res = mkSend(what->loc, move(rec), send->method, move(args), flags, node2Proc(ctx, move(block)));
                result.swap(res);
            },
            [&](parser::CSend *csend) {
                core::NameRef tempRecv =
                    ctx.state.freshNameUnique(core::UniqueNameKind::Desugar, core::Names::assignTemp());
                core::Loc recvLoc = csend->receiver->loc;

                // NOTE(nelhage): We actually desugar into a call to `nil?`. If an
                // object has overridden `nil?`, this technically will not match
                // Ruby's behavior.

                auto assgn = mkAssign(recvLoc, tempRecv, node2TreeImpl(ctx, csend->receiver));
                auto cond = mkSend0(csend->loc, mkLocal(recvLoc, tempRecv), core::Names::nil_p());

                unique_ptr<parser::Node> sendNode = make_unique<parser::Send>(
                    csend->loc, make_unique<parser::LVar>(recvLoc, tempRecv), csend->method, move(csend->args));
                auto send = node2TreeImpl(ctx, sendNode);

                unique_ptr<Expression> nil = mkIdent(csend->loc, core::GlobalState::defn_nil());
                auto iff = mkIf(csend->loc, move(cond), move(nil), move(send));
                InsSeq::STATS_store stats;
                stats.emplace_back(move(assgn));
                auto res = mkInsSeq(csend->loc, move(stats), move(iff));
                result.swap(res);
            },
            [&](parser::Self *self) {
                unique_ptr<Expression> res = mkSelf(what->loc);
                result.swap(res);
            },
            [&](parser::DString *dstring) {
                unique_ptr<Expression> res = desugarDString(ctx, dstring->loc, move(dstring->nodes));
                result.swap(res);
            },
            [&](parser::DSymbol *dsymbol) {
                if (dsymbol->nodes.empty()) {
                    unique_ptr<Expression> res = make_unique<SymbolLit>(dsymbol->loc, core::Names::empty());
                    result.swap(res);
                    return;
                }

                auto it = dsymbol->nodes.begin();
                auto end = dsymbol->nodes.end();
                unique_ptr<Expression> res;
                unique_ptr<Expression> first = node2TreeImpl(ctx, *it);
                if (cast_tree<StringLit>(first.get()) == nullptr) {
                    res = mkSend0(what->loc, move(first), core::Names::to_s());
                } else {
                    res = move(first);
                }
                ++it;
                for (; it != end; ++it) {
                    auto &stat = *it;
                    unique_ptr<Expression> narg = node2TreeImpl(ctx, stat);
                    if (cast_tree<StringLit>(narg.get()) == nullptr) {
                        narg = mkSend0(what->loc, move(narg), core::Names::to_s());
                    }
                    auto n = mkSend1(what->loc, move(res), core::Names::concat(), move(narg));
                    res.reset(n.release());
                };
                res = mkSend0(what->loc, move(res), core::Names::intern());

                result.swap(res);
            },
            [&](parser::Symbol *symbol) {
                unique_ptr<Expression> res = make_unique<ast::SymbolLit>(what->loc, symbol->val);
                result.swap(res);
            },
            [&](parser::String *string) {
                unique_ptr<Expression> res = make_unique<StringLit>(what->loc, string->val);
                result.swap(res);
            },
            [&](parser::FileLiteral *fileLiteral) {
                unique_ptr<Expression> res = make_unique<StringLit>(what->loc, core::Names::currentFile());
                result.swap(res);
            },
            [&](parser::Const *const_) {
                auto scope = node2TreeImpl(ctx, const_->scope);
                unique_ptr<Expression> res = mkConstant(what->loc, move(scope), const_->name);
                result.swap(res);
            },
            [&](parser::ConstLhs *constLhs) {
                auto scope = node2TreeImpl(ctx, constLhs->scope);
                unique_ptr<Expression> res = mkConstant(what->loc, move(scope), constLhs->name);
                result.swap(res);
            },
            [&](parser::Cbase *cbase) {
                unique_ptr<Expression> res = mkIdent(what->loc, core::GlobalState::defn_root());
                result.swap(res);
            },
            [&](parser::Begin *begin) {
                if (!begin->stmts.empty()) {
                    InsSeq::STATS_store stats;
                    stats.reserve(begin->stmts.size() - 1);
                    auto end = begin->stmts.end();
                    --end;
                    for (auto it = begin->stmts.begin(); it != end; ++it) {
                        auto &stat = *it;
                        stats.emplace_back(node2TreeImpl(ctx, stat));
                    };
                    auto &last = begin->stmts.back();
                    auto expr = node2TreeImpl(ctx, last);
                    auto block = mkInsSeq(what->loc, move(stats), move(expr));
                    result.swap(block);
                } else {
                    unique_ptr<Expression> res = mkEmptyTree(what->loc);
                    result.swap(res);
                }
            },
            [&](parser::Kwbegin *kwbegin) {
                if (!kwbegin->stmts.empty()) {
                    InsSeq::STATS_store stats;
                    stats.reserve(kwbegin->stmts.size() - 1);
                    auto end = kwbegin->stmts.end();
                    --end;
                    for (auto it = kwbegin->stmts.begin(); it != end; ++it) {
                        auto &stat = *it;
                        stats.emplace_back(node2TreeImpl(ctx, stat));
                    };
                    auto &last = kwbegin->stmts.back();
                    auto expr = node2TreeImpl(ctx, last);
                    auto block = mkInsSeq(what->loc, move(stats), move(expr));
                    result.swap(block);
                } else {
                    unique_ptr<Expression> res = mkEmptyTree(what->loc);
                    result.swap(res);
                }
            },
            [&](parser::Module *module) {
                ClassDef::RHS_store body;
                if (auto *begin = parser::cast_node<parser::Begin>(module->body.get())) {
                    body.reserve(begin->stmts.size());
                    for (auto &stat : begin->stmts) {
                        body.emplace_back(node2TreeImpl(ctx, stat));
                    };
                } else {
                    body.emplace_back(node2TreeImpl(ctx, module->body));
                }
                ClassDef::ANCESTORS_store ancestors;
                unique_ptr<Expression> res =
                    make_unique<ClassDef>(what->loc, ctx.state.defn_todo(), node2TreeImpl(ctx, module->name),
                                          move(ancestors), move(body), ClassDefKind::Module);
                result.swap(res);
            },
            [&](parser::Class *claz) {
                ClassDef::RHS_store body;
                if (auto *begin = parser::cast_node<parser::Begin>(claz->body.get())) {
                    body.reserve(begin->stmts.size());
                    for (auto &stat : begin->stmts) {
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
                    make_unique<ClassDef>(what->loc, ctx.state.defn_todo(), node2TreeImpl(ctx, claz->name),
                                          move(ancestors), move(body), ClassDefKind::Class);
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

                send->block = make_unique<Block>(what->loc, move(argsAndBody.first), move(argsAndBody.second));
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
                unique_ptr<Expression> res = mkIdent(what->loc, core::GlobalState::defn_nil());
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
            [&](parser::NthRef *var) {
                unique_ptr<Expression> res = make_unique<UnresolvedIdent>(what->loc, UnresolvedIdent::Global,
                                                                          ctx.state.enterNameUTF8(to_string(var->ref)));
                result.swap(res);
            },
            [&](parser::Assign *asgn) {
                auto lhs = node2TreeImpl(ctx, asgn->lhs);
                auto rhs = node2TreeImpl(ctx, asgn->rhs);
                auto res = mkAssign(what->loc, move(lhs), move(rhs));
                result.swap(res);
            },
            [&](parser::Super *super) {
                auto method = core::Names::super();

                Send::ARGS_store args;
                unique_ptr<parser::Node> block;
                args.reserve(super->args.size());
                for (auto &stat : super->args) {
                    if (auto bp = parser::cast_node<parser::BlockPass>(stat.get())) {
                        Error::check(block == nullptr);
                        block = move(bp->block);
                    } else {
                        args.emplace_back(node2TreeImpl(ctx, stat));
                    }
                };

                unique_ptr<Expression> res =
                    make_unique<Send>(what->loc, mkSelf(what->loc), method, move(args), node2Proc(ctx, move(block)));

                result.swap(res);
            },
            [&](parser::ZSuper *zuper) {
                Send::ARGS_store args;
                args.emplace_back(make_unique<ZSuperArgs>(zuper->loc));
                unique_ptr<Expression> res =
                    make_unique<Send>(what->loc, mkSelf(what->loc), core::Names::super(), move(args));
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

                unique_ptr<Expression> res = mkInt(what->loc, val);
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
                unique_ptr<Expression> lastMerge;
                for (auto &stat : array->elts) {
                    if (auto splat = parser::cast_node<parser::Splat>(stat.get())) {
                        // Desguar
                        //   [a, **x, remaining}
                        // into
                        //   a.concat(x.to_a).concat(remaining)
                        auto var = mkSend0(what->loc, node2TreeImpl(ctx, splat->var), core::Names::to_a());
                        if (elems.empty()) {
                            if (lastMerge != nullptr) {
                                lastMerge = mkSend1(what->loc, move(lastMerge), core::Names::concat(), move(var));
                            } else {
                                lastMerge = move(var);
                            }
                        } else {
                            unique_ptr<Expression> current = make_unique<Array>(what->loc, move(elems));
                            elems.clear();
                            if (lastMerge != nullptr) {
                                lastMerge = mkSend1(what->loc, move(lastMerge), core::Names::concat(), move(current));
                            } else {
                                lastMerge = move(current);
                            }
                            lastMerge = mkSend1(what->loc, move(lastMerge), core::Names::concat(), move(var));
                        }
                    } else {
                        elems.emplace_back(node2TreeImpl(ctx, stat));
                    }
                };

                unique_ptr<Expression> res;
                if (elems.empty()) {
                    if (lastMerge != nullptr) {
                        res = move(lastMerge);
                    } else {
                        // Empty array
                        res = make_unique<Array>(what->loc, move(elems));
                    }
                } else {
                    res = make_unique<Array>(what->loc, move(elems));
                    if (lastMerge != nullptr) {
                        res = mkSend1(what->loc, move(lastMerge), core::Names::concat(), move(res));
                    }
                }
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
                        Error::check(splat != nullptr);

                        // Desguar
                        //   {a: 'a', **x, remaining}
                        // into
                        //   {a: 'a'}.merge(x.to_h).merge(remaining)
                        auto expr = mkSend0(what->loc, node2TreeImpl(ctx, splat->expr), core::Names::to_hash());
                        if (keys.empty()) {
                            if (lastMerge != nullptr) {
                                lastMerge = mkSend1(what->loc, move(lastMerge), core::Names::merge(), move(expr));

                            } else {
                                lastMerge = move(expr);
                            }
                        } else {
                            unique_ptr<Expression> current = make_unique<Hash>(what->loc, move(keys), move(values));
                            keys.clear();
                            values.clear();
                            if (lastMerge != nullptr) {
                                lastMerge = mkSend1(what->loc, move(lastMerge), core::Names::merge(), move(current));
                            } else {
                                lastMerge = move(current);
                            }
                            lastMerge = mkSend1(what->loc, move(lastMerge), core::Names::merge(), move(expr));
                        }
                    }
                };

                unique_ptr<Expression> res;
                if (keys.empty()) {
                    if (lastMerge != nullptr) {
                        res = move(lastMerge);
                    } else {
                        // Empty array
                        res = make_unique<Hash>(what->loc, move(keys), move(values));
                    }
                } else {
                    res = make_unique<Hash>(what->loc, move(keys), move(values));
                    if (lastMerge != nullptr) {
                        res = mkSend1(what->loc, move(lastMerge), core::Names::merge(), move(res));
                    }
                }

                result.swap(res);
            },
            [&](parser::IRange *ret) {
                core::NameRef range_name = core::GlobalState::defn_Range().info(ctx).name;
                unique_ptr<Expression> range = mkConstant(what->loc, mkEmptyTree(what->loc), range_name);
                auto from = node2TreeImpl(ctx, ret->from);
                auto to = node2TreeImpl(ctx, ret->to);
                auto send = mkSend2(what->loc, move(range), core::Names::new_(), move(from), move(to));
                result.swap(send);
            },
            [&](parser::ERange *ret) {
                unique_ptr<Expression> range = mkIdent(what->loc, core::GlobalState::defn_Range());
                auto from = node2TreeImpl(ctx, ret->from);
                auto to = node2TreeImpl(ctx, ret->to);
                auto true_ = mkTrue(what->loc);
                auto send = mkSend3(what->loc, move(range), core::Names::new_(), move(from), move(to), move(true_));
                result.swap(send);
            },
            [&](parser::Regexp *regexpNode) {
                unique_ptr<Expression> regexp = mkIdent(what->loc, core::GlobalState::defn_Regexp());
                auto regex = desugarDString(ctx, what->loc, move(regexpNode->regex));
                auto opts = node2TreeImpl(ctx, regexpNode->opts);
                auto send = mkSend2(what->loc, move(regexp), core::Names::new_(), move(regex), move(opts));
                result.swap(send);
            },
            [&](parser::Regopt *regopt) {
                unique_ptr<Expression> acc = mkInt(what->loc, 0);
                for (auto &chr : regopt->opts) {
                    int flag = 0;
                    switch (chr) {
                        case 'i':
                            flag = 1; // Regexp::IGNORECASE
                            break;
                        case 'x':
                            flag = 2; // Regexp::EXTENDED
                            break;
                        case 'm':
                            flag = 4; // Regexp::MULILINE
                            break;
                        case 'n':
                        case 'e':
                        case 's':
                        case 'u':
                            // Encoding options that should already be handled by the parser
                            break;
                        default:
                            // The parser already yelled about this
                            break;
                    }
                    if (flag != 0) {
                        acc = mkSend1(what->loc, move(acc), core::Names::orOp(), mkInt(what->loc, flag));
                    }
                }
                result.swap(acc);
            },
            [&](parser::Return *ret) {
                if (ret->exprs.size() > 1) {
                    Array::ENTRY_store elems;
                    elems.reserve(ret->exprs.size());
                    for (auto &stat : ret->exprs) {
                        elems.emplace_back(node2TreeImpl(ctx, stat));
                    };
                    unique_ptr<Expression> arr = make_unique<Array>(what->loc, move(elems));
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
                    unique_ptr<Expression> arr = make_unique<Array>(what->loc, move(elems));
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
                    unique_ptr<Expression> arr = make_unique<Array>(what->loc, move(elems));
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
                    unique_ptr<Expression> arr = make_unique<Array>(what->loc, move(elems));
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
                unique_ptr<Expression> res = make_unique<Rescue>(what->loc, node2TreeImpl(ctx, rescue->body),
                                                                 move(cases), node2TreeImpl(ctx, rescue->else_));
                result.swap(res);
            },
            [&](parser::Resbody *resbody) {
                RescueCase::EXCEPTION_store exceptions;
                auto exceptionsExpr = node2TreeImpl(ctx, resbody->exception);
                if (cast_tree<EmptyTree>(exceptionsExpr.get()) != nullptr) {
                    // No exceptions captured
                } else if (auto exceptionsArray = cast_tree<ast::Array>(exceptionsExpr.get())) {
                    DEBUG_ONLY(Error::check(exceptionsArray != nullptr));

                    for (auto &elem : exceptionsArray->elems) {
                        exceptions.emplace_back(move(elem));
                    }
                } else if (auto exceptionsSend = cast_tree<ast::Send>(exceptionsExpr.get())) {
                    Error::check(exceptionsSend->fun == core::Names::splat() ||
                                 exceptionsSend->fun == core::Names::to_a() ||
                                 exceptionsSend->fun == core::Names::concat());
                    exceptions.emplace_back(move(exceptionsExpr));
                } else {
                    Error::raise("Bad inner node type");
                }

                auto varExpr = node2TreeImpl(ctx, resbody->var);
                auto body = node2TreeImpl(ctx, resbody->body);

                auto varLoc = varExpr->loc;
                core::NameRef var(0);
                if (auto *id = cast_tree<UnresolvedIdent>(varExpr.get())) {
                    if (id->kind == UnresolvedIdent::Local) {
                        var = id->name;
                        varExpr.reset(nullptr);
                    }
                }

                if (!var.exists()) {
                    var = ctx.state.freshNameUnique(core::UniqueNameKind::Desugar, core::Names::rescueTemp());
                }

                if (cast_tree<EmptyTree>(varExpr.get()) != nullptr) {
                    varLoc = resbody->loc;
                } else if (varExpr != nullptr) {
                    body = mkInsSeq1(varLoc, mkAssign(varLoc, move(varExpr), mkLocal(varLoc, var)), move(body));
                }

                unique_ptr<Expression> res =
                    make_unique<RescueCase>(what->loc, move(exceptions), mkLocal(varLoc, var), move(body));
                result.swap(res);
            },
            [&](parser::If *if_) {
                auto cond = node2TreeImpl(ctx, if_->condition);
                auto thenp = node2TreeImpl(ctx, if_->then_);
                auto elsep = node2TreeImpl(ctx, if_->else_);
                auto iff = mkIf(what->loc, move(cond), move(thenp), move(elsep));
                result.swap(iff);
            },
            [&](parser::Masgn *masgn) {
                parser::Mlhs *lhs = parser::cast_node<parser::Mlhs>(masgn->lhs.get());
                Error::check(lhs != nullptr);

                auto res = desugarMlhs(ctx, masgn->loc, lhs, node2TreeImpl(ctx, masgn->rhs));

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
            [&](parser::Case *case_) {
                unique_ptr<Expression> assign;
                core::NameRef temp(0);
                core::Loc cloc;

                if (case_->condition != nullptr) {
                    cloc = case_->condition->loc;
                    temp = ctx.state.freshNameUnique(core::UniqueNameKind::Desugar, core::Names::assignTemp());
                    assign = mkAssign(cloc, temp, node2TreeImpl(ctx, case_->condition));
                }
                unique_ptr<Expression> res = node2TreeImpl(ctx, case_->else_);
                for (auto it = case_->whens.rbegin(); it != case_->whens.rend(); ++it) {
                    auto when = parser::cast_node<parser::When>(it->get());
                    Error::check(when != nullptr);
                    unique_ptr<Expression> cond;
                    for (auto &cnode : when->patterns) {
                        auto ctree = node2TreeImpl(ctx, cnode);
                        unique_ptr<Expression> test;
                        if (temp.exists()) {
                            auto local = mkLocal(cloc, temp);
                            test = mkSend1(cnode->loc, move(local), core::Names::tripleEq(), move(ctree));
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
                    res = mkInsSeq1(case_->loc, move(assign), move(res));
                }
                result.swap(res);
            },
            [&](parser::Splat *splat) {
                auto res = mkSplat(what->loc, node2TreeImpl(ctx, splat->var));
                result.swap(res);
            },
            [&](parser::Alias *alias) {
                auto res = mkSend2(alias->loc, mkSelf(what->loc), core::Names::aliasMethod(),
                                   node2TreeImpl(ctx, alias->from), node2TreeImpl(ctx, alias->to));
                result.swap(res);
            },
            [&](parser::Defined *defined) {
                auto res =
                    mkSend1(what->loc, mkSelf(what->loc), core::Names::defined_p(), node2TreeImpl(ctx, defined->value));
                result.swap(res);
            },

            [&](parser::BlockPass *blockPass) { Error::raise("Send should have already handled the BlockPass"); },
            [&](parser::Node *node) {
                ctx.state.errors.error(what->loc, core::ErrorClass::UnsupportedNode, "Unsupported node type {}",
                                       node->nodeName());
                result.reset(new NotSupported(what->loc, node->nodeName()));
            });
        Error::check(result.get() != nullptr);
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
