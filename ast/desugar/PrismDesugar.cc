#include <algorithm>

#include "absl/strings/match.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_replace.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "ast/desugar/DuplicateHashKeyCheck.h"
#include "ast/desugar/PrismDesugar.h"
#include "ast/verifier/verifier.h"
#include "common/common.h"
#include "common/strings/formatting.h"
#include "core/Names.h"
#include "core/errors/desugar.h"
#include "core/errors/internal.h"

// The Prism Desugarer starts out as a near-exact copy of the legacy Desugarer. Over time, we migrate more and more of
// its logic out and into `parser/prism/Translator.cc`. Any changes made to the "upstream" Desugarer also need to be
// refected here, and in the Translator.
//
// One key difference of the Prism desugarer is that it calls `NodeWithExpr::cast_node` and `NodeWithExpr::isa_node`
// instead of the usual `parser::cast_node` and `parser::isa_node` functions. This adds extra overhead in the Prism
// case, but remains zero-cost in the case of the legacy parser and the regular `Desugar.cc`.
namespace sorbet::ast::prismDesugar {

using namespace std;
using sorbet::ast::desugar::DuplicateHashKeyCheck;

namespace {

struct DesugarContext final {
    core::MutableContext ctx;
    uint32_t &uniqueCounter;
    core::NameRef enclosingBlockParamName;
    core::LocOffsets enclosingMethodLoc;
    core::NameRef enclosingMethodName;
    bool inAnyBlock;
    bool inModule;
    bool preserveConcreteSyntax;

    DesugarContext(core::MutableContext ctx, uint32_t &uniqueCounter, core::NameRef enclosingBlockParamName,
                   core::LocOffsets enclosingMethodLoc, core::NameRef enclosingMethodName, bool inAnyBlock,
                   bool inModule, bool preserveConcreteSyntax)
        : ctx(ctx), uniqueCounter(uniqueCounter), enclosingBlockParamName(enclosingBlockParamName),
          enclosingMethodLoc(enclosingMethodLoc), enclosingMethodName(enclosingMethodName), inAnyBlock(inAnyBlock),
          inModule(inModule), preserveConcreteSyntax(preserveConcreteSyntax){};

