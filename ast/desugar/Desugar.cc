#include <algorithm>

#include "absl/strings/numbers.h"
#include "absl/strings/str_replace.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "ast/desugar/Desugar.h"
#include "ast/verifier/verifier.h"
#include "common/common.h"
#include "core/Names.h"
#include "core/errors/desugar.h"
#include "core/errors/internal.h"

namespace sorbet::ast::desugar {

using namespace std;

namespace {

struct DesugarContext final {
    core::MutableContext ctx;
    u2 &uniqueCounter;
    core::NameRef enclosingBlockArg;

    DesugarContext(core::MutableContext ctx, u2 &uniqueCounter, core::NameRef enclosingBlockArg)
        : ctx(ctx), uniqueCounter(uniqueCounter), enclosingBlockArg(enclosingBlockArg){};
};

core::NameRef blockArg2Name(DesugarContext dctx, const BlockArg &blkArg) {
    auto blkIdent = cast_tree<UnresolvedIdent>(blkArg.expr.get());
    ENFORCE(blkIdent != nullptr, "BlockArg must wrap UnresolvedIdent in desugar.");
    return blkIdent->name;
}

unique_ptr<Expression> node2TreeImpl(DesugarContext dctx, unique_ptr<parser::Node> what);

pair<MethodDef::ARGS_store, InsSeq::STATS_store> desugarArgs(DesugarContext dctx, core::Loc loc,
                                                             unique_ptr<parser::Node> &argnode) {
    MethodDef::ARGS_store args;
    InsSeq::STATS_store destructures;

    if (auto *oargs = parser::cast_node<parser::Args>(argnode.get())) {
        args.reserve(oargs->args.size());
        for (auto &arg : oargs->args) {
            if (auto *lhs = parser::cast_node<parser::Mlhs>(arg.get())) {
                core::NameRef temporary = dctx.ctx.state.freshNameUnique(
                    core::UniqueNameKind::Desugar, core::Names::destructureArg(), ++dctx.uniqueCounter);
                args.emplace_back(MK::Local(arg->loc, temporary));
                unique_ptr<parser::Node> lvarNode = make_unique<parser::LVar>(arg->loc, temporary);
                unique_ptr<parser::Node> destructure =
                    make_unique<parser::Masgn>(arg->loc, std::move(arg), std::move(lvarNode));
                destructures.emplace_back(node2TreeImpl(dctx, std::move(destructure)));
            } else {
                args.emplace_back(node2TreeImpl(dctx, std::move(arg)));
            }
        }
    } else if (argnode.get() == nullptr) {
        // do nothing
    } else {
        auto node = argnode.get();
        Exception::raise("not implemented: {}", demangle(typeid(*node).name()));
    }

    return make_pair(std::move(args), std::move(destructures));
}

unique_ptr<Expression> desugarBody(DesugarContext dctx, core::Loc loc, unique_ptr<parser::Node> &bodynode,
                                   InsSeq::STATS_store destructures) {
    auto body = node2TreeImpl(dctx, std::move(bodynode));
    if (!destructures.empty()) {
        core::Loc bodyLoc = body->loc;
        if (!bodyLoc.exists()) {
            bodyLoc = loc;
        }
        body = MK::InsSeq(loc, std::move(destructures), std::move(body));
    }

    return body;
}
bool isStringLit(DesugarContext dctx, unique_ptr<Expression> &expr) {
    Literal *lit;
    return (lit = cast_tree<Literal>(expr.get())) && lit->isString(dctx.ctx);
}

unique_ptr<Expression> desugarDString(DesugarContext dctx, core::Loc loc, parser::NodeVec nodes) {
    if (nodes.empty()) {
        return MK::String(loc, core::Names::empty());
    }
    auto it = nodes.begin();
    auto end = nodes.end();
    unique_ptr<Expression> res;
    unique_ptr<Expression> first = node2TreeImpl(dctx, std::move(*it));
    if (isStringLit(dctx, first) || isa_tree<EmptyTree>(first.get())) {
        res = std::move(first);
    } else {
        auto pieceLoc = first->loc;
        res = MK::Send0(pieceLoc, std::move(first), core::Names::to_s());
    }
    ++it;
    for (; it != end; ++it) {
        auto &stat = *it;
        unique_ptr<Expression> narg = node2TreeImpl(dctx, std::move(stat));
        if (!isStringLit(dctx, narg) && !isa_tree<EmptyTree>(narg.get())) {
            auto pieceLoc = narg->loc;
            narg = MK::Send0(pieceLoc, std::move(narg), core::Names::to_s());
        }
        auto n = MK::Send1(loc, std::move(res), core::Names::concat(), std::move(narg));
        res.reset(n.release());
    };
    return res;
}

unique_ptr<MethodDef> buildMethod(DesugarContext dctx, core::Loc loc, core::Loc declLoc, core::NameRef name,
                                  unique_ptr<parser::Node> &argnode, unique_ptr<parser::Node> &body, bool isSelf) {
    // Reset uniqueCounter within this scope (to keep numbers small)
    u2 uniqueCounter = 1;
    DesugarContext dctx1(dctx.ctx, uniqueCounter, dctx.enclosingBlockArg);
    auto [args, destructures] = desugarArgs(dctx1, loc, argnode);

    if (args.empty() || !isa_tree<BlockArg>(args.back().get())) {
        auto blkLoc = core::Loc::none(loc.file());
        args.emplace_back(make_unique<BlockArg>(blkLoc, MK::Local(blkLoc, core::Names::blkArg())));
    }

    const auto &blkArg = cast_tree<BlockArg>(args.back().get());
    ENFORCE(blkArg != nullptr, "Every method's last arg must be a block arg by now.");
    auto enclosingBlockArg = blockArg2Name(dctx, *blkArg);

    DesugarContext dctx2(dctx1.ctx, dctx1.uniqueCounter, enclosingBlockArg);
    auto desugaredBody = desugarBody(dctx2, loc, body, std::move(destructures));

    auto mdef = MK::Method(loc, declLoc, name, std::move(args), std::move(desugaredBody));
    if (isSelf) {
        mdef->flags |= MethodDef::SelfMethod;
    }
    return mdef;
}

unique_ptr<Block> symbol2Proc(DesugarContext dctx, unique_ptr<Expression> expr) {
    core::Loc loc = expr->loc;
    core::NameRef temp = dctx.ctx.state.freshNameUnique(core::UniqueNameKind::Desugar, core::Names::blockPassTemp(),
                                                        ++dctx.uniqueCounter);
    Literal *lit = cast_tree<Literal>(expr.get());
    ENFORCE(lit && lit->isSymbol(dctx.ctx));

    // &:foo => {|temp| temp.foo() }
    core::NameRef name(dctx.ctx, core::cast_type<core::LiteralType>(lit->value.get())->value);
    MethodDef::ARGS_store args;
    args.emplace_back(MK::Local(loc, temp));
    unique_ptr<Expression> recv = MK::Local(loc, temp);
    unique_ptr<Expression> body = MK::Send0(loc, std::move(recv), name);
    return make_unique<Block>(loc, std::move(args), std::move(body));
}

unique_ptr<Expression> unsupportedNode(DesugarContext dctx, parser::Node *node) {
    if (auto e = dctx.ctx.state.beginError(node->loc, core::errors::Desugar::UnsupportedNode)) {
        e.setHeader("Unsupported node type `{}`", node->nodeName());
    }
    return MK::EmptyTree();
}

unique_ptr<Expression> desugarMlhs(DesugarContext dctx, core::Loc loc, parser::Mlhs *lhs, unique_ptr<Expression> rhs) {
    InsSeq::STATS_store stats;

    core::NameRef tempName =
        dctx.ctx.state.freshNameUnique(core::UniqueNameKind::Desugar, core::Names::assignTemp(), ++dctx.uniqueCounter);

    int i = 0;
    int before = 0, after = 0;
    bool didSplat = false;

    for (auto &c : lhs->exprs) {
        if (auto *splat = parser::cast_node<parser::SplatLhs>(c.get())) {
            ENFORCE(!didSplat, "did splat already");
            didSplat = true;

            unique_ptr<Expression> lh = node2TreeImpl(dctx, std::move(splat->var));

            int left = i;
            int right = lhs->exprs.size() - left - 1;
            if (!isa_tree<EmptyTree>(lh.get())) {
                auto exclusive = MK::True(lh->loc);
                if (right == 0) {
                    right = 1;
                    exclusive = MK::False(lh->loc);
                }
                auto lhloc = lh->loc;
                auto index = MK::Send3(lhloc, MK::Constant(lhloc, core::Symbols::Range()), core::Names::new_(),
                                       MK::Int(lhloc, left), MK::Int(lhloc, -right), std::move(exclusive));
                stats.emplace_back(
                    MK::Assign(lhloc, std::move(lh),
                               MK::Send1(loc, MK::Local(loc, tempName), core::Names::slice(), std::move(index))));
            }
            i = -right;
        } else {
            if (didSplat) {
                ++after;
            } else {
                ++before;
            }
            auto val = MK::Send1(loc, MK::Local(loc, tempName), core::Names::squareBrackets(), MK::Int(loc, i));

            if (auto *mlhs = parser::cast_node<parser::Mlhs>(c.get())) {
                stats.emplace_back(desugarMlhs(dctx, mlhs->loc, mlhs, std::move(val)));
            } else {
                unique_ptr<Expression> lh = node2TreeImpl(dctx, std::move(c));
                if (auto restArg = ast::cast_tree<ast::RestArg>(lh.get())) {
                    if (auto e = dctx.ctx.state.beginError(lh->loc, core::errors::Desugar::UnsupportedNode)) {
                        e.setHeader("Unsupported rest args in destructure");
                    }
                    lh = move(restArg->expr);
                }
                auto lhloc = lh->loc;
                stats.emplace_back(MK::Assign(lhloc, std::move(lh), std::move(val)));
            }

            i++;
        }
    }

    auto expanded = MK::Send3(loc, MK::Constant(loc, core::Symbols::Magic()), core::Names::expandSplat(),
                              std::move(rhs), MK::Int(loc, before), MK::Int(loc, after));
    stats.insert(stats.begin(), MK::Assign(loc, tempName, std::move(expanded)));

    return MK::InsSeq(loc, std::move(stats), MK::Local(loc, tempName));
}

bool locReported = false;

ClassDef::RHS_store scopeNodeToBody(DesugarContext dctx, unique_ptr<parser::Node> node) {
    ClassDef::RHS_store body;
    // Reset uniqueCounter within this scope (to keep numbers small)
    u2 uniqueCounter = 1;
    DesugarContext dctx1(dctx.ctx, uniqueCounter, dctx.enclosingBlockArg);
    if (auto *begin = parser::cast_node<parser::Begin>(node.get())) {
        body.reserve(begin->stmts.size());
        for (auto &stat : begin->stmts) {
            body.emplace_back(node2TreeImpl(dctx1, std::move(stat)));
        };
    } else {
        body.emplace_back(node2TreeImpl(dctx1, std::move(node)));
    }
    return body;
}

unique_ptr<Expression> node2TreeImpl(DesugarContext dctx, unique_ptr<parser::Node> what) {
    try {
        if (what.get() == nullptr) {
            return MK::EmptyTree();
        }
        auto loc = what->loc;
        ENFORCE(loc.exists(), "parse-tree node has no location: {}", what->toString(dctx.ctx));
        unique_ptr<Expression> result;
        typecase(
            what.get(),
            // The top N clauses here are ordered according to observed
            // frequency in pay-server. Do not reorder the top of this list, or
            // add entries here, without consulting the "node.*" counters from a
            // run over a representative code base.
            [&](parser::Send *send) {
                u4 flags = 0;
                auto rec = node2TreeImpl(dctx, std::move(send->receiver));
                if (isa_tree<EmptyTree>(rec.get())) {
                    rec = MK::Self(loc);
                    flags |= Send::PRIVATE_OK;
                }
                if (absl::c_any_of(send->args, [](auto &arg) { return parser::isa_node<parser::Splat>(arg.get()); })) {
                    // If we have a splat anywhere in the argument list, desugar
                    // the argument list as a single Array node, and then
                    // synthesize a call to
                    //   Magic.callWithSplat(receiver, method, argArray, [&blk])
                    // The callWithSplat implementation (in C++) will unpack a
                    // tuple type and call into the normal call merchanism.
                    unique_ptr<parser::Node> block;
                    auto argnodes = std::move(send->args);
                    auto it = absl::c_find_if(argnodes,
                                              [](auto &arg) { return parser::isa_node<parser::BlockPass>(arg.get()); });
                    if (it != argnodes.end()) {
                        auto *bp = parser::cast_node<parser::BlockPass>(it->get());
                        block = std::move(bp->block);
                        argnodes.erase(it);
                    }

                    auto array = make_unique<parser::Array>(loc, std::move(argnodes));
                    auto args = node2TreeImpl(dctx, std::move(array));
                    auto method =
                        MK::Literal(loc, core::make_type<core::LiteralType>(core::Symbols::Symbol(), send->method));

                    Send::ARGS_store sendargs;
                    sendargs.emplace_back(std::move(rec));
                    sendargs.emplace_back(std::move(method));
                    sendargs.emplace_back(std::move(args));
                    unique_ptr<Expression> res;
                    if (block == nullptr) {
                        res = MK::Send(loc, MK::Constant(loc, core::Symbols::Magic()), core::Names::callWithSplat(),
                                       std::move(sendargs), 0);
                    } else {
                        auto convertedBlock = node2TreeImpl(dctx, std::move(block));
                        Literal *lit;
                        if ((lit = cast_tree<Literal>(convertedBlock.get())) && lit->isSymbol(dctx.ctx)) {
                            res = MK::Send(loc, MK::Constant(loc, core::Symbols::Magic()), core::Names::callWithSplat(),
                                           std::move(sendargs), 0, symbol2Proc(dctx, std::move(convertedBlock)));
                        } else {
                            sendargs.emplace_back(std::move(convertedBlock));
                            res = MK::Send(loc, MK::Constant(loc, core::Symbols::Magic()),
                                           core::Names::callWithSplatAndBlock(), std::move(sendargs), 0);
                        }
                    }
                    result.swap(res);
                } else {
                    Send::ARGS_store args;
                    unique_ptr<parser::Node> block;
                    args.reserve(send->args.size());
                    for (auto &stat : send->args) {
                        if (auto bp = parser::cast_node<parser::BlockPass>(stat.get())) {
                            ENFORCE(block == nullptr, "passing a block where there is no block");
                            block = std::move(bp->block);
                        } else {
                            args.emplace_back(node2TreeImpl(dctx, std::move(stat)));
                        }
                    };

                    unique_ptr<Expression> res;
                    if (block == nullptr) {
                        res = MK::Send(loc, std::move(rec), send->method, std::move(args), flags);
                    } else {
                        auto method =
                            MK::Literal(loc, core::make_type<core::LiteralType>(core::Symbols::Symbol(), send->method));
                        auto convertedBlock = node2TreeImpl(dctx, std::move(block));
                        Literal *lit;
                        if ((lit = cast_tree<Literal>(convertedBlock.get())) && lit->isSymbol(dctx.ctx)) {
                            res = MK::Send(loc, std::move(rec), send->method, std::move(args), flags,
                                           symbol2Proc(dctx, std::move(convertedBlock)));
                        } else {
                            Send::ARGS_store sendargs;
                            sendargs.emplace_back(std::move(rec));
                            sendargs.emplace_back(std::move(method));
                            sendargs.emplace_back(std::move(convertedBlock));
                            for (auto &arg : args) {
                                sendargs.emplace_back(std::move(arg));
                            }

                            res = MK::Send(loc, MK::Constant(loc, core::Symbols::Magic()), core::Names::callWithBlock(),
                                           std::move(sendargs), 0);
                        }
                    }

                    if (send->method == core::Names::blockGiven_p() && dctx.enclosingBlockArg.exists()) {
                        auto if_ = MK::If(loc, MK::Local(loc, dctx.enclosingBlockArg), std::move(res), MK::False(loc));
                        result.swap(if_);
                    } else {
                        result.swap(res);
                    }
                }
            },
            [&](parser::Const *const_) {
                auto scope = node2TreeImpl(dctx, std::move(const_->scope));
                unique_ptr<Expression> res = MK::UnresolvedConstant(loc, std::move(scope), const_->name);
                result.swap(res);
            },
            [&](parser::String *string) {
                unique_ptr<Expression> res = MK::String(loc, string->val);
                result.swap(res);
            },
            [&](parser::Symbol *symbol) {
                unique_ptr<Expression> res = MK::Symbol(loc, symbol->val);
                result.swap(res);
            },
            [&](parser::LVar *var) {
                unique_ptr<Expression> res = MK::Local(loc, var->name);
                result.swap(res);
            },
            [&](parser::DString *dstring) {
                unique_ptr<Expression> res = desugarDString(dctx, loc, std::move(dstring->nodes));
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
                        stats.emplace_back(node2TreeImpl(dctx, std::move(stat)));
                    };
                    auto &last = begin->stmts.back();
                    auto expr = node2TreeImpl(dctx, std::move(last));
                    auto block = MK::InsSeq(loc, std::move(stats), std::move(expr));
                    result.swap(block);
                } else {
                    unique_ptr<Expression> res = MK::EmptyTree();
                    result.swap(res);
                }
            },
            // END hand-ordered clauses
            [&](parser::And *and_) {
                auto lhs = node2TreeImpl(dctx, std::move(and_->left));
                if (auto i = cast_tree<Reference>(lhs.get())) {
                    auto cond = MK::cpRef(*i);
                    auto iff =
                        MK::If(loc, std::move(cond), node2TreeImpl(dctx, std::move(and_->right)), std::move(lhs));
                    result.swap(iff);
                } else {
                    core::NameRef tempName = dctx.ctx.state.freshNameUnique(
                        core::UniqueNameKind::Desugar, core::Names::andAnd(), ++dctx.uniqueCounter);
                    auto temp = MK::Assign(loc, tempName, std::move(lhs));

                    auto iff = MK::If(loc, MK::Local(loc, tempName), node2TreeImpl(dctx, std::move(and_->right)),
                                      MK::Local(loc, tempName));
                    auto wrapped = MK::InsSeq1(loc, std::move(temp), std::move(iff));

                    result.swap(wrapped);
                }
            },
            [&](parser::Or *or_) {
                auto lhs = node2TreeImpl(dctx, std::move(or_->left));
                if (auto i = cast_tree<Reference>(lhs.get())) {
                    auto cond = MK::cpRef(*i);
                    auto iff = MK::If(loc, std::move(cond), std::move(lhs), node2TreeImpl(dctx, std::move(or_->right)));
                    result.swap(iff);
                } else {
                    core::NameRef tempName = dctx.ctx.state.freshNameUnique(core::UniqueNameKind::Desugar,
                                                                            core::Names::orOr(), ++dctx.uniqueCounter);
                    auto temp = MK::Assign(loc, tempName, std::move(lhs));

                    auto iff = MK::If(loc, MK::Local(loc, tempName), MK::Local(loc, tempName),
                                      node2TreeImpl(dctx, std::move(or_->right)));
                    auto wrapped = MK::InsSeq1(loc, std::move(temp), std::move(iff));

                    result.swap(wrapped);
                }
            },
            [&](parser::AndAsgn *andAsgn) {
                auto recv = node2TreeImpl(dctx, std::move(andAsgn->left));
                auto arg = node2TreeImpl(dctx, std::move(andAsgn->right));
                if (auto s = cast_tree<Send>(recv.get())) {
                    auto sendLoc = s->loc;
                    InsSeq::STATS_store stats;
                    stats.reserve(s->args.size() + 2);
                    core::NameRef tempRecv =
                        dctx.ctx.state.freshNameUnique(core::UniqueNameKind::Desugar, s->fun, ++dctx.uniqueCounter);
                    stats.emplace_back(MK::Assign(sendLoc, tempRecv, std::move(s->recv)));
                    Send::ARGS_store readArgs;
                    Send::ARGS_store assgnArgs;
                    readArgs.reserve(s->args.size());
                    assgnArgs.reserve(s->args.size() + 1);

                    for (auto &arg : s->args) {
                        core::Loc argLoc = arg->loc;
                        core::NameRef name =
                            dctx.ctx.state.freshNameUnique(core::UniqueNameKind::Desugar, s->fun, ++dctx.uniqueCounter);
                        stats.emplace_back(MK::Assign(argLoc, name, std::move(arg)));
                        readArgs.emplace_back(MK::Local(argLoc, name));
                        assgnArgs.emplace_back(MK::Local(argLoc, name));
                    }
                    assgnArgs.emplace_back(std::move(arg));
                    auto cond = MK::Send(sendLoc, MK::Local(sendLoc, tempRecv), s->fun, std::move(readArgs), s->flags);
                    core::NameRef tempResult =
                        dctx.ctx.state.freshNameUnique(core::UniqueNameKind::Desugar, s->fun, ++dctx.uniqueCounter);
                    stats.emplace_back(MK::Assign(sendLoc, tempResult, std::move(cond)));

                    auto body = MK::Send(sendLoc, MK::Local(sendLoc, tempRecv), s->fun.addEq(dctx.ctx),
                                         std::move(assgnArgs), s->flags);
                    auto elsep = MK::Local(sendLoc, tempResult);
                    auto iff = MK::If(sendLoc, MK::Local(sendLoc, tempResult), std::move(body), std::move(elsep));
                    auto wrapped = MK::InsSeq(loc, std::move(stats), std::move(iff));
                    result.swap(wrapped);
                } else if (auto i = cast_tree<Reference>(recv.get())) {
                    auto cond = MK::cpRef(*i);
                    auto body = MK::Assign(loc, std::move(recv), std::move(arg));
                    auto elsep = MK::cpRef(*i);
                    auto iff = MK::If(loc, std::move(cond), std::move(body), std::move(elsep));
                    result.swap(iff);
                } else if (auto i = cast_tree<UnresolvedConstantLit>(recv.get())) {
                    if (auto e = dctx.ctx.state.beginError(what->loc, core::errors::Desugar::NoConstantReassignment)) {
                        e.setHeader("Constant reassignment is not supported");
                    }
                    unique_ptr<Expression> res = MK::EmptyTree();
                    result.swap(res);
                } else {
                    Exception::notImplemented();
                }
            },
            [&](parser::OrAsgn *orAsgn) {
                auto recv = node2TreeImpl(dctx, std::move(orAsgn->left));
                auto arg = node2TreeImpl(dctx, std::move(orAsgn->right));
                if (auto s = cast_tree<Send>(recv.get())) {
                    auto sendLoc = s->loc;
                    InsSeq::STATS_store stats;
                    stats.reserve(s->args.size() + 2);
                    core::NameRef tempRecv =
                        dctx.ctx.state.freshNameUnique(core::UniqueNameKind::Desugar, s->fun, ++dctx.uniqueCounter);
                    stats.emplace_back(MK::Assign(sendLoc, tempRecv, std::move(s->recv)));
                    Send::ARGS_store readArgs;
                    Send::ARGS_store assgnArgs;
                    readArgs.reserve(s->args.size());
                    assgnArgs.reserve(s->args.size() + 1);
                    for (auto &arg : s->args) {
                        core::Loc argLoc = arg->loc;
                        core::NameRef name =
                            dctx.ctx.state.freshNameUnique(core::UniqueNameKind::Desugar, s->fun, ++dctx.uniqueCounter);
                        stats.emplace_back(MK::Assign(argLoc, name, std::move(arg)));
                        readArgs.emplace_back(MK::Local(argLoc, name));
                        assgnArgs.emplace_back(MK::Local(argLoc, name));
                    }
                    assgnArgs.emplace_back(std::move(arg));
                    auto cond = MK::Send(sendLoc, MK::Local(sendLoc, tempRecv), s->fun, std::move(readArgs), s->flags);
                    core::NameRef tempResult =
                        dctx.ctx.state.freshNameUnique(core::UniqueNameKind::Desugar, s->fun, ++dctx.uniqueCounter);
                    stats.emplace_back(MK::Assign(sendLoc, tempResult, std::move(cond)));

                    auto elsep = MK::Send(sendLoc, MK::Local(sendLoc, tempRecv), s->fun.addEq(dctx.ctx),
                                          std::move(assgnArgs), s->flags);
                    auto body = MK::Local(sendLoc, tempResult);
                    auto iff = MK::If(sendLoc, MK::Local(sendLoc, tempResult), std::move(body), std::move(elsep));
                    auto wrapped = MK::InsSeq(loc, std::move(stats), std::move(iff));
                    result.swap(wrapped);
                } else if (auto i = cast_tree<Reference>(recv.get())) {
                    auto cond = MK::cpRef(*i);
                    auto body = MK::Assign(loc, std::move(recv), std::move(arg));
                    auto elsep = MK::cpRef(*i);
                    auto iff = MK::If(loc, std::move(cond), std::move(elsep), std::move(body));
                    result.swap(iff);
                } else if (auto i = cast_tree<UnresolvedConstantLit>(recv.get())) {
                    if (auto e = dctx.ctx.state.beginError(what->loc, core::errors::Desugar::NoConstantReassignment)) {
                        e.setHeader("Constant reassignment is not supported");
                    }
                    unique_ptr<Expression> res = MK::EmptyTree();
                    result.swap(res);
                } else {
                    Exception::notImplemented();
                }
            },
            [&](parser::OpAsgn *opAsgn) {
                auto recv = node2TreeImpl(dctx, std::move(opAsgn->left));
                auto rhs = node2TreeImpl(dctx, std::move(opAsgn->right));
                if (auto s = cast_tree<Send>(recv.get())) {
                    auto sendLoc = s->loc;
                    InsSeq::STATS_store stats;
                    stats.reserve(s->args.size() + 2);
                    core::NameRef tempRecv =
                        dctx.ctx.state.freshNameUnique(core::UniqueNameKind::Desugar, s->fun, ++dctx.uniqueCounter);
                    stats.emplace_back(MK::Assign(loc, tempRecv, std::move(s->recv)));
                    Send::ARGS_store readArgs;
                    Send::ARGS_store assgnArgs;
                    readArgs.reserve(s->args.size());
                    assgnArgs.reserve(s->args.size() + 1);
                    for (auto &arg : s->args) {
                        core::Loc argLoc = arg->loc;
                        core::NameRef name =
                            dctx.ctx.state.freshNameUnique(core::UniqueNameKind::Desugar, s->fun, ++dctx.uniqueCounter);
                        stats.emplace_back(MK::Assign(argLoc, name, std::move(arg)));
                        readArgs.emplace_back(MK::Local(argLoc, name));
                        assgnArgs.emplace_back(MK::Local(argLoc, name));
                    }
                    auto prevValue =
                        MK::Send(sendLoc, MK::Local(sendLoc, tempRecv), s->fun, std::move(readArgs), s->flags);
                    auto newValue = MK::Send1(sendLoc, std::move(prevValue), opAsgn->op, std::move(rhs));
                    assgnArgs.emplace_back(std::move(newValue));

                    auto res = MK::Send(sendLoc, MK::Local(sendLoc, tempRecv), s->fun.addEq(dctx.ctx),
                                        std::move(assgnArgs), s->flags);
                    auto wrapped = MK::InsSeq(loc, std::move(stats), std::move(res));
                    result.swap(wrapped);
                } else if (auto i = cast_tree<Reference>(recv.get())) {
                    auto lhs = MK::cpRef(*i);
                    auto send = MK::Send1(loc, std::move(recv), opAsgn->op, std::move(rhs));
                    auto res = MK::Assign(loc, std::move(lhs), std::move(send));
                    result.swap(res);
                } else if (auto i = cast_tree<UnresolvedConstantLit>(recv.get())) {
                    if (auto e = dctx.ctx.state.beginError(what->loc, core::errors::Desugar::NoConstantReassignment)) {
                        e.setHeader("Constant reassignment is not supported");
                    }
                    unique_ptr<Expression> res = MK::EmptyTree();
                    result.swap(res);
                } else {
                    Exception::notImplemented();
                }
            },
            [&](parser::CSend *csend) {
                core::NameRef tempRecv = dctx.ctx.state.freshNameUnique(
                    core::UniqueNameKind::Desugar, core::Names::assignTemp(), ++dctx.uniqueCounter);
                core::Loc recvLoc = csend->receiver->loc;

                // NOTE(nelhage): We actually desugar into a call to `nil?`. If an
                // object has overridden `nil?`, this technically will not match
                // Ruby's behavior.

                auto assgn = MK::Assign(recvLoc, tempRecv, node2TreeImpl(dctx, std::move(csend->receiver)));
                auto cond = MK::Send0(loc, MK::Local(recvLoc, tempRecv), core::Names::nil_p());

                unique_ptr<parser::Node> sendNode = make_unique<parser::Send>(
                    loc, make_unique<parser::LVar>(recvLoc, tempRecv), csend->method, std::move(csend->args));
                auto send = node2TreeImpl(dctx, std::move(sendNode));

                unique_ptr<Expression> nil = MK::Nil(loc);
                auto iff = MK::If(loc, std::move(cond), std::move(nil), std::move(send));
                auto res = MK::InsSeq1(loc, std::move(assgn), std::move(iff));
                result.swap(res);
            },
            [&](parser::Self *self) {
                unique_ptr<Expression> res = MK::Self(loc);
                result.swap(res);
            },
            [&](parser::DSymbol *dsymbol) {
                if (dsymbol->nodes.empty()) {
                    unique_ptr<Expression> res = MK::Symbol(loc, core::Names::empty());
                    result.swap(res);
                    return;
                }

                auto it = dsymbol->nodes.begin();
                auto end = dsymbol->nodes.end();
                unique_ptr<Expression> res;
                unique_ptr<Expression> first = node2TreeImpl(dctx, std::move(*it));
                if (isStringLit(dctx, first)) {
                    res = std::move(first);
                } else {
                    res = MK::Send0(loc, std::move(first), core::Names::to_s());
                }
                ++it;
                for (; it != end; ++it) {
                    auto &stat = *it;
                    unique_ptr<Expression> narg = node2TreeImpl(dctx, std::move(stat));
                    if (!isStringLit(dctx, narg)) {
                        narg = MK::Send0(loc, std::move(narg), core::Names::to_s());
                    }
                    auto n = MK::Send1(loc, std::move(res), core::Names::concat(), std::move(narg));
                    res.reset(n.release());
                };
                res = MK::Send0(loc, std::move(res), core::Names::intern());

                result.swap(res);
            },
            [&](parser::FileLiteral *fileLiteral) {
                unique_ptr<Expression> res = MK::String(loc, core::Names::currentFile());
                result.swap(res);
            },
            [&](parser::ConstLhs *constLhs) {
                auto scope = node2TreeImpl(dctx, std::move(constLhs->scope));
                unique_ptr<Expression> res = MK::UnresolvedConstant(loc, std::move(scope), constLhs->name);
                result.swap(res);
            },
            [&](parser::Cbase *cbase) {
                unique_ptr<Expression> res = MK::Constant(loc, core::Symbols::root());
                result.swap(res);
            },
            [&](parser::Kwbegin *kwbegin) {
                if (!kwbegin->stmts.empty()) {
                    InsSeq::STATS_store stats;
                    stats.reserve(kwbegin->stmts.size() - 1);
                    auto end = kwbegin->stmts.end();
                    --end;
                    for (auto it = kwbegin->stmts.begin(); it != end; ++it) {
                        auto &stat = *it;
                        stats.emplace_back(node2TreeImpl(dctx, std::move(stat)));
                    };
                    auto &last = kwbegin->stmts.back();
                    auto expr = node2TreeImpl(dctx, std::move(last));
                    auto block = MK::InsSeq(loc, std::move(stats), std::move(expr));
                    result.swap(block);
                } else {
                    unique_ptr<Expression> res = MK::EmptyTree();
                    result.swap(res);
                }
            },
            [&](parser::Module *module) {
                ClassDef::RHS_store body = scopeNodeToBody(dctx, std::move(module->body));
                ClassDef::ANCESTORS_store ancestors;
                unique_ptr<Expression> res = make_unique<ClassDef>(
                    module->loc, module->declLoc, core::Symbols::todo(), node2TreeImpl(dctx, std::move(module->name)),
                    std::move(ancestors), std::move(body), ClassDefKind::Module);
                result.swap(res);
            },
            [&](parser::Class *claz) {
                ClassDef::RHS_store body = scopeNodeToBody(dctx, std::move(claz->body));
                ClassDef::ANCESTORS_store ancestors;
                if (claz->superclass == nullptr) {
                    ancestors.emplace_back(MK::Constant(loc, core::Symbols::todo()));
                } else {
                    ancestors.emplace_back(node2TreeImpl(dctx, std::move(claz->superclass)));
                }
                unique_ptr<Expression> res = make_unique<ClassDef>(
                    claz->loc, claz->declLoc, core::Symbols::todo(), node2TreeImpl(dctx, std::move(claz->name)),
                    std::move(ancestors), std::move(body), ClassDefKind::Class);
                result.swap(res);
            },
            [&](parser::Arg *arg) {
                unique_ptr<Expression> res = MK::Local(loc, arg->name);
                result.swap(res);
            },
            [&](parser::Restarg *arg) {
                unique_ptr<Expression> res = make_unique<RestArg>(loc, MK::Local(arg->nameLoc, arg->name));
                result.swap(res);
            },
            [&](parser::Kwrestarg *arg) {
                unique_ptr<Expression> res =
                    make_unique<RestArg>(loc, make_unique<KeywordArg>(loc, MK::Local(loc, arg->name)));
                result.swap(res);
            },
            [&](parser::Kwarg *arg) {
                unique_ptr<Expression> res = make_unique<KeywordArg>(loc, MK::Local(loc, arg->name));
                result.swap(res);
            },
            [&](parser::Blockarg *arg) {
                unique_ptr<Expression> res = make_unique<BlockArg>(loc, MK::Local(loc, arg->name));
                result.swap(res);
            },
            [&](parser::Kwoptarg *arg) {
                unique_ptr<Expression> res =
                    make_unique<OptionalArg>(loc, make_unique<KeywordArg>(loc, MK::Local(arg->nameLoc, arg->name)),
                                             node2TreeImpl(dctx, std::move(arg->default_)));
                result.swap(res);
            },
            [&](parser::Optarg *arg) {
                unique_ptr<Expression> res = make_unique<OptionalArg>(loc, MK::Local(arg->nameLoc, arg->name),
                                                                      node2TreeImpl(dctx, std::move(arg->default_)));
                result.swap(res);
            },
            [&](parser::Shadowarg *arg) {
                unique_ptr<Expression> res = make_unique<ShadowArg>(loc, MK::Local(loc, arg->name));
                result.swap(res);
            },
            [&](parser::DefMethod *method) {
                bool isSelf = false;
                unique_ptr<Expression> res =
                    buildMethod(dctx, method->loc, method->declLoc, method->name, method->args, method->body, isSelf);
                result.swap(res);
            },
            [&](parser::DefS *method) {
                auto *self = parser::cast_node<parser::Self>(method->singleton.get());
                if (self == nullptr) {
                    if (auto e = dctx.ctx.state.beginError(method->singleton->loc,
                                                           core::errors::Desugar::InvalidSingletonDef)) {
                        e.setHeader("`{}` is only supported for `{}`", "def EXPRESSION.method", "def self.method");
                    }
                    unique_ptr<Expression> res = MK::EmptyTree();
                    result.swap(res);
                    return;
                }
                bool isSelf = true;
                unique_ptr<Expression> res =
                    buildMethod(dctx, method->loc, method->declLoc, method->name, method->args, method->body, isSelf);
                result.swap(res);
            },
            [&](parser::SClass *sclass) {
                // This will be a nested ClassDef which we leave in the tree
                // which will get the symbol of `class.singleton_class`
                auto *self = parser::cast_node<parser::Self>(sclass->expr.get());
                if (self == nullptr) {
                    if (auto e =
                            dctx.ctx.state.beginError(sclass->expr->loc, core::errors::Desugar::InvalidSingletonDef)) {
                        e.setHeader("`{}` is only supported for `{}`", "class << EXPRESSION", "class << self");
                    }
                    unique_ptr<Expression> res = MK::EmptyTree();
                    result.swap(res);
                    return;
                }

                ClassDef::RHS_store body = scopeNodeToBody(dctx, std::move(sclass->body));
                ClassDef::ANCESTORS_store emptyAncestors;
                unique_ptr<Expression> res = make_unique<ClassDef>(
                    sclass->loc, sclass->declLoc, core::Symbols::todo(),
                    make_unique<UnresolvedIdent>(sclass->expr->loc, UnresolvedIdent::Class, core::Names::singleton()),
                    std::move(emptyAncestors), std::move(body), ClassDefKind::Class);
                result.swap(res);
            },
            [&](parser::Block *block) {
                block->send->loc = loc;
                auto recv = node2TreeImpl(dctx, std::move(block->send));
                Send *send;
                unique_ptr<Expression> res;
                if ((send = cast_tree<Send>(recv.get())) != nullptr) {
                    res.swap(recv);
                } else {
                    // This must have been a csend; That will have been desugared
                    // into an insseq with an If in the expression.
                    res.swap(recv);
                    auto *is = cast_tree<InsSeq>(res.get());
                    if (!is) {
                        if (auto e = dctx.ctx.state.beginError(block->loc, core::errors::Desugar::UnsupportedNode)) {
                            e.setHeader("No body in block");
                        }
                        auto res = MK::EmptyTree();
                        result.swap(res);
                        return;
                    }
                    auto *iff = cast_tree<If>(is->expr.get());
                    ENFORCE(iff != nullptr, "DesugarBlock: failed to find If");
                    send = cast_tree<Send>(iff->elsep.get());
                    ENFORCE(send != nullptr, "DesugarBlock: failed to find Send");
                }
                auto [args, destructures] = desugarArgs(dctx, loc, block->args);
                auto desugaredBody = desugarBody(dctx, loc, block->body, std::move(destructures));

                // TODO the send->block's loc is too big and includes the whole send
                send->block = make_unique<Block>(loc, std::move(args), std::move(desugaredBody));
                result.swap(res);
            },
            [&](parser::While *wl) {
                auto cond = node2TreeImpl(dctx, std::move(wl->cond));
                auto body = node2TreeImpl(dctx, std::move(wl->body));
                unique_ptr<Expression> res = make_unique<While>(loc, std::move(cond), std::move(body));
                result.swap(res);
            },
            // Most of the time a WhilePost is a normal while.
            // But it might be a do-while, in which case we do this:
            //
            // while true
            //   <temp> = <body>
            //   if ! <cond>
            //     break <temp>
            //   end
            // end
            [&](parser::WhilePost *wl) {
                bool isDoWhile = parser::isa_node<parser::Kwbegin>(wl->body.get());
                auto body = node2TreeImpl(dctx, std::move(wl->body));

                if (isDoWhile) {
                    auto cond = MK::Send0(loc, node2TreeImpl(dctx, std::move(wl->cond)), core::Names::bang());

                    auto temp = dctx.ctx.state.freshNameUnique(core::UniqueNameKind::Desugar, core::Names::forTemp(),
                                                               ++dctx.uniqueCounter);
                    auto withResult = MK::Assign(loc, temp, std::move(body));
                    auto breaker = MK::If(loc, std::move(cond), MK::Break(loc, MK::Local(loc, temp)), MK::EmptyTree());
                    auto breakWithResult = MK::InsSeq1(loc, std::move(withResult), std::move(breaker));
                    unique_ptr<Expression> res = make_unique<While>(loc, MK::True(loc), std::move(breakWithResult));
                    result.swap(res);
                } else {
                    auto cond = node2TreeImpl(dctx, std::move(wl->cond));
                    unique_ptr<Expression> res = make_unique<While>(loc, std::move(cond), std::move(body));
                    result.swap(res);
                }
            },
            [&](parser::Until *wl) {
                auto cond = MK::Send0(loc, node2TreeImpl(dctx, std::move(wl->cond)), core::Names::bang());
                auto body = node2TreeImpl(dctx, std::move(wl->body));
                unique_ptr<Expression> res = make_unique<While>(loc, std::move(cond), std::move(body));
                result.swap(res);
            },
            // This is the same as WhilePost, but the cond negation in the other branch.
            [&](parser::UntilPost *wl) {
                bool isDoUntil = parser::isa_node<parser::Kwbegin>(wl->body.get());
                auto body = node2TreeImpl(dctx, std::move(wl->body));

                if (isDoUntil) {
                    auto cond = node2TreeImpl(dctx, std::move(wl->cond));

                    auto temp = dctx.ctx.state.freshNameUnique(core::UniqueNameKind::Desugar, core::Names::forTemp(),
                                                               ++dctx.uniqueCounter);
                    auto withResult = MK::Assign(loc, temp, std::move(body));
                    auto breaker = MK::If(loc, std::move(cond), MK::Break(loc, MK::Local(loc, temp)), MK::EmptyTree());
                    auto breakWithResult = MK::InsSeq1(loc, std::move(withResult), std::move(breaker));
                    unique_ptr<Expression> res = make_unique<While>(loc, MK::True(loc), std::move(breakWithResult));
                    result.swap(res);
                } else {
                    auto cond = MK::Send0(loc, node2TreeImpl(dctx, std::move(wl->cond)), core::Names::bang());
                    unique_ptr<Expression> res = make_unique<While>(loc, std::move(cond), std::move(body));
                    result.swap(res);
                }
            },
            [&](parser::Nil *wl) {
                unique_ptr<Expression> res = MK::Nil(loc);
                result.swap(res);
            },
            [&](parser::IVar *var) {
                unique_ptr<Expression> res = make_unique<UnresolvedIdent>(loc, UnresolvedIdent::Instance, var->name);
                result.swap(res);
            },
            [&](parser::GVar *var) {
                unique_ptr<Expression> res = make_unique<UnresolvedIdent>(loc, UnresolvedIdent::Global, var->name);
                result.swap(res);
            },
            [&](parser::CVar *var) {
                unique_ptr<Expression> res = make_unique<UnresolvedIdent>(loc, UnresolvedIdent::Class, var->name);
                result.swap(res);
            },
            [&](parser::LVarLhs *var) {
                unique_ptr<Expression> res = MK::Local(loc, var->name);
                result.swap(res);
            },
            [&](parser::GVarLhs *var) {
                unique_ptr<Expression> res = make_unique<UnresolvedIdent>(loc, UnresolvedIdent::Global, var->name);
                result.swap(res);
            },
            [&](parser::CVarLhs *var) {
                unique_ptr<Expression> res = make_unique<UnresolvedIdent>(loc, UnresolvedIdent::Class, var->name);
                result.swap(res);
            },
            [&](parser::IVarLhs *var) {
                unique_ptr<Expression> res = make_unique<UnresolvedIdent>(loc, UnresolvedIdent::Instance, var->name);
                result.swap(res);
            },
            [&](parser::NthRef *var) {
                unique_ptr<Expression> res = make_unique<UnresolvedIdent>(
                    loc, UnresolvedIdent::Global, dctx.ctx.state.enterNameUTF8(to_string(var->ref)));
                result.swap(res);
            },
            [&](parser::Assign *asgn) {
                auto lhs = node2TreeImpl(dctx, std::move(asgn->lhs));
                auto rhs = node2TreeImpl(dctx, std::move(asgn->rhs));
                auto res = MK::Assign(loc, std::move(lhs), std::move(rhs));
                result.swap(res);
            },
            [&](parser::Super *super) {
                // Desugar super into a call to a normal method named `super`;
                // Do this by synthesizing a `Send` parse node and letting our
                // Send desugar handle it.
                auto method = core::Names::super();
                auto send = make_unique<parser::Send>(super->loc, nullptr, method, std::move(super->args));
                auto res = node2TreeImpl(dctx, std::move(send));
                result.swap(res);
            },
            [&](parser::ZSuper *zuper) {
                unique_ptr<Expression> res =
                    MK::Send1(loc, MK::Self(loc), core::Names::super(), make_unique<ZSuperArgs>(zuper->loc));
                result.swap(res);
            },
            [&](parser::For *for_) {
                auto temp = dctx.ctx.state.freshNameUnique(core::UniqueNameKind::Desugar, core::Names::forTemp(),
                                                           ++dctx.uniqueCounter);

                auto mlhsNode = std::move(for_->vars);
                if (!parser::isa_node<parser::Mlhs>(mlhsNode.get())) {
                    parser::NodeVec vars;
                    vars.emplace_back(std::move(mlhsNode));
                    mlhsNode = make_unique<parser::Mlhs>(loc, std::move(vars));
                }
                unique_ptr<parser::Node> masgn =
                    make_unique<parser::Masgn>(loc, std::move(mlhsNode), make_unique<parser::LVar>(loc, temp));

                InsSeq::STATS_store stats;
                stats.emplace_back(node2TreeImpl(dctx, std::move(masgn)));
                auto body = make_unique<InsSeq>(loc, std::move(stats), node2TreeImpl(dctx, std::move(for_->body)));

                MethodDef::ARGS_store blockArgs;
                blockArgs.emplace_back(make_unique<RestArg>(loc, MK::Local(loc, temp)));
                auto block = make_unique<Block>(loc, std::move(blockArgs), std::move(body));

                Send::ARGS_store noargs;
                auto res = MK::Send(loc, node2TreeImpl(dctx, std::move(for_->expr)), core::Names::each(),
                                    std::move(noargs), 0, std::move(block));
                result.swap(res);
            },
            [&](parser::Integer *integer) {
                int64_t val;
                auto underscorePos = integer->val.find("_");

                const string &withoutUnderscores =
                    (underscorePos == string::npos) ? integer->val : absl::StrReplaceAll(integer->val, {{"_", ""}});
                if (!absl::SimpleAtoi(withoutUnderscores, &val)) {
                    val = 0;
                    if (auto e = dctx.ctx.state.beginError(loc, core::errors::Desugar::IntegerOutOfRange)) {
                        e.setHeader("Unsupported integer literal: `{}`", integer->val);
                    }
                }

                unique_ptr<Expression> res = MK::Int(loc, val);
                result.swap(res);
            },
            [&](parser::Float *floatNode) {
                double val;
                auto underscorePos = floatNode->val.find("_");

                const string &withoutUnderscores =
                    (underscorePos == string::npos) ? floatNode->val : absl::StrReplaceAll(floatNode->val, {{"_", ""}});
                if (!absl::SimpleAtod(withoutUnderscores, &val)) {
                    val = numeric_limits<double>::quiet_NaN();
                    if (auto e = dctx.ctx.state.beginError(loc, core::errors::Desugar::FloatOutOfRange)) {
                        e.setHeader("Unsupported float literal: `{}`", floatNode->val);
                    }
                }

                unique_ptr<Expression> res = MK::Float(loc, val);
                result.swap(res);
            },
            [&](parser::Complex *complex) {
                auto kernel = MK::Constant(loc, core::Symbols::Kernel());
                core::NameRef complex_name = core::Names::Constants::Complex().data(dctx.ctx)->cnst.original;
                core::NameRef value = dctx.ctx.state.enterNameUTF8(complex->value);
                auto send = MK::Send2(loc, std::move(kernel), complex_name, MK::Int(loc, 0), MK::String(loc, value));
                result.swap(send);
            },
            [&](parser::Rational *complex) {
                auto kernel = MK::Constant(loc, core::Symbols::Kernel());
                core::NameRef complex_name = core::Names::Constants::Rational().data(dctx.ctx)->cnst.original;
                core::NameRef value = dctx.ctx.state.enterNameUTF8(complex->val);
                auto send = MK::Send1(loc, std::move(kernel), complex_name, MK::String(loc, value));
                result.swap(send);
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
                        auto var = MK::Send0(loc, node2TreeImpl(dctx, std::move(splat->var)), core::Names::to_a());
                        if (elems.empty()) {
                            if (lastMerge != nullptr) {
                                lastMerge = MK::Send1(loc, std::move(lastMerge), core::Names::concat(), std::move(var));
                            } else {
                                lastMerge = std::move(var);
                            }
                        } else {
                            unique_ptr<Expression> current = make_unique<Array>(loc, std::move(elems));
                            /* reassign instead of clear to work around https://bugs.llvm.org/show_bug.cgi?id=37553 */
                            elems = Array::ENTRY_store();
                            if (lastMerge != nullptr) {
                                lastMerge =
                                    MK::Send1(loc, std::move(lastMerge), core::Names::concat(), std::move(current));
                            } else {
                                lastMerge = std::move(current);
                            }
                            lastMerge = MK::Send1(loc, std::move(lastMerge), core::Names::concat(), std::move(var));
                        }
                    } else {
                        elems.emplace_back(node2TreeImpl(dctx, std::move(stat)));
                    }
                };

