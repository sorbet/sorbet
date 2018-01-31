#include "builder.h"
#include "core/Names/cfg.h"

#include <algorithm> // sort
#include <unordered_map>

using namespace std;

namespace ruby_typer {
namespace cfg {

void jumpToDead(BasicBlock *from, CFG &inWhat, core::Loc loc);

unique_ptr<CFG> CFGBuilder::buildFor(const core::Context ctx, ast::MethodDef &md) {
    unique_ptr<CFG> res(new CFG); // private constructor
    res->symbol = md.symbol;
    core::LocalVariable retSym = ctx.state.newTemporary(core::Names::returnMethodTemp(), md.symbol);
    core::LocalVariable selfSym = ctx.state.newTemporary(core::Names::selfMethodTemp(), md.symbol);

    BasicBlock *entry = res->entry();

    entry->exprs.emplace_back(selfSym, md.loc, make_unique<Self>(md.symbol.data(ctx).owner));
    auto methodName = md.symbol.data(ctx).name;

    int i = -1;
    std::unordered_map<core::SymbolRef, core::LocalVariable> aliases;
    for (core::SymbolRef argSym : md.symbol.data(ctx).arguments()) {
        i++;
        core::LocalVariable arg(argSym.data(ctx).name, 0);
        entry->exprs.emplace_back(arg, argSym.data(ctx).definitionLoc, make_unique<LoadArg>(selfSym, methodName, i));
        aliases[argSym] = arg;
    }
    auto cont = walk(CFGContext(ctx, *res.get(), retSym, 0, nullptr, nullptr, nullptr, aliases), md.rhs.get(), entry);
    core::LocalVariable retSym1(core::Names::finalReturn(), 0);

    auto rvLoc = cont->exprs.empty() ? md.rhs->loc : cont->exprs.back().loc;
    cont->exprs.emplace_back(retSym1, rvLoc, make_unique<Return>(retSym)); // dead assign.
    jumpToDead(cont, *res.get(), rvLoc);

    std::vector<Binding> aliasesPrefix;
    for (auto kv : aliases) {
        core::SymbolRef global = kv.first;
        core::LocalVariable local = kv.second;
        if (global.data(ctx).isMethodArgument()) {
            res->minLoops[local] = 0; // method arguments are pinned only in loops
        } else {
            aliasesPrefix.emplace_back(local, md.symbol.data(ctx).definitionLoc, make_unique<Alias>(global));
            res->minLoops[local] = -1; // globals are pinned always
        }
    }
    histogramInc("cfgbuilder.aliases", aliasesPrefix.size());
    auto basicBlockCreated = res->basicBlocks.size();
    histogramInc("cfgbuilder.basicBlocksCreated", basicBlockCreated);
    std::sort(aliasesPrefix.begin(), aliasesPrefix.end(),
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
    histogramInc("cfgbuilder.basicBlocksSimplified", basicBlockCreated - res->basicBlocks.size());
    markLoopHeaders(ctx, *res);
    sanityCheck(ctx, *res);
    res->sanityCheck(ctx);
    return res;
}

void CFGBuilder::fillInTopoSorts(const core::Context ctx, CFG &cfg) {
    // needed to find loop headers.
    for (auto &bb : cfg.basicBlocks) {
        std::sort(bb->backEdges.begin(), bb->backEdges.end(),
                  [](const BasicBlock *a, const BasicBlock *b) -> bool { return a->outerLoops < b->outerLoops; });
    }

    auto &target1 = cfg.forwardsTopoSort;
    target1.resize(cfg.basicBlocks.size());
    int count = topoSortFwd(target1, 0, cfg.entry());
    cfg.forwardsTopoSort.resize(count);

    // Remove unreachable blocks (which were not found by the toposort)
    for (auto &bb : cfg.basicBlocks) {
        if ((bb->flags & CFG::FORWARD_TOPO_SORT_VISITED) == 0) {
            for (auto &prev : bb->backEdges) {
                ENFORCE((prev->flags & CFG::FORWARD_TOPO_SORT_VISITED) == 0);
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
                                    [](auto &bb) -> bool { return (bb->flags & CFG::FORWARD_TOPO_SORT_VISITED) == 0; }),
                          cfg.basicBlocks.end());

    auto &target2 = cfg.backwardsTopoSort;
    target2.resize(cfg.basicBlocks.size());
    count = topoSortBwd(target2, 0, cfg.deadBlock());
    ENFORCE(count == cfg.basicBlocks.size(),
            "Some block was unreachble from the bottom. Total blocks: ", cfg.basicBlocks.size(),
            ", Reachable blocks: ", count);
    return;
}

CFGContext CFGContext::withTarget(core::LocalVariable target) {
    auto ret = CFGContext(*this);
    ret.target = target;
    return ret;
}

CFGContext CFGContext::withLoopScope(BasicBlock *nextScope, BasicBlock *breakScope) {
    auto ret = CFGContext(*this);
    ret.nextScope = nextScope;
    ret.breakScope = breakScope;
    ret.loops += 1;
    return ret;
}

unique_ptr<CFG> CFGBuilder::addDebugEnvironment(core::Context ctx, unique_ptr<CFG> cfg) {
    for (auto *bb : cfg->backwardsTopoSort) {
        if (bb->exprs.empty()) {
            continue;
        }
        core::LocalVariable bind = ctx.state.newTemporary(core::Names::debugEnvironmentTemp(), cfg->symbol);
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
} // namespace ruby_typer