    core::NameRef freshNameUnique(core::NameRef name) {
        return ctx.state.freshNameUnique(core::UniqueNameKind::Desugar, name, ++uniqueCounter);
    }
};

core::NameRef blockParam2Name(DesugarContext dctx, const BlockParam &blockParam) {
    auto blkIdent = cast_tree<UnresolvedIdent>(blockParam.expr);
    ENFORCE(blkIdent != nullptr, "BlockParam must wrap UnresolvedIdent in desugar.");
    return blkIdent->name;
}

// Get the num from the name of the Node if it's a LVar.
// Return -1 otherwise.
int numparamNum(DesugarContext dctx, parser::Node *decl) {
    if (auto *lvar = parser::NodeWithExpr::cast_node<parser::LVar>(decl)) {
        auto name_str = lvar->name.show(dctx.ctx);
        return name_str[1] - '0';
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
        if (parser::NodeWithExpr::isa_node<parser::LVar>(decl.get())) {
            if (numparamNum(dctx, decl.get()) == num) {
                ENFORCE(decl != nullptr && decl->hasDesugaredExpr());
                return decl->takeDesugaredExpr();
            }
        } else {
            ENFORCE(false, "NumParams declaring node is not a LVar.");
        }
    }
    core::NameRef name = dctx.ctx.state.enterNameUTF8("_" + to_string(num));
    return MK::Local(core::LocOffsets::none(), name);
}

ExpressionPtr node2TreeImpl(DesugarContext dctx, unique_ptr<parser::Node> &what);

pair<MethodDef::PARAMS_store, InsSeq::STATS_store> desugarParams(DesugarContext dctx, parser::Node *anyParamsNode) {
    MethodDef::PARAMS_store params;
    InsSeq::STATS_store destructures;

    if (auto *paramsNode = parser::NodeWithExpr::cast_node<parser::Params>(anyParamsNode)) {
        params.reserve(paramsNode->params.size());
        for (auto &arg : paramsNode->params) {
            if (parser::NodeWithExpr::isa_node<parser::Mlhs>(arg.get())) {
                core::NameRef temporary = dctx.freshNameUnique(core::Names::destructureArg());
                params.emplace_back(MK::Local(arg->loc, temporary));
                unique_ptr<parser::Node> lvarNode = make_unique<parser::LVar>(arg->loc, temporary);
                unique_ptr<parser::Node> destructure = make_unique<parser::Masgn>(arg->loc, move(arg), move(lvarNode));
                destructures.emplace_back(node2TreeImpl(dctx, destructure));
            } else if (parser::NodeWithExpr::isa_node<parser::Kwnilarg>(arg.get())) {
                // TODO implement logic for `**nil` args
            } else if (auto *fargs = parser::NodeWithExpr::cast_node<parser::ForwardArg>(arg.get())) {
                // we desugar (m, n, ...) into (m, n, *<fwd-args>, **<fwd-kwargs>, &<fwd-block>)

                // add `*<fwd-args>`
                params.emplace_back(MK::RestParam(fargs->loc, MK::Local(fargs->loc, core::Names::fwdArgs())));

                // add `**<fwd-kwargs>`
                params.emplace_back(MK::RestParam(fargs->loc, MK::KeywordArg(fargs->loc, core::Names::fwdKwargs())));

                // add `&<fwd-block>`
                params.emplace_back(MK::BlockParam(fargs->loc, MK::Local(fargs->loc, core::Names::fwdBlock())));
            } else {
                params.emplace_back(node2TreeImpl(dctx, arg));
            }
        }
    } else if (auto *numParamsNode = parser::NodeWithExpr::cast_node<parser::NumParams>(anyParamsNode)) {
        // The parse tree only contains declarations for numbered parameters that were actually used in the block or
        // lambda body, listed in the order they were encountered in the body. The desugar tree always contains all
        // params (`_1, _2, ..., _9`) in numbered order.
        auto max = numparamMax(dctx, &numParamsNode->decls);

        // The block uses numbered parameters like `_1` or `_9` so we add them as parameters
        // from _1 to the highest number used.
        for (int i = 1; i <= max; i++) {
            params.emplace_back(numparamTree(dctx, i, &numParamsNode->decls));
        }
    } else if (anyParamsNode == nullptr) {
        // do nothing
    } else {
        Exception::raise("not implemented: {}", anyParamsNode->nodeName());
    }

    return make_pair(move(params), move(destructures));
}

ExpressionPtr desugarBody(DesugarContext dctx, core::LocOffsets loc, unique_ptr<parser::Node> &bodynode,
                          InsSeq::STATS_store destructures) {
    auto body = node2TreeImpl(dctx, bodynode);
    if (!destructures.empty()) {
        auto bodyLoc = body.loc();
        if (!bodyLoc.exists()) {
            bodyLoc = loc;
        }
        body = MK::InsSeq(loc, move(destructures), move(body));
    }

    return body;
}

// It's not possible to use an anonymous rest parameter in a block, as it always refers to the forwarded arguments
// from the method. This function raises an error if the anonymous rest arg is present in a parameter list.
void checkBlockRestParam(DesugarContext dctx, const MethodDef::PARAMS_store &args) {
    auto it = absl::c_find_if(args, [](const auto &arg) { return isa_tree<RestParam>(arg); });
    if (it == args.end()) {
        return;
    }

    auto &rest = cast_tree_nonnull<RestParam>(*it);
    if (auto local = cast_tree<UnresolvedIdent>(rest.expr)) {
        if (local->name != core::Names::star()) {
            return;
        }

        if (auto e = dctx.ctx.beginIndexerError(it->loc(), core::errors::Desugar::BlockAnonymousRestParam)) {
            e.setHeader("Anonymous rest parameter in block args");
            e.addErrorNote("Naming the rest parameter will ensure you can access it in the block");
            e.replaceWith("Name the rest parameter", dctx.ctx.locAt(it->loc().copyEndWithZeroLength()), "_");
        }
    }
}

ExpressionPtr desugarBlock(DesugarContext dctx, parser::Block *block) {
    block->send->loc = block->send->loc.join(block->loc);
    auto recv = node2TreeImpl(dctx, block->send);
    Send *send;
    ExpressionPtr res;
    if ((send = cast_tree<Send>(recv)) != nullptr) {
        res = move(recv);
    } else {
        // This must have been a csend; That will have been desugared
        // into an insseq with an If in the expression.
        res = move(recv);
        auto is = cast_tree<InsSeq>(res);
        if (!is) {
            if (auto e = dctx.ctx.beginIndexerError(block->loc, core::errors::Desugar::UnsupportedNode)) {
                e.setHeader("No body in block");
            }
            return MK::EmptyTree();
        }
        auto iff = cast_tree<If>(is->expr);
        ENFORCE(iff != nullptr, "DesugarBlock: failed to find If");
        send = cast_tree<Send>(iff->elsep);
        ENFORCE(send != nullptr, "DesugarBlock: failed to find Send");
    }
    auto [params, destructures] = desugarParams(dctx, block->params.get());

    checkBlockRestParam(dctx, params);

    auto inBlock = true;
    DesugarContext dctx1(dctx.ctx, dctx.uniqueCounter, dctx.enclosingBlockParamName, dctx.enclosingMethodLoc,
                         dctx.enclosingMethodName, inBlock, dctx.inModule, dctx.preserveConcreteSyntax);
    auto desugaredBody = desugarBody(dctx1, block->loc, block->body, move(destructures));

    // TODO the send->block's loc is too big and includes the whole send
    send->setBlock(MK::Block(block->loc, move(desugaredBody), move(params)));
    return res;
}

ExpressionPtr desugarBegin(DesugarContext dctx, core::LocOffsets loc, parser::NodeVec &stmts) {
    if (stmts.empty()) {
        return MK::Nil(loc);
    } else {
        InsSeq::STATS_store stats;
        stats.reserve(stmts.size() - 1);
        auto end = stmts.end();
        --end;
        for (auto it = stmts.begin(); it != end; ++it) {
            auto &stat = *it;
            stats.emplace_back(node2TreeImpl(dctx, stat));
        };
        auto &last = stmts.back();

        auto expr = node2TreeImpl(dctx, last);
        return MK::InsSeq(loc, move(stats), move(expr));
    }
}

core::NameRef maybeTypedSuper(DesugarContext dctx) {
    return (dctx.ctx.state.cacheSensitiveOptions.typedSuper && !dctx.inAnyBlock && !dctx.inModule)
               ? core::Names::super()
               : core::Names::untypedSuper();
}

bool isStringLit(DesugarContext dctx, ExpressionPtr &expr) {
    if (auto lit = cast_tree<Literal>(expr)) {
        return lit->isString();
    }

    return false;
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
    ExpressionPtr first = node2TreeImpl(dctx, *it);
    InlinedVector<ExpressionPtr, 4> stringsAccumulated;

    Send::ARGS_store interpArgs;

    bool allStringsSoFar;
    if (isStringLit(dctx, first) || isa_tree<EmptyTree>(first)) {
        stringsAccumulated.emplace_back(move(first));
        allStringsSoFar = true;
    } else {
        interpArgs.emplace_back(move(first));
        allStringsSoFar = false;
    }
    ++it;

    for (; it != end; ++it) {
        auto &stat = *it;
        ExpressionPtr narg = node2TreeImpl(dctx, stat);
        if (allStringsSoFar && isStringLit(dctx, narg)) {
            stringsAccumulated.emplace_back(move(narg));
        } else if (isa_tree<EmptyTree>(narg)) {
            // no op
        } else {
            if (allStringsSoFar) {
                allStringsSoFar = false;
                interpArgs.emplace_back(mergeStrings(dctx, loc, move(stringsAccumulated)));
            }
            interpArgs.emplace_back(move(narg));
        }
    };
    if (allStringsSoFar) {
        return mergeStrings(dctx, loc, move(stringsAccumulated));
    } else {
        auto recv = MK::Magic(loc);
        return MK::Send(loc, move(recv), core::Names::stringInterpolate(), loc.copyWithZeroLength(), interpArgs.size(),
                        move(interpArgs));
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
            if (auto e = dctx.ctx.beginIndexerError(body.loc(), core::errors::Desugar::CodeInRBI)) {
                e.setHeader("RBI methods must not have code");
                e.replaceWith("Delete the body", loc, "");
            }
        }
    } else if (auto inseq = cast_tree<InsSeq>(body)) {
        for (auto &stat : inseq->stats) {
            if (!isIVarAssign(stat)) {
                if (auto e = dctx.ctx.beginIndexerError(stat.loc(), core::errors::Desugar::CodeInRBI)) {
                    e.setHeader("RBI methods must not have code");
                    e.replaceWith("Delete the body", loc, "");
                }
            }
        }
        if (!isIVarAssign(inseq->expr)) {
            if (auto e = dctx.ctx.beginIndexerError(inseq->expr.loc(), core::errors::Desugar::CodeInRBI)) {
                e.setHeader("RBI methods must not have code");
                e.replaceWith("Delete the body", loc, "");
            }
        }
    } else {
        if (auto e = dctx.ctx.beginIndexerError(body.loc(), core::errors::Desugar::CodeInRBI)) {
            e.setHeader("RBI methods must not have code");
            e.replaceWith("Delete the body", loc, "");
        }
    }
    return body;
}

ExpressionPtr buildMethod(DesugarContext dctx, core::LocOffsets loc, core::LocOffsets declLoc, core::NameRef name,
                          parser::Node *argnode, unique_ptr<parser::Node> &body, bool isSelf) {
    // Reset uniqueCounter within this scope (to keep numbers small)
    uint32_t uniqueCounter = 1;
    auto inModule = dctx.inModule && !isSelf;
    DesugarContext dctx1(dctx.ctx, uniqueCounter, dctx.enclosingBlockParamName, declLoc, name, dctx.inAnyBlock,
                         inModule, dctx.preserveConcreteSyntax);
    auto [params, destructures] = desugarParams(dctx1, argnode);

    if (params.empty() || !isa_tree<BlockParam>(params.back())) {
        auto blkLoc = core::LocOffsets::none();
        params.emplace_back(MK::BlockParam(blkLoc, MK::Local(blkLoc, core::Names::blkArg())));
    }

    const auto &blockParam = cast_tree<BlockParam>(params.back());
    ENFORCE(blockParam != nullptr, "Every method's last param must be a block param by now.");
    auto enclosingBlockParamName = blockParam2Name(dctx, *blockParam);

    DesugarContext dctx2(dctx1.ctx, dctx1.uniqueCounter, enclosingBlockParamName, declLoc, name, dctx.inAnyBlock,
                         inModule, dctx.preserveConcreteSyntax);
    ExpressionPtr desugaredBody = desugarBody(dctx2, loc, body, move(destructures));
    desugaredBody = validateRBIBody(dctx2, move(desugaredBody));

    auto mdef = MK::Method(loc, declLoc, name, move(params), move(desugaredBody));
    cast_tree<MethodDef>(mdef)->flags.isSelfMethod = isSelf;
    return mdef;
}

// Desugar a Symbol block pass like the `&foo` in `m(&:foo)` into a block literal.
// `&:foo` => `{ |*temp| (temp[0]).foo(*temp[1, LONG_MAX]) }`
ExpressionPtr symbol2Proc(DesugarContext dctx, ExpressionPtr expr) {
    auto loc = expr.loc();
    core::NameRef temp = dctx.freshNameUnique(core::Names::blockPassTemp());
    auto lit = cast_tree<Literal>(expr);
    ENFORCE(lit && lit->isSymbol());

    core::NameRef name = lit->asName();
    // `temp` does not refer to any specific source text, so give it a 0-length Loc so LSP ignores it.
    auto zeroLengthLoc = loc.copyWithZeroLength();
    auto recv = MK::Send1(zeroLengthLoc, MK::Local(zeroLengthLoc, temp), core::Names::squareBrackets(), zeroLengthLoc,
                          MK::Int(zeroLengthLoc, 0));
    auto sliced = MK::Send2(zeroLengthLoc, MK::Local(zeroLengthLoc, temp), core::Names::squareBrackets(), zeroLengthLoc,
                            MK::Int(zeroLengthLoc, 1), MK::Int(zeroLengthLoc, LONG_MAX));
    auto body = MK::CallWithSplat(loc, move(recv), name, zeroLengthLoc, MK::Splat(zeroLengthLoc, move(sliced)));
    return MK::Block1(loc, move(body), MK::RestParam(zeroLengthLoc, MK::Local(zeroLengthLoc, temp)));
}

// Desugar multiple left hand side assignments into a sequence of assignments
//
// Considering this example:
// ```rb
// arr = [1, 2, 3]
// a, *b = arr
// ```
//
// We desugar the assignment `a, *b = arr` into:
// ```rb
// tmp = ::<Magic>.expandSplat(arr, 1, 0)
// a = tmp[0]
// b = tmp.to_ary
// ```
//
// While calling `to_ary` doesn't return the correct value if we were to execute this code,
// it returns the correct type from a static point of view.
ExpressionPtr desugarMlhs(DesugarContext dctx, core::LocOffsets loc, parser::Mlhs *lhs, ExpressionPtr rhs) {
    InsSeq::STATS_store stats;

    core::NameRef tempRhs = dctx.freshNameUnique(core::Names::assignTemp());
    core::NameRef tempExpanded = dctx.freshNameUnique(core::Names::assignTemp());

    int i = 0;
    int before = 0, after = 0;
    bool didSplat = false;
    auto zloc = loc.copyWithZeroLength();

    for (auto &c : lhs->exprs) {
        if (auto *splat = parser::NodeWithExpr::cast_node<parser::SplatLhs>(c.get())) {
            ENFORCE(!didSplat, "did splat already");
            didSplat = true;

            ExpressionPtr lh = node2TreeImpl(dctx, splat->var);

            int left = i;
            int right = lhs->exprs.size() - left - 1;
            if (!isa_tree<EmptyTree>(lh)) {
                if (right == 0) {
                    right = 1;
                }
                auto lhloc = lh.loc();
                auto zlhloc = lhloc.copyWithZeroLength();
                // Calling `to_ary` is not faithful to the runtime behavior,
                // but that it is faithful to the expected static type-checking behavior.
                auto ary = MK::Send0(zloc, MK::Local(zloc, tempExpanded), core::Names::toAry(), zlhloc);
                stats.emplace_back(MK::Assign(lhloc, move(lh), move(ary)));
            }
            i = -right;
        } else {
            if (didSplat) {
                ++after;
            } else {
                ++before;
            }
            auto zcloc = c->loc.copyWithZeroLength();
            auto val =
                MK::Send1(zcloc, MK::Local(zcloc, tempExpanded), core::Names::squareBrackets(), zloc, MK::Int(zloc, i));

            if (auto *mlhs = parser::NodeWithExpr::cast_node<parser::Mlhs>(c.get())) {
                stats.emplace_back(desugarMlhs(dctx, mlhs->loc, mlhs, move(val)));
            } else {
                ExpressionPtr lh = node2TreeImpl(dctx, c);
                if (auto restParam = cast_tree<RestParam>(lh)) {
                    if (auto e = dctx.ctx.beginIndexerError(lh.loc(),
                                                            core::errors::Desugar::UnsupportedRestArgsDestructure)) {
                        e.setHeader("Unsupported rest args in destructure");
                    }
                    lh = move(restParam->expr);
                }
                auto lhloc = lh.loc();
                stats.emplace_back(MK::Assign(lhloc, move(lh), move(val)));
            }

            i++;
        }
    }

    auto expanded = MK::Send3(loc, MK::Magic(loc), core::Names::expandSplat(), loc.copyWithZeroLength(),
                              MK::Local(loc, tempRhs), MK::Int(loc, before), MK::Int(loc, after));
    stats.insert(stats.begin(), MK::Assign(loc, tempExpanded, move(expanded)));
    stats.insert(stats.begin(), MK::Assign(loc, tempRhs, move(rhs)));

    // Regardless of how we destructure an assignment, Ruby evaluates the expression to the entire right hand side,
    // not any individual component of the destructured assignment.
    return MK::InsSeq(loc, move(stats), MK::Local(loc, tempRhs));
}

// Map all MatchVars used in `pattern` to local variables initialized from magic calls
void desugarPatternMatchingVars(InsSeq::STATS_store &vars, DesugarContext dctx, parser::Node *node) {
    if (auto var = parser::NodeWithExpr::cast_node<parser::MatchVar>(node)) {
        auto loc = var->loc;
        auto val = MK::RaiseUnimplemented(loc);
        vars.emplace_back(MK::Assign(loc, var->name, move(val)));
    } else if (auto rest = parser::NodeWithExpr::cast_node<parser::MatchRest>(node)) {
        desugarPatternMatchingVars(vars, dctx, rest->var.get());
    } else if (auto pair = parser::NodeWithExpr::cast_node<parser::Pair>(node)) {
        desugarPatternMatchingVars(vars, dctx, pair->value.get());
    } else if (auto as_pattern = parser::NodeWithExpr::cast_node<parser::MatchAs>(node)) {
        auto loc = as_pattern->as->loc;
        auto name = parser::NodeWithExpr::cast_node<parser::MatchVar>(as_pattern->as.get())->name;
        auto val = MK::RaiseUnimplemented(loc);
        vars.emplace_back(MK::Assign(loc, name, move(val)));
        desugarPatternMatchingVars(vars, dctx, as_pattern->value.get());
    } else if (auto array_pattern = parser::NodeWithExpr::cast_node<parser::ArrayPattern>(node)) {
        for (auto &elt : array_pattern->elts) {
            desugarPatternMatchingVars(vars, dctx, elt.get());
        }
    } else if (auto array_pattern = parser::NodeWithExpr::cast_node<parser::ArrayPatternWithTail>(node)) {
        for (auto &elt : array_pattern->elts) {
            desugarPatternMatchingVars(vars, dctx, elt.get());
        }
    } else if (auto hash_pattern = parser::NodeWithExpr::cast_node<parser::HashPattern>(node)) {
        for (auto &elt : hash_pattern->pairs) {
            desugarPatternMatchingVars(vars, dctx, elt.get());
        }
    } else if (auto alt_pattern = parser::NodeWithExpr::cast_node<parser::MatchAlt>(node)) {
        desugarPatternMatchingVars(vars, dctx, alt_pattern->left.get());
        desugarPatternMatchingVars(vars, dctx, alt_pattern->right.get());
    }
}

bool locReported = false;

ClassDef::RHS_store scopeNodeToBody(DesugarContext dctx, unique_ptr<parser::Node> node) {
    ClassDef::RHS_store body;
    // Reset uniqueCounter within this scope (to keep numbers small)
    uint32_t uniqueCounter = 1;
    // Blocks never persist across a class/module boundary
    auto inAnyBlock = false;
    DesugarContext dctx1(dctx.ctx, uniqueCounter, dctx.enclosingBlockParamName, dctx.enclosingMethodLoc,
                         dctx.enclosingMethodName, inAnyBlock, dctx.inModule, dctx.preserveConcreteSyntax);
    if (auto *begin = parser::NodeWithExpr::cast_node<parser::Begin>(node.get())) {
        body.reserve(begin->stmts.size());
        for (auto &stat : begin->stmts) {
            body.emplace_back(node2TreeImpl(dctx1, stat));
        };
    } else {
        body.emplace_back(node2TreeImpl(dctx1, node));
    }
    return body;
}

Send *asTLet(ExpressionPtr &arg) {
    auto send = cast_tree<Send>(arg);
    if (send == nullptr || send->fun != core::Names::let() || send->numPosArgs() < 2) {
        return nullptr;
    }

    if (!ast::MK::isT(send->recv)) {
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
    stats.emplace_back(MK::Assign(s->loc, tempRecv, move(s->recv)));
    Send::ARGS_store readArgs;
    Send::ARGS_store assgnArgs;
    // these are the arguments for the first send, e.g. x.y(). The number of arguments should be identical to whatever
    // we saw on the LHS.
    readArgs.reserve(numPosArgs);
    // these are the arguments for the second send, e.g. x.y=(val). That's why we need the space for the extra argument
    // here: to accommodate the call to field= instead of just field.
    assgnArgs.reserve(numPosArgs + 1);

    for (auto &arg : s->posArgs()) {
        auto argLoc = arg.loc();
        core::NameRef name = dctx.freshNameUnique(s->fun);
        stats.emplace_back(MK::Assign(argLoc, name, move(arg)));
        readArgs.emplace_back(MK::Local(argLoc, name));
        assgnArgs.emplace_back(MK::Local(argLoc, name));
    }

    return {tempRecv, move(stats), numPosArgs, move(readArgs), move(assgnArgs)};
}

// while true
//   body
//   if cond
//     break
//   end
// end
ExpressionPtr doUntil(DesugarContext dctx, core::LocOffsets loc, ExpressionPtr cond, ExpressionPtr body) {
    auto breaker = MK::If(loc, move(cond), MK::Break(loc, MK::EmptyTree()), MK::EmptyTree());
    auto breakWithBody = MK::InsSeq1(loc, move(body), move(breaker));
    return MK::While(loc, MK::True(loc), move(breakWithBody));
}

// Flattens the key/value pairs from the Kwargs Hash into the destination container.
// If Kwargs Hash contains any splats, we skip the flattening and append the hash as-is.
template <typename Container> void flattenKwargs(unique_ptr<parser::Hash> kwargsHash, Container &destination) {
    ENFORCE(kwargsHash != nullptr);

    // Skip inlining the kwargs if there are any kwsplat nodes present
    if (absl::c_any_of(kwargsHash->pairs, [](auto &node) {
            // the parser guarantees that if we see a kwargs hash it only contains pair,
            // kwsplat, or forwarded kwrest arg nodes
            ENFORCE(parser::NodeWithExpr::isa_node<parser::Kwsplat>(node.get()) ||
                    parser::NodeWithExpr::isa_node<parser::Pair>(node.get()) ||
                    parser::NodeWithExpr::isa_node<parser::ForwardedKwrestArg>(node.get()));

            return parser::NodeWithExpr::isa_node<parser::Kwsplat>(node.get()) ||
                   parser::NodeWithExpr::isa_node<parser::ForwardedKwrestArg>(node.get());
        })) {
        destination.emplace_back(move(kwargsHash));
        return;
    }

    // Flatten the key/value pairs into the destination
    for (auto &entry : kwargsHash->pairs) {
        if (auto pair = parser::NodeWithExpr::cast_node<parser::Pair>(entry.get())) {
            destination.emplace_back(move(pair->key));
            destination.emplace_back(move(pair->value));
        } else {
            Exception::raise("Unhandled case");
        }
    }

    return;
}

// Detects calls to `block_given?`
bool isCallToBlockGivenP(parser::Send *sendNode) {
    return sendNode->method == core::Names::blockGiven_p();
};

[[noreturn]] void desugaredByPrismTranslator(parser::Node *node) {
    Exception::raise("The {} node should have already been desugared by the Prism Translator.", node->nodeName());
}

// Translate a tree to an expression. NOTE: this should only be called from `node2TreeImpl`.
ExpressionPtr node2TreeImplBody(DesugarContext dctx, parser::Node *what) {
    try {
        if (what == nullptr) {
            return MK::EmptyTree();
        }
        auto loc = what->loc;
        auto locZeroLen = what->loc.copyWithZeroLength();
        ENFORCE(loc.exists(), "parse-tree node has no location: {}", what->toString(dctx.ctx));
        ExpressionPtr result;
        typecase(
            what,
            // The top N clauses here are ordered according to observed
            // frequency in pay-server. Do not reorder the top of this list, or
            // add entries here, without consulting the "node.*" counters from a
            // run over a representative code base.
            [&](parser::Const *const_) {
                auto scope = node2TreeImpl(dctx, const_->scope);
                ExpressionPtr res = MK::UnresolvedConstant(loc, move(scope), const_->name);
                result = move(res);
            },
            [&](parser::Send *send) {
                Send::Flags flags;
                auto rec = node2TreeImpl(dctx, send->receiver);
                if (isa_tree<EmptyTree>(rec)) {
                    // 0-sized Loc, since `self.` doesn't appear in the original file.
                    rec = MK::Self(loc.copyWithZeroLength());
                    flags.isPrivateOk = true;
                } else if (rec.isSelfReference()) {
                    // In Ruby 2.7 `self.foo()` is also allowed for private method calls,
                    // not only `foo()`. This pre-emptively allow the new syntax.
                    flags.isPrivateOk = true;
                }

                auto methodName = MK::Symbol(locZeroLen, send->method);

                // Pop the BlockPass off the end of the arguments, if there is one. (e.g. the `&:b` in `foo(&:b)`)
                // Note: this does *not* handle regular block arguments (e.g. `foo { }`),
                //       which are handled separately in the`parser::Block *` case.
                ExpressionPtr blockPassArg;
                // We'll using the loc of the `<Magic>` node to communicate the loc of the whole `&blk` BlockPass node
                // Meanwhile, the blockPassArg will use its own loc, which is the expression to the right of the `&`.
                core::LocOffsets blockPassLoc;
                if (!send->args.empty() && parser::NodeWithExpr::isa_node<parser::BlockPass>(send->args.back().get())) {
                    auto *bp = parser::NodeWithExpr::cast_node<parser::BlockPass>(send->args.back().get());
                    blockPassLoc = bp->loc;
                    if (bp->block == nullptr) {
                        // Replace an anonymous block pass like `f(&)` with a local variable reference, like `f(&&)`.
                        blockPassArg = MK::Local(bp->loc.copyEndWithZeroLength(), core::Names::ampersand());
                    } else {
                        blockPassArg = node2TreeImpl(dctx, bp->block);
                    }

                    send->args.pop_back();
                }

                // Pop the Kwargs Hash off the end of the arguments, if there is one.
                unique_ptr<parser::Hash> kwargsHash;
                if (!send->args.empty()) {
                    auto *hash = parser::NodeWithExpr::cast_node<parser::Hash>(send->args.back().get());

                    // Only pop if it's a Hash, and if it's a kwargs Hash (as opposed to a regular Hash literal)
                    if (hash != nullptr && hash->kwargs) {
                        kwargsHash = unique_ptr<parser::Hash>(static_cast<parser::Hash *>(send->args.back().release()));
                        send->args.pop_back();
                    }
                }

                // Remove "special" arguments from the list, and keep track of what we found along the way.
                auto hasFwdArgs = false;
                auto hasFwdRestArg = false;
                auto hasSplat = false;
                auto newEndIt = remove_if(send->args.begin(), send->args.end(), [&](auto &arg) {
                    bool eraseFromArgs = false;

                    if (parser::NodeWithExpr::isa_node<parser::ForwardedArgs>(arg.get())) {
                        // Pull out the ForwardedArgs (the `...` argument in a method call, like `foo(...)`)

                        ENFORCE(blockPassArg == nullptr, "The parser should have rejected `foo(&, ...)`");
                        // Desugar a call like `foo(...)` so it has a block argument like `foo(..., &<fwd-block>)`.
                        blockPassArg = MK::Local(loc, core::Names::fwdBlock());
                        blockPassLoc = loc.copyEndWithZeroLength();

                        hasFwdArgs = true;
                        eraseFromArgs = true;
                    } else if (parser::NodeWithExpr::isa_node<parser::ForwardedRestArg>(arg.get())) {
                        // Pull out the ForwardedRestArg (an anonymous splat like `f(*)`)
                        hasFwdRestArg = true;
                        eraseFromArgs = true;
                    } else if (parser::NodeWithExpr::isa_node<parser::Splat>(arg.get())) {
                        // Detect if there's a splat in the argument list
                        hasSplat = true;
                        eraseFromArgs = false;
                    } else {
                        eraseFromArgs = false;
                    }

                    return eraseFromArgs;
                });
                send->args.erase(newEndIt, send->args.end());

                if (hasFwdArgs || hasFwdRestArg || hasSplat) {
                    // If we have a splat anywhere in the argument list, desugar
                    // the argument list as a single Array node, and then
                    // synthesize a call to
                    //   Magic.callWithSplat(receiver, method, argArray[, &blk])
                    // The callWithSplat implementation (in C++) will unpack a
                    // tuple type and call into the normal call mechanism.

                    auto argsEmpty = send->args.empty();
                    unique_ptr<parser::Node> array = make_unique<parser::Array>(locZeroLen, move(send->args));
                    auto args = node2TreeImpl(dctx, array);

                    if (hasFwdArgs) {
                        auto fwdArgs = MK::Local(loc, core::Names::fwdArgs());
                        auto argsSplat = MK::Splat(loc, move(fwdArgs));
                        auto argsConcat =
                            argsEmpty ? move(argsSplat)
                                      : MK::Send1(loc, move(args), core::Names::concat(), locZeroLen, move(argsSplat));

                        auto fwdKwargs = MK::Local(loc, core::Names::fwdKwargs());
                        auto kwargsSplat =
                            MK::Send1(loc, MK::Magic(loc), core::Names::toHashDup(), locZeroLen, move(fwdKwargs));

                        Array::ENTRY_store kwargsEntries;
                        kwargsEntries.emplace_back(move(kwargsSplat));
                        auto kwargsArray = MK::Array(loc, move(kwargsEntries));

                        argsConcat =
                            MK::Send1(loc, move(argsConcat), core::Names::concat(), locZeroLen, move(kwargsArray));

                        args = move(argsConcat);
                    } else if (hasFwdRestArg) {
                        auto fwdArgs = MK::Local(loc, core::Names::fwdArgs());
                        auto argsSplat = MK::Send0(loc, move(fwdArgs), core::Names::toA(), locZeroLen);
                        auto tUnsafe = MK::Unsafe(loc, move(argsSplat));
                        auto argsConcat = MK::Send1(loc, move(args), core::Names::concat(), locZeroLen, move(tUnsafe));

                        args = move(argsConcat);
                    }

                    // Build up an array that represents the keyword args for the send.
                    // When there is a Kwsplat, treat all keyword arguments as a single argument.
                    // If the kwargs hash is not present, make a `nil` to put in the place of that argument.
                    // This will be used in the implementation of the intrinsic to tell the difference between keyword
                    // args, keyword args with kw splats, and no keyword args at all.
                    ExpressionPtr kwargs;
                    if (kwargsHash != nullptr) {
                        parser::NodeVec kwargElements;
                        flattenKwargs(move(kwargsHash), kwargElements);

                        unique_ptr<parser::Node> kwArray = make_unique<parser::Array>(loc, move(kwargElements));

                        kwargs = node2TreeImpl(dctx, kwArray);

                        DuplicateHashKeyCheck::checkSendArgs(dctx.ctx, 0, cast_tree<Array>(kwargs)->elems);
                    } else {
                        kwargs = MK::Nil(loc);
                    }

                    Send::ARGS_store sendargs;
                    sendargs.emplace_back(move(rec));
                    sendargs.emplace_back(move(methodName));
                    sendargs.emplace_back(move(args));
                    sendargs.emplace_back(move(kwargs));
                    ExpressionPtr res;
                    if (blockPassArg == nullptr) {
                        // Desugar any call with a splat and without a block pass argument.
                        // If there's a literal block argument, that's handled here, too.
                        // E.g. `foo(*splat)` or `foo(*splat) { |x| puts(x) }`
                        res = MK::Send(loc, MK::Magic(loc), core::Names::callWithSplat(), send->methodLoc, 4,
                                       move(sendargs), flags);
                    } else {
                        if (auto lit = cast_tree<Literal>(blockPassArg); lit && lit->isSymbol()) {
                            // Desugar a call with a splat and a Symbol block pass argument.
                            // E.g. `foo(*splat, &:to_s)`

                            auto desugaredBlockLiteral = symbol2Proc(dctx, move(blockPassArg));
                            sendargs.emplace_back(move(desugaredBlockLiteral));
                            flags.hasBlock = true;

                            res = MK::Send(loc, MK::Magic(loc), core::Names::callWithSplat(), send->methodLoc, 4,
                                           move(sendargs), flags);
                        } else {
                            // Desugar a call with a splat, and any other expression as a block pass argument.
                            // E.g. `foo(*splat, &block)`

                            sendargs.emplace_back(move(blockPassArg));
                            res = MK::Send(loc, MK::Magic(blockPassLoc), core::Names::callWithSplatAndBlockPass(),
                                           send->methodLoc, 5, move(sendargs), flags);
                        }
                    }
                    result = move(res);
                } else {
                    // Count the arguments before we concat in the Kwarg key/value pairs
                    int numPosArgs = send->args.size();

                    if (kwargsHash != nullptr) {
                        // Deconstruct the kwargs hash if it's present,
                        // concating the key/value pairs to the end of the args list
                        flattenKwargs(move(kwargsHash), send->args);
                    }

                    Send::ARGS_store args;
                    args.reserve(send->args.size());
                    for (auto &stat : send->args) {
                        args.emplace_back(node2TreeImpl(dctx, stat));
                    };

                    DuplicateHashKeyCheck::checkSendArgs(dctx.ctx, numPosArgs, args);

                    ExpressionPtr res;
                    if (blockPassArg == nullptr) {
                        // Desugar any call without a splat and without a block pass argument.
                        // If there's a literal block argument, that's handled here, too.
                        // E.g. `a.each` or `a.each { |x| puts(x) }`
                        res = MK::Send(loc, move(rec), send->method, send->methodLoc, numPosArgs, move(args), flags);
                    } else {
                        if (auto lit = cast_tree<Literal>(blockPassArg); lit && lit->isSymbol()) {
                            // Desugar a call without a splat and a Symbol block pass argument.
                            // E.g. `a.map(:to_s)`

                            auto desugaredBlockLiteral = symbol2Proc(dctx, move(blockPassArg));
                            args.emplace_back(move(desugaredBlockLiteral));
                            flags.hasBlock = true;

                            res =
                                MK::Send(loc, move(rec), send->method, send->methodLoc, numPosArgs, move(args), flags);
                        } else {
                            // Desugar a call without a splat, and any other expression as a block pass argument.
                            // E.g. `a.each(&block)`

                            Send::ARGS_store sendargs;
                            sendargs.reserve(3 + args.size());
                            sendargs.emplace_back(move(rec));
                            sendargs.emplace_back(move(methodName));
                            sendargs.emplace_back(move(blockPassArg));

                            numPosArgs += 3;

                            for (auto &arg : args) {
                                sendargs.emplace_back(move(arg));
                            }

                            res = MK::Send(loc, MK::Magic(blockPassLoc), core::Names::callWithBlockPass(),
                                           send->methodLoc, numPosArgs, move(sendargs), flags);
                        }
                    }

                    if (isCallToBlockGivenP(send) && dctx.enclosingBlockParamName.exists()) {
                        auto if_ = MK::If(loc, MK::Local(loc, dctx.enclosingBlockParamName), move(res), MK::False(loc));
                        result = move(if_);
                    } else {
                        result = move(res);
                    }
                }
            },
            [&](parser::String *string) {
                ExpressionPtr res = MK::String(loc, string->val);
                result = move(res);
            },
            [&](parser::Symbol *symbol) {
                ExpressionPtr res = MK::Symbol(loc, symbol->val);
                result = move(res);
            },
            [&](parser::LVar *var) {
                ExpressionPtr res = MK::Local(loc, var->name);
                result = move(res);
            },
            [&](parser::Hash *hash) {
                InsSeq::STATS_store updateStmts;
                updateStmts.reserve(hash->pairs.size());

                auto acc = dctx.freshNameUnique(core::Names::hashTemp());

                DuplicateHashKeyCheck hashKeyDupes(dctx.ctx);
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

                    // skip the first positional argument for the accumulator that would have been mutated
                    auto args = absl::MakeSpan(mergeValues.begin() + 1, mergeValues.end());
                    keys.reserve(args.size() / 2);
                    values.reserve(args.size() / 2);

                    for (auto [key, val] : core::KwPairSpan<ExpressionPtr>{args}) {
                        keys.emplace_back(move(key));
                        values.emplace_back(move(val));
                    }

                    return MK::Hash(loc, move(keys), move(values));
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
                    auto *pair = parser::NodeWithExpr::cast_node<parser::Pair>(pairAsExpression.get());
                    if (pair != nullptr) {
                        auto key = node2TreeImpl(dctx, pair->key);
                        hashKeyDupes.check(key);
                        mergeValues.emplace_back(move(key));

                        auto value = node2TreeImpl(dctx, pair->value);
                        mergeValues.emplace_back(move(value));

                        havePairsToMerge = true;
                        continue;
                    }

                    auto *splat = parser::NodeWithExpr::cast_node<parser::Kwsplat>(pairAsExpression.get());

                    ExpressionPtr expr;
                    if (splat != nullptr) {
                        expr = node2TreeImpl(dctx, splat->expr);
                    } else {
                        auto *fwdKwrestArg =
                            parser::NodeWithExpr::cast_node<parser::ForwardedKwrestArg>(pairAsExpression.get());
                        ENFORCE(fwdKwrestArg != nullptr, "kwsplat and fwdkwrestarg cast failed");

                        auto fwdKwargs = MK::Local(loc, core::Names::fwdKwargs());
                        expr = MK::Unsafe(loc, move(fwdKwargs));
                    }

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
                                                    numPosArgs, move(mergeValues))));
                        }

                        mergeValues.clear();
                        mergeValues.emplace_back(MK::Local(loc, acc));
                    }

