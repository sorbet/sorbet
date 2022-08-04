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
    uint32_t &uniqueCounter;
    core::NameRef enclosingBlockArg;
    core::LocOffsets enclosingMethodLoc;
    core::NameRef enclosingMethodName;

    DesugarContext(core::MutableContext ctx, uint32_t &uniqueCounter, core::NameRef enclosingBlockArg,
                   core::LocOffsets enclosingMethodLoc, core::NameRef enclosingMethodName)
        : ctx(ctx), uniqueCounter(uniqueCounter), enclosingBlockArg(enclosingBlockArg),
          enclosingMethodLoc(enclosingMethodLoc), enclosingMethodName(enclosingMethodName){};

    core::NameRef freshNameUnique(core::NameRef name) {
        return ctx.state.freshNameUnique(core::UniqueNameKind::Desugar, name, ++uniqueCounter);
    }
};

core::NameRef blockArg2Name(DesugarContext dctx, const BlockArg &blkArg) {
    auto blkIdent = cast_tree<UnresolvedIdent>(blkArg.expr);
    ENFORCE(blkIdent != nullptr, "BlockArg must wrap UnresolvedIdent in desugar.");
    return blkIdent->name;
}

// Get the num from the name of the Node if it's a LVar.
// Return -1 otherwise.
int numparamNum(DesugarContext dctx, parser::Node *decl) {
    if (auto *lvar = parser::cast_node<parser::LVar>(decl)) {
        auto name_str = lvar->name.show(dctx.ctx);
        return name_str[1] - 48;
    }
    return -1;
}

// Get the highest numparams used in `decls`
// Return 0 if the list of declarations is empty.
int numparamMax(DesugarContext dctx, parser::NodeVec *decls) {
    int max = 0;
    for (auto &decl : *decls) {
        auto num = numparamNum(dctx, decl.get());
        if (num > max) {
            max = num;
        }
    }
    return max;
}

// Create a local variable from the first declaration for the name "_num" from all the `decls` if any.
// Return a dummy variable if no declaration is found for `num`.
ExpressionPtr numparamTree(DesugarContext dctx, int num, parser::NodeVec *decls) {
    for (auto &decl : *decls) {
        if (auto *lvar = parser::cast_node<parser::LVar>(decl.get())) {
            if (numparamNum(dctx, decl.get()) == num) {
                return MK::Local(lvar->loc, lvar->name);
            }
        } else {
            ENFORCE(false, "NumParams declaring node is not a LVar.");
        }
    }
    core::NameRef name = dctx.ctx.state.enterNameUTF8("_" + std::to_string(num));
    return MK::Local(core::LocOffsets::none(), name);
}

ExpressionPtr node2TreeImpl(DesugarContext dctx, unique_ptr<parser::Node> what);

pair<MethodDef::ARGS_store, InsSeq::STATS_store> desugarArgs(DesugarContext dctx, core::LocOffsets loc,
                                                             unique_ptr<parser::Node> &argnode) {
    MethodDef::ARGS_store args;
    InsSeq::STATS_store destructures;

    if (auto *oargs = parser::cast_node<parser::Args>(argnode.get())) {
        args.reserve(oargs->args.size());
        for (auto &arg : oargs->args) {
            if (auto *lhs = parser::cast_node<parser::Mlhs>(arg.get())) {
                core::NameRef temporary = dctx.freshNameUnique(core::Names::destructureArg());
                args.emplace_back(MK::Local(arg->loc, temporary));
                unique_ptr<parser::Node> lvarNode = make_unique<parser::LVar>(arg->loc, temporary);
                unique_ptr<parser::Node> destructure =
                    make_unique<parser::Masgn>(arg->loc, std::move(arg), std::move(lvarNode));
                destructures.emplace_back(node2TreeImpl(dctx, std::move(destructure)));
            } else if (auto *lhs = parser::cast_node<parser::Kwnilarg>(arg.get())) {
                // TODO implement logic for `**nil` args
            } else if (auto *fargs = parser::cast_node<parser::ForwardArg>(arg.get())) {
                // we desugar (m, n, ...) into (m, n, *<fwd-args>, **<fwd-kwargs>, &<fwd-block>)
                // add `*<fwd-args>`
                unique_ptr<parser::Node> rest =
                    make_unique<parser::Restarg>(fargs->loc, core::Names::fwdArgs(), fargs->loc);
                args.emplace_back(node2TreeImpl(dctx, std::move(rest)));
                // add `**<fwd-kwargs>`
                unique_ptr<parser::Node> kwrest = make_unique<parser::Kwrestarg>(fargs->loc, core::Names::fwdKwargs());
                args.emplace_back(node2TreeImpl(dctx, std::move(kwrest)));
                // add `&<fwd-block>`
                unique_ptr<parser::Node> block = make_unique<parser::Blockarg>(fargs->loc, core::Names::fwdBlock());
                args.emplace_back(node2TreeImpl(dctx, std::move(block)));
            } else {
                args.emplace_back(node2TreeImpl(dctx, std::move(arg)));
            }
        }
    } else if (auto *numparams = parser::cast_node<parser::NumParams>(argnode.get())) {
        // The block uses numbered parameters like `_1` or `_9` so we add them as parameters
        // from _1 to the highest number used.
        for (int i = 1; i <= numparamMax(dctx, &numparams->decls); i++) {
            args.emplace_back(numparamTree(dctx, i, &numparams->decls));
        }
    } else if (argnode.get() == nullptr) {
        // do nothing
    } else {
        Exception::raise("not implemented: {}", argnode->nodeName());
    }

    return make_pair(std::move(args), std::move(destructures));
}

ExpressionPtr desugarBody(DesugarContext dctx, core::LocOffsets loc, unique_ptr<parser::Node> &bodynode,
                          InsSeq::STATS_store destructures) {
    auto body = node2TreeImpl(dctx, std::move(bodynode));
    if (!destructures.empty()) {
        auto bodyLoc = body.loc();
        if (!bodyLoc.exists()) {
            bodyLoc = loc;
        }
        body = MK::InsSeq(loc, std::move(destructures), std::move(body));
    }

    return body;
}

ExpressionPtr desugarBlock(DesugarContext dctx, core::LocOffsets loc, core::LocOffsets blockLoc,
                           unique_ptr<parser::Node> &blockSend, unique_ptr<parser::Node> &blockArgs,
                           unique_ptr<parser::Node> &blockBody) {
    blockSend->loc = loc;
    auto recv = node2TreeImpl(dctx, std::move(blockSend));
    Send *send;
    ExpressionPtr res;
    if ((send = cast_tree<Send>(recv)) != nullptr) {
        res = std::move(recv);
    } else {
        // This must have been a csend; That will have been desugared
        // into an insseq with an If in the expression.
        res = std::move(recv);
        auto *is = cast_tree<InsSeq>(res);
        if (!is) {
            if (auto e = dctx.ctx.beginError(blockLoc, core::errors::Desugar::UnsupportedNode)) {
                e.setHeader("No body in block");
            }
            return MK::EmptyTree();
        }
        auto *iff = cast_tree<If>(is->expr);
        ENFORCE(iff != nullptr, "DesugarBlock: failed to find If");
        send = cast_tree<Send>(iff->elsep);
        ENFORCE(send != nullptr, "DesugarBlock: failed to find Send");
    }
    auto [args, destructures] = desugarArgs(dctx, loc, blockArgs);
    auto desugaredBody = desugarBody(dctx, loc, blockBody, std::move(destructures));

    // TODO the send->block's loc is too big and includes the whole send
    send->setBlock(MK::Block(loc, std::move(desugaredBody), std::move(args)));
    return res;
}

bool isStringLit(DesugarContext dctx, ExpressionPtr &expr) {
    Literal *lit;
    return (lit = cast_tree<Literal>(expr)) && lit->isString();
}

ExpressionPtr mergeStrings(DesugarContext dctx, core::LocOffsets loc,
                           InlinedVector<ExpressionPtr, 4> stringsAccumulated) {
    if (stringsAccumulated.size() == 1) {
        return move(stringsAccumulated[0]);
    } else {
        return MK::String(loc, dctx.ctx.state.enterNameUTF8(
                                   fmt::format("{}", fmt::map_join(stringsAccumulated, "", [&](const auto &expr) {
                                                   if (isa_tree<EmptyTree>(expr)) {
                                                       return ""sv;
                                                   } else {
                                                       return cast_tree<Literal>(expr)->asString().shortName(dctx.ctx);
                                                   }
                                               }))));
    }
}

ExpressionPtr desugarDString(DesugarContext dctx, core::LocOffsets loc, parser::NodeVec nodes) {
    if (nodes.empty()) {
        return MK::String(loc, core::Names::empty());
    }
    auto it = nodes.begin();
    auto end = nodes.end();
    ExpressionPtr first = node2TreeImpl(dctx, std::move(*it));
    InlinedVector<ExpressionPtr, 4> stringsAccumulated;

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
        ExpressionPtr narg = node2TreeImpl(dctx, std::move(stat));
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
        auto recv = MK::Magic(loc);
        return MK::Send(loc, std::move(recv), core::Names::stringInterpolate(), loc.copyWithZeroLength(),
                        interpArgs.size(), std::move(interpArgs));
    }
}