                unique_ptr<Expression> res;
                if (elems.empty()) {
                    if (lastMerge != nullptr) {
                        res = std::move(lastMerge);
                    } else {
                        // Empty array
                        res = make_unique<Array>(loc, std::move(elems));
                    }
                } else {
                    res = make_unique<Array>(loc, std::move(elems));
                    if (lastMerge != nullptr) {
                        res = MK::Send1(loc, std::move(lastMerge), core::Names::concat(), std::move(res));
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
                    auto *pair = parser::cast_node<parser::Pair>(pairAsExpression.get());
                    if (pair != nullptr) {
                        auto key = node2TreeImpl(dctx, std::move(pair->key));
                        auto value = node2TreeImpl(dctx, std::move(pair->value));
                        keys.emplace_back(std::move(key));
                        values.emplace_back(std::move(value));
                    } else {
                        auto *splat = parser::cast_node<parser::Kwsplat>(pairAsExpression.get());
                        ENFORCE(splat != nullptr, "kwsplat cast failed");

                        // Desguar
                        //   {a: 'a', **x, remaining}
                        // into
                        //   {a: 'a'}.merge(x.to_h).merge(remaining)
                        auto expr = MK::Send0(loc, node2TreeImpl(dctx, std::move(splat->expr)), core::Names::to_hash());
                        if (keys.empty()) {
                            if (lastMerge != nullptr) {
                                lastMerge = MK::Send1(loc, std::move(lastMerge), core::Names::merge(), std::move(expr));

                            } else {
                                lastMerge = std::move(expr);
                            }
                        } else {
                            unique_ptr<Expression> current = make_unique<Hash>(loc, std::move(keys), std::move(values));
                            /* reassign instead of clear to work around https://bugs.llvm.org/show_bug.cgi?id=37553 */
                            keys = Hash::ENTRY_store();
                            values = Hash::ENTRY_store();

                            if (lastMerge != nullptr) {
                                lastMerge =
                                    MK::Send1(loc, std::move(lastMerge), core::Names::merge(), std::move(current));
                            } else {
                                lastMerge = std::move(current);
                            }
                            lastMerge = MK::Send1(loc, std::move(lastMerge), core::Names::merge(), std::move(expr));
                        }
                    }
                };

                unique_ptr<Expression> res;
                if (keys.empty()) {
                    if (lastMerge != nullptr) {
                        res = std::move(lastMerge);
                    } else {
                        // Empty array
                        res = make_unique<Hash>(loc, std::move(keys), std::move(values));
                    }
                } else {
                    res = make_unique<Hash>(loc, std::move(keys), std::move(values));
                    if (lastMerge != nullptr) {
                        res = MK::Send1(loc, std::move(lastMerge), core::Names::merge(), std::move(res));
                    }
                }

                result.swap(res);
            },
            [&](parser::IRange *ret) {
                core::NameRef range_name = core::Symbols::Range().data(dctx.ctx)->name;
                unique_ptr<Expression> range = MK::UnresolvedConstant(loc, MK::EmptyTree(), range_name);
                auto from = node2TreeImpl(dctx, std::move(ret->from));
                auto to = node2TreeImpl(dctx, std::move(ret->to));
                auto send = MK::Send2(loc, std::move(range), core::Names::new_(), std::move(from), std::move(to));
                result.swap(send);
            },
            [&](parser::ERange *ret) {
                unique_ptr<Expression> range = MK::Constant(loc, core::Symbols::Range());
                auto from = node2TreeImpl(dctx, std::move(ret->from));
                auto to = node2TreeImpl(dctx, std::move(ret->to));
                auto true_ = MK::True(loc);
                auto send = MK::Send3(loc, std::move(range), core::Names::new_(), std::move(from), std::move(to),
                                      std::move(true_));
                result.swap(send);
            },
            [&](parser::Regexp *regexpNode) {
                unique_ptr<Expression> regexp = MK::Constant(loc, core::Symbols::Regexp());
                auto regex = desugarDString(dctx, loc, std::move(regexpNode->regex));
                auto opts = node2TreeImpl(dctx, std::move(regexpNode->opts));
                auto send = MK::Send2(loc, std::move(regexp), core::Names::new_(), std::move(regex), std::move(opts));
                result.swap(send);
            },
            [&](parser::Regopt *regopt) {
                unique_ptr<Expression> acc = MK::Int(loc, 0);
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
                        acc = MK::Send1(loc, std::move(acc), core::Names::orOp(), MK::Int(loc, flag));
                    }
                }
                result.swap(acc);
            },
            [&](parser::Return *ret) {
                if (ret->exprs.size() > 1) {
                    Array::ENTRY_store elems;
                    elems.reserve(ret->exprs.size());
                    for (auto &stat : ret->exprs) {
                        if (parser::isa_node<parser::BlockPass>(stat.get())) {
                            if (auto e = dctx.ctx.state.beginError(ret->loc, core::errors::Desugar::UnsupportedNode)) {
                                e.setHeader("Block argument should not be given");
                            }
                            continue;
                        }
                        elems.emplace_back(node2TreeImpl(dctx, std::move(stat)));
                    };
                    unique_ptr<Expression> arr = make_unique<Array>(loc, std::move(elems));
                    unique_ptr<Expression> res = make_unique<Return>(loc, std::move(arr));
                    result.swap(res);
                } else if (ret->exprs.size() == 1) {
                    if (parser::isa_node<parser::BlockPass>(ret->exprs[0].get())) {
                        if (auto e = dctx.ctx.state.beginError(ret->loc, core::errors::Desugar::UnsupportedNode)) {
                            e.setHeader("Block argument should not be given");
                        }
                        unique_ptr<Expression> res = make_unique<Break>(loc, MK::EmptyTree());
                        result.swap(res);
                    } else {
                        unique_ptr<Expression> res =
                            make_unique<Return>(loc, node2TreeImpl(dctx, std::move(ret->exprs[0])));
                        result.swap(res);
                    }
                } else {
                    unique_ptr<Expression> res = make_unique<Return>(loc, MK::EmptyTree());
                    result.swap(res);
                }
            },
            [&](parser::Break *ret) {
                if (ret->exprs.size() > 1) {
                    Array::ENTRY_store elems;
                    elems.reserve(ret->exprs.size());
                    for (auto &stat : ret->exprs) {
                        if (parser::isa_node<parser::BlockPass>(stat.get())) {
                            if (auto e = dctx.ctx.state.beginError(ret->loc, core::errors::Desugar::UnsupportedNode)) {
                                e.setHeader("Block argument should not be given");
                            }
                            continue;
                        }
                        elems.emplace_back(node2TreeImpl(dctx, std::move(stat)));
                    };
                    unique_ptr<Expression> arr = make_unique<Array>(loc, std::move(elems));
                    unique_ptr<Expression> res = make_unique<Break>(loc, std::move(arr));
                    result.swap(res);
                } else if (ret->exprs.size() == 1) {
                    if (parser::isa_node<parser::BlockPass>(ret->exprs[0].get())) {
                        if (auto e = dctx.ctx.state.beginError(ret->loc, core::errors::Desugar::UnsupportedNode)) {
                            e.setHeader("Block argument should not be given");
                        }
                        unique_ptr<Expression> res = make_unique<Break>(loc, MK::EmptyTree());
                        result.swap(res);
                    } else {
                        unique_ptr<Expression> res =
                            make_unique<Break>(loc, node2TreeImpl(dctx, std::move(ret->exprs[0])));
                        result.swap(res);
                    }
                } else {
                    unique_ptr<Expression> res = make_unique<Break>(loc, MK::EmptyTree());
                    result.swap(res);
                }
            },
            [&](parser::Next *ret) {
                if (ret->exprs.size() > 1) {
                    Array::ENTRY_store elems;
                    elems.reserve(ret->exprs.size());
                    for (auto &stat : ret->exprs) {
                        if (parser::isa_node<parser::BlockPass>(stat.get())) {
                            if (auto e = dctx.ctx.state.beginError(ret->loc, core::errors::Desugar::UnsupportedNode)) {
                                e.setHeader("Block argument should not be given");
                            }
                            continue;
                        }
                        elems.emplace_back(node2TreeImpl(dctx, std::move(stat)));
                    };
                    unique_ptr<Expression> arr = make_unique<Array>(loc, std::move(elems));
                    unique_ptr<Expression> res = make_unique<Next>(loc, std::move(arr));
                    result.swap(res);
                } else if (ret->exprs.size() == 1) {
                    if (parser::isa_node<parser::BlockPass>(ret->exprs[0].get())) {
                        if (auto e = dctx.ctx.state.beginError(ret->loc, core::errors::Desugar::UnsupportedNode)) {
                            e.setHeader("Block argument should not be given");
                        }
                        unique_ptr<Expression> res = make_unique<Break>(loc, MK::EmptyTree());
                        result.swap(res);
                    } else {
                        unique_ptr<Expression> res =
                            make_unique<Next>(loc, node2TreeImpl(dctx, std::move(ret->exprs[0])));
                        result.swap(res);
                    }
                } else {
                    unique_ptr<Expression> res = make_unique<Next>(loc, MK::EmptyTree());
                    result.swap(res);
                }
            },
            [&](parser::Retry *ret) {
                unique_ptr<Expression> res = make_unique<Retry>(loc);
                result.swap(res);
            },
            [&](parser::Yield *ret) {
                Send::ARGS_store args;
                args.reserve(ret->exprs.size());
                for (auto &stat : ret->exprs) {
                    args.emplace_back(node2TreeImpl(dctx, std::move(stat)));
                };

                unique_ptr<Expression> recv;
                if (dctx.enclosingBlockArg.exists()) {
                    recv = MK::Local(loc, dctx.enclosingBlockArg);
                } else {
                    // No enclosing block arg can happen when e.g. yield is called in a class / at the top-level.
                    recv = MK::Unsafe(loc, ast::MK::Nil(loc));
                }
                unique_ptr<Expression> res = MK::Send(loc, std::move(recv), core::Names::call(), std::move(args));
                result.swap(res);
            },
            [&](parser::Rescue *rescue) {
                Rescue::RESCUE_CASE_store cases;
                cases.reserve(rescue->rescue.size());
                for (auto &node : rescue->rescue) {
                    unique_ptr<Expression> rescueCaseExpr = node2TreeImpl(dctx, std::move(node));
                    auto rescueCase = cast_tree<ast::RescueCase>(rescueCaseExpr.get());
                    ENFORCE(rescueCase != nullptr, "rescue case cast failed");
                    cases.emplace_back(rescueCase);
                    rescueCaseExpr.release();
                }
                unique_ptr<Expression> res =
                    make_unique<Rescue>(loc, node2TreeImpl(dctx, std::move(rescue->body)), std::move(cases),
                                        node2TreeImpl(dctx, std::move(rescue->else_)), MK::EmptyTree());
                result.swap(res);
            },
            [&](parser::Resbody *resbody) {
                RescueCase::EXCEPTION_store exceptions;
                auto exceptionsExpr = node2TreeImpl(dctx, std::move(resbody->exception));
                if (isa_tree<EmptyTree>(exceptionsExpr.get())) {
                    // No exceptions captured
                } else if (auto exceptionsArray = cast_tree<ast::Array>(exceptionsExpr.get())) {
                    ENFORCE(exceptionsArray != nullptr, "exception array cast failed");

                    for (auto &elem : exceptionsArray->elems) {
                        exceptions.emplace_back(std::move(elem));
                    }
                } else if (auto exceptionsSend = cast_tree<ast::Send>(exceptionsExpr.get())) {
                    ENFORCE(exceptionsSend->fun == core::Names::splat() || exceptionsSend->fun == core::Names::to_a() ||
                                exceptionsSend->fun == core::Names::concat(),
                            "Unknown exceptionSend function");
                    exceptions.emplace_back(std::move(exceptionsExpr));
                } else {
                    Exception::raise("Bad inner node type");
                }

                auto varExpr = node2TreeImpl(dctx, std::move(resbody->var));
                auto body = node2TreeImpl(dctx, std::move(resbody->body));

                auto varLoc = varExpr->loc;
                auto var = core::NameRef::noName();
                if (auto *id = cast_tree<UnresolvedIdent>(varExpr.get())) {
                    if (id->kind == UnresolvedIdent::Local) {
                        var = id->name;
                        varExpr.reset(nullptr);
                    }
                }

                if (!var.exists()) {
                    var = dctx.ctx.state.freshNameUnique(core::UniqueNameKind::Desugar, core::Names::rescueTemp(),
                                                         ++dctx.uniqueCounter);
                }

                if (isa_tree<EmptyTree>(varExpr.get())) {
                    varLoc = loc;
                } else if (varExpr != nullptr) {
                    body = MK::InsSeq1(varLoc, MK::Assign(varLoc, std::move(varExpr), MK::Local(varLoc, var)),
                                       std::move(body));
                }

                unique_ptr<Expression> res =
                    make_unique<RescueCase>(loc, std::move(exceptions), MK::Local(varLoc, var), std::move(body));
                result.swap(res);
            },
            [&](parser::Ensure *ensure) {
                auto bodyExpr = node2TreeImpl(dctx, std::move(ensure->body));
                auto ensureExpr = node2TreeImpl(dctx, std::move(ensure->ensure));
                auto rescue = cast_tree<ast::Rescue>(bodyExpr.get());
                if (rescue != nullptr) {
                    rescue->ensure = std::move(ensureExpr);
                    result.swap(bodyExpr);
                } else {
                    Rescue::RESCUE_CASE_store cases;
                    unique_ptr<Expression> res = make_unique<Rescue>(loc, std::move(bodyExpr), std::move(cases),
                                                                     MK::EmptyTree(), std::move(ensureExpr));
                    result.swap(res);
                }
            },
            [&](parser::If *if_) {
                auto cond = node2TreeImpl(dctx, std::move(if_->condition));
                auto thenp = node2TreeImpl(dctx, std::move(if_->then_));
                auto elsep = node2TreeImpl(dctx, std::move(if_->else_));
                auto iff = MK::If(loc, std::move(cond), std::move(thenp), std::move(elsep));
                result.swap(iff);
            },
            [&](parser::Masgn *masgn) {
                auto *lhs = parser::cast_node<parser::Mlhs>(masgn->lhs.get());
                ENFORCE(lhs != nullptr, "Failed to get lhs of Masgn");

                auto res = desugarMlhs(dctx, loc, lhs, node2TreeImpl(dctx, std::move(masgn->rhs)));

                result.swap(res);
            },
            [&](parser::True *t) {
                auto res = MK::True(loc);
                result.swap(res);
            },
            [&](parser::False *t) {
                auto res = MK::False(loc);
                result.swap(res);
            },
            [&](parser::Case *case_) {
                unique_ptr<Expression> assign;
                auto temp = core::NameRef::noName();
                core::Loc cloc;

                if (case_->condition != nullptr) {
                    cloc = case_->condition->loc;
                    temp = dctx.ctx.state.freshNameUnique(core::UniqueNameKind::Desugar, core::Names::assignTemp(),
                                                          ++dctx.uniqueCounter);
                    assign = MK::Assign(cloc, temp, node2TreeImpl(dctx, std::move(case_->condition)));
                }
                unique_ptr<Expression> res = node2TreeImpl(dctx, std::move(case_->else_));
                for (auto it = case_->whens.rbegin(); it != case_->whens.rend(); ++it) {
                    auto when = parser::cast_node<parser::When>(it->get());
                    ENFORCE(when != nullptr, "case without a when?");
                    unique_ptr<Expression> cond;
                    for (auto &cnode : when->patterns) {
                        auto ctree = node2TreeImpl(dctx, std::move(cnode));
                        unique_ptr<Expression> test;
                        if (temp.exists()) {
                            auto local = MK::Local(cloc, temp);
                            auto patternloc = ctree->loc;
                            test = MK::Send1(patternloc, std::move(ctree), core::Names::tripleEq(), std::move(local));
                        } else {
                            test.swap(ctree);
                        }
                        if (cond == nullptr) {
                            cond.swap(test);
                        } else {
                            auto true_ = MK::True(test->loc);
                            auto loc = test->loc;
                            cond = MK::If(loc, std::move(test), std::move(true_), std::move(cond));
                        }
                    }
                    res =
                        MK::If(when->loc, std::move(cond), node2TreeImpl(dctx, std::move(when->body)), std::move(res));
                }
                if (assign != nullptr) {
                    res = MK::InsSeq1(loc, std::move(assign), std::move(res));
                }
                result.swap(res);
            },
            [&](parser::Splat *splat) {
                auto res = MK::Splat(loc, node2TreeImpl(dctx, std::move(splat->var)));
                result.swap(res);
            },
            [&](parser::Alias *alias) {
                auto res =
                    MK::Send2(loc, MK::Self(loc), core::Names::aliasMethod(),
                              node2TreeImpl(dctx, std::move(alias->from)), node2TreeImpl(dctx, std::move(alias->to)));
                result.swap(res);
            },
            [&](parser::Defined *defined) {
                auto res = MK::Send1(loc, MK::Constant(loc, core::Symbols::Magic()), core::Names::defined_p(),
                                     node2TreeImpl(dctx, std::move(defined->value)));
                result.swap(res);
            },
            [&](parser::LineLiteral *line) {
                auto pos = loc.position(dctx.ctx);
                ENFORCE(pos.first.line == pos.second.line, "position corrupted");
                auto res = MK::Int(loc, pos.first.line);
                result.swap(res);
            },
            [&](parser::XString *xstring) {
                auto res = MK::Send1(loc, MK::Self(loc), core::Names::backtick(),
                                     desugarDString(dctx, loc, std::move(xstring->nodes)));
                result.swap(res);
            },
            [&](parser::Preexe *preexe) {
                auto res = unsupportedNode(dctx, preexe);
                result.swap(res);
            },
            [&](parser::Postexe *postexe) {
                auto res = unsupportedNode(dctx, postexe);
                result.swap(res);
            },
            [&](parser::Undef *undef) {
                auto res = unsupportedNode(dctx, undef);
                result.swap(res);
            },
            [&](parser::Backref *backref) {
                auto res = unsupportedNode(dctx, backref);
                result.swap(res);
            },
            [&](parser::EFlipflop *eflipflop) {
                auto res = unsupportedNode(dctx, eflipflop);
                result.swap(res);
            },
            [&](parser::IFlipflop *iflipflop) {
                auto res = unsupportedNode(dctx, iflipflop);
                result.swap(res);
            },
            [&](parser::MatchCurLine *matchCurLine) {
                auto res = unsupportedNode(dctx, matchCurLine);
                result.swap(res);
            },
            [&](parser::Redo *redo) {
                auto res = unsupportedNode(dctx, redo);
                result.swap(res);
            },

            [&](parser::BlockPass *blockPass) { Exception::raise("Send should have already handled the BlockPass"); },
            [&](parser::Node *node) { Exception::raise("Unimplemented Parser Node: {}", node->nodeName()); });
        ENFORCE(result.get() != nullptr, "desugar result unset");
        return result;
    } catch (SorbetException &) {
        Exception::failInFuzzer();
        if (!locReported) {
            locReported = true;
            if (auto e = dctx.ctx.state.beginError(what->loc, core::errors::Internal::InternalError)) {
                e.setHeader("Failed to process tree (backtrace is above)");
            }
        }
        throw;
    }
}