                    // If this is the first argument to `<Magic>.<merge-hash>`, it needs to be duplicated as that
                    // intrinsic is assumed to mutate its first argument.
                    if (updateStmts.empty()) {
                        updateStmts.emplace_back(MK::Assign(
                            loc, acc,
                            MK::Send1(loc, MK::Magic(loc), core::Names::toHashDup(), locZeroLen, move(expr))));
                    } else {
                        updateStmts.emplace_back(MK::Assign(
                            loc, acc,
                            MK::Send2(
                                loc, MK::Magic(loc), core::Names::mergeHash(), locZeroLen, MK::Local(loc, acc),
                                MK::Send1(loc, MK::Magic(loc), core::Names::toHashNoDup(), locZeroLen, move(expr)))));
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
                                                                 locZeroLen, numPosArgs, move(mergeValues))));
                }

                if (updateStmts.empty()) {
                    result = MK::Hash0(loc);
                } else {
                    result = MK::InsSeq(loc, move(updateStmts), MK::Local(loc, acc));
                }
            },
            [&](parser::Block *block) { result = desugarBlock(dctx, block); },
            [&](parser::Begin *begin) { result = desugarBegin(dctx, loc, begin->stmts); },
            [&](parser::Assign *asgn) {
                auto lhs = node2TreeImpl(dctx, asgn->lhs);
                auto rhs = node2TreeImpl(dctx, asgn->rhs);
                // Ensure that X = <ErrorNode> always looks like a proper static field, rather
                // than a class alias.  Leaving it as a class alias would require taking the
                // slow path; turning it into a proper static field gives us a chance to take
                // the fast path.  T.let(<ErrorNode>, T.untyped) ensures that we don't get
                // warnings in `typed: strict` files.
                if (isa_tree<UnresolvedConstantLit>(lhs) && isa_tree<UnresolvedConstantLit>(rhs)) {
                    auto &rhsConst = cast_tree_nonnull<UnresolvedConstantLit>(rhs);
                    if (rhsConst.cnst == core::Names::Constants::ErrorNode()) {
                        auto rhsLocZero = rhs.loc().copyWithZeroLength();
                        rhs = MK::Let(rhsLocZero, move(rhs), MK::Untyped(rhsLocZero));
                    }
                }
                auto res = MK::Assign(loc, move(lhs), move(rhs));
                result = move(res);
            },
            // END hand-ordered clauses
            [&](parser::And *and_) {
                auto lhs = node2TreeImpl(dctx, and_->left);
                auto rhs = node2TreeImpl(dctx, and_->right);
                if (dctx.preserveConcreteSyntax) {
                    auto andAndLoc = core::LocOffsets{lhs.loc().endPos(), rhs.loc().beginPos()};
                    result =
                        MK::Send2(loc, MK::Magic(locZeroLen), core::Names::andAnd(), andAndLoc, move(lhs), move(rhs));
                    return;
                }
                if (isa_reference(lhs)) {
                    auto cond = MK::cpRef(lhs);
                    // Note that this case doesn't currently get the same "always truthy" dead code
                    // error that the other case would get.
                    auto iff = MK::If(loc, move(cond), move(rhs), move(lhs));
                    result = move(iff);
                } else {
                    auto andAndTemp = dctx.freshNameUnique(core::Names::andAnd());

                    auto lhsSend = ast::cast_tree<ast::Send>(lhs);
                    auto rhsSend = ast::cast_tree<ast::Send>(rhs);

                    ExpressionPtr thenp;
                    if (lhsSend != nullptr && rhsSend != nullptr) {
                        auto lhsSource = dctx.ctx.locAt(lhsSend->loc).source(dctx.ctx);
                        auto rhsRecvSource = dctx.ctx.locAt(rhsSend->recv.loc()).source(dctx.ctx);
                        if (lhsSource.has_value() && lhsSource == rhsRecvSource) {
                            // Have to use zero-width locs here so that these auto-generated things
                            // don't show up in e.g. completion requests.
                            rhsSend->insertPosArg(0, move(rhsSend->recv));
                            rhsSend->insertPosArg(1, MK::Symbol(rhsSend->funLoc.copyWithZeroLength(), rhsSend->fun));
                            rhsSend->insertPosArg(2, MK::Local(loc.copyWithZeroLength(), andAndTemp));
                            rhsSend->recv = MK::Magic(loc.copyWithZeroLength());
                            rhsSend->fun = core::Names::checkAndAnd();
                            thenp = move(rhs);
                        } else {
                            thenp = move(rhs);
                        }
                    } else {
                        thenp = move(rhs);
                    }
                    auto lhsLoc = lhs.loc();
                    auto condLoc = lhsLoc.exists() && thenp.loc().exists()
                                       ? core::LocOffsets{lhsLoc.endPos(), thenp.loc().beginPos()}
                                       : lhsLoc;
                    auto temp = MK::Assign(loc, andAndTemp, move(lhs));
                    auto iff = MK::If(loc, MK::Local(condLoc, andAndTemp), move(thenp), MK::Local(lhsLoc, andAndTemp));
                    auto wrapped = MK::InsSeq1(loc, move(temp), move(iff));
                    result = move(wrapped);
                }
            },
            [&](parser::Or *or_) {
                auto lhs = node2TreeImpl(dctx, or_->left);
                auto rhs = node2TreeImpl(dctx, or_->right);
                if (dctx.preserveConcreteSyntax) {
                    auto orOrLoc = core::LocOffsets{lhs.loc().endPos(), rhs.loc().beginPos()};
                    result = MK::Send2(loc, MK::Magic(locZeroLen), core::Names::orOr(), orOrLoc, move(lhs), move(rhs));
                    return;
                }
                if (isa_reference(lhs)) {
                    auto cond = MK::cpRef(lhs);
                    auto iff = MK::If(loc, move(cond), move(lhs), move(rhs));
                    result = move(iff);
                } else {
                    core::NameRef tempName = dctx.freshNameUnique(core::Names::orOr());
                    auto lhsLoc = lhs.loc();
                    auto condLoc = lhsLoc.exists() && rhs.loc().exists()
                                       ? core::LocOffsets{lhsLoc.endPos(), rhs.loc().beginPos()}
                                       : lhsLoc;
                    auto temp = MK::Assign(loc, tempName, move(lhs));
                    auto iff = MK::If(loc, MK::Local(condLoc, tempName), MK::Local(lhsLoc, tempName), move(rhs));
                    auto wrapped = MK::InsSeq1(loc, move(temp), move(iff));
                    result = move(wrapped);
                }
            },
            [&](parser::AndAsgn *andAsgn) {
                if (dctx.preserveConcreteSyntax) {
                    result = MK::Send2(loc, MK::Magic(locZeroLen), core::Names::andAsgn(), locZeroLen,
                                       node2TreeImpl(dctx, andAsgn->left), node2TreeImpl(dctx, andAsgn->right));
                    return;
                }
                auto recv = node2TreeImpl(dctx, andAsgn->left);
                auto arg = node2TreeImpl(dctx, andAsgn->right);
                if (auto s = cast_tree<Send>(recv)) {
                    auto sendLoc = s->loc;
                    auto [tempRecv, stats, numPosArgs, readArgs, assgnArgs] = copyArgsForOpAsgn(dctx, s);
                    auto numPosAssgnArgs = numPosArgs + 1;

                    assgnArgs.emplace_back(move(arg));
                    auto cond = MK::Send(sendLoc, MK::Local(sendLoc, tempRecv), s->fun, s->funLoc, numPosArgs,
                                         move(readArgs), s->flags);
                    core::NameRef tempResult = dctx.freshNameUnique(s->fun);
                    stats.emplace_back(MK::Assign(sendLoc, tempResult, move(cond)));

                    auto body = MK::Send(sendLoc, MK::Local(sendLoc, tempRecv), s->fun.addEq(dctx.ctx),
                                         sendLoc.copyWithZeroLength(), numPosAssgnArgs, move(assgnArgs), s->flags);
                    auto elsep = MK::Local(sendLoc, tempResult);
                    auto iff = MK::If(sendLoc, MK::Local(sendLoc, tempResult), move(body), move(elsep));
                    auto wrapped = MK::InsSeq(loc, move(stats), move(iff));
                    result = move(wrapped);
                } else if (isa_reference(recv)) {
                    auto cond = MK::cpRef(recv);
                    auto elsep = MK::cpRef(recv);
                    auto body = MK::Assign(loc, move(recv), move(arg));
                    auto iff = MK::If(loc, move(cond), move(body), move(elsep));
                    result = move(iff);
                } else if (isa_tree<UnresolvedConstantLit>(recv)) {
                    if (auto e = dctx.ctx.beginIndexerError(what->loc, core::errors::Desugar::NoConstantReassignment)) {
                        e.setHeader("Constant reassignment is not supported");
                    }
                    ExpressionPtr res = MK::EmptyTree();
                    result = move(res);
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
                    assgnArgs.emplace_back(move(arg));
                    auto cond = MK::Send(sendLoc, MK::Local(sendLoc, tempRecv), s->fun, s->funLoc, numPosArgs,
                                         move(readArgs), s->flags);
                    core::NameRef tempResult = dctx.freshNameUnique(s->fun);
                    stats.emplace_back(MK::Assign(sendLoc, tempResult, move(cond)));

                    auto body = MK::Send(sendLoc, MK::Local(sendLoc, tempRecv), s->fun.addEq(dctx.ctx),
                                         sendLoc.copyWithZeroLength(), numPosAssgnArgs, move(assgnArgs), s->flags);
                    auto elsep = MK::Local(sendLoc, tempResult);
                    auto iff = MK::If(sendLoc, MK::Local(sendLoc, tempResult), move(body), move(elsep));
                    auto wrapped = MK::InsSeq(loc, move(stats), move(iff));
                    ifExpr->elsep = move(wrapped);
                    result = move(recv);

                } else {
                    // the LHS has been desugared to something we haven't expected
                    Exception::notImplemented();
                }
            },
            [&](parser::OrAsgn *orAsgn) {
                if (dctx.preserveConcreteSyntax) {
                    result = MK::Send2(loc, MK::Magic(locZeroLen), core::Names::orAsgn(), locZeroLen,
                                       node2TreeImpl(dctx, orAsgn->left), node2TreeImpl(dctx, orAsgn->right));
                    return;
                }
                auto recvIsIvarLhs = parser::NodeWithExpr::isa_node<parser::IVarLhs>(orAsgn->left.get());
                auto recvIsCvarLhs = parser::NodeWithExpr::isa_node<parser::CVarLhs>(orAsgn->left.get());
                auto recv = node2TreeImpl(dctx, orAsgn->left);
                auto arg = node2TreeImpl(dctx, orAsgn->right);
                if (auto s = cast_tree<Send>(recv)) {
                    auto sendLoc = s->loc;
                    auto [tempRecv, stats, numPosArgs, readArgs, assgnArgs] = copyArgsForOpAsgn(dctx, s);
                    auto numPosAssgnArgs = numPosArgs + 1;
                    assgnArgs.emplace_back(move(arg));
                    auto cond = MK::Send(sendLoc, MK::Local(sendLoc, tempRecv), s->fun, s->funLoc, numPosArgs,
                                         move(readArgs), s->flags);
                    core::NameRef tempResult = dctx.freshNameUnique(s->fun);
                    stats.emplace_back(MK::Assign(sendLoc, tempResult, move(cond)));

                    auto elsep = MK::Send(sendLoc, MK::Local(sendLoc, tempRecv), s->fun.addEq(dctx.ctx),
                                          sendLoc.copyWithZeroLength(), numPosAssgnArgs, move(assgnArgs), s->flags);
                    auto body = MK::Local(sendLoc, tempResult);
                    auto iff = MK::If(sendLoc, MK::Local(sendLoc, tempResult), move(body), move(elsep));
                    auto wrapped = MK::InsSeq(loc, move(stats), move(iff));
                    result = move(wrapped);
                } else if (isa_reference(recv)) {
                    // When it's a reference (something variable-like), using the recv/Send terminology only
                    // confuses things. Let's just call it LHS like we would for normal assignments.
                    auto lhs = move(recv);
                    auto cond = MK::cpRef(lhs);
                    auto body = MK::cpRef(lhs);
                    ExpressionPtr elsep;
                    ast::Send *tlet;
                    if ((recvIsIvarLhs || recvIsCvarLhs) && (tlet = asTLet(arg))) {
                        auto val = move(tlet->getPosArg(0));
                        tlet->getPosArg(0) = MK::cpRef(lhs);

                        auto tempLocalName = dctx.freshNameUnique(core::Names::statTemp());
                        auto tempLocal = MK::Local(loc, tempLocalName);
                        auto value = MK::Assign(loc, MK::cpRef(tempLocal), move(val));

                        auto decl = MK::Assign(loc, MK::cpRef(lhs), move(arg));
                        auto assign = MK::Assign(loc, MK::cpRef(lhs), move(tempLocal));

                        InsSeq::STATS_store stats;
                        stats.emplace_back(move(decl));
                        stats.emplace_back(move(value));

                        elsep = MK::InsSeq(loc, move(stats), move(assign));
                    } else {
                        elsep = MK::Assign(loc, move(lhs), move(arg));
                    }
                    auto iff = MK::If(loc, move(cond), move(body), move(elsep));
                    result = move(iff);
                } else if (isa_tree<UnresolvedConstantLit>(recv)) {
                    if (auto e = dctx.ctx.beginIndexerError(what->loc, core::errors::Desugar::NoConstantReassignment)) {
                        e.setHeader("Constant reassignment is not supported");
                    }
                    ExpressionPtr res = MK::EmptyTree();
                    result = move(res);
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
                    assgnArgs.emplace_back(move(arg));
                    auto cond = MK::Send(sendLoc, MK::Local(sendLoc, tempRecv), s->fun, s->funLoc, numPosArgs,
                                         move(readArgs), s->flags);
                    core::NameRef tempResult = dctx.freshNameUnique(s->fun);
                    stats.emplace_back(MK::Assign(sendLoc, tempResult, move(cond)));

                    auto elsep = MK::Send(sendLoc, MK::Local(sendLoc, tempRecv), s->fun.addEq(dctx.ctx),
                                          sendLoc.copyWithZeroLength(), numPosAssgnArgs, move(assgnArgs), s->flags);
                    auto body = MK::Local(sendLoc, tempResult);
                    auto iff = MK::If(sendLoc, MK::Local(sendLoc, tempResult), move(body), move(elsep));
                    auto wrapped = MK::InsSeq(loc, move(stats), move(iff));
                    ifExpr->elsep = move(wrapped);
                    result = move(recv);

                } else {
                    // the LHS has been desugared to something that we haven't expected
                    Exception::notImplemented();
                }
            },
            [&](parser::OpAsgn *opAsgn) {
                if (dctx.preserveConcreteSyntax) {
                    result = MK::Send2(loc, MK::Magic(locZeroLen), core::Names::opAsgn(), locZeroLen,
                                       node2TreeImpl(dctx, opAsgn->left), node2TreeImpl(dctx, opAsgn->right));
                    return;
                }
                auto recv = node2TreeImpl(dctx, opAsgn->left);
                auto rhs = node2TreeImpl(dctx, opAsgn->right);
                if (auto s = cast_tree<Send>(recv)) {
                    auto sendLoc = s->loc;
                    auto [tempRecv, stats, numPosArgs, readArgs, assgnArgs] = copyArgsForOpAsgn(dctx, s);

                    auto prevValue = MK::Send(sendLoc, MK::Local(sendLoc, tempRecv), s->fun, s->funLoc, numPosArgs,
                                              move(readArgs), s->flags);
                    auto newValue = MK::Send1(sendLoc, move(prevValue), opAsgn->op, opAsgn->opLoc, move(rhs));
                    auto numPosAssgnArgs = numPosArgs + 1;
                    assgnArgs.emplace_back(move(newValue));

                    auto res = MK::Send(sendLoc, MK::Local(sendLoc, tempRecv), s->fun.addEq(dctx.ctx),
                                        sendLoc.copyWithZeroLength(), numPosAssgnArgs, move(assgnArgs), s->flags);
                    auto wrapped = MK::InsSeq(loc, move(stats), move(res));
                    result = move(wrapped);
                } else if (isa_reference(recv)) {
                    auto lhs = MK::cpRef(recv);
                    auto send = MK::Send1(loc, move(recv), opAsgn->op, opAsgn->opLoc, move(rhs));
                    auto res = MK::Assign(loc, move(lhs), move(send));
                    result = move(res);
                } else if (isa_tree<UnresolvedConstantLit>(recv)) {
                    if (auto e = dctx.ctx.beginIndexerError(what->loc, core::errors::Desugar::NoConstantReassignment)) {
                        e.setHeader("Constant reassignment is not supported");
                    }
                    ExpressionPtr res = MK::EmptyTree();
                    result = move(res);
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
                                              move(readArgs), s->flags);
                    auto newValue = MK::Send1(sendLoc, move(prevValue), opAsgn->op, opAsgn->opLoc, move(rhs));
                    auto numPosAssgnArgs = numPosArgs + 1;
                    assgnArgs.emplace_back(move(newValue));

                    auto res = MK::Send(sendLoc, MK::Local(sendLoc, tempRecv), s->fun.addEq(dctx.ctx),
                                        sendLoc.copyWithZeroLength(), numPosAssgnArgs, move(assgnArgs), s->flags);
                    auto wrapped = MK::InsSeq(loc, move(stats), move(res));
                    ifExpr->elsep = move(wrapped);
                    result = move(recv);

                } else {
                    // the LHS has been desugared to something we haven't expected
                    Exception::notImplemented();
                }
            },
            [&](parser::CSend *csend) {
                if (dctx.preserveConcreteSyntax) {
                    // Desugaring to a InsSeq + If causes a problem for Extract to Variable; the fake If will be where
                    // the new variable is inserted, which is incorrect. Instead, desugar to a regular send, so that the
                    // insertion happens in the correct place (what the csend is inside);

                    // Replace the original method name with a new special one that conveys that this is a CSend, so
                    // that a&.foo is treated as different from a.foo when checking for structural equality.
                    auto newFun = dctx.ctx.state.freshNameUnique(core::UniqueNameKind::DesugarCsend, csend->method, 1);
                    unique_ptr<parser::Node> sendNode = make_unique<parser::Send>(loc, move(csend->receiver), newFun,
                                                                                  csend->methodLoc, move(csend->args));
                    auto send = node2TreeImpl(dctx, sendNode);
                    result = move(send);
                    return;
                }
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

                auto assgn = MK::Assign(zeroLengthRecvLoc, tempRecv, node2TreeImpl(dctx, csend->receiver));

                // Just compare with `NilClass` to avoid potentially calling into a class-defined `==`
                auto cond =
                    MK::Send1(zeroLengthLoc, ast::MK::Constant(zeroLengthRecvLoc, core::Symbols::NilClass()),
                              core::Names::tripleEq(), zeroLengthRecvLoc, MK::Local(zeroLengthRecvLoc, tempRecv));

                unique_ptr<parser::Node> sendNode =
                    make_unique<parser::Send>(loc, make_unique<parser::LVar>(zeroLengthRecvLoc, tempRecv),
                                              csend->method, csend->methodLoc, move(csend->args));
                auto send = node2TreeImpl(dctx, sendNode);

                ExpressionPtr nil =
                    MK::Send1(recvLoc.copyEndWithZeroLength(), MK::Magic(zeroLengthLoc),
                              core::Names::nilForSafeNavigation(), zeroLengthLoc, MK::Local(csendLoc, tempRecv));
                auto iff = MK::If(zeroLengthLoc, move(cond), move(nil), move(send));
                auto res = MK::InsSeq1(csend->loc, move(assgn), move(iff));
                result = move(res);
            },
            [&](parser::Self *self) { desugaredByPrismTranslator(self); },
            [&](parser::DSymbol *dsymbol) {
                if (dsymbol->nodes.empty()) {
                    ExpressionPtr res = MK::Symbol(loc, core::Names::empty());
                    result = move(res);
                    return;
                }

                auto str = desugarDString(dctx, loc, move(dsymbol->nodes));
                ExpressionPtr res = MK::Send0(loc, move(str), core::Names::intern(), locZeroLen);

                result = move(res);
            },
            [&](parser::FileLiteral *fileLiteral) { desugaredByPrismTranslator(fileLiteral); },
            [&](parser::ConstLhs *constLhs) {
                auto scope = node2TreeImpl(dctx, constLhs->scope);
                ExpressionPtr res = MK::UnresolvedConstant(loc, move(scope), constLhs->name);
                result = move(res);
            },
            [&](parser::Cbase *cbase) { desugaredByPrismTranslator(cbase); },
            [&](parser::Kwbegin *kwbegin) { result = desugarBegin(dctx, loc, kwbegin->stmts); },
            [&](parser::Module *module) {
                DesugarContext dctx1(dctx.ctx, dctx.uniqueCounter, dctx.enclosingBlockParamName,
                                     dctx.enclosingMethodLoc, dctx.enclosingMethodName, dctx.inAnyBlock, true,
                                     dctx.preserveConcreteSyntax);
                ClassDef::RHS_store body = scopeNodeToBody(dctx1, move(module->body));
                ExpressionPtr res =
                    MK::Module(module->loc, module->declLoc, node2TreeImpl(dctx, module->name), move(body));
                result = move(res);
            },
            [&](parser::Class *klass) {
                DesugarContext dctx1(dctx.ctx, dctx.uniqueCounter, dctx.enclosingBlockParamName,
                                     dctx.enclosingMethodLoc, dctx.enclosingMethodName, dctx.inAnyBlock, false,
                                     dctx.preserveConcreteSyntax);
                ClassDef::RHS_store body = scopeNodeToBody(dctx1, move(klass->body));
                ClassDef::ANCESTORS_store ancestors;
                if (klass->superclass == nullptr) {
                    ancestors.emplace_back(MK::Constant(loc, core::Symbols::todo()));
                } else {
                    ancestors.emplace_back(node2TreeImpl(dctx, klass->superclass));
                }
                ExpressionPtr res = MK::Class(klass->loc, klass->declLoc, node2TreeImpl(dctx, klass->name),
                                              move(ancestors), move(body));
                result = move(res);
            },
            [&](parser::Param *param) { desugaredByPrismTranslator(param); },
            [&](parser::RestParam *param) {
                ExpressionPtr res = MK::RestParam(loc, MK::Local(param->nameLoc, param->name));
                result = move(res);
            },
            [&](parser::Kwrestarg *arg) {
                ExpressionPtr res = MK::RestParam(loc, MK::KeywordArg(loc, arg->name));
                result = move(res);
            },
            [&](parser::Kwarg *arg) { desugaredByPrismTranslator(arg); },
            [&](parser::BlockParam *param) {
                ExpressionPtr res = MK::BlockParam(loc, MK::Local(loc, param->name));
                result = move(res);
            },
            [&](parser::Kwoptarg *arg) {
                ExpressionPtr res =
                    MK::OptionalParam(loc, MK::KeywordArg(arg->nameLoc, arg->name), node2TreeImpl(dctx, arg->default_));
                result = move(res);
            },
            [&](parser::OptParam *param) {
                ExpressionPtr res = MK::OptionalParam(loc, MK::Local(param->nameLoc, param->name),
                                                      node2TreeImpl(dctx, param->default_));
                result = move(res);
            },
            [&](parser::Shadowarg *arg) { desugaredByPrismTranslator(arg); },
            [&](parser::DefMethod *method) {
                bool isSelf = false;
                ExpressionPtr res = buildMethod(dctx, method->loc, method->declLoc, method->name, method->params.get(),
                                                method->body, isSelf);
                result = move(res);
            },
            [&](parser::DefS *method) {
                auto *self = parser::NodeWithExpr::cast_node<parser::Self>(method->singleton.get());
                if (self == nullptr) {
                    if (auto e = dctx.ctx.beginIndexerError(method->singleton->loc,
                                                            core::errors::Desugar::InvalidSingletonDef)) {
                        e.setHeader("`{}` is only supported for `{}`", "def EXPRESSION.method", "def self.method");
                        e.addErrorNote("When it's imperative to define a singleton method on an object,\n"
                                       "    use `{}` instead.\n"
                                       "    The method will NOT be visible to Sorbet.",
                                       "EXPRESSION.define_singleton_method(:method) { ... }");
                    }
                }
                bool isSelf = true;
                ExpressionPtr res = buildMethod(dctx, method->loc, method->declLoc, method->name, method->params.get(),
                                                method->body, isSelf);
                result = move(res);
            },
            [&](parser::SClass *sclass) {
                // This will be a nested ClassDef which we leave in the tree
                // which will get the symbol of `class.singleton_class`
                auto *self = parser::NodeWithExpr::cast_node<parser::Self>(sclass->expr.get());
                if (self == nullptr) {
                    if (auto e =
                            dctx.ctx.beginIndexerError(sclass->expr->loc, core::errors::Desugar::InvalidSingletonDef)) {
                        e.setHeader("`{}` is only supported for `{}`", "class << EXPRESSION", "class << self");
                    }
                    ExpressionPtr res = MK::EmptyTree();
                    result = move(res);
                    return;
                }

                DesugarContext dctx1(dctx.ctx, dctx.uniqueCounter, dctx.enclosingBlockParamName,
                                     dctx.enclosingMethodLoc, dctx.enclosingMethodName, dctx.inAnyBlock, false,
                                     dctx.preserveConcreteSyntax);
                ClassDef::RHS_store body = scopeNodeToBody(dctx1, move(sclass->body));
                ClassDef::ANCESTORS_store emptyAncestors;
                ExpressionPtr res =
                    MK::Class(sclass->loc, sclass->declLoc,
                              make_expression<UnresolvedIdent>(sclass->expr->loc, UnresolvedIdent::Kind::Class,
                                                               core::Names::singleton()),
                              move(emptyAncestors), move(body));
                result = move(res);
            },
            [&](parser::While *wl) {
                auto cond = node2TreeImpl(dctx, wl->cond);
                auto body = node2TreeImpl(dctx, wl->body);
                ExpressionPtr res = MK::While(loc, move(cond), move(body));
                result = move(res);
            },
            [&](parser::WhilePost *wl) {
                bool isKwbegin = parser::NodeWithExpr::isa_node<parser::Kwbegin>(wl->body.get());
                auto cond = node2TreeImpl(dctx, wl->cond);
                auto body = node2TreeImpl(dctx, wl->body);
                // TODO using bang (aka !) is not semantically correct because it can be overridden by the user.
                ExpressionPtr res =
                    isKwbegin
                        ? doUntil(dctx, loc, MK::Send0(loc, move(cond), core::Names::bang(), locZeroLen), move(body))
                        : MK::While(loc, move(cond), move(body));
                result = move(res);
            },
            [&](parser::Until *wl) {
                auto cond = node2TreeImpl(dctx, wl->cond);
                auto body = node2TreeImpl(dctx, wl->body);
                ExpressionPtr res =
                    MK::While(loc, MK::Send0(loc, move(cond), core::Names::bang(), locZeroLen), move(body));
                result = move(res);
            },
            // This is the same as WhilePost, but the cond negation is in the other branch.
            [&](parser::UntilPost *wl) {
                bool isKwbegin = parser::NodeWithExpr::isa_node<parser::Kwbegin>(wl->body.get());
                auto cond = node2TreeImpl(dctx, wl->cond);
                auto body = node2TreeImpl(dctx, wl->body);
                ExpressionPtr res =
                    isKwbegin ? doUntil(dctx, loc, move(cond), move(body))
                              : MK::While(loc, MK::Send0(loc, move(cond), core::Names::bang(), locZeroLen), move(body));
                result = move(res);
            },
            [&](parser::Nil *wl) {
                ExpressionPtr res = MK::Nil(loc);
                result = move(res);
            },
            [&](parser::IVar *var) { desugaredByPrismTranslator(var); },
            [&](parser::GVar *var) { desugaredByPrismTranslator(var); },
            [&](parser::CVar *var) { desugaredByPrismTranslator(var); },
            [&](parser::LVarLhs *var) {
                ExpressionPtr res = MK::Local(loc, var->name);
                result = move(res);
            },
            [&](parser::GVarLhs *var) {
                ExpressionPtr res = make_expression<UnresolvedIdent>(loc, UnresolvedIdent::Kind::Global, var->name);
                result = move(res);
            },
            [&](parser::CVarLhs *var) {
                ExpressionPtr res = make_expression<UnresolvedIdent>(loc, UnresolvedIdent::Kind::Class, var->name);
                result = move(res);
            },
            [&](parser::IVarLhs *var) {
                ExpressionPtr res = make_expression<UnresolvedIdent>(loc, UnresolvedIdent::Kind::Instance, var->name);
                result = move(res);
            },
            [&](parser::NthRef *var) { desugaredByPrismTranslator(var); },
            [&](parser::Super *super) {
                // Desugar super into a call to a normal method named `super`;
                // Do this by synthesizing a `Send` parse node and letting our
                // Send desugar handle it.
                auto method = maybeTypedSuper(dctx);
                unique_ptr<parser::Node> send =
                    make_unique<parser::Send>(super->loc, nullptr, method, super->loc, move(super->args));
                auto res = node2TreeImpl(dctx, send);
                result = move(res);
            },
            [&](parser::ZSuper *zuper) { desugaredByPrismTranslator(zuper); },
            [&](parser::For *for_) {
                MethodDef::PARAMS_store params;
                bool canProvideNiceDesugar = true;
                auto mlhsNode = move(for_->vars);
                if (auto *mlhs = parser::NodeWithExpr::cast_node<parser::Mlhs>(mlhsNode.get())) {
                    for (auto &c : mlhs->exprs) {
                        if (!parser::NodeWithExpr::isa_node<parser::LVarLhs>(c.get())) {
                            canProvideNiceDesugar = false;
                            break;
                        }
                    }
                    if (canProvideNiceDesugar) {
                        for (auto &c : mlhs->exprs) {
                            params.emplace_back(node2TreeImpl(dctx, c));
                        }
                    }
                } else {
                    canProvideNiceDesugar = parser::NodeWithExpr::isa_node<parser::LVarLhs>(mlhsNode.get());
                    if (canProvideNiceDesugar) {
                        ExpressionPtr lhs = node2TreeImpl(dctx, mlhsNode);
                        params.emplace_back(move(lhs));
                    } else {
                        mlhsNode = make_unique<parser::Mlhs>(loc, NodeVec1(move(mlhsNode)));
                    }
                }

                auto body = node2TreeImpl(dctx, for_->body);

                ExpressionPtr block;
                if (canProvideNiceDesugar) {
                    block = MK::Block(loc, move(body), move(params));
                } else {
                    auto temp = dctx.freshNameUnique(core::Names::forTemp());

                    unique_ptr<parser::Node> masgn =
                        make_unique<parser::Masgn>(loc, move(mlhsNode), make_unique<parser::LVar>(loc, temp));

                    body = MK::InsSeq1(loc, node2TreeImpl(dctx, masgn), move(body));
                    block = MK::Block(loc, move(body), move(params));
                }

                auto res =
                    MK::Send0Block(loc, node2TreeImpl(dctx, for_->expr), core::Names::each(), locZeroLen, move(block));
                result = move(res);
            },
            [&](parser::Integer *integer) { desugaredByPrismTranslator(integer); },
            [&](parser::DString *dstring) {
                ExpressionPtr res = desugarDString(dctx, loc, move(dstring->nodes));
                result = move(res);
            },
            [&](parser::Float *floatNode) { desugaredByPrismTranslator(floatNode); },
            [&](parser::Complex *complex) { desugaredByPrismTranslator(complex); },
            [&](parser::Rational *rational) { desugaredByPrismTranslator(rational); },
            [&](parser::Array *array) {
                Array::ENTRY_store elems;
                elems.reserve(array->elts.size());
                ExpressionPtr lastMerge;
                for (auto &stat : array->elts) {
                    if (parser::NodeWithExpr::isa_node<parser::Splat>(stat.get()) ||
                        parser::NodeWithExpr::isa_node<parser::ForwardedRestArg>(stat.get())) {
                        // The parser::Send case makes a fake parser::Array with locZeroLen to hide callWithSplat
                        // methods from hover. Using the array's loc means that we will get a zero-length loc for
                        // the Splat in that case, and non-zero if there was a real Array literal.
                        stat->loc = loc;
                        // Desguar
                        //   [a, *x, remaining]
                        // into
                        //   a.concat(<splat>(x)).concat(remaining)
                        auto var = node2TreeImpl(dctx, stat);
                        if (elems.empty()) {
                            if (lastMerge != nullptr) {
                                lastMerge =
                                    MK::Send1(loc, move(lastMerge), core::Names::concat(), locZeroLen, move(var));
                            } else {
                                lastMerge = move(var);
                            }
                        } else {
                            ExpressionPtr current = MK::Array(loc, move(elems));
                            /* reassign instead of clear to work around https://bugs.llvm.org/show_bug.cgi?id=37553 */
                            elems = Array::ENTRY_store();
                            if (lastMerge != nullptr) {
                                lastMerge =
                                    MK::Send1(loc, move(lastMerge), core::Names::concat(), locZeroLen, move(current));
                            } else {
                                lastMerge = move(current);
                            }
                            lastMerge = MK::Send1(loc, move(lastMerge), core::Names::concat(), locZeroLen, move(var));
                        }
                    } else {
                        elems.emplace_back(node2TreeImpl(dctx, stat));
                    }
                };

                ExpressionPtr res;
                if (elems.empty()) {
                    if (lastMerge != nullptr) {
                        res = move(lastMerge);
                    } else {
                        // Empty array
                        res = MK::Array(loc, move(elems));
                    }
                } else {
                    res = MK::Array(loc, move(elems));
                    if (lastMerge != nullptr) {
                        res = MK::Send1(loc, move(lastMerge), core::Names::concat(), locZeroLen, move(res));
                    }
                }
                result = move(res);
            },
            [&](parser::IRange *ret) {
                auto recv = MK::Magic(loc);
                auto from = node2TreeImpl(dctx, ret->from);
                auto to = node2TreeImpl(dctx, ret->to);
                auto excludeEnd = MK::False(loc);
                auto send = MK::Send3(loc, move(recv), core::Names::buildRange(), locZeroLen, move(from), move(to),
                                      move(excludeEnd));
                result = move(send);
            },
            [&](parser::ERange *ret) {
                auto recv = MK::Magic(loc);
                auto from = node2TreeImpl(dctx, ret->from);
                auto to = node2TreeImpl(dctx, ret->to);
                auto excludeEnd = MK::True(loc);
                auto send = MK::Send3(loc, move(recv), core::Names::buildRange(), locZeroLen, move(from), move(to),
                                      move(excludeEnd));
                result = move(send);
            },
            [&](parser::Regexp *regexpNode) {
                ExpressionPtr cnst = MK::Constant(loc, core::Symbols::Regexp());
                auto pattern = desugarDString(dctx, loc, move(regexpNode->regex));
                auto opts = node2TreeImpl(dctx, regexpNode->opts);
                auto send = MK::Send2(loc, move(cnst), core::Names::new_(), locZeroLen, move(pattern), move(opts));
                result = move(send);
            },
            [&](parser::Regopt *regopt) { desugaredByPrismTranslator(regopt); },
            [&](parser::Return *ret) {
                if (ret->exprs.size() > 1) {
                    auto arrayLoc = ret->exprs.front()->loc.join(ret->exprs.back()->loc);
                    Array::ENTRY_store elems;
                    elems.reserve(ret->exprs.size());
                    for (auto &stat : ret->exprs) {
                        if (parser::NodeWithExpr::isa_node<parser::BlockPass>(stat.get())) {
                            if (auto e = dctx.ctx.beginIndexerError(ret->loc, core::errors::Desugar::UnsupportedNode)) {
                                e.setHeader("Block argument should not be given");
                            }
                            continue;
                        }
                        elems.emplace_back(node2TreeImpl(dctx, stat));
                    };
                    ExpressionPtr arr = MK::Array(arrayLoc, move(elems));
                    ExpressionPtr res = MK::Return(loc, move(arr));
                    result = move(res);
                } else if (ret->exprs.size() == 1) {
                    if (parser::NodeWithExpr::isa_node<parser::BlockPass>(ret->exprs[0].get())) {
                        if (auto e = dctx.ctx.beginIndexerError(ret->loc, core::errors::Desugar::UnsupportedNode)) {
                            e.setHeader("Block argument should not be given");
                        }
                        ExpressionPtr res = MK::Break(loc, MK::EmptyTree());
                        result = move(res);
                    } else {
                        ExpressionPtr res = MK::Return(loc, node2TreeImpl(dctx, ret->exprs[0]));
                        result = move(res);
                    }
                } else {
                    ExpressionPtr res = MK::Return(loc, MK::EmptyTree());
                    result = move(res);
                }
            },
            [&](parser::Break *ret) {
                if (ret->exprs.size() > 1) {
                    auto arrayLoc = ret->exprs.front()->loc.join(ret->exprs.back()->loc);
                    Array::ENTRY_store elems;
                    elems.reserve(ret->exprs.size());
                    for (auto &stat : ret->exprs) {
                        if (parser::NodeWithExpr::isa_node<parser::BlockPass>(stat.get())) {
                            if (auto e = dctx.ctx.beginIndexerError(ret->loc, core::errors::Desugar::UnsupportedNode)) {
                                e.setHeader("Block argument should not be given");
                            }
                            continue;
                        }
                        elems.emplace_back(node2TreeImpl(dctx, stat));
                    };
                    ExpressionPtr arr = MK::Array(arrayLoc, move(elems));
                    ExpressionPtr res = MK::Break(loc, move(arr));
                    result = move(res);
                } else if (ret->exprs.size() == 1) {
                    if (parser::NodeWithExpr::isa_node<parser::BlockPass>(ret->exprs[0].get())) {
                        if (auto e = dctx.ctx.beginIndexerError(ret->loc, core::errors::Desugar::UnsupportedNode)) {
                            e.setHeader("Block argument should not be given");
                        }
                        ExpressionPtr res = MK::Break(loc, MK::EmptyTree());
                        result = move(res);
                    } else {
                        ExpressionPtr res = MK::Break(loc, node2TreeImpl(dctx, ret->exprs[0]));
                        result = move(res);
                    }
                } else {
                    ExpressionPtr res = MK::Break(loc, MK::EmptyTree());
                    result = move(res);
                }
            },
            [&](parser::Next *ret) {
                if (ret->exprs.size() > 1) {
                    auto arrayLoc = ret->exprs.front()->loc.join(ret->exprs.back()->loc);
                    Array::ENTRY_store elems;
                    elems.reserve(ret->exprs.size());
                    for (auto &stat : ret->exprs) {
                        if (parser::NodeWithExpr::isa_node<parser::BlockPass>(stat.get())) {
                            if (auto e = dctx.ctx.beginIndexerError(ret->loc, core::errors::Desugar::UnsupportedNode)) {
                                e.setHeader("Block argument should not be given");
                            }
                            continue;
                        }
                        elems.emplace_back(node2TreeImpl(dctx, stat));
                    };
                    ExpressionPtr arr = MK::Array(arrayLoc, move(elems));
                    ExpressionPtr res = MK::Next(loc, move(arr));
                    result = move(res);
                } else if (ret->exprs.size() == 1) {
                    if (parser::NodeWithExpr::isa_node<parser::BlockPass>(ret->exprs[0].get())) {
                        if (auto e = dctx.ctx.beginIndexerError(ret->loc, core::errors::Desugar::UnsupportedNode)) {
                            e.setHeader("Block argument should not be given");
                        }
                        ExpressionPtr res = MK::Break(loc, MK::EmptyTree());
                        result = move(res);
                    } else {
                        ExpressionPtr res = MK::Next(loc, node2TreeImpl(dctx, ret->exprs[0]));
                        result = move(res);
                    }
                } else {
                    ExpressionPtr res = MK::Next(loc, MK::EmptyTree());
                    result = move(res);
                }
            },
            [&](parser::Retry *ret) { desugaredByPrismTranslator(ret); },
            [&](parser::Yield *ret) {
                Send::ARGS_store args;
                args.reserve(ret->exprs.size());
                for (auto &stat : ret->exprs) {
                    args.emplace_back(node2TreeImpl(dctx, stat));
                };

                ExpressionPtr recv;
                if (dctx.enclosingBlockParamName.exists()) {
                    // we always want to report an error if we're using yield with a synthesized name in strict mode
                    if (dctx.enclosingBlockParamName == core::Names::blkArg()) {
                        if (auto e = dctx.ctx.beginIndexerError(dctx.enclosingMethodLoc,
                                                                core::errors::Desugar::UnnamedBlockParameter)) {
                            e.setHeader("Method `{}` uses `{}` but does not mention a block parameter",
                                        dctx.enclosingMethodName.show(dctx.ctx), "yield");
                            e.addErrorLine(dctx.ctx.locAt(loc), "Arising from use of `{}` in method body", "yield");
                        }
                    }

                    recv = MK::Local(loc, dctx.enclosingBlockParamName);
                } else {
                    // No enclosing block arg can happen when e.g. yield is called in a class / at the top-level.
                    recv = MK::RaiseUnimplemented(loc);
                }
                ExpressionPtr res = MK::Send(loc, move(recv), core::Names::call(), locZeroLen, args.size(), move(args));
                result = move(res);
            },
            [&](parser::Rescue *rescue) {
                Rescue::RESCUE_CASE_store cases;
                cases.reserve(rescue->rescue.size());
                for (auto &node : rescue->rescue) {
                    cases.emplace_back(node2TreeImpl(dctx, node));
                    ENFORCE(isa_tree<RescueCase>(cases.back()), "node2TreeImpl failed to produce a rescue case");
                }
                ExpressionPtr res = make_expression<Rescue>(loc, node2TreeImpl(dctx, rescue->body), move(cases),
                                                            node2TreeImpl(dctx, rescue->else_), MK::EmptyTree());
                result = move(res);
            },
            [&](parser::Resbody *resbody) {
                RescueCase::EXCEPTION_store exceptions;
                auto exceptionsExpr = node2TreeImpl(dctx, resbody->exception);
                if (isa_tree<EmptyTree>(exceptionsExpr)) {
                    // No exceptions captured
                } else if (auto exceptionsArray = cast_tree<Array>(exceptionsExpr)) {
                    ENFORCE(exceptionsArray != nullptr, "exception array cast failed");

                    for (auto &elem : exceptionsArray->elems) {
                        exceptions.emplace_back(move(elem));
                    }
                } else if (auto exceptionsSend = cast_tree<Send>(exceptionsExpr)) {
                    ENFORCE(exceptionsSend->fun == core::Names::splat() || exceptionsSend->fun == core::Names::toA() ||
                                exceptionsSend->fun == core::Names::concat(),
                            "Unknown exceptionSend function");
                    exceptions.emplace_back(move(exceptionsExpr));
                } else {
                    Exception::raise("Bad inner node type");
                }

                auto varExpr = node2TreeImpl(dctx, resbody->var);
                auto body = node2TreeImpl(dctx, resbody->body);

                auto varLoc = varExpr.loc();
                auto var = core::NameRef::noName();
                if (auto id = cast_tree<UnresolvedIdent>(varExpr)) {
                    if (id->kind == UnresolvedIdent::Kind::Local) {
                        var = id->name;
                        varExpr.reset(nullptr);
                    }
                }

                if (!var.exists()) {
                    var = dctx.freshNameUnique(core::Names::rescueTemp());
                }

                if (isa_tree<EmptyTree>(varExpr)) {
                    // In `rescue; ...; end`, we don't want the magic <rescueTemp> variable to look
                    // as if its loc is the entire `rescue; ...; end` span. Better to just point at
                    // the `rescue` keyword.
                    varLoc = (loc.length() > 6) ? core::LocOffsets{loc.beginPos(), loc.beginPos() + 6}
                                                : loc.copyWithZeroLength();
                } else if (varExpr != nullptr) {
                    body = MK::InsSeq1(varLoc, MK::Assign(varLoc, move(varExpr), MK::Local(varLoc, var)), move(body));
                }

                ExpressionPtr res =
                    make_expression<RescueCase>(loc, move(exceptions), MK::Local(varLoc, var), move(body));
                result = move(res);
            },
            [&](parser::Ensure *ensure) {
                auto bodyExpr = node2TreeImpl(dctx, ensure->body);
                auto ensureExpr = node2TreeImpl(dctx, ensure->ensure);
                auto rescue = cast_tree<Rescue>(bodyExpr);
                if (rescue != nullptr) {
                    rescue->ensure = move(ensureExpr);
                    result = move(bodyExpr);
                } else {
                    Rescue::RESCUE_CASE_store cases;
                    ExpressionPtr res =
                        make_expression<Rescue>(loc, move(bodyExpr), move(cases), MK::EmptyTree(), move(ensureExpr));
                    result = move(res);
                }
            },
            [&](parser::If *if_) {
                auto cond = node2TreeImpl(dctx, if_->condition);
                auto thenp = node2TreeImpl(dctx, if_->then_);
                auto elsep = node2TreeImpl(dctx, if_->else_);
                auto iff = MK::If(loc, move(cond), move(thenp), move(elsep));
                result = move(iff);
            },
            [&](parser::Masgn *masgn) {
                auto *lhs = parser::NodeWithExpr::cast_node<parser::Mlhs>(masgn->lhs.get());
                ENFORCE(lhs != nullptr, "Failed to get lhs of Masgn");

                auto res = desugarMlhs(dctx, loc, lhs, node2TreeImpl(dctx, masgn->rhs));

                result = move(res);
            },
            [&](parser::True *t) { desugaredByPrismTranslator(t); },
            [&](parser::False *t) { desugaredByPrismTranslator(t); },
            [&](parser::Case *case_) {
                if (dctx.preserveConcreteSyntax) {
                    // Desugar to:
                    //   Magic.caseWhen(condition, numPatterns, <pattern 1>, ..., <pattern N>, <body 1>, ..., <body M>)
                    // Putting all the patterns at the start so that we can skip them when checking which body to insert
                    // into.
                    Send::ARGS_store args;
                    args.emplace_back(node2TreeImpl(dctx, case_->condition));

                    Send::ARGS_store patterns;
                    Send::ARGS_store bodies;
                    for (auto it = case_->whens.begin(); it != case_->whens.end(); ++it) {
                        auto when = parser::NodeWithExpr::cast_node<parser::When>(it->get());
                        ENFORCE(when != nullptr, "case without a when?");
                        for (auto &cnode : when->patterns) {
                            patterns.emplace_back(node2TreeImpl(dctx, cnode));
                        }
                        bodies.emplace_back(node2TreeImpl(dctx, when->body));
                    }
                    bodies.emplace_back(node2TreeImpl(dctx, case_->else_));

                    args.emplace_back(MK::Int(locZeroLen, patterns.size()));
                    move(patterns.begin(), patterns.end(), back_inserter(args));
                    move(bodies.begin(), bodies.end(), back_inserter(args));

                    result = MK::Send(loc, MK::Magic(locZeroLen), core::Names::caseWhen(), locZeroLen, args.size(),
                                      move(args));
                    return;
                }

                ExpressionPtr assign;
                auto temp = core::NameRef::noName();
                core::LocOffsets cloc;

                if (case_->condition != nullptr) {
                    cloc = case_->condition->loc;
                    temp = dctx.freshNameUnique(core::Names::assignTemp());
                    assign = MK::Assign(cloc, temp, node2TreeImpl(dctx, case_->condition));
                }
                ExpressionPtr res = node2TreeImpl(dctx, case_->else_);
                for (auto it = case_->whens.rbegin(); it != case_->whens.rend(); ++it) {
                    auto when = parser::NodeWithExpr::cast_node<parser::When>(it->get());
                    ENFORCE(when != nullptr, "case without a when?");
                    ExpressionPtr cond;
                    for (auto &cnode : when->patterns) {
                        ExpressionPtr test;
                        if (parser::NodeWithExpr::isa_node<parser::Splat>(cnode.get())) {
                            ENFORCE(temp.exists(), "splats need something to test against");
                            auto recv = MK::Magic(loc);
                            auto local = MK::Local(cloc, temp);
                            // TODO(froydnj): use the splat's var directly so we can elide the
                            // coercion to an array where possible.
                            auto splat = node2TreeImpl(dctx, cnode);
                            auto patternloc = splat.loc();
                            test = MK::Send2(patternloc, move(recv), core::Names::checkMatchArray(),
                                             patternloc.copyWithZeroLength(), move(local), move(splat));
                        } else {
                            auto ctree = node2TreeImpl(dctx, cnode);
                            if (temp.exists()) {
                                auto local = MK::Local(cloc, temp);
                                auto patternloc = ctree.loc();
                                test = MK::Send1(patternloc, move(ctree), core::Names::tripleEq(),
                                                 patternloc.copyWithZeroLength(), move(local));
                            } else {
                                test = move(ctree);
                            }
                        }
                        if (cond == nullptr) {
                            cond = move(test);
                        } else {
                            auto true_ = MK::True(test.loc());
                            auto loc = test.loc();
                            cond = MK::If(loc, move(test), move(true_), move(cond));
                        }
                    }
                    res = MK::If(when->loc, move(cond), node2TreeImpl(dctx, when->body), move(res));
                }
                if (assign != nullptr) {
                    res = MK::InsSeq1(loc, move(assign), move(res));
                }
                result = move(res);
            },
            [&](parser::Splat *splat) {
                auto res = MK::Splat(loc, node2TreeImpl(dctx, splat->var));
                result = move(res);
            },
            [&](parser::ForwardedRestArg *fra) {
                auto var = ast::MK::Local(loc, core::Names::star());
                result = MK::Splat(loc, move(var));
            },
            [&](parser::Alias *alias) {
                auto res = MK::Send2(loc, MK::Self(loc), core::Names::aliasMethod(), locZeroLen,
                                     node2TreeImpl(dctx, alias->from), node2TreeImpl(dctx, alias->to));
                result = move(res);
            },
            [&](parser::Defined *defined) { desugaredByPrismTranslator(defined); },
            [&](parser::LineLiteral *line) { desugaredByPrismTranslator(line); },
            [&](parser::XString *xstring) {
                auto res = MK::Send1(loc, MK::Self(loc), core::Names::backtick(), locZeroLen,
                                     desugarDString(dctx, loc, move(xstring->nodes)));
                result = move(res);
            },
            [&](parser::Preexe *preexe) { desugaredByPrismTranslator(preexe); },
            [&](parser::Postexe *postexe) { desugaredByPrismTranslator(postexe); },
            [&](parser::Undef *undef) { desugaredByPrismTranslator(undef); },
            [&](parser::CaseMatch *caseMatch) {
                // Create a local var to store the expression used in each match clause
                auto exprLoc = caseMatch->expr->loc;
                auto exprName = dctx.freshNameUnique(core::Names::assignTemp());
                auto exprVar = MK::Assign(exprLoc, exprName, node2TreeImpl(dctx, caseMatch->expr));

                // Desugar the `else` block
                ExpressionPtr res = node2TreeImpl(dctx, caseMatch->elseBody);

                // Desugar each `in` as an `if` branch calling `Magic.<pattern-match>()`
                for (auto it = caseMatch->inBodies.rbegin(); it != caseMatch->inBodies.rend(); ++it) {
                    auto inPattern = parser::NodeWithExpr::cast_node<parser::InPattern>(it->get());
                    ENFORCE(inPattern != nullptr, "case pattern without a in?");

                    // Keep the `in` body for the `then` body of the new `if`
                    auto pattern = move(inPattern->pattern);
                    auto body = node2TreeImpl(dctx, inPattern->body);

                    // Desugar match variables found inside the pattern
                    InsSeq::STATS_store vars;
                    desugarPatternMatchingVars(vars, dctx, pattern.get());
                    if (!vars.empty()) {
                        body = MK::InsSeq(pattern->loc, move(vars), move(body));
                    }

                    // Create a new `if` for the branch:
                    // `in A` => `if (TODO)`
                    auto match = MK::RaiseUnimplemented(pattern->loc);
                    res = MK::If(inPattern->loc, move(match), move(body), move(res));
                }
                res = MK::InsSeq1(loc, move(exprVar), move(res));
                result = move(res);
            },
            [&](parser::Backref *backref) { desugaredByPrismTranslator(backref); },
            [&](parser::EFlipflop *eflipflop) { desugaredByPrismTranslator(eflipflop); },
            [&](parser::IFlipflop *iflipflop) { desugaredByPrismTranslator(iflipflop); },
            [&](parser::MatchCurLine *matchCurLine) { desugaredByPrismTranslator(matchCurLine); },
            [&](parser::Redo *redo) { desugaredByPrismTranslator(redo); },
            [&](parser::EncodingLiteral *encodingLiteral) { desugaredByPrismTranslator(encodingLiteral); },
            [&](parser::MatchPattern *pattern) { desugaredByPrismTranslator(pattern); },
            [&](parser::MatchPatternP *pattern) { desugaredByPrismTranslator(pattern); },
            [&](parser::EmptyElse *else_) { result = MK::EmptyTree(); },
            [&](parser::ResolvedConst *resolvedConst) {
                result = make_expression<ConstantLit>(resolvedConst->loc, move(resolvedConst->symbol));
            },

            [&](parser::NodeWithExpr *nodeWithExpr) {
                if (parser::NodeWithExpr::isa_node<parser::Splat>(nodeWithExpr->wrappedNode.get())) {
                    // Special case for Splats in method calls where we want zero-length locations
                    // The `parser::Send` case makes a fake parser::Array with locZeroLen to hide callWithSplat
                    // methods from hover.
                    auto splat = parser::NodeWithExpr::cast_node<parser::Splat>(nodeWithExpr->wrappedNode.get());

                    if (splat->var->hasDesugaredExpr()) {
                        result = MK::Splat(loc, splat->var->takeDesugaredExpr());
                    } else {
                        result = node2TreeImpl(dctx, splat->var);
                    }
                } else {
                    result = nodeWithExpr->takeDesugaredExpr();
                    ENFORCE(result != nullptr, "NodeWithExpr has no cached desugared expr");
                }
            },

            [&](parser::BlockPass *blockPass) { Exception::raise("Send should have already handled the BlockPass"); },
            [&](parser::Node *node) {
                Exception::raise("Unimplemented Parser Node: PrismDesugar: {} (class: {})", node->nodeName(),
                                 demangle(typeid(*node).name()));
            });
        ENFORCE(result.get() != nullptr, "desugar result unset, (node class was: {})", demangle(typeid(*what).name()));
        return result;
    } catch (SorbetException &) {
        Exception::failInFuzzer();
        if (!locReported) {
            locReported = true;
            if (auto e = dctx.ctx.beginIndexerError(what->loc, core::errors::Internal::InternalError)) {
                e.setHeader("Failed to process tree (backtrace is above)");
            }
        }
        throw;
    }
}

