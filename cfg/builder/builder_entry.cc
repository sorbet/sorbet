#include "builder.h"
#include "core/Names/cfg.h"

#include <algorithm> // sort
#include <unordered_map>

using namespace std;

namespace sorbet {
namespace cfg {

void jumpToDead(BasicBlock *from, CFG &inWhat, core::Loc loc);

unique_ptr<CFG> CFGBuilder::buildFor(core::Context ctx, ast::MethodDef &md) {
    ENFORCE(md.symbol.exists());
    ENFORCE(!md.symbol.data(ctx).isOverloaded());
    unique_ptr<CFG> res(new CFG); // private constructor
    res->symbol = md.symbol;
    if (res->symbol.data(ctx).isAbstract()) {
        res->basicBlocks.clear();
        return res;
    }
    u4 temporaryCounter = 1;
    unordered_map<core::SymbolRef, core::LocalVariable> aliases;
    CFGContext cctx(ctx, *res.get(), core::LocalVariable(), 0, nullptr, nullptr, nullptr, aliases, temporaryCounter);

    core::LocalVariable retSym = cctx.newTemporary(core::Names::returnMethodTemp());
    core::LocalVariable selfSym = cctx.newTemporary(core::Names::selfMethodTemp());

    BasicBlock *entry = res->entry();

    entry->exprs.emplace_back(selfSym, md.loc, make_unique<Self>(md.symbol.data(ctx).owner));
    auto methodName = md.symbol.data(ctx).name;

    int i = -1;
    for (auto &argExpr : md.args) {
        i++;
        auto *a = ast::cast_tree<ast::Local>(argExpr.get());
        ENFORCE(a, "Namer did not do replace arguments with locals");
        entry->exprs.emplace_back(a->localVariable, a->loc, make_unique<LoadArg>(selfSym, methodName, i));
        entry->exprs.back().value->isSynthetic = true;
        aliases[md.symbol.data(ctx).arguments()[i]] = a->localVariable;
    }
    auto cont = walk(cctx.withTarget(retSym), md.rhs.get(), entry);
    core::LocalVariable retSym1(core::Names::finalReturn(), 0);

    auto rvLoc = cont->exprs.empty() ? md.rhs->loc : cont->exprs.back().loc;
    cont->exprs.emplace_back(retSym1, rvLoc, make_unique<Return>(retSym)); // dead assign.
    cont->exprs.back().value->isSynthetic = true;
    jumpToDead(cont, *res.get(), rvLoc);

    vector<Binding> aliasesPrefix;
    for (auto kv : aliases) {
        core::SymbolRef global = kv.first;
        core::LocalVariable local = kv.second;
        if (global.data(ctx).isMethodArgument()) {
            res->minLoops[local] = 0; // method arguments are pinned only in loops
        } else {
            aliasesPrefix.emplace_back(local, global.data(ctx).loc(), make_unique<Alias>(global));
            if (global.data(ctx).isField() || global.data(ctx).isStaticField()) {
                res->minLoops[local] = CFG::MIN_LOOP_FIELD;
            } else {
                res->minLoops[local] = CFG::MIN_LOOP_GLOBAL;
            }
        }
    }
    core::histogramInc("cfgbuilder.aliases", aliasesPrefix.size());
    auto basicBlockCreated = res->basicBlocks.size();
    core::histogramInc("cfgbuilder.basicBlocksCreated", basicBlockCreated);
    sort(aliasesPrefix.begin(), aliasesPrefix.end(),
         [](const Binding &l, const Binding &r) -> bool { return l.bind < r.bind; });

    entry->exprs.insert(entry->exprs.begin(), make_move_iterator(aliasesPrefix.begin()),
                        make_move_iterator(aliasesPrefix.end()));
    res->sanityCheck(ctx);
    sanityCheck(ctx, *res);
    fillInTopoSorts(ctx, *res);
    dealias(ctx, *res);
    CFG::ReadsAndWrites RnW = res->findAllReadsAndWrites(ctx);
    computeMinMaxLoops(ctx, RnW, *res);
    removeDeadAssigns(ctx, RnW, *res);
    fillInBlockArguments(ctx, RnW, *res);
    simplify(ctx, *res);
    core::histogramInc("cfgbuilder.basicBlocksSimplified", basicBlockCreated - res->basicBlocks.size());
    markLoopHeaders(ctx, *res);
    sanityCheck(ctx, *res);
    res->sanityCheck(ctx);
    return res;
}

void CFGBuilder::fillInTopoSorts(core::Context ctx, CFG &cfg) {
    auto &target1 = cfg.forwardsTopoSort;
    target1.resize(cfg.basicBlocks.size());
    int count = topoSortFwd(target1, 0, cfg.entry());
    cfg.forwardsTopoSort.resize(count);

    // Remove unreachable blocks (which were not found by the toposort)
    for (auto &bb : cfg.basicBlocks) {
        if (bb->fwdId == -1) {
            for (auto &prev : bb->backEdges) {
                ENFORCE(prev->fwdId == -1);
            }

            bb->bexit.thenb->backEdges.erase(
                remove(bb->bexit.thenb->backEdges.begin(), bb->bexit.thenb->backEdges.end(), bb.get()),
                bb->bexit.thenb->backEdges.end());
            bb->bexit.elseb->backEdges.erase(
                remove(bb->bexit.elseb->backEdges.begin(), bb->bexit.elseb->backEdges.end(), bb.get()),
                bb->bexit.elseb->backEdges.end());
        }
    }
    cfg.basicBlocks.erase(
        remove_if(cfg.basicBlocks.begin(), cfg.basicBlocks.end(), [](auto &bb) -> bool { return bb->fwdId == -1; }),
        cfg.basicBlocks.end());

    // needed to find loop headers.
    for (auto &bb : cfg.basicBlocks) {
        sort(bb->backEdges.begin(), bb->backEdges.end(),
             [](const BasicBlock *a, const BasicBlock *b) -> bool { return a->fwdId > b->fwdId; });
    }
}

CFGContext CFGContext::withTarget(core::LocalVariable target) {
    auto ret = CFGContext(*this);
    ret.target = target;
    return ret;
}

CFGContext CFGContext::withLoopScope(BasicBlock *nextScope, BasicBlock *breakScope, core::SymbolRef rubyBlock) {
    auto ret = CFGContext(*this);
    ret.nextScope = nextScope;
    ret.breakScope = breakScope;
    ret.rubyBlock = rubyBlock;
    ret.loops += 1;
    return ret;
}

CFGContext CFGContext::withSendAndBlockLink(shared_ptr<core::SendAndBlockLink> link) {
    auto ret = CFGContext(*this);
    ret.link = move(link);
    return ret;
}

unique_ptr<CFG> CFGBuilder::addDebugEnvironment(core::Context ctx, unique_ptr<CFG> cfg) {
    for (auto &bb : cfg->basicBlocks) {
        if (bb->exprs.empty()) {
            continue;
        }
        core::LocalVariable bind(core::Names::debugEnvironmentTemp(), 0);
        auto &firstExpr = bb->exprs[0];
        bb->exprs.emplace(bb->exprs.begin(), bind, firstExpr.loc,
                          make_unique<cfg::DebugEnvironment>(core::GlobalState::AnnotationPos::BEFORE));

        auto &lastExpr = bb->exprs[bb->exprs.size() - 1];
        bb->exprs.emplace_back(bind, lastExpr.loc,
                               make_unique<cfg::DebugEnvironment>(core::GlobalState::AnnotationPos::AFTER));
    }
    return cfg;
}

} // namespace cfg
} // namespace sorbet
