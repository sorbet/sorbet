#include <algorithm>

#include "absl/strings/numbers.h"
#include "absl/strings/str_replace.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "ast/desugar/Desugar.h"
#include "ast/verifier/verifier.h"
#include "common/common.h"
#include "common/formatting.h"
#include "core/Names.h"
#include "core/errors/desugar.h"
#include "core/errors/internal.h"

namespace sorbet::ast::desugar {

using namespace std;

namespace {

struct DesugarContext final {
    core::MutableContext ctx;
    u4 &uniqueCounter;
    core::NameRef enclosingBlockArg;
    core::Loc enclosingMethodLoc;
    core::NameRef enclosingMethodName;

    DesugarContext(core::MutableContext ctx, u4 &uniqueCounter, core::NameRef enclosingBlockArg,
                   core::Loc enclosingMethodLoc, core::NameRef enclosingMethodName)
        : ctx(ctx), uniqueCounter(uniqueCounter), enclosingBlockArg(enclosingBlockArg),
          enclosingMethodLoc(enclosingMethodLoc), enclosingMethodName(enclosingMethodName){};
};

core::NameRef blockArg2Name(DesugarContext dctx, const BlockArg &blkArg) {
    auto blkIdent = cast_tree_const<UnresolvedIdent>(blkArg.expr);
    ENFORCE(blkIdent != nullptr, "BlockArg must wrap UnresolvedIdent in desugar.");
    return blkIdent->name;
}

TreePtr node2TreeImpl(DesugarContext dctx, unique_ptr<parser::Node> what);

pair<MethodDef::ARGS_store, InsSeq::STATS_store> desugarArgs(DesugarContext dctx, core::LocOffsets loc,
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
        Exception::raise("not implemented: {}", argnode->nodeName());
    }