// Translate trees by calling `node2TreeBody`, and manually reset the unique_ptr argument when it's done.
ExpressionPtr node2TreeImpl(DesugarContext dctx, unique_ptr<parser::Node> &what) {
    auto res = node2TreeImplBody(dctx, what.get());
    what.reset();
    return res;
}

ExpressionPtr liftTopLevel(DesugarContext dctx, core::LocOffsets loc, ExpressionPtr what) {
    ClassDef::RHS_store rhs;
    ClassDef::ANCESTORS_store ancestors;
    ancestors.emplace_back(MK::Constant(loc, core::Symbols::todo()));
    auto insSeq = cast_tree<InsSeq>(what);
    if (insSeq) {
        rhs.reserve(insSeq->stats.size() + 1);
        for (auto &stat : insSeq->stats) {
            rhs.emplace_back(move(stat));
        }
        rhs.emplace_back(move(insSeq->expr));
    } else {
        rhs.emplace_back(move(what));
    }
    return make_expression<ClassDef>(loc, loc, core::Symbols::root(), MK::EmptyTree(), move(ancestors), move(rhs),
                                     ClassDef::Kind::Class);
}
} // namespace

ExpressionPtr node2Tree(core::MutableContext ctx, unique_ptr<parser::Node> what, bool preserveConcreteSyntax) {
    try {
        uint32_t uniqueCounter = 1;
        // We don't have an enclosing block arg to start off.
        DesugarContext dctx(ctx, uniqueCounter, core::NameRef::noName(), core::LocOffsets::none(),
                            core::NameRef::noName(), false, false, preserveConcreteSyntax);
        auto liftedClassDefLoc = what->loc;
        auto result = node2TreeImpl(dctx, what);
        if (result.loc().exists()) {
            // If the desugared expression has a different loc, we want to use that. This can happen
            // because (:block (:send)) desugars to (:send (:block)), but the (:block) node just has
            // the loc of the `do ... end`, while the (:send) has the whole loc
            //
            // But if we desugared to EmptyTree (either intentionally or because there was an
            // unsupported node type), we want to use the loc of the original node.
            liftedClassDefLoc = result.loc();
        }
        result = liftTopLevel(dctx, liftedClassDefLoc, move(result));
        auto verifiedResult = Verifier::run(ctx, move(result));
        return verifiedResult;
    } catch (SorbetException &) {
        locReported = false;
        throw;
    }
}
} // namespace sorbet::ast::prismDesugar