unique_ptr<Expression> liftTopLevel(DesugarContext dctx, core::Loc loc, unique_ptr<Expression> what) {
    if (isa_tree<ClassDef>(what.get())) {
        return what;
    }

    ClassDef::RHS_store rhs;
    auto insSeq = cast_tree<InsSeq>(what.get());
    if (insSeq) {
        rhs.reserve(insSeq->stats.size() + 1);
        for (auto &stat : insSeq->stats) {
            rhs.emplace_back(std::move(stat));
        }
        rhs.emplace_back(std::move(insSeq->expr));
    } else {
        rhs.emplace_back(std::move(what));
    }
    return make_unique<ClassDef>(loc, loc, core::Symbols::root(), MK::EmptyTree(), ClassDef::ANCESTORS_store(),
                                 std::move(rhs), Class);
}
} // namespace

unique_ptr<Expression> node2Tree(core::MutableContext ctx, unique_ptr<parser::Node> what) {
    try {
        u2 uniqueCounter = 1;
        // We don't have an enclosing block arg to start off.
        DesugarContext dctx(ctx, uniqueCounter, core::NameRef::noName());
        auto loc = what->loc;
        auto result = node2TreeImpl(dctx, std::move(what));
        result = liftTopLevel(dctx, loc, std::move(result));
        auto verifiedResult = Verifier::run(ctx, std::move(result));
        return verifiedResult;
    } catch (SorbetException &) {
        locReported = false;
        throw;
    }
}
} // namespace sorbet::ast::desugar