    return make_pair(std::move(args), std::move(destructures));
}

TreePtr desugarBody(DesugarContext dctx, core::LocOffsets loc, unique_ptr<parser::Node> &bodynode,
                    InsSeq::STATS_store destructures) {
    auto body = node2TreeImpl(dctx, std::move(bodynode));
    if (!destructures.empty()) {
        auto bodyLoc = body->loc;
        if (!bodyLoc.exists()) {
            bodyLoc = loc;
        }
        body = MK::InsSeq(loc, std::move(destructures), std::move(body));
    }

    return body;
}
bool isStringLit(DesugarContext dctx, TreePtr &expr) {
    Literal *lit;
    return (lit = cast_tree<Literal>(expr)) && lit->isString(dctx.ctx);
}

TreePtr mergeStrings(DesugarContext dctx, core::LocOffsets loc, InlinedVector<TreePtr, 4> stringsAccumulated) {
    if (stringsAccumulated.size() == 1) {
        return move(stringsAccumulated[0]);
    } else {
        return MK::String(
            loc,
            dctx.ctx.state.enterNameUTF8(fmt::format(
                "{}", fmt::map_join(stringsAccumulated.begin(), stringsAccumulated.end(), "", [&](const auto &expr) {
                    if (isa_tree<EmptyTree>(expr))
                        return ""sv;
                    else
                        return cast_tree_const<Literal>(expr)->asString(dctx.ctx).data(dctx.ctx)->shortName(dctx.ctx);
                }))));
    }
}

TreePtr desugarDString(DesugarContext dctx, core::LocOffsets loc, parser::NodeVec nodes) {
    if (nodes.empty()) {
        return MK::String(loc, core::Names::empty());
    }
    auto it = nodes.begin();
    auto end = nodes.end();
    TreePtr first = node2TreeImpl(dctx, std::move(*it));
    InlinedVector<TreePtr, 4> stringsAccumulated;

    Send::ARGS_store interpArgs;

    bool allStringsSoFar;
    if (isStringLit(dctx, first) || isa_tree<EmptyTree>(first)) {
        stringsAccumulated.emplace_back(std::move(first));
        allStringsSoFar = true;
    } else {
        interpArgs.emplace_back(std::move(first));
        allStringsSoFar = false;
    }
    ++it;

    for (; it != end; ++it) {
        auto &stat = *it;
        TreePtr narg = node2TreeImpl(dctx, std::move(stat));
        if (allStringsSoFar && isStringLit(dctx, narg)) {
            stringsAccumulated.emplace_back(std::move(narg));
        } else if (isa_tree<EmptyTree>(narg)) {
            // no op
        } else {
            if (allStringsSoFar) {
                allStringsSoFar = false;
                interpArgs.emplace_back(mergeStrings(dctx, loc, std::move(stringsAccumulated)));
            }
            interpArgs.emplace_back(std::move(narg));
        }
    };
    if (allStringsSoFar) {
        return mergeStrings(dctx, loc, std::move(stringsAccumulated));
    } else {
        auto recv = MK::Constant(loc, core::Symbols::Magic());
        return MK::Send(loc, std::move(recv), core::Names::stringInterpolate(), std::move(interpArgs));
    }
}

bool isIVarAssign(TreePtr &stat) {
    auto assign = cast_tree<Assign>(stat);
    if (!assign) {
        return false;
    }
    auto ident = cast_tree<UnresolvedIdent>(assign->lhs);
    if (!ident) {
        return false;
    }
    if (ident->kind != UnresolvedIdent::Kind::Instance) {
        return false;
    }
    return true;
}

TreePtr validateRBIBody(DesugarContext dctx, TreePtr body) {
    if (!dctx.enclosingMethodLoc.file().data(dctx.ctx).isRBI()) {
        return body;
    }
    if (!body->loc.exists()) {
        return body;
    }

    auto loc = core::Loc(dctx.enclosingMethodLoc.file(), body->loc);
    if (isa_tree<EmptyTree>(body)) {
        return body;
    } else if (isa_tree<Assign>(body)) {
        if (!isIVarAssign(body)) {
            if (auto e = dctx.ctx.beginError(body->loc, core::errors::Desugar::CodeInRBI)) {
                e.setHeader("RBI methods must not have code");
                e.replaceWith("Delete the body", loc, "");
            }
        }
    } else if (auto inseq = cast_tree<InsSeq>(body)) {
        for (auto &stat : inseq->stats) {
            if (!isIVarAssign(stat)) {
                if (auto e = dctx.ctx.beginError(stat->loc, core::errors::Desugar::CodeInRBI)) {
                    e.setHeader("RBI methods must not have code");
                    e.replaceWith("Delete the body", loc, "");
                }
            }
        }
        if (!isIVarAssign(inseq->expr)) {
            if (auto e = dctx.ctx.beginError(inseq->expr->loc, core::errors::Desugar::CodeInRBI)) {
                e.setHeader("RBI methods must not have code");
                e.replaceWith("Delete the body", loc, "");
            }
        }
    } else {
        if (auto e = dctx.ctx.beginError(body->loc, core::errors::Desugar::CodeInRBI)) {
            e.setHeader("RBI methods must not have code");
            e.replaceWith("Delete the body", loc, "");
        }
    }
    return body;
}

TreePtr buildMethod(DesugarContext dctx, core::LocOffsets loc, core::Loc declLoc, core::NameRef name,
                    unique_ptr<parser::Node> &argnode, unique_ptr<parser::Node> &body, bool isSelf) {
    // Reset uniqueCounter within this scope (to keep numbers small)
    u4 uniqueCounter = 1;
    DesugarContext dctx1(dctx.ctx, uniqueCounter, dctx.enclosingBlockArg, declLoc, name);
    auto [args, destructures] = desugarArgs(dctx1, loc, argnode);

    if (args.empty() || !isa_tree<BlockArg>(args.back())) {
        auto blkLoc = core::LocOffsets::none();
        args.emplace_back(MK::BlockArg(blkLoc, MK::Local(blkLoc, core::Names::blkArg())));
    }

    const auto &blkArg = cast_tree<BlockArg>(args.back());
    ENFORCE(blkArg != nullptr, "Every method's last arg must be a block arg by now.");
    auto enclosingBlockArg = blockArg2Name(dctx, *blkArg);

    DesugarContext dctx2(dctx1.ctx, dctx1.uniqueCounter, enclosingBlockArg, declLoc, name);
    TreePtr desugaredBody = desugarBody(dctx2, loc, body, std::move(destructures));
    desugaredBody = validateRBIBody(dctx2, move(desugaredBody));

    auto mdef = MK::Method(loc, declLoc, name, std::move(args), std::move(desugaredBody));
    cast_tree<MethodDef>(mdef)->flags.isSelfMethod = isSelf;
    return mdef;
}

TreePtr symbol2Proc(DesugarContext dctx, TreePtr expr) {
    auto loc = expr->loc;
    core::NameRef temp = dctx.ctx.state.freshNameUnique(core::UniqueNameKind::Desugar, core::Names::blockPassTemp(),
                                                        ++dctx.uniqueCounter);
    Literal *lit = cast_tree<Literal>(expr);
    ENFORCE(lit && lit->isSymbol(dctx.ctx));

    // &:foo => {|temp| temp.foo() }
    core::NameRef name(dctx.ctx, core::cast_type<core::LiteralType>(lit->value.get())->value);
    // `temp` does not refer to any specific source text, so give it a 0-length Loc so LSP ignores it.
    auto zeroLengthLoc = loc.copyWithZeroLength();
    TreePtr recv = MK::Local(zeroLengthLoc, temp);
    TreePtr body = MK::Send0(loc, std::move(recv), name);
    return MK::Block1(loc, std::move(body), MK::Local(zeroLengthLoc, temp));
}

TreePtr unsupportedNode(DesugarContext dctx, parser::Node *node) {
    if (auto e = dctx.ctx.beginError(node->loc, core::errors::Desugar::UnsupportedNode)) {
        e.setHeader("Unsupported node type `{}`", node->nodeName());
    }
    return MK::EmptyTree();
}

TreePtr desugarMlhs(DesugarContext dctx, core::LocOffsets loc, parser::Mlhs *lhs, TreePtr rhs) {
    InsSeq::STATS_store stats;

    core::NameRef tempRhs =
        dctx.ctx.state.freshNameUnique(core::UniqueNameKind::Desugar, core::Names::assignTemp(), ++dctx.uniqueCounter);
    core::NameRef tempExpanded =
        dctx.ctx.state.freshNameUnique(core::UniqueNameKind::Desugar, core::Names::assignTemp(), ++dctx.uniqueCounter);

    int i = 0;
    int before = 0, after = 0;
    bool didSplat = false;

    for (auto &c : lhs->exprs) {
        if (auto *splat = parser::cast_node<parser::SplatLhs>(c.get())) {
            ENFORCE(!didSplat, "did splat already");
            didSplat = true;

            TreePtr lh = node2TreeImpl(dctx, std::move(splat->var));

            int left = i;
            int right = lhs->exprs.size() - left - 1;
            if (!isa_tree<EmptyTree>(lh)) {
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
                               MK::Send1(loc, MK::Local(loc, tempExpanded), core::Names::slice(), std::move(index))));
            }
            i = -right;
        } else {
            if (didSplat) {
                ++after;
            } else {
                ++before;
            }
            auto val = MK::Send1(loc, MK::Local(loc, tempExpanded), core::Names::squareBrackets(), MK::Int(loc, i));

            if (auto *mlhs = parser::cast_node<parser::Mlhs>(c.get())) {
                stats.emplace_back(desugarMlhs(dctx, mlhs->loc, mlhs, std::move(val)));
            } else {
                TreePtr lh = node2TreeImpl(dctx, std::move(c));
                if (auto restArg = cast_tree<RestArg>(lh)) {
                    if (auto e = dctx.ctx.beginError(lh->loc, core::errors::Desugar::UnsupportedRestArgsDestructure)) {
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
                              MK::Local(loc, tempRhs), MK::Int(loc, before), MK::Int(loc, after));
    stats.insert(stats.begin(), MK::Assign(loc, tempExpanded, std::move(expanded)));
    stats.insert(stats.begin(), MK::Assign(loc, tempRhs, std::move(rhs)));

    // Regardless of how we destructure an assignment, Ruby evaluates the expression to the entire right hand side,
    // not any individual component of the destructured assignment.
    return MK::InsSeq(loc, std::move(stats), MK::Local(loc, tempRhs));
}

bool locReported = false;

ClassDef::RHS_store scopeNodeToBody(DesugarContext dctx, unique_ptr<parser::Node> node) {
    ClassDef::RHS_store body;
    // Reset uniqueCounter within this scope (to keep numbers small)
    u4 uniqueCounter = 1;
    DesugarContext dctx1(dctx.ctx, uniqueCounter, dctx.enclosingBlockArg, dctx.enclosingMethodLoc,
                         dctx.enclosingMethodName);
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

struct OpAsgnScaffolding {
    core::NameRef temporaryName;
    InsSeq::STATS_store statementBody;
    Send::ARGS_store readArgs;
    Send::ARGS_store assgnArgs;
};

// Desugaring passes for op-assignments (like += or &&=) will first desugar the LHS, which often results in a send if
// there's a dot anywhere on the LHS. Consider an expression like `x.y += 1`. We'll want to desugar this to
//
//   { $tmp = x.y; x.y = $tmp + 1 }
//
// which now involves two (slightly different) sends: the .y() in the first statement, and the .y=() in the second
// statement. The first one will have as many arguments as the original, while the second will have one more than the
// original (to allow for the passed value). This function creates both argument lists as well as the instruction block
// and the temporary variable: how these will be used will differ slightly depending on whether we're desugaring &&=,
// ||=, or some other op-assign, but the logic contained here will stay in common.
OpAsgnScaffolding copyArgsForOpAsgn(DesugarContext dctx, Send *s) {
    // this is for storing the temporary assignments followed by the final update. In the case that we have other
    // arguments to the send (e.g. in the case of x.y[z] += 1) we'll want to store the other parameters (z) in a
    // temporary as well, producing a sequence like
    //
    //   { $arg = z; $tmp = x.y[$arg]; x.y[$arg] = $tmp + 1 }
    //
    // This means we'll always need statements for as many arguments as the send has, plus two more: one for the
    // temporary assignment and the last for the actual update we're desugaring.
    InsSeq::STATS_store stats;
    stats.reserve(s->args.size() + 2);
    core::NameRef tempRecv =
        dctx.ctx.state.freshNameUnique(core::UniqueNameKind::Desugar, s->fun, ++dctx.uniqueCounter);
    stats.emplace_back(MK::Assign(s->loc, tempRecv, std::move(s->recv)));
    Send::ARGS_store readArgs;
    Send::ARGS_store assgnArgs;
    // these are the arguments for the first send, e.g. x.y(). The number of arguments should be identical to whatever
    // we saw on the LHS.
    readArgs.reserve(s->args.size());
    // these are the arguments for the second send, e.g. x.y=(val). That's why we need the space for the extra argument
    // here: to accomodate the call to field= instead of just field.
    assgnArgs.reserve(s->args.size() + 1);

    for (auto &arg : s->args) {
        auto argLoc = arg->loc;
        core::NameRef name =
            dctx.ctx.state.freshNameUnique(core::UniqueNameKind::Desugar, s->fun, ++dctx.uniqueCounter);
        stats.emplace_back(MK::Assign(argLoc, name, std::move(arg)));
        readArgs.emplace_back(MK::Local(argLoc, name));
        assgnArgs.emplace_back(MK::Local(argLoc, name));
    }

    return {tempRecv, std::move(stats), std::move(readArgs), std::move(assgnArgs)};
}

// while true
//   body
//   if cond
//     break
//   end
// end
TreePtr doUntil(DesugarContext dctx, core::LocOffsets loc, TreePtr cond, TreePtr body) {
    auto breaker = MK::If(loc, std::move(cond), MK::Break(loc, MK::EmptyTree()), MK::EmptyTree());
    auto breakWithBody = MK::InsSeq1(loc, std::move(body), std::move(breaker));
    return MK::While(loc, MK::True(loc), std::move(breakWithBody));
}

TreePtr node2TreeImpl(DesugarContext dctx, unique_ptr<parser::Node> what) {
    try {
        if (what.get() == nullptr) {
            return MK::EmptyTree();
        }
        auto loc = what->loc;
        ENFORCE(loc.exists(), "parse-tree node has no location: {}", what->toString(dctx.ctx));
        TreePtr result;
        typecase(
            what.get(),
            // The top N clauses here are ordered according to observed
            // frequency in pay-server. Do not reorder the top of this list, or
            // add entries here, without consulting the "node.*" counters from a
            // run over a representative code base.
            [&](parser::Send *send) {
                Send::Flags flags;
                auto rec = node2TreeImpl(dctx, std::move(send->receiver));
                if (isa_tree<EmptyTree>(rec)) {
                    // 0-sized Loc, since `self.` doesn't appear in the original file.
                    rec = MK::Self(loc.copyWithZeroLength());
                    flags.isPrivateOk = true;
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
                    TreePtr res;
                    if (block == nullptr) {
                        res = MK::Send(loc, MK::Constant(loc, core::Symbols::Magic()), core::Names::callWithSplat(),
                                       std::move(sendargs), {});
                    } else {
                        auto convertedBlock = node2TreeImpl(dctx, std::move(block));
                        Literal *lit;
                        if ((lit = cast_tree<Literal>(convertedBlock)) && lit->isSymbol(dctx.ctx)) {
                            res = MK::Send(loc, MK::Constant(loc, core::Symbols::Magic()), core::Names::callWithSplat(),
                                           std::move(sendargs), {}, symbol2Proc(dctx, std::move(convertedBlock)));
                        } else {
                            sendargs.emplace_back(std::move(convertedBlock));
                            res = MK::Send(loc, MK::Constant(loc, core::Symbols::Magic()),
                                           core::Names::callWithSplatAndBlock(), std::move(sendargs), {});
                        }
                    }
                    result = std::move(res);
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

                    TreePtr res;
                    if (block == nullptr) {
                        res = MK::Send(loc, std::move(rec), send->method, std::move(args), flags);
                    } else {
                        auto method =
                            MK::Literal(loc, core::make_type<core::LiteralType>(core::Symbols::Symbol(), send->method));
                        auto convertedBlock = node2TreeImpl(dctx, std::move(block));
                        Literal *lit;
                        if ((lit = cast_tree<Literal>(convertedBlock)) && lit->isSymbol(dctx.ctx)) {
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
                                           std::move(sendargs), {});
                        }
                    }

                    if (send->method == core::Names::blockGiven_p() && dctx.enclosingBlockArg.exists()) {
                        auto if_ = MK::If(loc, MK::Local(loc, dctx.enclosingBlockArg), std::move(res), MK::False(loc));
                        result = std::move(if_);
                    } else {
                        result = std::move(res);
                    }
                }
            },
            [&](parser::Const *const_) {
                auto scope = node2TreeImpl(dctx, std::move(const_->scope));
                TreePtr res = MK::UnresolvedConstant(loc, std::move(scope), const_->name);
                result = std::move(res);
            },
            [&](parser::String *string) {
                TreePtr res = MK::String(loc, string->val);
                result = std::move(res);
            },
            [&](parser::Symbol *symbol) {
                TreePtr res = MK::Symbol(loc, symbol->val);
                result = std::move(res);
            },
            [&](parser::LVar *var) {
                TreePtr res = MK::Local(loc, var->name);
                result = std::move(res);
            },
            [&](parser::DString *dstring) {
                TreePtr res = desugarDString(dctx, loc, std::move(dstring->nodes));
                result = std::move(res);
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
                    result = std::move(block);
                } else {
                    TreePtr res = MK::Nil(loc);
                    result = std::move(res);
                }
            },
            // END hand-ordered clauses
            [&](parser::And *and_) {
                auto lhs = node2TreeImpl(dctx, std::move(and_->left));
                auto rhs = node2TreeImpl(dctx, std::move(and_->right));
                if (isa_reference(lhs)) {
                    auto cond = MK::cpRef(lhs);
                    auto iff = MK::If(loc, std::move(cond), std::move(rhs), std::move(lhs));
                    result = std::move(iff);
                } else {
                    core::NameRef tempName = dctx.ctx.state.freshNameUnique(
                        core::UniqueNameKind::Desugar, core::Names::andAnd(), ++dctx.uniqueCounter);
                    auto temp = MK::Assign(loc, tempName, std::move(lhs));
                    auto iff = MK::If(loc, MK::Local(loc, tempName), std::move(rhs), MK::Local(loc, tempName));
                    auto wrapped = MK::InsSeq1(loc, std::move(temp), std::move(iff));
                    result = std::move(wrapped);
                }
            },
            [&](parser::Or *or_) {
                auto lhs = node2TreeImpl(dctx, std::move(or_->left));
                auto rhs = node2TreeImpl(dctx, std::move(or_->right));
                if (isa_reference(lhs)) {
                    auto cond = MK::cpRef(lhs);
                    auto iff = MK::If(loc, std::move(cond), std::move(lhs), std::move(rhs));
                    result = std::move(iff);
                } else {
                    core::NameRef tempName = dctx.ctx.state.freshNameUnique(core::UniqueNameKind::Desugar,
                                                                            core::Names::orOr(), ++dctx.uniqueCounter);
                    auto temp = MK::Assign(loc, tempName, std::move(lhs));
                    auto iff = MK::If(loc, MK::Local(loc, tempName), MK::Local(loc, tempName), std::move(rhs));
                    auto wrapped = MK::InsSeq1(loc, std::move(temp), std::move(iff));
                    result = std::move(wrapped);
                }
            },
            [&](parser::AndAsgn *andAsgn) {
                auto recv = node2TreeImpl(dctx, std::move(andAsgn->left));
                auto arg = node2TreeImpl(dctx, std::move(andAsgn->right));
                if (auto s = cast_tree<Send>(recv)) {
                    auto sendLoc = s->loc;
                    auto [tempRecv, stats, readArgs, assgnArgs] = copyArgsForOpAsgn(dctx, s);

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
                    result = std::move(wrapped);
                } else if (isa_reference(recv)) {
                    auto cond = MK::cpRef(recv);
                    auto elsep = MK::cpRef(recv);
                    auto body = MK::Assign(loc, std::move(recv), std::move(arg));
                    auto iff = MK::If(loc, std::move(cond), std::move(body), std::move(elsep));
                    result = std::move(iff);
                } else if (auto i = cast_tree<UnresolvedConstantLit>(recv)) {
                    if (auto e = dctx.ctx.beginError(what->loc, core::errors::Desugar::NoConstantReassignment)) {
                        e.setHeader("Constant reassignment is not supported");
                    }
                    TreePtr res = MK::EmptyTree();
                    result = std::move(res);
                } else if (auto i = cast_tree<InsSeq>(recv)) {
                    // The logic below is explained more fully in the OpAsgn case
                    auto ifExpr = cast_tree<If>(i->expr);
                    if (!ifExpr) {
                        Exception::raise("Unexpected left-hand side of &&=: please file an issue");
                    }
                    auto s = cast_tree<Send>(ifExpr->elsep);
                    if (!s) {
                        Exception::raise("Unexpected left-hand side of &&=: please file an issue");
                    }

                    auto sendLoc = s->loc;
                    auto [tempRecv, stats, readArgs, assgnArgs] = copyArgsForOpAsgn(dctx, s);
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
                    ifExpr->elsep = std::move(wrapped);
                    result = std::move(recv);

                } else {
                    // the LHS has been desugared to something we haven't expected
                    Exception::notImplemented();
                }
            },
            [&](parser::OrAsgn *orAsgn) {
                auto recv = node2TreeImpl(dctx, std::move(orAsgn->left));
                auto arg = node2TreeImpl(dctx, std::move(orAsgn->right));
                if (auto s = cast_tree<Send>(recv)) {
                    auto sendLoc = s->loc;
                    auto [tempRecv, stats, readArgs, assgnArgs] = copyArgsForOpAsgn(dctx, s);
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
                    result = std::move(wrapped);
                } else if (isa_reference(recv)) {
                    auto cond = MK::cpRef(recv);
                    auto elsep = MK::cpRef(recv);
                    auto body = MK::Assign(loc, std::move(recv), std::move(arg));
                    auto iff = MK::If(loc, std::move(cond), std::move(elsep), std::move(body));
                    result = std::move(iff);
                } else if (auto i = cast_tree<UnresolvedConstantLit>(recv)) {
                    if (auto e = dctx.ctx.beginError(what->loc, core::errors::Desugar::NoConstantReassignment)) {
                        e.setHeader("Constant reassignment is not supported");
                    }
                    TreePtr res = MK::EmptyTree();
                    result = std::move(res);
                } else if (auto i = cast_tree<InsSeq>(recv)) {
                    // The logic below is explained more fully in the OpAsgn case
                    auto ifExpr = cast_tree<If>(i->expr);
                    if (!ifExpr) {
                        Exception::raise("Unexpected left-hand side of &&=: please file an issue");
                    }
                    auto s = cast_tree<Send>(ifExpr->elsep);
                    if (!s) {
                        Exception::raise("Unexpected left-hand side of &&=: please file an issue");
                    }

                    auto sendLoc = s->loc;
                    auto [tempRecv, stats, readArgs, assgnArgs] = copyArgsForOpAsgn(dctx, s);
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
                    ifExpr->elsep = std::move(wrapped);
                    result = std::move(recv);

                } else {
                    // the LHS has been desugared to something that we haven't expected
                    Exception::notImplemented();
                }
            },
            [&](parser::OpAsgn *opAsgn) {
                auto recv = node2TreeImpl(dctx, std::move(opAsgn->left));
                auto rhs = node2TreeImpl(dctx, std::move(opAsgn->right));
                if (auto s = cast_tree<Send>(recv)) {
                    auto sendLoc = s->loc;
                    auto [tempRecv, stats, readArgs, assgnArgs] = copyArgsForOpAsgn(dctx, s);

                    auto prevValue =
                        MK::Send(sendLoc, MK::Local(sendLoc, tempRecv), s->fun, std::move(readArgs), s->flags);
                    auto newValue = MK::Send1(sendLoc, std::move(prevValue), opAsgn->op, std::move(rhs));
                    assgnArgs.emplace_back(std::move(newValue));

                    auto res = MK::Send(sendLoc, MK::Local(sendLoc, tempRecv), s->fun.addEq(dctx.ctx),
                                        std::move(assgnArgs), s->flags);
                    auto wrapped = MK::InsSeq(loc, std::move(stats), std::move(res));
                    result = std::move(wrapped);
                } else if (isa_reference(recv)) {
                    auto lhs = MK::cpRef(recv);
                    auto send = MK::Send1(loc, std::move(recv), opAsgn->op, std::move(rhs));
                    auto res = MK::Assign(loc, std::move(lhs), std::move(send));
                    result = std::move(res);
                } else if (auto i = cast_tree<UnresolvedConstantLit>(recv)) {
                    if (auto e = dctx.ctx.beginError(what->loc, core::errors::Desugar::NoConstantReassignment)) {
                        e.setHeader("Constant reassignment is not supported");
                    }
                    TreePtr res = MK::EmptyTree();
                    result = std::move(res);
                } else if (auto i = cast_tree<InsSeq>(recv)) {
                    // if this is an InsSeq, then is probably the result of a safe send (i.e. an expression of the form
                    // x&.y on the LHS) which means it'll take the rough shape:
                    //   { $temp = x; if $temp == nil then nil else $temp.y }
                    // on the LHS. We want to insert the y= into the if-expression at the end, like:
                    //   { $temp = x; if $temp == nil then nil else { $t2 = $temp.y; $temp.y = $t2 op RHS } }
                    // that means we first need to find out whether the final expression is an If...
                    auto ifExpr = cast_tree<If>(i->expr);
                    if (!ifExpr) {
                        Exception::raise("Unexpected left-hand side of &&=: please file an issue");
                    }
                    // and if so, find out whether the else-case is a send...
                    auto s = cast_tree<Send>(ifExpr->elsep);
                    if (!s) {
                        Exception::raise("Unexpected left-hand side of &&=: please file an issue");
                    }
                    // and then perform basically the same logic as above for a send, but replacing it within
                    // the else-case of the if at the end instead
                    auto sendLoc = s->loc;
                    auto [tempRecv, stats, readArgs, assgnArgs] = copyArgsForOpAsgn(dctx, s);
                    auto prevValue =
                        MK::Send(sendLoc, MK::Local(sendLoc, tempRecv), s->fun, std::move(readArgs), s->flags);
                    auto newValue = MK::Send1(sendLoc, std::move(prevValue), opAsgn->op, std::move(rhs));
                    assgnArgs.emplace_back(std::move(newValue));

                    auto res = MK::Send(sendLoc, MK::Local(sendLoc, tempRecv), s->fun.addEq(dctx.ctx),
                                        std::move(assgnArgs), s->flags);
                    auto wrapped = MK::InsSeq(loc, std::move(stats), std::move(res));
                    ifExpr->elsep = std::move(wrapped);
                    result = std::move(recv);

                } else {
                    // the LHS has been desugared to something we haven't expected
                    Exception::notImplemented();
                }
            },
            [&](parser::CSend *csend) {
                core::NameRef tempRecv = dctx.ctx.state.freshNameUnique(
                    core::UniqueNameKind::Desugar, core::Names::assignTemp(), ++dctx.uniqueCounter);
                auto recvLoc = csend->receiver->loc;
                // Assign some desugar-produced nodes with zero-length Locs so IDE ignores them when mapping text
                // location to node.
                auto zeroLengthLoc = loc.copyWithZeroLength();
                auto zeroLengthRecvLoc = recvLoc.copyWithZeroLength();

                auto assgn = MK::Assign(zeroLengthRecvLoc, tempRecv, node2TreeImpl(dctx, std::move(csend->receiver)));

                // Just compare with `NilClass` to avoid potentially calling into a class-defined `==`
                auto cond = MK::Send1(zeroLengthLoc, ast::MK::Constant(zeroLengthRecvLoc, core::Symbols::NilClass()),
                                      core::Names::tripleEq(), MK::Local(zeroLengthRecvLoc, tempRecv));

                unique_ptr<parser::Node> sendNode = make_unique<parser::Send>(
                    loc, make_unique<parser::LVar>(recvLoc, tempRecv), csend->method, std::move(csend->args));
                auto send = node2TreeImpl(dctx, std::move(sendNode));

                TreePtr nil = MK::Nil(zeroLengthLoc);
                auto iff = MK::If(zeroLengthLoc, std::move(cond), std::move(nil), std::move(send));
                auto res = MK::InsSeq1(zeroLengthLoc, std::move(assgn), std::move(iff));
                result = std::move(res);
            },
            [&](parser::Self *self) {
                TreePtr res = MK::Self(loc);
                result = std::move(res);
            },
            [&](parser::DSymbol *dsymbol) {
                if (dsymbol->nodes.empty()) {
                    TreePtr res = MK::Symbol(loc, core::Names::empty());
                    result = std::move(res);
                    return;
                }

                auto str = desugarDString(dctx, loc, std::move(dsymbol->nodes));
                TreePtr res = MK::Send0(loc, std::move(str), core::Names::intern());

                result = std::move(res);
            },
            [&](parser::FileLiteral *fileLiteral) {
                TreePtr res = MK::String(loc, core::Names::currentFile());
                result = std::move(res);
            },
            [&](parser::ConstLhs *constLhs) {
                auto scope = node2TreeImpl(dctx, std::move(constLhs->scope));
                TreePtr res = MK::UnresolvedConstant(loc, std::move(scope), constLhs->name);
                result = std::move(res);
            },
            [&](parser::Cbase *cbase) {
                TreePtr res = MK::Constant(loc, core::Symbols::root());
                result = std::move(res);
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
                    result = std::move(block);
                } else {
                    TreePtr res = MK::EmptyTree();
                    result = std::move(res);
                }
            },
            [&](parser::Module *module) {
                ClassDef::RHS_store body = scopeNodeToBody(dctx, std::move(module->body));
                ClassDef::ANCESTORS_store ancestors;
                TreePtr res =
                    MK::Module(module->loc, core::Loc(dctx.ctx.file, module->declLoc),
                               node2TreeImpl(dctx, std::move(module->name)), std::move(ancestors), std::move(body));
                result = std::move(res);
            },
            [&](parser::Class *claz) {
                ClassDef::RHS_store body = scopeNodeToBody(dctx, std::move(claz->body));
                ClassDef::ANCESTORS_store ancestors;
                if (claz->superclass == nullptr) {
                    ancestors.emplace_back(MK::Constant(loc, core::Symbols::todo()));
                } else {
                    ancestors.emplace_back(node2TreeImpl(dctx, std::move(claz->superclass)));
                }
                TreePtr res =
                    MK::Class(claz->loc, core::Loc(dctx.ctx.file, claz->declLoc),
                              node2TreeImpl(dctx, std::move(claz->name)), std::move(ancestors), std::move(body));
                result = std::move(res);
            },
            [&](parser::Arg *arg) {
                TreePtr res = MK::Local(loc, arg->name);
                result = std::move(res);
            },
            [&](parser::Restarg *arg) {
                TreePtr res = MK::RestArg(loc, MK::Local(arg->nameLoc, arg->name));
                result = std::move(res);
            },
            [&](parser::Kwrestarg *arg) {
                TreePtr res = MK::RestArg(loc, MK::KeywordArg(loc, MK::Local(loc, arg->name)));
                result = std::move(res);
            },
            [&](parser::Kwarg *arg) {
                TreePtr res = MK::KeywordArg(loc, MK::Local(loc, arg->name));
                result = std::move(res);
            },
            [&](parser::Blockarg *arg) {
                TreePtr res = MK::BlockArg(loc, MK::Local(loc, arg->name));
                result = std::move(res);
            },
            [&](parser::Kwoptarg *arg) {
                TreePtr res = MK::OptionalArg(loc, MK::KeywordArg(loc, MK::Local(arg->nameLoc, arg->name)),
                                              node2TreeImpl(dctx, std::move(arg->default_)));
                result = std::move(res);
            },
            [&](parser::Optarg *arg) {
                TreePtr res = MK::OptionalArg(loc, MK::Local(arg->nameLoc, arg->name),
                                              node2TreeImpl(dctx, std::move(arg->default_)));
                result = std::move(res);
            },
            [&](parser::Shadowarg *arg) {
                TreePtr res = MK::ShadowArg(loc, MK::Local(loc, arg->name));
                result = std::move(res);
            },
            [&](parser::DefMethod *method) {
                bool isSelf = false;
                TreePtr res = buildMethod(dctx, method->loc, core::Loc(dctx.ctx.file, method->declLoc), method->name,
                                          method->args, method->body, isSelf);
                result = std::move(res);
            },
            [&](parser::DefS *method) {
                auto *self = parser::cast_node<parser::Self>(method->singleton.get());
                if (self == nullptr) {
                    if (auto e =
                            dctx.ctx.beginError(method->singleton->loc, core::errors::Desugar::InvalidSingletonDef)) {
                        e.setHeader("`{}` is only supported for `{}`", "def EXPRESSION.method", "def self.method");
                    }
                    TreePtr res = MK::EmptyTree();
                    result = std::move(res);
                    return;
                }
                bool isSelf = true;
                TreePtr res = buildMethod(dctx, method->loc, core::Loc(dctx.ctx.file, method->declLoc), method->name,
                                          method->args, method->body, isSelf);
                result = std::move(res);
            },
            [&](parser::SClass *sclass) {
                // This will be a nested ClassDef which we leave in the tree
                // which will get the symbol of `class.singleton_class`
                auto *self = parser::cast_node<parser::Self>(sclass->expr.get());
                if (self == nullptr) {
                    if (auto e = dctx.ctx.beginError(sclass->expr->loc, core::errors::Desugar::InvalidSingletonDef)) {
                        e.setHeader("`{}` is only supported for `{}`", "class << EXPRESSION", "class << self");
                    }
                    TreePtr res = MK::EmptyTree();
                    result = std::move(res);
                    return;
                }

                ClassDef::RHS_store body = scopeNodeToBody(dctx, std::move(sclass->body));
                ClassDef::ANCESTORS_store emptyAncestors;
                TreePtr res = MK::Class(sclass->loc, core::Loc(dctx.ctx.file, sclass->declLoc),
                                        make_tree<UnresolvedIdent>(sclass->expr->loc, UnresolvedIdent::Kind::Class,
                                                                   core::Names::singleton()),
                                        std::move(emptyAncestors), std::move(body));
                result = std::move(res);
            },
            [&](parser::Block *block) {
                block->send->loc = loc;
                auto recv = node2TreeImpl(dctx, std::move(block->send));
                Send *send;
                TreePtr res;
                if ((send = cast_tree<Send>(recv)) != nullptr) {
                    res = std::move(recv);
                } else {
                    // This must have been a csend; That will have been desugared
                    // into an insseq with an If in the expression.
                    res = std::move(recv);
                    auto *is = cast_tree<InsSeq>(res);
                    if (!is) {
                        if (auto e = dctx.ctx.beginError(block->loc, core::errors::Desugar::UnsupportedNode)) {
                            e.setHeader("No body in block");
                        }
                        auto res = MK::EmptyTree();
                        result = std::move(res);
                        return;
                    }
                    auto *iff = cast_tree<If>(is->expr);
                    ENFORCE(iff != nullptr, "DesugarBlock: failed to find If");
                    send = cast_tree<Send>(iff->elsep);
                    ENFORCE(send != nullptr, "DesugarBlock: failed to find Send");
                }
                auto [args, destructures] = desugarArgs(dctx, loc, block->args);
                auto desugaredBody = desugarBody(dctx, loc, block->body, std::move(destructures));

                // TODO the send->block's loc is too big and includes the whole send
                send->block = MK::Block(loc, std::move(desugaredBody), std::move(args));
                result = std::move(res);
            },
            [&](parser::While *wl) {
                auto cond = node2TreeImpl(dctx, std::move(wl->cond));
                auto body = node2TreeImpl(dctx, std::move(wl->body));
                TreePtr res = MK::While(loc, std::move(cond), std::move(body));
                result = std::move(res);
            },
            [&](parser::WhilePost *wl) {
                bool isKwbegin = parser::isa_node<parser::Kwbegin>(wl->body.get());
                auto cond = node2TreeImpl(dctx, std::move(wl->cond));
                auto body = node2TreeImpl(dctx, std::move(wl->body));
                // TODO using bang (aka !) is not semantically correct because it can be overridden by the user.
                TreePtr res = isKwbegin ? doUntil(dctx, loc, MK::Send0(loc, std::move(cond), core::Names::bang()),
                                                  std::move(body))
                                        : MK::While(loc, std::move(cond), std::move(body));
                result = std::move(res);
            },
            [&](parser::Until *wl) {
                auto cond = node2TreeImpl(dctx, std::move(wl->cond));
                auto body = node2TreeImpl(dctx, std::move(wl->body));
                TreePtr res = MK::While(loc, MK::Send0(loc, std::move(cond), core::Names::bang()), std::move(body));
                result = std::move(res);
            },
            // This is the same as WhilePost, but the cond negation is in the other branch.
            [&](parser::UntilPost *wl) {
                bool isKwbegin = parser::isa_node<parser::Kwbegin>(wl->body.get());
                auto cond = node2TreeImpl(dctx, std::move(wl->cond));
                auto body = node2TreeImpl(dctx, std::move(wl->body));
                TreePtr res =
                    isKwbegin ? doUntil(dctx, loc, std::move(cond), std::move(body))
                              : MK::While(loc, MK::Send0(loc, std::move(cond), core::Names::bang()), std::move(body));
                result = std::move(res);
            },
            [&](parser::Nil *wl) {
                TreePtr res = MK::Nil(loc);
                result = std::move(res);
            },
            [&](parser::IVar *var) {
                TreePtr res = make_tree<UnresolvedIdent>(loc, UnresolvedIdent::Kind::Instance, var->name);
                result = std::move(res);
            },
            [&](parser::GVar *var) {
                TreePtr res = make_tree<UnresolvedIdent>(loc, UnresolvedIdent::Kind::Global, var->name);
                result = std::move(res);
            },
            [&](parser::CVar *var) {
                TreePtr res = make_tree<UnresolvedIdent>(loc, UnresolvedIdent::Kind::Class, var->name);
                result = std::move(res);
            },
            [&](parser::LVarLhs *var) {
                TreePtr res = MK::Local(loc, var->name);
                result = std::move(res);
            },
            [&](parser::GVarLhs *var) {
                TreePtr res = make_tree<UnresolvedIdent>(loc, UnresolvedIdent::Kind::Global, var->name);
                result = std::move(res);
            },
            [&](parser::CVarLhs *var) {
                TreePtr res = make_tree<UnresolvedIdent>(loc, UnresolvedIdent::Kind::Class, var->name);
                result = std::move(res);
            },
            [&](parser::IVarLhs *var) {
                TreePtr res = make_tree<UnresolvedIdent>(loc, UnresolvedIdent::Kind::Instance, var->name);
                result = std::move(res);
            },
            [&](parser::NthRef *var) {
                TreePtr res = make_tree<UnresolvedIdent>(loc, UnresolvedIdent::Kind::Global,
                                                         dctx.ctx.state.enterNameUTF8(to_string(var->ref)));
                result = std::move(res);
            },
            [&](parser::Assign *asgn) {
                auto lhs = node2TreeImpl(dctx, std::move(asgn->lhs));
                auto rhs = node2TreeImpl(dctx, std::move(asgn->rhs));
                auto res = MK::Assign(loc, std::move(lhs), std::move(rhs));
                result = std::move(res);
            },
            [&](parser::Super *super) {
                // Desugar super into a call to a normal method named `super`;
                // Do this by synthesizing a `Send` parse node and letting our
                // Send desugar handle it.
                auto method = core::Names::super();
                auto send = make_unique<parser::Send>(super->loc, nullptr, method, std::move(super->args));
                auto res = node2TreeImpl(dctx, std::move(send));
                result = std::move(res);
            },
            [&](parser::ZSuper *zuper) { result = MK::ZSuper(loc); },
            [&](parser::For *for_) {
                MethodDef::ARGS_store args;
                bool canProvideNiceDesugar = true;
                auto mlhsNode = std::move(for_->vars);
                if (auto *mlhs = parser::cast_node<parser::Mlhs>(mlhsNode.get())) {
                    for (auto &c : mlhs->exprs) {
                        if (!parser::isa_node<parser::LVarLhs>(c.get())) {
                            canProvideNiceDesugar = false;
                            break;
                        }
                    }
                    if (canProvideNiceDesugar) {
                        for (auto &c : mlhs->exprs) {
                            args.emplace_back(node2TreeImpl(dctx, move(c)));
                        }
                    }
                } else {
                    canProvideNiceDesugar = parser::isa_node<parser::LVarLhs>(mlhsNode.get());
                    if (canProvideNiceDesugar) {
                        TreePtr lhs = node2TreeImpl(dctx, std::move(mlhsNode));
                        args.emplace_back(move(lhs));
                    } else {
                        parser::NodeVec vars;
                        vars.emplace_back(std::move(mlhsNode));
                        mlhsNode = make_unique<parser::Mlhs>(loc, std::move(vars));
                    }
                }

                auto body = node2TreeImpl(dctx, std::move(for_->body));

                TreePtr block;
                if (canProvideNiceDesugar) {
                    block = MK::Block(loc, std::move(body), std::move(args));
                } else {
                    auto temp = dctx.ctx.state.freshNameUnique(core::UniqueNameKind::Desugar, core::Names::forTemp(),
                                                               ++dctx.uniqueCounter);

                    unique_ptr<parser::Node> masgn =
                        make_unique<parser::Masgn>(loc, std::move(mlhsNode), make_unique<parser::LVar>(loc, temp));

                    body = MK::InsSeq1(loc, node2TreeImpl(dctx, std::move(masgn)), move(body));
                    block = MK::Block(loc, std::move(body), std::move(args));
                }

                Send::ARGS_store noargs;
                auto res = MK::Send(loc, node2TreeImpl(dctx, std::move(for_->expr)), core::Names::each(),
                                    std::move(noargs), {}, std::move(block));
                result = std::move(res);
            },
            [&](parser::Integer *integer) {
                int64_t val;

                // complemented literals
                bool hasTilde = integer->val.find("~") != string::npos;
                const string &withoutTilde = !hasTilde ? integer->val : absl::StrReplaceAll(integer->val, {{"~", ""}});

                auto underscorePos = withoutTilde.find("_");
                const string &withoutUnderscores =
                    (underscorePos == string::npos) ? withoutTilde : absl::StrReplaceAll(withoutTilde, {{"_", ""}});

                if (!absl::SimpleAtoi(withoutUnderscores, &val)) {
                    val = 0;
                    if (auto e = dctx.ctx.beginError(loc, core::errors::Desugar::IntegerOutOfRange)) {
                        e.setHeader("Unsupported integer literal: `{}`", integer->val);
                    }
                }

                TreePtr res = MK::Int(loc, hasTilde ? ~val : val);
                result = std::move(res);
            },
            [&](parser::Float *floatNode) {
                double val;
                auto underscorePos = floatNode->val.find("_");

                const string &withoutUnderscores =
                    (underscorePos == string::npos) ? floatNode->val : absl::StrReplaceAll(floatNode->val, {{"_", ""}});
                if (!absl::SimpleAtod(withoutUnderscores, &val)) {
                    val = numeric_limits<double>::quiet_NaN();
                    if (auto e = dctx.ctx.beginError(loc, core::errors::Desugar::FloatOutOfRange)) {
                        e.setHeader("Unsupported float literal: `{}`", floatNode->val);
                    }
                }

                TreePtr res = MK::Float(loc, val);
                result = std::move(res);
            },
            [&](parser::Complex *complex) {
                auto kernel = MK::Constant(loc, core::Symbols::Kernel());
                core::NameRef complex_name = core::Names::Constants::Complex().data(dctx.ctx)->cnst.original;
                core::NameRef value = dctx.ctx.state.enterNameUTF8(complex->value);
                auto send = MK::Send2(loc, std::move(kernel), complex_name, MK::Int(loc, 0), MK::String(loc, value));
                result = std::move(send);
            },
            [&](parser::Rational *complex) {
                auto kernel = MK::Constant(loc, core::Symbols::Kernel());
                core::NameRef complex_name = core::Names::Constants::Rational().data(dctx.ctx)->cnst.original;
                core::NameRef value = dctx.ctx.state.enterNameUTF8(complex->val);
                auto send = MK::Send1(loc, std::move(kernel), complex_name, MK::String(loc, value));
                result = std::move(send);
            },
            [&](parser::Array *array) {
                Array::ENTRY_store elems;
                elems.reserve(array->elts.size());
                TreePtr lastMerge;
                for (auto &stat : array->elts) {
                    if (auto splat = parser::cast_node<parser::Splat>(stat.get())) {
                        // Desguar
                        //   [a, **x, remaining}
                        // into
                        //   a.concat(x.to_a).concat(remaining)
                        auto var = MK::Send0(loc, node2TreeImpl(dctx, std::move(splat->var)), core::Names::toA());
                        if (elems.empty()) {
                            if (lastMerge != nullptr) {
                                lastMerge = MK::Send1(loc, std::move(lastMerge), core::Names::concat(), std::move(var));
                            } else {
                                lastMerge = std::move(var);
                            }
                        } else {
                            TreePtr current = MK::Array(loc, std::move(elems));
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

                TreePtr res;
                if (elems.empty()) {
                    if (lastMerge != nullptr) {
                        res = std::move(lastMerge);
                    } else {
                        // Empty array
                        res = MK::Array(loc, std::move(elems));
                    }
                } else {
                    res = MK::Array(loc, std::move(elems));
                    if (lastMerge != nullptr) {
                        res = MK::Send1(loc, std::move(lastMerge), core::Names::concat(), std::move(res));
                    }
                }
                result = std::move(res);
            },
            [&](parser::Hash *hash) {
                Hash::ENTRY_store keys;
                Hash::ENTRY_store values;
                keys.reserve(hash->pairs.size());   // overapproximation in case there are KwSpats
                values.reserve(hash->pairs.size()); // overapproximation in case there are KwSpats
                TreePtr lastMerge;

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
                        auto expr = MK::Send0(loc, node2TreeImpl(dctx, std::move(splat->expr)), core::Names::toHash());
                        if (keys.empty()) {
                            if (lastMerge != nullptr) {
                                lastMerge = MK::Send1(loc, std::move(lastMerge), core::Names::merge(), std::move(expr));

                            } else {
                                lastMerge = std::move(expr);
                            }
                        } else {
                            TreePtr current = MK::Hash(loc, std::move(keys), std::move(values));
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

                TreePtr res;
                if (keys.empty()) {
                    if (lastMerge != nullptr) {
                        res = std::move(lastMerge);
                    } else {
                        res = MK::Hash0(loc);
                    }
                } else {
                    res = MK::Hash(loc, std::move(keys), std::move(values));
                    if (lastMerge != nullptr) {
                        res = MK::Send1(loc, std::move(lastMerge), core::Names::merge(), std::move(res));
                    }
                }

                result = std::move(res);
            },
            [&](parser::IRange *ret) {
                auto recv = MK::Constant(loc, core::Symbols::Magic());
                auto from = node2TreeImpl(dctx, std::move(ret->from));
                auto to = node2TreeImpl(dctx, std::move(ret->to));
                auto send = MK::Send2(loc, std::move(recv), core::Names::buildRange(), std::move(from), std::move(to));
                result = std::move(send);
            },
            [&](parser::ERange *ret) {
                auto recv = MK::Constant(loc, core::Symbols::Magic());
                auto from = node2TreeImpl(dctx, std::move(ret->from));
                auto to = node2TreeImpl(dctx, std::move(ret->to));
                auto send = MK::Send2(loc, std::move(recv), core::Names::buildRange(), std::move(from), std::move(to));
                result = std::move(send);
            },
            [&](parser::Regexp *regexpNode) {
                TreePtr cnst = MK::Constant(loc, core::Symbols::Regexp());
                auto pattern = desugarDString(dctx, loc, std::move(regexpNode->regex));
                auto opts = node2TreeImpl(dctx, std::move(regexpNode->opts));
                auto send = MK::Send2(loc, std::move(cnst), core::Names::new_(), std::move(pattern), std::move(opts));
                result = std::move(send);
            },
            [&](parser::Regopt *regopt) {
                TreePtr acc = MK::Int(loc, 0);
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
                result = std::move(acc);
            },
            [&](parser::Return *ret) {
                if (ret->exprs.size() > 1) {
                    Array::ENTRY_store elems;
                    elems.reserve(ret->exprs.size());
                    for (auto &stat : ret->exprs) {
                        if (parser::isa_node<parser::BlockPass>(stat.get())) {
                            if (auto e = dctx.ctx.beginError(ret->loc, core::errors::Desugar::UnsupportedNode)) {
                                e.setHeader("Block argument should not be given");
                            }
                            continue;
                        }
                        elems.emplace_back(node2TreeImpl(dctx, std::move(stat)));
                    };
                    TreePtr arr = MK::Array(loc, std::move(elems));
                    TreePtr res = MK::Return(loc, std::move(arr));
                    result = std::move(res);
                } else if (ret->exprs.size() == 1) {
                    if (parser::isa_node<parser::BlockPass>(ret->exprs[0].get())) {
                        if (auto e = dctx.ctx.beginError(ret->loc, core::errors::Desugar::UnsupportedNode)) {
                            e.setHeader("Block argument should not be given");
                        }
                        TreePtr res = MK::Break(loc, MK::EmptyTree());
                        result = std::move(res);
                    } else {
                        TreePtr res = MK::Return(loc, node2TreeImpl(dctx, std::move(ret->exprs[0])));
                        result = std::move(res);
                    }
                } else {
                    TreePtr res = MK::Return(loc, MK::EmptyTree());
                    result = std::move(res);
                }
            },
            [&](parser::Break *ret) {
                if (ret->exprs.size() > 1) {
                    Array::ENTRY_store elems;
                    elems.reserve(ret->exprs.size());
                    for (auto &stat : ret->exprs) {
                        if (parser::isa_node<parser::BlockPass>(stat.get())) {
                            if (auto e = dctx.ctx.beginError(ret->loc, core::errors::Desugar::UnsupportedNode)) {
                                e.setHeader("Block argument should not be given");
                            }
                            continue;
                        }
                        elems.emplace_back(node2TreeImpl(dctx, std::move(stat)));
                    };
                    TreePtr arr = MK::Array(loc, std::move(elems));
                    TreePtr res = MK::Break(loc, std::move(arr));
                    result = std::move(res);
                } else if (ret->exprs.size() == 1) {
                    if (parser::isa_node<parser::BlockPass>(ret->exprs[0].get())) {
                        if (auto e = dctx.ctx.beginError(ret->loc, core::errors::Desugar::UnsupportedNode)) {
                            e.setHeader("Block argument should not be given");
                        }
                        TreePtr res = MK::Break(loc, MK::EmptyTree());
                        result = std::move(res);
                    } else {
                        TreePtr res = MK::Break(loc, node2TreeImpl(dctx, std::move(ret->exprs[0])));
                        result = std::move(res);
                    }
                } else {
                    TreePtr res = MK::Break(loc, MK::EmptyTree());
                    result = std::move(res);
                }
            },
            [&](parser::Next *ret) {
                if (ret->exprs.size() > 1) {
                    Array::ENTRY_store elems;
                    elems.reserve(ret->exprs.size());
                    for (auto &stat : ret->exprs) {
                        if (parser::isa_node<parser::BlockPass>(stat.get())) {
                            if (auto e = dctx.ctx.beginError(ret->loc, core::errors::Desugar::UnsupportedNode)) {
                                e.setHeader("Block argument should not be given");
                            }
                            continue;
                        }
                        elems.emplace_back(node2TreeImpl(dctx, std::move(stat)));
                    };
                    TreePtr arr = MK::Array(loc, std::move(elems));
                    TreePtr res = MK::Next(loc, std::move(arr));
                    result = std::move(res);
                } else if (ret->exprs.size() == 1) {
                    if (parser::isa_node<parser::BlockPass>(ret->exprs[0].get())) {
                        if (auto e = dctx.ctx.beginError(ret->loc, core::errors::Desugar::UnsupportedNode)) {
                            e.setHeader("Block argument should not be given");
                        }
                        TreePtr res = MK::Break(loc, MK::EmptyTree());
                        result = std::move(res);
                    } else {
                        TreePtr res = MK::Next(loc, node2TreeImpl(dctx, std::move(ret->exprs[0])));
                        result = std::move(res);
                    }
                } else {
                    TreePtr res = MK::Next(loc, MK::EmptyTree());
                    result = std::move(res);
                }
            },
            [&](parser::Retry *ret) {
                TreePtr res = make_tree<Retry>(loc);
                result = std::move(res);
            },
            [&](parser::Yield *ret) {
                Send::ARGS_store args;
                args.reserve(ret->exprs.size());
                for (auto &stat : ret->exprs) {
                    args.emplace_back(node2TreeImpl(dctx, std::move(stat)));
                };

                TreePtr recv;
                if (dctx.enclosingBlockArg.exists()) {
                    // we always want to report an error if we're using yield with a synthesized name in
                    // strict mode
                    auto blockArgName = dctx.enclosingBlockArg;
                    if (blockArgName == core::Names::blkArg()) {
                        if (auto e = dctx.ctx.state.beginError(dctx.enclosingMethodLoc,
                                                               core::errors::Desugar::UnnamedBlockParameter)) {
                            e.setHeader("Method `{}` uses `{}` but does not mention a block parameter",
                                        dctx.enclosingMethodName.data(dctx.ctx)->show(dctx.ctx), "yield");
                            e.addErrorLine(core::Loc(dctx.ctx.file, loc), "Arising from use of `{}` in method body",
                                           "yield");
                        }
                    }

                    recv = MK::Local(loc, dctx.enclosingBlockArg);
                } else {
                    // No enclosing block arg can happen when e.g. yield is called in a class / at the top-level.
                    recv = MK::RaiseUnimplemented(loc);
                }
                TreePtr res = MK::Send(loc, std::move(recv), core::Names::call(), std::move(args));
                result = std::move(res);
            },
            [&](parser::Rescue *rescue) {
                Rescue::RESCUE_CASE_store cases;
                cases.reserve(rescue->rescue.size());
                for (auto &node : rescue->rescue) {
                    cases.emplace_back(node2TreeImpl(dctx, std::move(node)));
                    ENFORCE(isa_tree<RescueCase>(cases.back()), "node2TreeImpl failed to produce a rescue case");
                }
                TreePtr res = make_tree<Rescue>(loc, node2TreeImpl(dctx, std::move(rescue->body)), std::move(cases),
                                                node2TreeImpl(dctx, std::move(rescue->else_)), MK::EmptyTree());
                result = std::move(res);
            },
            [&](parser::Resbody *resbody) {
                RescueCase::EXCEPTION_store exceptions;
                auto exceptionsExpr = node2TreeImpl(dctx, std::move(resbody->exception));
                if (isa_tree<EmptyTree>(exceptionsExpr)) {
                    // No exceptions captured
                } else if (auto exceptionsArray = cast_tree<Array>(exceptionsExpr)) {
                    ENFORCE(exceptionsArray != nullptr, "exception array cast failed");

                    for (auto &elem : exceptionsArray->elems) {
                        exceptions.emplace_back(std::move(elem));
                    }
                } else if (auto exceptionsSend = cast_tree<Send>(exceptionsExpr)) {
                    ENFORCE(exceptionsSend->fun == core::Names::splat() || exceptionsSend->fun == core::Names::toA() ||
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
                if (auto *id = cast_tree<UnresolvedIdent>(varExpr)) {
                    if (id->kind == UnresolvedIdent::Kind::Local) {
                        var = id->name;
                        varExpr.reset(nullptr);
                    }
                }

                if (!var.exists()) {
                    var = dctx.ctx.state.freshNameUnique(core::UniqueNameKind::Desugar, core::Names::rescueTemp(),
                                                         ++dctx.uniqueCounter);
                }

                if (isa_tree<EmptyTree>(varExpr)) {
                    varLoc = loc;
                } else if (varExpr != nullptr) {
                    body = MK::InsSeq1(varLoc, MK::Assign(varLoc, std::move(varExpr), MK::Local(varLoc, var)),
                                       std::move(body));
                }

                TreePtr res =
                    make_tree<RescueCase>(loc, std::move(exceptions), MK::Local(varLoc, var), std::move(body));
                result = std::move(res);
            },
            [&](parser::Ensure *ensure) {
                auto bodyExpr = node2TreeImpl(dctx, std::move(ensure->body));
                auto ensureExpr = node2TreeImpl(dctx, std::move(ensure->ensure));
                auto rescue = cast_tree<Rescue>(bodyExpr);
                if (rescue != nullptr) {
                    rescue->ensure = std::move(ensureExpr);
                    result = std::move(bodyExpr);
                } else {
                    Rescue::RESCUE_CASE_store cases;
                    TreePtr res = make_tree<Rescue>(loc, std::move(bodyExpr), std::move(cases), MK::EmptyTree(),
                                                    std::move(ensureExpr));
                    result = std::move(res);
                }
            },
            [&](parser::If *if_) {
                auto cond = node2TreeImpl(dctx, std::move(if_->condition));
                auto thenp = node2TreeImpl(dctx, std::move(if_->then_));
                auto elsep = node2TreeImpl(dctx, std::move(if_->else_));
                auto iff = MK::If(loc, std::move(cond), std::move(thenp), std::move(elsep));
                result = std::move(iff);
            },
            [&](parser::Masgn *masgn) {
                auto *lhs = parser::cast_node<parser::Mlhs>(masgn->lhs.get());
                ENFORCE(lhs != nullptr, "Failed to get lhs of Masgn");

                auto res = desugarMlhs(dctx, loc, lhs, node2TreeImpl(dctx, std::move(masgn->rhs)));

                result = std::move(res);
            },
            [&](parser::True *t) {
                auto res = MK::True(loc);
                result = std::move(res);
            },
            [&](parser::False *t) {
                auto res = MK::False(loc);
                result = std::move(res);
            },
            [&](parser::Case *case_) {
                TreePtr assign;
                auto temp = core::NameRef::noName();
                core::LocOffsets cloc;

                if (case_->condition != nullptr) {
                    cloc = case_->condition->loc;
                    temp = dctx.ctx.state.freshNameUnique(core::UniqueNameKind::Desugar, core::Names::assignTemp(),
                                                          ++dctx.uniqueCounter);
                    assign = MK::Assign(cloc, temp, node2TreeImpl(dctx, std::move(case_->condition)));
                }
                TreePtr res = node2TreeImpl(dctx, std::move(case_->else_));
                for (auto it = case_->whens.rbegin(); it != case_->whens.rend(); ++it) {
                    auto when = parser::cast_node<parser::When>(it->get());
                    ENFORCE(when != nullptr, "case without a when?");
                    TreePtr cond;
                    for (auto &cnode : when->patterns) {
                        auto ctree = node2TreeImpl(dctx, std::move(cnode));
                        TreePtr test;
                        if (temp.exists()) {
                            auto local = MK::Local(cloc, temp);
                            auto patternloc = ctree->loc;
                            test = MK::Send1(patternloc, std::move(ctree), core::Names::tripleEq(), std::move(local));
                        } else {
                            test = std::move(ctree);
                        }
                        if (cond == nullptr) {
                            cond = std::move(test);
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
                result = std::move(res);
            },
            [&](parser::Splat *splat) {
                auto res = MK::Splat(loc, node2TreeImpl(dctx, std::move(splat->var)));
                result = std::move(res);
            },
            [&](parser::Alias *alias) {
                auto res =
                    MK::Send2(loc, MK::Self(loc), core::Names::aliasMethod(),
                              node2TreeImpl(dctx, std::move(alias->from)), node2TreeImpl(dctx, std::move(alias->to)));
                result = std::move(res);
            },
            [&](parser::Defined *defined) {
                auto value = node2TreeImpl(dctx, std::move(defined->value));
                auto loc = value->loc;
                Send::ARGS_store args;
                while (!isa_tree<EmptyTree>(value)) {
                    auto lit = cast_tree<UnresolvedConstantLit>(value);
                    if (lit == nullptr) {
                        args.clear();
                        break;
                    }
                    args.emplace_back(MK::String(lit->loc, lit->cnst));
                    value = std::move(lit->scope);
                }
                absl::c_reverse(args);
                auto res =
                    MK::Send(loc, MK::Constant(loc, core::Symbols::Magic()), core::Names::defined_p(), std::move(args));
                result = std::move(res);
            },
            [&](parser::LineLiteral *line) {
                auto pos = core::Loc(dctx.ctx.file, loc).position(dctx.ctx);
                ENFORCE(pos.first.line == pos.second.line, "position corrupted");
                auto res = MK::Int(loc, pos.first.line);
                result = std::move(res);
            },
            [&](parser::XString *xstring) {
                auto res = MK::Send1(loc, MK::Self(loc), core::Names::backtick(),
                                     desugarDString(dctx, loc, std::move(xstring->nodes)));
                result = std::move(res);
            },
            [&](parser::Preexe *preexe) {
                auto res = unsupportedNode(dctx, preexe);
                result = std::move(res);
            },
            [&](parser::Postexe *postexe) {
                auto res = unsupportedNode(dctx, postexe);
                result = std::move(res);
            },
            [&](parser::Undef *undef) {
                if (auto e = dctx.ctx.beginError(what->loc, core::errors::Desugar::UndefUsage)) {
                    e.setHeader("Unsuppored method: undef");
                }
                Send::ARGS_store args;
                for (auto &expr : undef->exprs) {
                    args.emplace_back(node2TreeImpl(dctx, move(expr)));
                }
                auto res =
                    MK::Send(loc, MK::Constant(loc, core::Symbols::Kernel()), core::Names::undef(), std::move(args));
                result = std::move(res);
            },
            [&](parser::Backref *backref) {
                auto res = unsupportedNode(dctx, backref);
                result = std::move(res);
            },
            [&](parser::EFlipflop *eflipflop) {
                auto res = unsupportedNode(dctx, eflipflop);
                result = std::move(res);
            },
            [&](parser::IFlipflop *iflipflop) {
                auto res = unsupportedNode(dctx, iflipflop);
                result = std::move(res);
            },
            [&](parser::MatchCurLine *matchCurLine) {
                auto res = unsupportedNode(dctx, matchCurLine);
                result = std::move(res);
            },
            [&](parser::Redo *redo) {
                auto res = unsupportedNode(dctx, redo);
                result = std::move(res);
            },
            [&](parser::EncodingLiteral *encodingLiteral) {
                auto recv = MK::Constant(loc, core::Symbols::Magic());
                result = MK::Send0(loc, std::move(recv), core::Names::getEncoding());
            },

            [&](parser::BlockPass *blockPass) { Exception::raise("Send should have already handled the BlockPass"); },
            [&](parser::Node *node) { Exception::raise("Unimplemented Parser Node: {}", node->nodeName()); });
        ENFORCE(result.get() != nullptr, "desugar result unset");
        return result;
    } catch (SorbetException &) {
        Exception::failInFuzzer();
        if (!locReported) {
            locReported = true;
            if (auto e = dctx.ctx.beginError(what->loc, core::errors::Internal::InternalError)) {
                e.setHeader("Failed to process tree (backtrace is above)");
            }
        }
        throw;
    }
}

TreePtr liftTopLevel(DesugarContext dctx, core::LocOffsets loc, TreePtr what) {
    ClassDef::RHS_store rhs;
    ClassDef::ANCESTORS_store ancestors;
    ancestors.emplace_back(MK::Constant(loc, core::Symbols::todo()));
    auto insSeq = cast_tree<InsSeq>(what);
    if (insSeq) {
        rhs.reserve(insSeq->stats.size() + 1);
        for (auto &stat : insSeq->stats) {
            rhs.emplace_back(std::move(stat));
        }
        rhs.emplace_back(std::move(insSeq->expr));
    } else {
        rhs.emplace_back(std::move(what));
    }
    return make_tree<ClassDef>(loc, core::Loc(dctx.ctx.file, loc), core::Symbols::root(), MK::EmptyTree(),
                               std::move(ancestors), std::move(rhs), ClassDef::Kind::Class);
}
} // namespace

TreePtr node2Tree(core::MutableContext ctx, unique_ptr<parser::Node> what) {
    try {
        u4 uniqueCounter = 1;
        // We don't have an enclosing block arg to start off.
        DesugarContext dctx(ctx, uniqueCounter, core::NameRef::noName(), core::Loc::none(ctx.file),
                            core::NameRef::noName());
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
