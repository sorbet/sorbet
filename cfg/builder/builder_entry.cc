#include "ast/Helpers.h"
#include "cfg/builder/builder.h"
#include "common/sort/sort.h"
#include "core/Names.h"

using namespace std;

namespace sorbet::cfg {

namespace {

bool shouldBuildFor(core::Context ctx, const ast::ExpressionPtr &what) {
    if (ast::isa_tree<ast::MethodDef>(what)) {
        return false;
    }
    if (ast::isa_tree<ast::ClassDef>(what)) {
        return false;
    }
    if (ast::isa_tree<ast::EmptyTree>(what)) {
        return false;
    }

    if (auto asgn = ast::cast_tree<ast::Assign>(what)) {
        return !ast::isa_tree<ast::UnresolvedConstantLit>(asgn->lhs);
    }

    return true;
}

} // namespace

unique_ptr<CFG> CFGBuilder::buildFor(core::Context ctx, const ast::MethodDef &md) {
    ENFORCE(md.symbol.exists());
    ENFORCE(!md.symbol.data(ctx)->flags.isOverloaded);

    unique_ptr<CFG> res(new CFG); // private constructor
    res->loc = md.loc;
    res->declLoc = md.declLoc;
    res->symbol = md.symbol.data(ctx)->dealiasMethod(ctx);

    UnorderedMap<core::SymbolRef, LocalRef> aliases;
    UnorderedMap<core::NameRef, LocalRef> discoveredUndeclaredFields;
    uint32_t temporaryCounter = 1;
    CFGContext cctx(ctx, *res.get(), LocalRef::noVariable(), 0, nullptr, nullptr, nullptr, aliases,
                    discoveredUndeclaredFields, temporaryCounter);

    return buildFor(cctx, move(res), md.params, {}, md.rhs);
}

unique_ptr<CFG> CFGBuilder::buildFor(core::Context ctx, const ast::ClassDef &cd, core::MethodRef symbol) {
    unique_ptr<CFG> res(new CFG); // private constructor
    res->loc = cd.loc;
    res->declLoc = cd.declLoc;
    res->symbol = symbol;

    UnorderedMap<core::SymbolRef, LocalRef> aliases;
    UnorderedMap<core::NameRef, LocalRef> discoveredUndeclaredFields;
    uint32_t temporaryCounter = 1;
    CFGContext cctx(ctx, *res.get(), LocalRef::noVariable(), 0, nullptr, nullptr, nullptr, aliases,
                    discoveredUndeclaredFields, temporaryCounter);

    // Synthesize a block argument for this <static-init> block. This is rather fiddly,
    // because we have to know exactly what invariants desugar and namer set up about
    // methods and block arguments before us.
    auto blkLoc = core::LocOffsets::none();
    core::LocalVariable blkLocalVar(core::Names::blkArg(), 0);
    ast::MethodDef::PARAMS_store params;
    params.emplace_back(ast::make_expression<ast::Local>(blkLoc, blkLocalVar));

    return buildFor(cctx, move(res), params, cd.rhs, ast::MK::EmptyTree());
}

unique_ptr<CFG> CFGBuilder::buildFor(CFGContext cctx, unique_ptr<CFG> res, absl::Span<const ast::ExpressionPtr> params,
                                     absl::Span<const ast::ExpressionPtr> stats, const ast::ExpressionPtr &expr) {
    auto ctx = cctx.ctx;
    Timer timeit(ctx.state.tracer(), "cfg");

    LocalRef retSym;
    BasicBlock *entry = res->entry();
    BasicBlock *cont;
    {
        CFG::UnfreezeCFGLocalVariables unfreezeVars(*res);
        retSym = cctx.newTemporary(core::Names::returnMethodTemp());

        auto selfClaz = res->symbol.data(ctx)->rebind;
        if (!selfClaz.exists()) {
            selfClaz = res->symbol.data(ctx)->owner;
        }
        synthesizeExpr(entry, LocalRef::selfVariable(), res->declLoc.copyWithZeroLength(),
                       make_insn<Cast>(LocalRef::selfVariable(), res->declLoc.copyWithZeroLength(),
                                       selfClaz.data(ctx)->selfType(ctx), core::Names::cast()));

        BasicBlock *presentCont = entry;
        BasicBlock *defaultCont = nullptr;

        auto &paramInfos = res->symbol.data(ctx)->parameters;
        bool isAbstract = res->symbol.data(ctx)->flags.isAbstract;
        bool seenKeyword = false;
        int i = -1;
        for (auto &paramExpr : params) {
            i++;
            auto *p = ast::MK::arg2Local(paramExpr);
            auto local = res->enterLocal(p->localVariable);
            auto &paramInfo = paramInfos[i];

            seenKeyword = seenKeyword || paramInfo.flags.isKeyword;

            // If defaultCont is non-null, that means that the previous argument had a default. If the current argument
            // has a default and also is not a keyword, block or repeated arg, then we can continue by extending that
            // fall-through case. However if any of those conditions fail, we must merge the two paths back together,
            // and break out of the fast-path for defaulting.
            if (defaultCont &&
                (seenKeyword || paramInfo.flags.isBlock || paramInfo.flags.isRepeated || !paramInfo.flags.isDefault)) {
                presentCont = joinBlocks(cctx, presentCont, defaultCont);
                defaultCont = nullptr;
            }

            // Ignore defaults for abstract methods, because abstract methods do not have bodies and are not called.
            if (!isAbstract) {
                // Only emit conditional arg loading if the arg has a default
                if (auto opt = ast::cast_tree<ast::OptionalParam>(paramExpr)) {
                    auto [result, presentNext, defaultNext] =
                        walkDefault(cctx, i, paramInfo, local, p->loc, opt->default_, presentCont, defaultCont);

                    synthesizeExpr(defaultNext, local, p->loc, make_insn<Ident>(result));

                    presentCont = presentNext;
                    defaultCont = defaultNext;
                }
            }

            synthesizeExpr(presentCont, local, p->loc, make_insn<LoadArg>(res->symbol, i));
        }

        // Join the presentCont and defaultCont paths together
        if (defaultCont) {
            presentCont = joinBlocks(cctx, presentCont, defaultCont);
        }

        cont = presentCont;

        auto shouldBuildForStats = vector<bool>(stats.size(), false);
        int lastStat = -1;
        for (int i = 0; i < stats.size(); i++) {
            auto shouldBuildForStat = shouldBuildFor(cctx.ctx, stats[i]);
            shouldBuildForStats[i] = shouldBuildForStat;
            if (shouldBuildForStat) {
                lastStat = i;
            }
        }

        if (lastStat != -1) {
            for (int i = 0; i < lastStat; i++) {
                if (shouldBuildForStats[i]) {
                    cont = walk(cctx.withTarget(cctx.newTemporary(core::Names::statTemp())), stats[i], cont);
                }
            }
            cont = walk(cctx.withTarget(retSym), stats[lastStat], cont);
        } else {
            cont = walk(cctx.withTarget(retSym), expr, cont);
        }
    }
    // Past this point, res->localVariables is a fixed size.

    LocalRef retSym1 = LocalRef::finalReturn();

    core::LocOffsets rvLoc;
    if (cont->exprs.empty() || isa_instruction<LoadArg>(cont->exprs.back().value)) {
        auto beginAdjust = res->loc.length() - 3;
        auto endLoc = ctx.locAt(res->loc).adjust(ctx, beginAdjust, 0);
        if (endLoc.source(ctx) == "end") {
            rvLoc = endLoc.offsets();
            res->implicitReturnLoc = rvLoc;
        } else {
            rvLoc = res->loc;
        }
    } else {
        rvLoc = cont->exprs.back().loc;
    }
    synthesizeExpr(cont, retSym1, rvLoc, make_insn<Return>(retSym, rvLoc)); // dead assign.
    jumpToDead(cont, *res.get(), rvLoc);

    vector<Binding> aliasesPrefix;
    for (auto kv : cctx.aliases) {
        core::SymbolRef global = kv.first;
        LocalRef local = kv.second;
        aliasesPrefix.emplace_back(local, core::LocOffsets::none(), make_insn<Alias>(global));
        if (global.isFieldOrStaticField()) {
            res->minLoops[local.id()] = CFG::MIN_LOOP_FIELD;
        } else {
            // We used to have special handling here for "MIN_LOOP_GLOBAL" but it was meaningless,
            // because it only happened for type members, and we already prohibit re-assigning type
            // members (in namer). If this ENFORCE fails, we might have to resurrect the old logic
            // we had for handling MIN_LOOP_GLOBAL (or at least, add some tests that would trigger
            // pinning errors).
            ENFORCE(global.isTypeMember());
        }
    }
    for (auto kv : cctx.discoveredUndeclaredFields) {
        aliasesPrefix.emplace_back(kv.second, core::LocOffsets::none(),
                                   make_insn<Alias>(core::Symbols::Magic_undeclaredFieldStub(), kv.first));
        res->minLoops[kv.second.id()] = CFG::MIN_LOOP_FIELD;
    }
    histogramInc("cfgbuilder.aliases", aliasesPrefix.size());
    auto basicBlockCreated = res->basicBlocks.size();
    histogramInc("cfgbuilder.basicBlocksCreated", basicBlockCreated);
    fast_sort(aliasesPrefix,
              [](const Binding &l, const Binding &r) -> bool { return l.bind.variable.id() < r.bind.variable.id(); });

    entry->exprs.insert(entry->exprs.begin(), make_move_iterator(aliasesPrefix.begin()),
                        make_move_iterator(aliasesPrefix.end()));
    res->sanityCheck(ctx);
    sanityCheck(ctx, *res);
    fillInTopoSorts(ctx, *res);
    dealias(ctx, *res);
    CFG::ReadsAndWrites RnW = res->findAllReadsAndWrites(ctx);
    computeMinMaxLoops(ctx, RnW, *res);
    auto blockArgs = fillInBlockArguments(ctx, RnW, *res);
    removeDeadAssigns(ctx, RnW, *res, blockArgs); // requires block arguments to be filled
    simplify(ctx, *res);
    histogramInc("cfgbuilder.basicBlocksSimplified", basicBlockCreated - res->basicBlocks.size());
    markLoopHeaders(ctx, *res);
    sanityCheck(ctx, *res);
    res->sanityCheck(ctx);
    histogramInc("cfgbuilder.numLocalVariables", res->numLocalVariables());
    return res;
}

void CFGBuilder::fillInTopoSorts(core::Context ctx, CFG &cfg) {
    // A map from the index space of the basicBlocks index to forwardsTopoSort index.
    auto forwardsIds = topoSortFwd(cfg.forwardsTopoSort, cfg.maxBasicBlockId, cfg.entry());

    // Remove unreachable blocks (which were not found by the toposort)
    for (auto &bb : cfg.basicBlocks) {
        if (forwardsIds[bb->id] == -1) {
            for (auto &prev : bb->backEdges) {
                ENFORCE(forwardsIds[prev->id] == -1);
            }

            bb->bexit.thenb->backEdges.erase(
                remove(bb->bexit.thenb->backEdges.begin(), bb->bexit.thenb->backEdges.end(), bb.get()),
                bb->bexit.thenb->backEdges.end());
            bb->bexit.elseb->backEdges.erase(
                remove(bb->bexit.elseb->backEdges.begin(), bb->bexit.elseb->backEdges.end(), bb.get()),
                bb->bexit.elseb->backEdges.end());
        }
    }
    cfg.basicBlocks.erase(remove_if(cfg.basicBlocks.begin(), cfg.basicBlocks.end(),
                                    [&forwardsIds](auto &bb) -> bool { return forwardsIds[bb->id] == -1; }),
                          cfg.basicBlocks.end());

    // needed to find loop headers.
    for (auto &bb : cfg.basicBlocks) {
        fast_sort(bb->backEdges, [&forwardsIds](const BasicBlock *a, const BasicBlock *b) -> bool {
            return forwardsIds[a->id] > forwardsIds[b->id];
        });
    }
}

CFGContext CFGContext::withTarget(LocalRef target) {
    auto ret = CFGContext(*this);
    ret.target = target;
    return ret;
}

CFGContext CFGContext::withBlockBreakTarget(LocalRef blockBreakTarget) {
    auto ret = CFGContext(*this);
    ret.blockBreakTarget = blockBreakTarget;
    ret.breakIsJump = false;
    return ret;
}

CFGContext CFGContext::withLoopBreakTarget(LocalRef blockBreakTarget) {
    auto ret = CFGContext(*this);
    ret.blockBreakTarget = blockBreakTarget;
    ret.breakIsJump = true;
    return ret;
}

CFGContext CFGContext::withLoopScope(BasicBlock *nextScope, BasicBlock *breakScope, bool insideRubyBlock) {
    auto ret = CFGContext(*this);
    ret.nextScope = nextScope;
    ret.breakScope = breakScope;
    ret.isInsideRubyBlock = insideRubyBlock;
    ret.loops += 1;
    return ret;
}

CFGContext CFGContext::withSendAndBlockLink(shared_ptr<core::SendAndBlockLink> &link) {
    auto ret = CFGContext(*this);
    ret.link = &link;
    return ret;
}

} // namespace sorbet::cfg