bool isIVarAssign(ExpressionPtr &stat) {
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

ExpressionPtr validateRBIBody(DesugarContext dctx, ExpressionPtr body) {
    if (!dctx.ctx.file.data(dctx.ctx).isRBI()) {
        return body;
    }
    if (!body.loc().exists()) {
        return body;
    }

    auto loc = dctx.ctx.locAt(body.loc());
    if (isa_tree<EmptyTree>(body)) {
        return body;
    } else if (isa_tree<Assign>(body)) {
        if (!isIVarAssign(body)) {
            if (auto e = dctx.ctx.beginError(body.loc(), core::errors::Desugar::CodeInRBI)) {
                e.setHeader("RBI methods must not have code");
                e.replaceWith("Delete the body", loc, "");
            }
        }
    } else if (auto inseq = cast_tree<InsSeq>(body)) {
        for (auto &stat : inseq->stats) {
            if (!isIVarAssign(stat)) {
                if (auto e = dctx.ctx.beginError(stat.loc(), core::errors::Desugar::CodeInRBI)) {
                    e.setHeader("RBI methods must not have code");
                    e.replaceWith("Delete the body", loc, "");
                }
            }
        }
        if (!isIVarAssign(inseq->expr)) {
            if (auto e = dctx.ctx.beginError(inseq->expr.loc(), core::errors::Desugar::CodeInRBI)) {
                e.setHeader("RBI methods must not have code");
                e.replaceWith("Delete the body", loc, "");
            }
        }
    } else {
        if (auto e = dctx.ctx.beginError(body.loc(), core::errors::Desugar::CodeInRBI)) {
            e.setHeader("RBI methods must not have code");
            e.replaceWith("Delete the body", loc, "");
        }
    }
    return body;
}

ExpressionPtr buildMethod(DesugarContext dctx, core::LocOffsets loc, core::LocOffsets declLoc, core::NameRef name,
                          unique_ptr<parser::Node> &argnode, unique_ptr<parser::Node> &body, bool isSelf) {
    // Reset uniqueCounter within this scope (to keep numbers small)
    uint32_t uniqueCounter = 1;
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
    ExpressionPtr desugaredBody = desugarBody(dctx2, loc, body, std::move(destructures));
    desugaredBody = validateRBIBody(dctx2, move(desugaredBody));

    auto mdef = MK::Method(loc, declLoc, name, std::move(args), std::move(desugaredBody));
    cast_tree<MethodDef>(mdef)->flags.isSelfMethod = isSelf;
    return mdef;
}

ExpressionPtr symbol2Proc(DesugarContext dctx, ExpressionPtr expr) {
    auto loc = expr.loc();
    core::NameRef temp = dctx.freshNameUnique(core::Names::blockPassTemp());
    Literal *lit = cast_tree<Literal>(expr);
    ENFORCE(lit && lit->isSymbol());

    // &:foo => {|temp| temp.foo() }
    core::NameRef name = core::cast_type_nonnull<core::NamedLiteralType>(lit->value).asName();
    // `temp` does not refer to any specific source text, so give it a 0-length Loc so LSP ignores it.
    auto zeroLengthLoc = loc.copyWithZeroLength();
    ExpressionPtr recv = MK::Local(zeroLengthLoc, temp);
    ExpressionPtr body = MK::Send0(loc, std::move(recv), name, zeroLengthLoc);
    return MK::Block1(loc, std::move(body), MK::Local(zeroLengthLoc, temp));
}

ExpressionPtr unsupportedNode(DesugarContext dctx, parser::Node *node) {
    if (auto e = dctx.ctx.beginError(node->loc, core::errors::Desugar::UnsupportedNode)) {
        e.setHeader("Unsupported node type `{}`", node->nodeName());
    }
    return MK::EmptyTree();
}

ExpressionPtr desugarMlhs(DesugarContext dctx, core::LocOffsets loc, parser::Mlhs *lhs, ExpressionPtr rhs) {
    InsSeq::STATS_store stats;

    core::NameRef tempRhs = dctx.freshNameUnique(core::Names::assignTemp());
    core::NameRef tempExpanded = dctx.freshNameUnique(core::Names::assignTemp());

    int i = 0;
    int before = 0, after = 0;
    bool didSplat = false;

    for (auto &c : lhs->exprs) {
        if (auto *splat = parser::cast_node<parser::SplatLhs>(c.get())) {
            ENFORCE(!didSplat, "did splat already");
            didSplat = true;

            ExpressionPtr lh = node2TreeImpl(dctx, std::move(splat->var));

            int left = i;
            int right = lhs->exprs.size() - left - 1;
            if (!isa_tree<EmptyTree>(lh)) {
                auto exclusive = MK::True(lh.loc());
                if (right == 0) {
                    right = 1;
                    exclusive = MK::False(lh.loc());
                }
                auto lhloc = lh.loc();
                auto zlhloc = lhloc.copyWithZeroLength();
                auto index = MK::Send3(lhloc, MK::Constant(lhloc, core::Symbols::Range()), core::Names::new_(), zlhloc,
                                       MK::Int(lhloc, left), MK::Int(lhloc, -right), std::move(exclusive));
                stats.emplace_back(MK::Assign(
                    lhloc, std::move(lh),
                    MK::Send1(loc, MK::Local(loc, tempExpanded), core::Names::slice(), zlhloc, std::move(index))));
            }
            i = -right;
        } else {
            if (didSplat) {
                ++after;
            } else {
                ++before;
            }
            auto val = MK::Send1(loc, MK::Local(loc, tempExpanded), core::Names::squareBrackets(),
                                 loc.copyWithZeroLength(), MK::Int(loc, i));

            if (auto *mlhs = parser::cast_node<parser::Mlhs>(c.get())) {
                stats.emplace_back(desugarMlhs(dctx, mlhs->loc, mlhs, std::move(val)));
            } else {
                ExpressionPtr lh = node2TreeImpl(dctx, std::move(c));
                if (auto restArg = cast_tree<RestArg>(lh)) {
                    if (auto e = dctx.ctx.beginError(lh.loc(), core::errors::Desugar::UnsupportedRestArgsDestructure)) {
                        e.setHeader("Unsupported rest args in destructure");
                    }
                    lh = move(restArg->expr);
                }
                auto lhloc = lh.loc();
                stats.emplace_back(MK::Assign(lhloc, std::move(lh), std::move(val)));
            }

            i++;
        }
    }

    auto expanded = MK::Send3(loc, MK::Magic(loc), core::Names::expandSplat(), loc.copyWithZeroLength(),
                              MK::Local(loc, tempRhs), MK::Int(loc, before), MK::Int(loc, after));
    stats.insert(stats.begin(), MK::Assign(loc, tempExpanded, std::move(expanded)));
    stats.insert(stats.begin(), MK::Assign(loc, tempRhs, std::move(rhs)));

    // Regardless of how we destructure an assignment, Ruby evaluates the expression to the entire right hand side,
    // not any individual component of the destructured assignment.
    return MK::InsSeq(loc, std::move(stats), MK::Local(loc, tempRhs));
}

// Map all MatchVars used in `pattern` to local variables initialized from magic calls
void desugarPatternMatchingVars(InsSeq::STATS_store &vars, DesugarContext dctx, unique_ptr<parser::Node> &node) {
    if (auto var = parser::cast_node<parser::MatchVar>(node.get())) {
        auto loc = var->loc;
        auto val = MK::RaiseUnimplemented(loc);
        vars.emplace_back(MK::Assign(loc, var->name, std::move(val)));
    } else if (auto rest = parser::cast_node<parser::MatchRest>(node.get())) {
        desugarPatternMatchingVars(vars, dctx, rest->var);
    } else if (auto pair = parser::cast_node<parser::Pair>(node.get())) {
        desugarPatternMatchingVars(vars, dctx, pair->value);
    } else if (auto as_pattern = parser::cast_node<parser::MatchAs>(node.get())) {
        auto loc = as_pattern->as->loc;
        auto name = parser::cast_node<parser::MatchVar>(as_pattern->as.get())->name;
        auto val = MK::RaiseUnimplemented(loc);
        vars.emplace_back(MK::Assign(loc, name, std::move(val)));
        desugarPatternMatchingVars(vars, dctx, as_pattern->value);
    } else if (auto array_pattern = parser::cast_node<parser::ArrayPattern>(node.get())) {
        for (auto &elt : array_pattern->elts) {
            desugarPatternMatchingVars(vars, dctx, elt);
        }
    } else if (auto array_pattern = parser::cast_node<parser::ArrayPatternWithTail>(node.get())) {
        for (auto &elt : array_pattern->elts) {
            desugarPatternMatchingVars(vars, dctx, elt);
        }
    } else if (auto hash_pattern = parser::cast_node<parser::HashPattern>(node.get())) {
        for (auto &elt : hash_pattern->pairs) {
            desugarPatternMatchingVars(vars, dctx, elt);
        }
    } else if (auto alt_pattern = parser::cast_node<parser::MatchAlt>(node.get())) {
        desugarPatternMatchingVars(vars, dctx, alt_pattern->left);
        desugarPatternMatchingVars(vars, dctx, alt_pattern->right);
    }
}

// Desugar `in` and `=>` oneline pattern matching
ExpressionPtr desugarOnelinePattern(DesugarContext dctx, core::LocOffsets loc, unique_ptr<parser::Node> &match) {
    auto matchExpr = MK::RaiseUnimplemented(loc);
    auto bodyExpr = MK::RaiseUnimplemented(loc);
    auto elseExpr = MK::EmptyTree();

    InsSeq::STATS_store vars;
    desugarPatternMatchingVars(vars, dctx, match);
    if (!vars.empty()) {
        bodyExpr = MK::InsSeq(match->loc, std::move(vars), std::move(bodyExpr));
    }

    return MK::If(loc, std::move(matchExpr), std::move(bodyExpr), std::move(elseExpr));
}

bool locReported = false;

ClassDef::RHS_store scopeNodeToBody(DesugarContext dctx, unique_ptr<parser::Node> node) {
    ClassDef::RHS_store body;
    // Reset uniqueCounter within this scope (to keep numbers small)
    uint32_t uniqueCounter = 1;
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

Send *asTLet(ExpressionPtr &arg) {
    auto *send = cast_tree<Send>(arg);
    if (send == nullptr || send->fun != core::Names::let() || send->numPosArgs() < 2) {
        return nullptr;
    }

    auto *recv = cast_tree<UnresolvedConstantLit>(send->recv);
    if (recv == nullptr || recv->cnst != core::Names::Constants::T() || !isa_tree<EmptyTree>(recv->scope)) {
        return nullptr;
    }

    return send;
}

struct OpAsgnScaffolding {
    core::NameRef temporaryName;
    InsSeq::STATS_store statementBody;
    uint16_t numPosArgs;
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
    ENFORCE(!s->hasKwArgs() && !s->hasBlock());
    const auto numPosArgs = s->numPosArgs();
    InsSeq::STATS_store stats;
    stats.reserve(numPosArgs + 2);
    core::NameRef tempRecv = dctx.freshNameUnique(s->fun);
    stats.emplace_back(MK::Assign(s->loc, tempRecv, std::move(s->recv)));
    Send::ARGS_store readArgs;
    Send::ARGS_store assgnArgs;
    // these are the arguments for the first send, e.g. x.y(). The number of arguments should be identical to whatever
    // we saw on the LHS.
    readArgs.reserve(numPosArgs);
    // these are the arguments for the second send, e.g. x.y=(val). That's why we need the space for the extra argument
    // here: to accomodate the call to field= instead of just field.
    assgnArgs.reserve(numPosArgs + 1);

    for (auto &arg : s->posArgs()) {
        auto argLoc = arg.loc();
        core::NameRef name = dctx.freshNameUnique(s->fun);
        stats.emplace_back(MK::Assign(argLoc, name, std::move(arg)));
        readArgs.emplace_back(MK::Local(argLoc, name));
        assgnArgs.emplace_back(MK::Local(argLoc, name));
    }

    return {tempRecv, std::move(stats), numPosArgs, std::move(readArgs), std::move(assgnArgs)};
}

// while true
//   body
//   if cond
//     break
//   end
// end
ExpressionPtr doUntil(DesugarContext dctx, core::LocOffsets loc, ExpressionPtr cond, ExpressionPtr body) {
    auto breaker = MK::If(loc, std::move(cond), MK::Break(loc, MK::EmptyTree()), MK::EmptyTree());
    auto breakWithBody = MK::InsSeq1(loc, std::move(body), std::move(breaker));
    return MK::While(loc, MK::True(loc), std::move(breakWithBody));
}

class DuplicateHashKeyCheck {
    DesugarContext dctx;
    const core::GlobalState &gs;
    UnorderedMap<core::NameRef, core::LocOffsets> hashKeySymbols;
    UnorderedMap<core::NameRef, core::LocOffsets> hashKeyStrings;

public:
    DuplicateHashKeyCheck(DesugarContext dctx) : dctx{dctx}, gs{dctx.ctx.state}, hashKeySymbols(), hashKeyStrings() {}

    void check(const ExpressionPtr &key) {
        auto lit = ast::cast_tree<ast::Literal>(key);
        if (lit == nullptr) {
            return;
        }

        auto isSymbol = lit->isSymbol();
        core::NameRef nameRef;
        if (!lit) {
            return;
        }
        if (isSymbol) {
            nameRef = lit->asSymbol();
        } else if (lit->isString()) {
            nameRef = lit->asString();
        } else {
            return;
        }

        if (isSymbol && !hashKeySymbols.contains(nameRef)) {
            hashKeySymbols[nameRef] = key.loc();
        } else if (!isSymbol && !hashKeyStrings.contains(nameRef)) {
            hashKeyStrings[nameRef] = key.loc();
        } else {
            if (auto e = dctx.ctx.beginError(key.loc(), core::errors::Desugar::DuplicatedHashKeys)) {
                core::LocOffsets originalLoc;
                if (isSymbol) {
                    originalLoc = hashKeySymbols[nameRef];
                } else {
                    originalLoc = hashKeyStrings[nameRef];
                }

                e.setHeader("Hash key `{}` is duplicated", nameRef.toString(gs));
                e.addErrorLine(dctx.ctx.locAt(originalLoc), "First occurrence of `{}` hash key", nameRef.toString(gs));
            }
        }
    }

    void reset() {
        hashKeySymbols.clear();
        hashKeyStrings.clear();
    }

    // This is only used with Send::ARGS_store and Array::ELEMS_store
    template <typename T> static void checkSendArgs(DesugarContext dctx, int numPosArgs, const T &args) {
        DuplicateHashKeyCheck duplicateKeyCheck{dctx};

        // increment by two so that a keyword args splat gets skipped.
        for (int i = numPosArgs; i < args.size(); i += 2) {
            duplicateKeyCheck.check(args[i]);
        }
    }
};

ExpressionPtr node2TreeImpl(DesugarContext dctx, unique_ptr<parser::Node> what) {
    try {
        if (what.get() == nullptr) {
            return MK::EmptyTree();
        }
        auto loc = what->loc;
        auto locZeroLen = what->loc.copyWithZeroLength();
        ENFORCE(loc.exists(), "parse-tree node has no location: {}", what->toString(dctx.ctx));
        ExpressionPtr result;
        typecase(
            what.get(),
            // The top N clauses here are ordered according to observed
            // frequency in pay-server. Do not reorder the top of this list, or
            // add entries here, without consulting the "node.*" counters from a
            // run over a representative code base.
            [&](parser::Const *const_) {
                auto scope = node2TreeImpl(dctx, std::move(const_->scope));
                ExpressionPtr res = MK::UnresolvedConstant(loc, std::move(scope), const_->name);
                result = std::move(res);
            },
            [&](parser::Send *send) {
                Send::Flags flags;
                auto rec = node2TreeImpl(dctx, std::move(send->receiver));
                if (isa_tree<EmptyTree>(rec)) {
                    // 0-sized Loc, since `self.` doesn't appear in the original file.
                    rec = MK::Self(loc.copyWithZeroLength());
                    flags.isPrivateOk = true;
                } else if (rec.isSelfReference()) {
                    // In Ruby 2.7 `self.foo()` is also allowed for private method calls,
                    // not only `foo()`. This pre-emptively allow the new syntax.
                    flags.isPrivateOk = true;
                }

                if (absl::c_any_of(send->args, [](auto &arg) {
                        return parser::isa_node<parser::Splat>(arg.get()) ||
                               parser::isa_node<parser::ForwardedArgs>(arg.get());
                    })) {
                    // Build up an array that represents the keyword args for the send. When there is a Kwsplat, treat
                    // all keyword arguments as a single argument.
                    unique_ptr<parser::Node> kwArray;

                    // If there's a &blk node in the last position, pop that off (we'll put it back later, but
                    // subsequent logic for dealing with the kwargs hash is simpler this way).
                    unique_ptr<parser::Node> savedBlockPass = nullptr;

                    if (!send->args.empty() && parser::isa_node<parser::BlockPass>(send->args.back().get())) {
                        savedBlockPass = std::move(send->args.back());
                        send->args.pop_back();
                    }

                    // Deconstruct the kwargs hash if it's present.
                    if (!send->args.empty()) {
                        if (auto *hash = parser::cast_node<parser::Hash>(send->args.back().get())) {
                            if (hash->kwargs) {
                                // hold a reference to the node, and remove it from the back of the send list
                                auto node = std::move(send->args.back());
                                send->args.pop_back();

                                parser::NodeVec elts;

                                // skip inlining the kwargs if there are any kwsplat nodes present
                                if (absl::c_any_of(hash->pairs, [](auto &node) {
                                        // the parser guarantees that if we see a kwargs hash it only contains pair or
                                        // kwsplat nodes
                                        ENFORCE(parser::isa_node<parser::Kwsplat>(node.get()) ||
                                                parser::isa_node<parser::Pair>(node.get()));
                                        return parser::isa_node<parser::Kwsplat>(node.get());
                                    })) {
                                    elts.emplace_back(std::move(node));
                                } else {
                                    // inline the hash into the send args
                                    for (auto &entry : hash->pairs) {
                                        typecase(
                                            entry.get(),
                                            [&](parser::Pair *pair) {
                                                elts.emplace_back(std::move(pair->key));
                                                elts.emplace_back(std::move(pair->value));
                                            },
                                            [&](parser::Node *node) { Exception::raise("Unhandled case"); });
                                    }
                                }

                                kwArray = make_unique<parser::Array>(loc, std::move(elts));
                            }
                        }
                    }

                    // Put the &blk arg back, if present.
                    if (savedBlockPass) {
                        send->args.emplace_back(std::move(savedBlockPass));
                    }

                    // If the kwargs hash is not present, make a `nil` to put in the place of that argument. This
                    // will be used in the implementation of the intrinsic to tell the difference between keyword
                    // args, keyword args with kw splats, and no keyword args at all.
                    if (kwArray == nullptr) {
                        kwArray = make_unique<parser::Nil>(loc);
                    }

                    // If we have a splat anywhere in the argument list, desugar
                    // the argument list as a single Array node, and then
                    // synthesize a call to
                    //   Magic.callWithSplat(receiver, method, argArray, [&blk])
                    // The callWithSplat implementation (in C++) will unpack a
                    // tuple type and call into the normal call merchanism.
                    unique_ptr<parser::Node> block;
                    auto argnodes = std::move(send->args);
                    bool anonymousBlockPass = false;
                    core::LocOffsets bpLoc;
                    auto it = absl::c_find_if(argnodes,
                                              [](auto &arg) { return parser::isa_node<parser::BlockPass>(arg.get()); });
                    if (it != argnodes.end()) {
                        auto *bp = parser::cast_node<parser::BlockPass>(it->get());
                        if (bp->block == nullptr) {
                            anonymousBlockPass = true;
                            bpLoc = bp->loc;
                        } else {
                            block = std::move(bp->block);
                        }
                        argnodes.erase(it);
                    }

                    auto hasFwdArgs = false;
                    auto fwdIt = absl::c_find_if(
                        argnodes, [](auto &arg) { return parser::isa_node<parser::ForwardedArgs>(arg.get()); });
                    if (fwdIt != argnodes.end()) {
                        block = make_unique<parser::LVar>(loc, core::Names::fwdBlock());
                        hasFwdArgs = true;
                        argnodes.erase(fwdIt);
                    }

                    auto array = make_unique<parser::Array>(locZeroLen, std::move(argnodes));
                    auto args = node2TreeImpl(dctx, std::move(array));

                    if (hasFwdArgs) {
                        auto fwdArgs = MK::Local(loc, core::Names::fwdArgs());
                        auto argsSplat = MK::Send0(loc, std::move(fwdArgs), core::Names::toA(), locZeroLen);
                        auto argsConcat =
                            MK::Send1(loc, std::move(args), core::Names::concat(), locZeroLen, std::move(argsSplat));

                        auto fwdKwargs = MK::Local(loc, core::Names::fwdKwargs());
                        auto kwargsSplat =
                            MK::Send1(loc, MK::Magic(loc), core::Names::toHashDup(), locZeroLen, std::move(fwdKwargs));

                        Array::ENTRY_store kwargsEntries;
                        kwargsEntries.emplace_back(std::move(kwargsSplat));
                        auto kwargsArray = MK::Array(loc, std::move(kwargsEntries));

                        argsConcat = MK::Send1(loc, std::move(argsConcat), core::Names::concat(), locZeroLen,
                                               std::move(kwargsArray));

                        args = std::move(argsConcat);
                    }

                    auto kwargs = node2TreeImpl(dctx, std::move(kwArray));
                    auto method = MK::Literal(
                        loc, core::make_type<core::NamedLiteralType>(core::Symbols::Symbol(), send->method));

                    if (auto *array = cast_tree<Array>(kwargs)) {
                        DuplicateHashKeyCheck::checkSendArgs(dctx, 0, array->elems);
                    }

                    Send::ARGS_store sendargs;
                    sendargs.emplace_back(std::move(rec));
                    sendargs.emplace_back(std::move(method));
                    sendargs.emplace_back(std::move(args));
                    sendargs.emplace_back(std::move(kwargs));
                    ExpressionPtr res;
                    if (block == nullptr && !anonymousBlockPass) {
                        res = MK::Send(loc, MK::Magic(loc), core::Names::callWithSplat(), locZeroLen, 4,
                                       std::move(sendargs), flags);
                    } else {
                        ExpressionPtr convertedBlock;
                        if (anonymousBlockPass) {
                            ENFORCE(block == nullptr, "encountered a block while processing an anonymous block pass");
                            convertedBlock = MK::Local(bpLoc, core::Names::ampersand());
                        } else {
                            convertedBlock = node2TreeImpl(dctx, std::move(block));
                        }
                        Literal *lit;
                        if ((lit = cast_tree<Literal>(convertedBlock)) && lit->isSymbol()) {
                            res = MK::Send(loc, MK::Magic(loc), core::Names::callWithSplat(), locZeroLen, 4,
                                           std::move(sendargs), flags);
                            ast::cast_tree_nonnull<ast::Send>(res).setBlock(
                                symbol2Proc(dctx, std::move(convertedBlock)));
                        } else {
                            sendargs.emplace_back(std::move(convertedBlock));
                            res = MK::Send(loc, MK::Magic(loc), core::Names::callWithSplatAndBlock(), locZeroLen, 5,
                                           std::move(sendargs), flags);
                        }
                    }
                    result = std::move(res);
                } else {
                    int numPosArgs = send->args.size();
                    if (numPosArgs > 0) {
                        // Deconstruct the kwargs hash in the last argument if it's present.
                        if (auto *hash = parser::cast_node<parser::Hash>(send->args.back().get())) {
                            if (hash->kwargs) {
                                numPosArgs--;

                                // skip inlining the kwargs if there are any non-key/value pairs present
                                if (!absl::c_any_of(hash->pairs, [](auto &node) {
                                        // the parser guarantees that if we see a kwargs hash it only contains pair or
                                        // kwsplat nodes
                                        ENFORCE(parser::isa_node<parser::Kwsplat>(node.get()) ||
                                                parser::isa_node<parser::Pair>(node.get()));
                                        return parser::isa_node<parser::Kwsplat>(node.get());
                                    })) {
                                    // hold a reference to the node, and remove it from the back fo the send list
                                    auto node = std::move(send->args.back());
                                    send->args.pop_back();

                                    // inline the hash into the send args
                                    for (auto &entry : hash->pairs) {
                                        typecase(
                                            entry.get(),
                                            [&](parser::Pair *pair) {
                                                send->args.emplace_back(std::move(pair->key));
                                                send->args.emplace_back(std::move(pair->value));
                                            },
                                            [&](parser::Node *node) { Exception::raise("Unhandled case"); });
                                    }
                                }
                            }
                        }
                    }

                    Send::ARGS_store args;
                    unique_ptr<parser::Node> block;
                    args.reserve(send->args.size());
                    bool anonymousBlockPass = false;
                    core::LocOffsets bpLoc;
                    for (auto &stat : send->args) {
                        if (auto bp = parser::cast_node<parser::BlockPass>(stat.get())) {
                            ENFORCE(block == nullptr, "passing a block where there is no block");
                            if (bp->block == nullptr) {
                                anonymousBlockPass = true;
                                bpLoc = bp->loc;
                            } else {
                                block = std::move(bp->block);
                            }

                            // we don't count the block arg as part of the positional arguments anymore.
                            numPosArgs = max(0, numPosArgs - 1);
                        } else {
                            args.emplace_back(node2TreeImpl(dctx, std::move(stat)));
                        }
                    };

                    DuplicateHashKeyCheck::checkSendArgs(dctx, numPosArgs, args);

                    ExpressionPtr res;
                    if (block == nullptr && !anonymousBlockPass) {
                        res = MK::Send(loc, std::move(rec), send->method, send->methodLoc, numPosArgs, std::move(args),
                                       flags);
                    } else {
                        auto method = MK::Literal(
                            loc, core::make_type<core::NamedLiteralType>(core::Symbols::Symbol(), send->method));
                        ExpressionPtr convertedBlock;
                        if (anonymousBlockPass) {
                            ENFORCE(block == nullptr, "encountered a block while processing an anonymous block pass");
                            convertedBlock = MK::Local(bpLoc, core::Names::ampersand());
                        } else {
                            convertedBlock = node2TreeImpl(dctx, std::move(block));
                        }
                        Literal *lit;
                        if ((lit = cast_tree<Literal>(convertedBlock)) && lit->isSymbol()) {
                            res = MK::Send(loc, std::move(rec), send->method, send->methodLoc, numPosArgs,
                                           std::move(args), flags);
                            ast::cast_tree_nonnull<ast::Send>(res).setBlock(
                                symbol2Proc(dctx, std::move(convertedBlock)));
                        } else {
                            Send::ARGS_store sendargs;
                            sendargs.emplace_back(std::move(rec));
                            sendargs.emplace_back(std::move(method));
                            sendargs.emplace_back(std::move(convertedBlock));

                            numPosArgs += 3;

                            for (auto &arg : args) {
                                sendargs.emplace_back(std::move(arg));
                            }

                            res = MK::Send(loc, MK::Magic(loc), core::Names::callWithBlock(), locZeroLen, numPosArgs,
                                           std::move(sendargs), flags);
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
            [&](parser::String *string) {
                ExpressionPtr res = MK::String(loc, string->val);
                result = std::move(res);
            },
            [&](parser::Symbol *symbol) {
                ExpressionPtr res = MK::Symbol(loc, symbol->val);
                result = std::move(res);
            },
            [&](parser::LVar *var) {
                ExpressionPtr res = MK::Local(loc, var->name);
                result = std::move(res);
            },
            [&](parser::Hash *hash) {
                InsSeq::STATS_store updateStmts;
                updateStmts.reserve(hash->pairs.size());

                auto acc = dctx.freshNameUnique(core::Names::hashTemp());

                DuplicateHashKeyCheck hashKeyDupes(dctx);
                Send::ARGS_store mergeValues;
                mergeValues.reserve(hash->pairs.size() * 2 + 1);
                mergeValues.emplace_back(MK::Local(loc, acc));
                bool havePairsToMerge = false;

                // build a hash literal assuming that the argument follows the same format as `mergeValues`:
                // arg 0: the hash to merge into
                // arg 1: key
                // arg 2: value
                // ...
                // arg n: key
                // arg n+1: value
                auto buildHashLiteral = [loc](Send::ARGS_store &mergeValues) {
                    Hash::ENTRY_store keys;
                    Hash::ENTRY_store values;

                    keys.reserve(mergeValues.size() / 2);
                    values.reserve(mergeValues.size() / 2);

                    // skip the first positional argument for the accumulator that would have been mutated
                    for (auto it = mergeValues.begin() + 1; it != mergeValues.end();) {
                        keys.emplace_back(std::move(*it++));
                        values.emplace_back(std::move(*it++));
                    }

                    return MK::Hash(loc, std::move(keys), std::move(values));
                };

                // Desguar
                //   {**x, a: 'a', **y, remaining}
                // into
                //   acc = <Magic>.<to-hash-dup>(x)
                //   acc = <Magic>.<merge-hash-values>(acc, :a, 'a')
                //   acc = <Magic>.<merge-hash>(acc, <Magic>.<to-hash-nodup>(y))
                //   acc = <Magic>.<merge-hash>(acc, remaining)
                //   acc
                for (auto &pairAsExpression : hash->pairs) {
                    auto *pair = parser::cast_node<parser::Pair>(pairAsExpression.get());
                    if (pair != nullptr) {
                        auto key = node2TreeImpl(dctx, std::move(pair->key));
                        hashKeyDupes.check(key);
                        mergeValues.emplace_back(std::move(key));

                        auto value = node2TreeImpl(dctx, std::move(pair->value));
                        mergeValues.emplace_back(std::move(value));

                        havePairsToMerge = true;
                        continue;
                    }

                    auto *splat = parser::cast_node<parser::Kwsplat>(pairAsExpression.get());
                    ENFORCE(splat != nullptr, "kwsplat cast failed");

                    if (havePairsToMerge) {
                        havePairsToMerge = false;

                        // ensure that there's something to update
                        if (updateStmts.empty()) {
                            updateStmts.emplace_back(MK::Assign(loc, acc, buildHashLiteral(mergeValues)));
                        } else {
                            int numPosArgs = mergeValues.size();
                            updateStmts.emplace_back(
                                MK::Assign(loc, acc,
                                           MK::Send(loc, MK::Magic(loc), core::Names::mergeHashValues(), locZeroLen,
                                                    numPosArgs, std::move(mergeValues))));
                        }

                        mergeValues.clear();
                        mergeValues.emplace_back(MK::Local(loc, acc));
                    }

                    auto expr = node2TreeImpl(dctx, std::move(splat->expr));

                    // If this is the first argument to `<Magic>.<merge-hash>`, it needs to be duplicated as that
                    // intrinsic is assumed to mutate its first argument.
                    if (updateStmts.empty()) {
                        updateStmts.emplace_back(MK::Assign(
                            loc, acc,
                            MK::Send1(loc, MK::Magic(loc), core::Names::toHashDup(), locZeroLen, std::move(expr))));
                    } else {
                        updateStmts.emplace_back(MK::Assign(
                            loc, acc,
                            MK::Send2(loc, MK::Magic(loc), core::Names::mergeHash(), locZeroLen, MK::Local(loc, acc),
                                      MK::Send1(loc, MK::Magic(loc), core::Names::toHashNoDup(), locZeroLen,
                                                std::move(expr)))));
                    }
                };

                if (havePairsToMerge) {
                    // There were only keyword args/values present, so construct a hash literal directly
                    if (updateStmts.empty()) {
                        result = buildHashLiteral(mergeValues);
                        return;
                    }

                    // there are already other entries in updateStmts, so append the `merge-hash-values` call and fall
                    // through to the rest of the processing
                    int numPosArgs = mergeValues.size();
                    updateStmts.emplace_back(MK::Assign(loc, acc,
                                                        MK::Send(loc, MK::Magic(loc), core::Names::mergeHashValues(),
                                                                 locZeroLen, numPosArgs, std::move(mergeValues))));
                }

                if (updateStmts.empty()) {
                    result = MK::Hash0(loc);
                } else {
                    result = MK::InsSeq(loc, std::move(updateStmts), MK::Local(loc, acc));
                }
            },
            [&](parser::Block *block) {
                result = desugarBlock(dctx, loc, block->loc, block->send, block->args, block->body);
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
                    ExpressionPtr res = MK::Nil(loc);
                    result = std::move(res);
                }
            },
            [&](parser::Assign *asgn) {
                auto lhs = node2TreeImpl(dctx, std::move(asgn->lhs));
                auto rhs = node2TreeImpl(dctx, std::move(asgn->rhs));
                // Ensure that X = <ErrorNode> always looks like a proper static field, rather
                // than a class alias.  Leaving it as a class alias would require taking the
                // slow path; turning it into a proper static field gives us a chance to take
                // the fast path.  T.let(<ErrorNode>, T.untyped) ensures that we don't get
                // warnings in `typed: strict` files.
                if (isa_tree<UnresolvedConstantLit>(lhs) && isa_tree<UnresolvedConstantLit>(rhs)) {
                    auto &rhsConst = cast_tree_nonnull<UnresolvedConstantLit>(rhs);
                    if (rhsConst.cnst == core::Names::Constants::ErrorNode()) {
                        auto rhsLocZero = rhs.loc().copyWithZeroLength();
                        rhs = MK::Let(rhsLocZero, std::move(rhs), MK::Untyped(rhsLocZero));
                    }
                }
                auto res = MK::Assign(loc, std::move(lhs), std::move(rhs));
                result = std::move(res);
            },
            // END hand-ordered clauses
            [&](parser::And *and_) {
                auto lhs = node2TreeImpl(dctx, std::move(and_->left));
                auto rhs = node2TreeImpl(dctx, std::move(and_->right));
                if (isa_reference(lhs)) {
                    auto cond = MK::cpRef(lhs);
                    // Note that this case doesn't currently get the same "always truthy" dead code
                    // error that the other case would get.
                    auto iff = MK::If(loc, std::move(cond), std::move(rhs), std::move(lhs));
                    result = std::move(iff);
                } else {
                    auto andAndTemp = dctx.freshNameUnique(core::Names::andAnd());

                    auto *lhsSend = ast::cast_tree<ast::Send>(lhs);
                    auto *rhsSend = ast::cast_tree<ast::Send>(rhs);

                    ExpressionPtr thenp;
                    if (lhsSend != nullptr && rhsSend != nullptr) {
                        auto lhsSource = dctx.ctx.locAt(lhsSend->loc).source(dctx.ctx);
                        auto rhsRecvSource = dctx.ctx.locAt(rhsSend->recv.loc()).source(dctx.ctx);
                        if (lhsSource.has_value() && lhsSource == rhsRecvSource) {
                            // Have to use zero-width locs here so that these auto-generated things
                            // don't show up in e.g. completion requests.
                            rhsSend->insertPosArg(0, std::move(rhsSend->recv));
                            rhsSend->insertPosArg(1, MK::Local(loc.copyWithZeroLength(), andAndTemp));
                            rhsSend->insertPosArg(2, MK::Symbol(rhsSend->funLoc.copyWithZeroLength(), rhsSend->fun));
                            rhsSend->recv = MK::Magic(loc.copyWithZeroLength());
                            rhsSend->fun = core::Names::checkAndAnd();
                            thenp = std::move(rhs);
                        } else {
                            thenp = std::move(rhs);
                        }
                    } else {
                        thenp = std::move(rhs);
                    }
                    auto lhsLoc = lhs.loc();
                    auto temp = MK::Assign(loc, andAndTemp, std::move(lhs));
                    auto iff =
                        MK::If(loc, MK::Local(lhsLoc, andAndTemp), std::move(thenp), MK::Local(lhsLoc, andAndTemp));
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
                    core::NameRef tempName = dctx.freshNameUnique(core::Names::orOr());
                    auto lhsLoc = lhs.loc();
                    auto temp = MK::Assign(loc, tempName, std::move(lhs));
                    auto iff = MK::If(loc, MK::Local(lhsLoc, tempName), MK::Local(lhsLoc, tempName), std::move(rhs));
                    auto wrapped = MK::InsSeq1(loc, std::move(temp), std::move(iff));
                    result = std::move(wrapped);
                }
            },
            [&](parser::AndAsgn *andAsgn) {
                auto recv = node2TreeImpl(dctx, std::move(andAsgn->left));
                auto arg = node2TreeImpl(dctx, std::move(andAsgn->right));
                if (auto s = cast_tree<Send>(recv)) {
                    auto sendLoc = s->loc;
                    auto [tempRecv, stats, numPosArgs, readArgs, assgnArgs] = copyArgsForOpAsgn(dctx, s);
                    auto numPosAssgnArgs = numPosArgs + 1;

                    assgnArgs.emplace_back(std::move(arg));
                    auto cond = MK::Send(sendLoc, MK::Local(sendLoc, tempRecv), s->fun, s->funLoc, numPosArgs,
                                         std::move(readArgs), s->flags);
                    core::NameRef tempResult = dctx.freshNameUnique(s->fun);
                    stats.emplace_back(MK::Assign(sendLoc, tempResult, std::move(cond)));

                    auto body = MK::Send(sendLoc, MK::Local(sendLoc, tempRecv), s->fun.addEq(dctx.ctx),
                                         sendLoc.copyWithZeroLength(), numPosAssgnArgs, std::move(assgnArgs), s->flags);
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
                    ExpressionPtr res = MK::EmptyTree();
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
                    auto [tempRecv, stats, numPosArgs, readArgs, assgnArgs] = copyArgsForOpAsgn(dctx, s);
                    auto numPosAssgnArgs = numPosArgs + 1;
                    assgnArgs.emplace_back(std::move(arg));
                    auto cond = MK::Send(sendLoc, MK::Local(sendLoc, tempRecv), s->fun, s->funLoc, numPosArgs,
                                         std::move(readArgs), s->flags);
                    core::NameRef tempResult = dctx.freshNameUnique(s->fun);
                    stats.emplace_back(MK::Assign(sendLoc, tempResult, std::move(cond)));

                    auto body = MK::Send(sendLoc, MK::Local(sendLoc, tempRecv), s->fun.addEq(dctx.ctx),
                                         sendLoc.copyWithZeroLength(), numPosAssgnArgs, std::move(assgnArgs), s->flags);
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
                auto recvIsIvarLhs = parser::isa_node<parser::IVarLhs>(orAsgn->left.get());
                auto recvIsCvarLhs = parser::isa_node<parser::CVarLhs>(orAsgn->left.get());
                auto recv = node2TreeImpl(dctx, std::move(orAsgn->left));
                auto arg = node2TreeImpl(dctx, std::move(orAsgn->right));
                if (auto s = cast_tree<Send>(recv)) {
                    auto sendLoc = s->loc;
                    auto [tempRecv, stats, numPosArgs, readArgs, assgnArgs] = copyArgsForOpAsgn(dctx, s);
                    auto numPosAssgnArgs = numPosArgs + 1;
                    assgnArgs.emplace_back(std::move(arg));
                    auto cond = MK::Send(sendLoc, MK::Local(sendLoc, tempRecv), s->fun, s->funLoc, numPosArgs,
                                         std::move(readArgs), s->flags);
                    core::NameRef tempResult = dctx.freshNameUnique(s->fun);
                    stats.emplace_back(MK::Assign(sendLoc, tempResult, std::move(cond)));

                    auto elsep =
                        MK::Send(sendLoc, MK::Local(sendLoc, tempRecv), s->fun.addEq(dctx.ctx),
                                 sendLoc.copyWithZeroLength(), numPosAssgnArgs, std::move(assgnArgs), s->flags);
                    auto body = MK::Local(sendLoc, tempResult);
                    auto iff = MK::If(sendLoc, MK::Local(sendLoc, tempResult), std::move(body), std::move(elsep));
                    auto wrapped = MK::InsSeq(loc, std::move(stats), std::move(iff));
                    result = std::move(wrapped);
                } else if (isa_reference(recv)) {
                    // When it's a reference (something variable-like), using the recv/Send terminology only
                    // confuses things. Let's just call it LHS like we would for normal assignments.
                    auto lhs = move(recv);
                    auto cond = MK::cpRef(lhs);
                    auto body = MK::cpRef(lhs);
                    ExpressionPtr elsep;
                    ast::Send *tlet;
                    if ((recvIsIvarLhs || recvIsCvarLhs) && (tlet = asTLet(arg))) {
                        auto val = std::move(tlet->getPosArg(0));
                        tlet->getPosArg(0) = MK::cpRef(lhs);

                        auto tempLocalName = dctx.freshNameUnique(core::Names::statTemp());
                        auto tempLocal = MK::Local(loc, tempLocalName);
                        auto value = MK::Assign(loc, MK::cpRef(tempLocal), std::move(val));

                        auto decl = MK::Assign(loc, MK::cpRef(lhs), std::move(arg));
                        auto assign = MK::Assign(loc, MK::cpRef(lhs), std::move(tempLocal));

                        InsSeq::STATS_store stats;
                        stats.emplace_back(std::move(decl));
                        stats.emplace_back(std::move(value));

                        elsep = MK::InsSeq(loc, std::move(stats), std::move(assign));
                    } else {
                        elsep = MK::Assign(loc, std::move(lhs), std::move(arg));
                    }
                    auto iff = MK::If(loc, std::move(cond), std::move(body), std::move(elsep));
                    result = std::move(iff);
                } else if (auto i = cast_tree<UnresolvedConstantLit>(recv)) {
                    if (auto e = dctx.ctx.beginError(what->loc, core::errors::Desugar::NoConstantReassignment)) {
                        e.setHeader("Constant reassignment is not supported");
                    }
                    ExpressionPtr res = MK::EmptyTree();
                    result = std::move(res);
                } else if (auto i = cast_tree<InsSeq>(recv)) {
                    // The logic below is explained more fully in the OpAsgn case
                    auto ifExpr = cast_tree<If>(i->expr);
                    if (!ifExpr) {
                        Exception::raise("Unexpected left-hand side of ||=: please file an issue");
                    }
                    auto s = cast_tree<Send>(ifExpr->elsep);
                    if (!s) {
                        Exception::raise("Unexpected left-hand side of ||=: please file an issue");
                    }

                    auto sendLoc = s->loc;
                    auto [tempRecv, stats, numPosArgs, readArgs, assgnArgs] = copyArgsForOpAsgn(dctx, s);
                    auto numPosAssgnArgs = numPosArgs + 1;
                    assgnArgs.emplace_back(std::move(arg));
                    auto cond = MK::Send(sendLoc, MK::Local(sendLoc, tempRecv), s->fun, s->funLoc, numPosArgs,
                                         std::move(readArgs), s->flags);
                    core::NameRef tempResult = dctx.freshNameUnique(s->fun);
                    stats.emplace_back(MK::Assign(sendLoc, tempResult, std::move(cond)));

                    auto elsep =
                        MK::Send(sendLoc, MK::Local(sendLoc, tempRecv), s->fun.addEq(dctx.ctx),
                                 sendLoc.copyWithZeroLength(), numPosAssgnArgs, std::move(assgnArgs), s->flags);
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
                    auto [tempRecv, stats, numPosArgs, readArgs, assgnArgs] = copyArgsForOpAsgn(dctx, s);

                    auto prevValue = MK::Send(sendLoc, MK::Local(sendLoc, tempRecv), s->fun, s->funLoc, numPosArgs,
                                              std::move(readArgs), s->flags);
                    auto newValue = MK::Send1(sendLoc, std::move(prevValue), opAsgn->op, opAsgn->opLoc, std::move(rhs));
                    auto numPosAssgnArgs = numPosArgs + 1;
                    assgnArgs.emplace_back(std::move(newValue));

                    auto res = MK::Send(sendLoc, MK::Local(sendLoc, tempRecv), s->fun.addEq(dctx.ctx),
                                        sendLoc.copyWithZeroLength(), numPosAssgnArgs, std::move(assgnArgs), s->flags);
                    auto wrapped = MK::InsSeq(loc, std::move(stats), std::move(res));
                    result = std::move(wrapped);
                } else if (isa_reference(recv)) {
                    auto lhs = MK::cpRef(recv);
                    auto send = MK::Send1(loc, std::move(recv), opAsgn->op, opAsgn->opLoc, std::move(rhs));
                    auto res = MK::Assign(loc, std::move(lhs), std::move(send));
                    result = std::move(res);
                } else if (auto i = cast_tree<UnresolvedConstantLit>(recv)) {
                    if (auto e = dctx.ctx.beginError(what->loc, core::errors::Desugar::NoConstantReassignment)) {
                        e.setHeader("Constant reassignment is not supported");
                    }
                    ExpressionPtr res = MK::EmptyTree();
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
                    auto [tempRecv, stats, numPosArgs, readArgs, assgnArgs] = copyArgsForOpAsgn(dctx, s);
                    auto prevValue = MK::Send(sendLoc, MK::Local(sendLoc, tempRecv), s->fun, s->funLoc, numPosArgs,
                                              std::move(readArgs), s->flags);
                    auto newValue = MK::Send1(sendLoc, std::move(prevValue), opAsgn->op, opAsgn->opLoc, std::move(rhs));
                    auto numPosAssgnArgs = numPosArgs + 1;
                    assgnArgs.emplace_back(std::move(newValue));

                    auto res = MK::Send(sendLoc, MK::Local(sendLoc, tempRecv), s->fun.addEq(dctx.ctx),
                                        sendLoc.copyWithZeroLength(), numPosAssgnArgs, std::move(assgnArgs), s->flags);
                    auto wrapped = MK::InsSeq(loc, std::move(stats), std::move(res));
                    ifExpr->elsep = std::move(wrapped);
                    result = std::move(recv);

                } else {
                    // the LHS has been desugared to something we haven't expected
                    Exception::notImplemented();
                }
            },
            [&](parser::CSend *csend) {
                core::NameRef tempRecv = dctx.freshNameUnique(core::Names::assignTemp());
                auto recvLoc = csend->receiver->loc;
                // Assign some desugar-produced nodes with zero-length Locs so IDE ignores them when mapping text
                // location to node.
                auto zeroLengthLoc = loc.copyWithZeroLength();
                auto zeroLengthRecvLoc = recvLoc.copyWithZeroLength();
                auto csendLoc = recvLoc.copyEndWithZeroLength();
                if (recvLoc.endPos() + 1 <= dctx.ctx.file.data(dctx.ctx).source().size()) {
                    auto ampersandLoc = core::LocOffsets{recvLoc.endPos(), recvLoc.endPos() + 1};
                    // The arg loc for the synthetic variable created for the purpose of this safe navigation
                    // check is a bit of a hack. It's intentionally one character too short so that for
                    // completion requests it doesn't match `x&.|` (which would defeat completion requests.)
                    if (dctx.ctx.locAt(ampersandLoc).source(dctx.ctx) == "&") {
                        csendLoc = ampersandLoc;
                    }
                }

                auto assgn = MK::Assign(zeroLengthRecvLoc, tempRecv, node2TreeImpl(dctx, std::move(csend->receiver)));

                // Just compare with `NilClass` to avoid potentially calling into a class-defined `==`
                auto cond =
                    MK::Send1(zeroLengthLoc, ast::MK::Constant(zeroLengthRecvLoc, core::Symbols::NilClass()),
                              core::Names::tripleEq(), zeroLengthRecvLoc, MK::Local(zeroLengthRecvLoc, tempRecv));

                unique_ptr<parser::Node> sendNode =
                    make_unique<parser::Send>(loc, make_unique<parser::LVar>(recvLoc, tempRecv), csend->method,
                                              csend->methodLoc, std::move(csend->args));
                auto send = node2TreeImpl(dctx, std::move(sendNode));

                ExpressionPtr nil =
                    MK::Send1(recvLoc.copyEndWithZeroLength(), MK::Magic(zeroLengthLoc),
                              core::Names::nilForSafeNavigation(), zeroLengthLoc, MK::Local(csendLoc, tempRecv));
                auto iff = MK::If(zeroLengthLoc, std::move(cond), std::move(nil), std::move(send));
                auto res = MK::InsSeq1(csend->loc, std::move(assgn), std::move(iff));
                result = std::move(res);
            },
            [&](parser::Self *self) {
                ExpressionPtr res = MK::Self(loc);
                result = std::move(res);
            },
            [&](parser::DSymbol *dsymbol) {
                if (dsymbol->nodes.empty()) {
                    ExpressionPtr res = MK::Symbol(loc, core::Names::empty());
                    result = std::move(res);
                    return;
                }

                auto str = desugarDString(dctx, loc, std::move(dsymbol->nodes));
                ExpressionPtr res = MK::Send0(loc, std::move(str), core::Names::intern(), locZeroLen);

                result = std::move(res);
            },
            [&](parser::FileLiteral *fileLiteral) {
                ExpressionPtr res = MK::String(loc, core::Names::currentFile());
                result = std::move(res);
            },
            [&](parser::ConstLhs *constLhs) {
                auto scope = node2TreeImpl(dctx, std::move(constLhs->scope));
                ExpressionPtr res = MK::UnresolvedConstant(loc, std::move(scope), constLhs->name);
                result = std::move(res);
            },
            [&](parser::Cbase *cbase) {
                ExpressionPtr res = MK::Constant(loc, core::Symbols::root());
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
                    ExpressionPtr res = MK::EmptyTree();
                    result = std::move(res);
                }
            },
            [&](parser::Module *module) {
                ClassDef::RHS_store body = scopeNodeToBody(dctx, std::move(module->body));
                ClassDef::ANCESTORS_store ancestors;
                ExpressionPtr res =
                    MK::Module(module->loc, module->declLoc, node2TreeImpl(dctx, std::move(module->name)),
                               std::move(ancestors), std::move(body));
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
                ExpressionPtr res = MK::Class(claz->loc, claz->declLoc, node2TreeImpl(dctx, std::move(claz->name)),
                                              std::move(ancestors), std::move(body));
                result = std::move(res);
            },
            [&](parser::Arg *arg) {
                ExpressionPtr res = MK::Local(loc, arg->name);
                result = std::move(res);
            },
            [&](parser::Restarg *arg) {
                ExpressionPtr res = MK::RestArg(loc, MK::Local(arg->nameLoc, arg->name));
                result = std::move(res);
            },
            [&](parser::Kwrestarg *arg) {
                ExpressionPtr res = MK::RestArg(loc, MK::KeywordArg(loc, arg->name));
                result = std::move(res);
            },
            [&](parser::Kwarg *arg) {
                ExpressionPtr res = MK::KeywordArg(loc, arg->name);
                result = std::move(res);
            },
            [&](parser::Blockarg *arg) {
                ExpressionPtr res = MK::BlockArg(loc, MK::Local(loc, arg->name));
                result = std::move(res);
            },
            [&](parser::Kwoptarg *arg) {
                ExpressionPtr res = MK::OptionalArg(loc, MK::KeywordArg(arg->nameLoc, arg->name),
                                                    node2TreeImpl(dctx, std::move(arg->default_)));
                result = std::move(res);
            },
            [&](parser::Optarg *arg) {
                ExpressionPtr res = MK::OptionalArg(loc, MK::Local(arg->nameLoc, arg->name),
                                                    node2TreeImpl(dctx, std::move(arg->default_)));
                result = std::move(res);
            },
            [&](parser::Shadowarg *arg) {
                ExpressionPtr res = MK::ShadowArg(loc, MK::Local(loc, arg->name));
                result = std::move(res);
            },
            [&](parser::DefMethod *method) {
                bool isSelf = false;
                ExpressionPtr res =
                    buildMethod(dctx, method->loc, method->declLoc, method->name, method->args, method->body, isSelf);
                result = std::move(res);
            },
            [&](parser::DefS *method) {
                auto *self = parser::cast_node<parser::Self>(method->singleton.get());
                if (self == nullptr) {
                    if (auto e =
                            dctx.ctx.beginError(method->singleton->loc, core::errors::Desugar::InvalidSingletonDef)) {
                        e.setHeader("`{}` is only supported for `{}`", "def EXPRESSION.method", "def self.method");
                    }
                }
                bool isSelf = true;
                ExpressionPtr res =
                    buildMethod(dctx, method->loc, method->declLoc, method->name, method->args, method->body, isSelf);
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
                    ExpressionPtr res = MK::EmptyTree();
                    result = std::move(res);
                    return;
                }

                ClassDef::RHS_store body = scopeNodeToBody(dctx, std::move(sclass->body));
                ClassDef::ANCESTORS_store emptyAncestors;
                ExpressionPtr res =
                    MK::Class(sclass->loc, sclass->declLoc,
                              make_expression<UnresolvedIdent>(sclass->expr->loc, UnresolvedIdent::Kind::Class,
                                                               core::Names::singleton()),
                              std::move(emptyAncestors), std::move(body));
                result = std::move(res);
            },
            [&](parser::NumBlock *block) {
                result = desugarBlock(dctx, loc, block->loc, block->send, block->args, block->body);
            },
            [&](parser::While *wl) {
                auto cond = node2TreeImpl(dctx, std::move(wl->cond));
                auto body = node2TreeImpl(dctx, std::move(wl->body));
                ExpressionPtr res = MK::While(loc, std::move(cond), std::move(body));
                result = std::move(res);
            },
            [&](parser::WhilePost *wl) {
                bool isKwbegin = parser::isa_node<parser::Kwbegin>(wl->body.get());
                auto cond = node2TreeImpl(dctx, std::move(wl->cond));
                auto body = node2TreeImpl(dctx, std::move(wl->body));
                // TODO using bang (aka !) is not semantically correct because it can be overridden by the user.
                ExpressionPtr res =
                    isKwbegin ? doUntil(dctx, loc, MK::Send0(loc, std::move(cond), core::Names::bang(), locZeroLen),
                                        std::move(body))
                              : MK::While(loc, std::move(cond), std::move(body));
                result = std::move(res);
            },
            [&](parser::Until *wl) {
                auto cond = node2TreeImpl(dctx, std::move(wl->cond));
                auto body = node2TreeImpl(dctx, std::move(wl->body));
                ExpressionPtr res =
                    MK::While(loc, MK::Send0(loc, std::move(cond), core::Names::bang(), locZeroLen), std::move(body));
                result = std::move(res);
            },
            // This is the same as WhilePost, but the cond negation is in the other branch.
            [&](parser::UntilPost *wl) {
                bool isKwbegin = parser::isa_node<parser::Kwbegin>(wl->body.get());
                auto cond = node2TreeImpl(dctx, std::move(wl->cond));
                auto body = node2TreeImpl(dctx, std::move(wl->body));
                ExpressionPtr res =
                    isKwbegin ? doUntil(dctx, loc, std::move(cond), std::move(body))
                              : MK::While(loc, MK::Send0(loc, std::move(cond), core::Names::bang(), locZeroLen),
                                          std::move(body));
                result = std::move(res);
            },
            [&](parser::Nil *wl) {
                ExpressionPtr res = MK::Nil(loc);
                result = std::move(res);
            },
            [&](parser::IVar *var) {
                ExpressionPtr res = make_expression<UnresolvedIdent>(loc, UnresolvedIdent::Kind::Instance, var->name);
                result = std::move(res);
            },
            [&](parser::GVar *var) {
                ExpressionPtr res = make_expression<UnresolvedIdent>(loc, UnresolvedIdent::Kind::Global, var->name);
                result = std::move(res);
            },
            [&](parser::CVar *var) {
                ExpressionPtr res = make_expression<UnresolvedIdent>(loc, UnresolvedIdent::Kind::Class, var->name);
                result = std::move(res);
            },
            [&](parser::LVarLhs *var) {
                ExpressionPtr res = MK::Local(loc, var->name);
                result = std::move(res);
            },
            [&](parser::GVarLhs *var) {
                ExpressionPtr res = make_expression<UnresolvedIdent>(loc, UnresolvedIdent::Kind::Global, var->name);
                result = std::move(res);
            },
            [&](parser::CVarLhs *var) {
                ExpressionPtr res = make_expression<UnresolvedIdent>(loc, UnresolvedIdent::Kind::Class, var->name);
                result = std::move(res);
            },
            [&](parser::IVarLhs *var) {
                ExpressionPtr res = make_expression<UnresolvedIdent>(loc, UnresolvedIdent::Kind::Instance, var->name);
                result = std::move(res);
            },
            [&](parser::NthRef *var) {
                ExpressionPtr res = make_expression<UnresolvedIdent>(loc, UnresolvedIdent::Kind::Global,
                                                                     dctx.ctx.state.enterNameUTF8(to_string(var->ref)));
                result = std::move(res);
            },
            [&](parser::Super *super) {
                // Desugar super into a call to a normal method named `super`;
                // Do this by synthesizing a `Send` parse node and letting our
                // Send desugar handle it.
                auto method = core::Names::super();
                auto send = make_unique<parser::Send>(super->loc, nullptr, method, super->loc, std::move(super->args));
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
                        ExpressionPtr lhs = node2TreeImpl(dctx, std::move(mlhsNode));
                        args.emplace_back(move(lhs));
                    } else {
                        parser::NodeVec vars;
                        vars.emplace_back(std::move(mlhsNode));
                        mlhsNode = make_unique<parser::Mlhs>(loc, std::move(vars));
                    }
                }

                auto body = node2TreeImpl(dctx, std::move(for_->body));

                ExpressionPtr block;
                if (canProvideNiceDesugar) {
                    block = MK::Block(loc, std::move(body), std::move(args));
                } else {
                    auto temp = dctx.freshNameUnique(core::Names::forTemp());

                    unique_ptr<parser::Node> masgn =
                        make_unique<parser::Masgn>(loc, std::move(mlhsNode), make_unique<parser::LVar>(loc, temp));

                    body = MK::InsSeq1(loc, node2TreeImpl(dctx, std::move(masgn)), move(body));
                    block = MK::Block(loc, std::move(body), std::move(args));
                }

                Send::ARGS_store noargs;
                auto res = MK::Send0Block(loc, node2TreeImpl(dctx, std::move(for_->expr)), core::Names::each(),
                                          locZeroLen, std::move(block));
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

                ExpressionPtr res = MK::Int(loc, hasTilde ? ~val : val);
                result = std::move(res);
            },
            [&](parser::DString *dstring) {
                ExpressionPtr res = desugarDString(dctx, loc, std::move(dstring->nodes));
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
                        e.addErrorNote("This likely represents a bug in Sorbet. Please report an issue:\n"
                                       "    https://github.com/sorbet/sorbet/issues");
                    }
                }

                ExpressionPtr res = MK::Float(loc, val);
                result = std::move(res);
            },
            [&](parser::Complex *complex) {
                auto kernel = MK::Constant(loc, core::Symbols::Kernel());
                core::NameRef complex_name = core::Names::Constants::Complex().dataCnst(dctx.ctx)->original;
                core::NameRef value = dctx.ctx.state.enterNameUTF8(complex->value);
                auto send = MK::Send2(loc, std::move(kernel), complex_name, locZeroLen, MK::Int(loc, 0),
                                      MK::String(loc, value));
                result = std::move(send);
            },
            [&](parser::Rational *complex) {
                auto kernel = MK::Constant(loc, core::Symbols::Kernel());
                core::NameRef complex_name = core::Names::Constants::Rational().dataCnst(dctx.ctx)->original;
                core::NameRef value = dctx.ctx.state.enterNameUTF8(complex->val);
                auto send = MK::Send1(loc, std::move(kernel), complex_name, locZeroLen, MK::String(loc, value));
                result = std::move(send);
            },
            [&](parser::Array *array) {
                Array::ENTRY_store elems;
                elems.reserve(array->elts.size());
                ExpressionPtr lastMerge;
                for (auto &stat : array->elts) {
                    if (auto splat = parser::cast_node<parser::Splat>(stat.get())) {
                        // Desguar
                        //   [a, *x, remaining}
                        // into
                        //   a.concat(<splat>(x)).concat(remaining)
                        auto var = MK::Splat(loc, node2TreeImpl(dctx, std::move(splat->var)));
                        if (elems.empty()) {
                            if (lastMerge != nullptr) {
                                lastMerge = MK::Send1(loc, std::move(lastMerge), core::Names::concat(), locZeroLen,
                                                      std::move(var));
                            } else {
                                lastMerge = std::move(var);
                            }
                        } else {
                            ExpressionPtr current = MK::Array(loc, std::move(elems));
                            /* reassign instead of clear to work around https://bugs.llvm.org/show_bug.cgi?id=37553 */
                            elems = Array::ENTRY_store();
                            if (lastMerge != nullptr) {
                                lastMerge = MK::Send1(loc, std::move(lastMerge), core::Names::concat(), locZeroLen,
                                                      std::move(current));
                            } else {
                                lastMerge = std::move(current);
                            }
                            lastMerge =
                                MK::Send1(loc, std::move(lastMerge), core::Names::concat(), locZeroLen, std::move(var));
                        }
                    } else {
                        elems.emplace_back(node2TreeImpl(dctx, std::move(stat)));
                    }
                };

                ExpressionPtr res;
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
                        res = MK::Send1(loc, std::move(lastMerge), core::Names::concat(), locZeroLen, std::move(res));
                    }
                }
                result = std::move(res);
            },
            [&](parser::IRange *ret) {
                auto recv = MK::Magic(loc);
                auto from = node2TreeImpl(dctx, std::move(ret->from));
                auto to = node2TreeImpl(dctx, std::move(ret->to));
                auto excludeEnd = MK::False(loc);
                auto send = MK::Send3(loc, std::move(recv), core::Names::buildRange(), locZeroLen, std::move(from),
                                      std::move(to), std::move(excludeEnd));
                result = std::move(send);
            },
            [&](parser::ERange *ret) {
                auto recv = MK::Magic(loc);
                auto from = node2TreeImpl(dctx, std::move(ret->from));
                auto to = node2TreeImpl(dctx, std::move(ret->to));
                auto excludeEnd = MK::True(loc);
                auto send = MK::Send3(loc, std::move(recv), core::Names::buildRange(), locZeroLen, std::move(from),
                                      std::move(to), std::move(excludeEnd));
                result = std::move(send);
            },
            [&](parser::Regexp *regexpNode) {
                ExpressionPtr cnst = MK::Constant(loc, core::Symbols::Regexp());
                auto pattern = desugarDString(dctx, loc, std::move(regexpNode->regex));
                auto opts = node2TreeImpl(dctx, std::move(regexpNode->opts));
                auto send = MK::Send2(loc, std::move(cnst), core::Names::new_(), locZeroLen, std::move(pattern),
                                      std::move(opts));
                result = std::move(send);
            },
            [&](parser::Regopt *regopt) {
                int flags = 0;
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
                    flags |= flag;
                }
                result = MK::Int(loc, flags);
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
                    ExpressionPtr arr = MK::Array(loc, std::move(elems));
                    ExpressionPtr res = MK::Return(loc, std::move(arr));
                    result = std::move(res);
                } else if (ret->exprs.size() == 1) {
                    if (parser::isa_node<parser::BlockPass>(ret->exprs[0].get())) {
                        if (auto e = dctx.ctx.beginError(ret->loc, core::errors::Desugar::UnsupportedNode)) {
                            e.setHeader("Block argument should not be given");
                        }
                        ExpressionPtr res = MK::Break(loc, MK::EmptyTree());
                        result = std::move(res);
                    } else {
                        ExpressionPtr res = MK::Return(loc, node2TreeImpl(dctx, std::move(ret->exprs[0])));
                        result = std::move(res);
                    }
                } else {
                    ExpressionPtr res = MK::Return(loc, MK::EmptyTree());
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
                    ExpressionPtr arr = MK::Array(loc, std::move(elems));
                    ExpressionPtr res = MK::Break(loc, std::move(arr));
                    result = std::move(res);
                } else if (ret->exprs.size() == 1) {
                    if (parser::isa_node<parser::BlockPass>(ret->exprs[0].get())) {
                        if (auto e = dctx.ctx.beginError(ret->loc, core::errors::Desugar::UnsupportedNode)) {
                            e.setHeader("Block argument should not be given");
                        }
                        ExpressionPtr res = MK::Break(loc, MK::EmptyTree());
                        result = std::move(res);
                    } else {
                        ExpressionPtr res = MK::Break(loc, node2TreeImpl(dctx, std::move(ret->exprs[0])));
                        result = std::move(res);
                    }
                } else {
                    ExpressionPtr res = MK::Break(loc, MK::EmptyTree());
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
                    ExpressionPtr arr = MK::Array(loc, std::move(elems));
                    ExpressionPtr res = MK::Next(loc, std::move(arr));
                    result = std::move(res);
                } else if (ret->exprs.size() == 1) {
                    if (parser::isa_node<parser::BlockPass>(ret->exprs[0].get())) {
                        if (auto e = dctx.ctx.beginError(ret->loc, core::errors::Desugar::UnsupportedNode)) {
                            e.setHeader("Block argument should not be given");
                        }
                        ExpressionPtr res = MK::Break(loc, MK::EmptyTree());
                        result = std::move(res);
                    } else {
                        ExpressionPtr res = MK::Next(loc, node2TreeImpl(dctx, std::move(ret->exprs[0])));
                        result = std::move(res);
                    }
                } else {
                    ExpressionPtr res = MK::Next(loc, MK::EmptyTree());
                    result = std::move(res);
                }
            },
            [&](parser::Retry *ret) {
                ExpressionPtr res = make_expression<Retry>(loc);
                result = std::move(res);
            },
            [&](parser::Yield *ret) {
                Send::ARGS_store args;
                args.reserve(ret->exprs.size());
                for (auto &stat : ret->exprs) {
                    args.emplace_back(node2TreeImpl(dctx, std::move(stat)));
                };

                ExpressionPtr recv;
                if (dctx.enclosingBlockArg.exists()) {
                    // we always want to report an error if we're using yield with a synthesized name in
                    // strict mode
                    auto blockArgName = dctx.enclosingBlockArg;
                    if (blockArgName == core::Names::blkArg()) {
                        if (auto e = dctx.ctx.beginError(dctx.enclosingMethodLoc,
                                                         core::errors::Desugar::UnnamedBlockParameter)) {
                            e.setHeader("Method `{}` uses `{}` but does not mention a block parameter",
                                        dctx.enclosingMethodName.show(dctx.ctx), "yield");
                            e.addErrorLine(dctx.ctx.locAt(loc), "Arising from use of `{}` in method body", "yield");
                        }
                    }

                    recv = MK::Local(loc, dctx.enclosingBlockArg);
                } else {
                    // No enclosing block arg can happen when e.g. yield is called in a class / at the top-level.
                    recv = MK::RaiseUnimplemented(loc);
                }
                ExpressionPtr res =
                    MK::Send(loc, std::move(recv), core::Names::call(), locZeroLen, args.size(), std::move(args));
                result = std::move(res);
            },
            [&](parser::Rescue *rescue) {
                Rescue::RESCUE_CASE_store cases;
                cases.reserve(rescue->rescue.size());
                for (auto &node : rescue->rescue) {
                    cases.emplace_back(node2TreeImpl(dctx, std::move(node)));
                    ENFORCE(isa_tree<RescueCase>(cases.back()), "node2TreeImpl failed to produce a rescue case");
                }
                ExpressionPtr res =
                    make_expression<Rescue>(loc, node2TreeImpl(dctx, std::move(rescue->body)), std::move(cases),
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

                auto varLoc = varExpr.loc();
                auto var = core::NameRef::noName();
                if (auto *id = cast_tree<UnresolvedIdent>(varExpr)) {
                    if (id->kind == UnresolvedIdent::Kind::Local) {
                        var = id->name;
                        varExpr.reset(nullptr);
                    }
                }

                if (!var.exists()) {
                    var = dctx.freshNameUnique(core::Names::rescueTemp());
                }

                if (isa_tree<EmptyTree>(varExpr)) {
                    varLoc = loc;
                } else if (varExpr != nullptr) {
                    body = MK::InsSeq1(varLoc, MK::Assign(varLoc, std::move(varExpr), MK::Local(varLoc, var)),
                                       std::move(body));
                }

                ExpressionPtr res =
                    make_expression<RescueCase>(loc, std::move(exceptions), MK::Local(varLoc, var), std::move(body));
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
                    ExpressionPtr res = make_expression<Rescue>(loc, std::move(bodyExpr), std::move(cases),
                                                                MK::EmptyTree(), std::move(ensureExpr));
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
                ExpressionPtr assign;
                auto temp = core::NameRef::noName();
                core::LocOffsets cloc;

                if (case_->condition != nullptr) {
                    cloc = case_->condition->loc;
                    temp = dctx.freshNameUnique(core::Names::assignTemp());
                    assign = MK::Assign(cloc, temp, node2TreeImpl(dctx, std::move(case_->condition)));
                }
                ExpressionPtr res = node2TreeImpl(dctx, std::move(case_->else_));
                for (auto it = case_->whens.rbegin(); it != case_->whens.rend(); ++it) {
                    auto when = parser::cast_node<parser::When>(it->get());
                    ENFORCE(when != nullptr, "case without a when?");
                    ExpressionPtr cond;
                    for (auto &cnode : when->patterns) {
                        ExpressionPtr test;
                        if (parser::isa_node<parser::Splat>(cnode.get())) {
                            ENFORCE(temp.exists(), "splats need something to test against");
                            auto recv = MK::Magic(loc);
                            auto local = MK::Local(cloc, temp);
                            // TODO(froydnj): use the splat's var directly so we can elide the
                            // coercion to an array where possible.
                            auto splat = node2TreeImpl(dctx, std::move(cnode));
                            auto patternloc = splat.loc();
                            test = MK::Send2(patternloc, std::move(recv), core::Names::checkMatchArray(),
                                             patternloc.copyWithZeroLength(), std::move(local), std::move(splat));
                        } else {
                            auto ctree = node2TreeImpl(dctx, std::move(cnode));
                            if (temp.exists()) {
                                auto local = MK::Local(cloc, temp);
                                auto patternloc = ctree.loc();
                                test = MK::Send1(patternloc, std::move(ctree), core::Names::tripleEq(),
                                                 patternloc.copyWithZeroLength(), std::move(local));
                            } else {
                                test = std::move(ctree);
                            }
                        }
                        if (cond == nullptr) {
                            cond = std::move(test);
                        } else {
                            auto true_ = MK::True(test.loc());
                            auto loc = test.loc();
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
                    MK::Send2(loc, MK::Self(loc), core::Names::aliasMethod(), locZeroLen,
                              node2TreeImpl(dctx, std::move(alias->from)), node2TreeImpl(dctx, std::move(alias->to)));
                result = std::move(res);
            },
            [&](parser::Defined *defined) {
                auto value = node2TreeImpl(dctx, std::move(defined->value));
                auto loc = value.loc();
                auto *ident = cast_tree<UnresolvedIdent>(value);
                if (ident &&
                    (ident->kind == UnresolvedIdent::Kind::Instance || ident->kind == UnresolvedIdent::Kind::Class)) {
                    auto methodName = ident->kind == UnresolvedIdent::Kind::Instance ? core::Names::definedInstanceVar()
                                                                                     : core::Names::definedClassVar();
                    auto sym = MK::Symbol(loc, ident->name);
                    auto res = MK::Send1(loc, MK::Magic(loc), methodName, locZeroLen, std::move(sym));
                    result = std::move(res);
                    return;
                }

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

                auto numPosArgs = args.size();
                auto res =
                    MK::Send(loc, MK::Magic(loc), core::Names::defined_p(), locZeroLen, numPosArgs, std::move(args));
                result = std::move(res);
            },
            [&](parser::LineLiteral *line) {
                auto pos = dctx.ctx.locAt(loc).position(dctx.ctx);
                ENFORCE(pos.first.line == pos.second.line, "position corrupted");
                auto res = MK::Int(loc, pos.first.line);
                result = std::move(res);
            },
            [&](parser::XString *xstring) {
                auto res = MK::Send1(loc, MK::Self(loc), core::Names::backtick(), locZeroLen,
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
                auto numPosArgs = args.size();
                auto res = MK::Send(loc, MK::Constant(loc, core::Symbols::Kernel()), core::Names::undef(), locZeroLen,
                                    numPosArgs, std::move(args));
                // It wasn't a Send to begin with--there's no way this could result in a private
                // method call error.
                ast::cast_tree_nonnull<ast::Send>(res).flags.isPrivateOk = true;
                result = std::move(res);
            },
            [&](parser::CaseMatch *caseMatch) {
                // Create a local var to store the expression used in each match clause
                auto exprLoc = caseMatch->expr->loc;
                auto exprName = dctx.freshNameUnique(core::Names::assignTemp());
                auto exprVar = MK::Assign(exprLoc, exprName, node2TreeImpl(dctx, std::move(caseMatch->expr)));

                // Desugar the `else` block
                ExpressionPtr res = node2TreeImpl(dctx, std::move(caseMatch->elseBody));

                // Desugar each `in` as an `if` branch calling `Magic.<pattern-match>()`
                for (auto it = caseMatch->inBodies.rbegin(); it != caseMatch->inBodies.rend(); ++it) {
                    auto inPattern = parser::cast_node<parser::InPattern>(it->get());
                    ENFORCE(inPattern != nullptr, "case pattern without a in?");

                    // Keep the `in` body for the `then` body of the new `if`
                    auto pattern = std::move(inPattern->pattern);
                    auto body = node2TreeImpl(dctx, std::move(inPattern->body));

                    // Desugar match variables found inside the pattern
                    InsSeq::STATS_store vars;
                    desugarPatternMatchingVars(vars, dctx, pattern);
                    if (!vars.empty()) {
                        body = MK::InsSeq(pattern->loc, std::move(vars), std::move(body));
                    }

                    // Create a new `if` for the branch:
                    // `in A` => `if (TODO)`
                    auto match = MK::RaiseUnimplemented(pattern->loc);
                    res = MK::If(inPattern->loc, std::move(match), std::move(body), std::move(res));
                }
                res = MK::InsSeq1(loc, std::move(exprVar), std::move(res));
                result = std::move(res);
            },
            [&](parser::Backref *backref) {
                auto recv = MK::Magic(loc);
                auto arg = MK::Symbol(backref->loc, backref->name);
                result = MK::Send1(loc, std::move(recv), core::Names::regexBackref(), locZeroLen, std::move(arg));
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
                auto recv = MK::Magic(loc);
                result = MK::Send0(loc, std::move(recv), core::Names::getEncoding(), locZeroLen);
            },
            [&](parser::MatchPattern *pattern) {
                auto res = desugarOnelinePattern(dctx, pattern->loc, pattern->rhs);
                result = std::move(res);
            },
            [&](parser::MatchPatternP *pattern) {
                auto res = desugarOnelinePattern(dctx, pattern->loc, pattern->rhs);
                result = std::move(res);
            },
            [&](parser::EmptyElse *else_) { result = MK::EmptyTree(); },

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

ExpressionPtr liftTopLevel(DesugarContext dctx, core::LocOffsets loc, ExpressionPtr what) {
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
    return make_expression<ClassDef>(loc, loc, core::Symbols::root(), MK::EmptyTree(), std::move(ancestors),
                                     std::move(rhs), ClassDef::Kind::Class);
}
} // namespace

ExpressionPtr node2Tree(core::MutableContext ctx, unique_ptr<parser::Node> what) {
    try {
        uint32_t uniqueCounter = 1;
        // We don't have an enclosing block arg to start off.
        DesugarContext dctx(ctx, uniqueCounter, core::NameRef::noName(), core::LocOffsets::none(),
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
