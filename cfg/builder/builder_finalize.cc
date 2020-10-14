#include "cfg/builder/builder.h"
#include "cfg/sorted_vector_helpers.h"
#include "common/Timer.h"
#include "common/sort.h"
#include "core/Names.h"

#include <algorithm> // sort, remove, unique
#include <climits>   // INT_MAX
using namespace std;

namespace sorbet::cfg {

void CFGBuilder::simplify(core::Context ctx, CFG &cfg) {
    if (!ctx.state.lspQuery.isEmpty()) {
        return;
    }

    sanityCheck(ctx, cfg);
    bool changed = true;
    while (changed) {
        changed = false;
        for (auto it = cfg.basicBlocks.begin(); it != cfg.basicBlocks.end(); /*nothing*/) {
            BasicBlock *bb = it->get();
            auto *const thenb = bb->bexit.thenb;
            auto *const elseb = bb->bexit.elseb;
            if (bb != cfg.deadBlock() && bb != cfg.entry()) {
                if (bb->backEdges.empty()) { // remove non reachable
                    thenb->backEdges.erase(remove(thenb->backEdges.begin(), thenb->backEdges.end(), bb),
                                           thenb->backEdges.end());
                    if (elseb != thenb) {
                        elseb->backEdges.erase(remove(elseb->backEdges.begin(), elseb->backEdges.end(), bb),
                                               elseb->backEdges.end());
                    }
                    it = cfg.basicBlocks.erase(it);
                    cfg.forwardsTopoSort.erase(remove(cfg.forwardsTopoSort.begin(), cfg.forwardsTopoSort.end(), bb),
                                               cfg.forwardsTopoSort.end());
                    changed = true;
                    sanityCheck(ctx, cfg);
                    continue;
                }
            }

            // Dedupe back edges
            fast_sort(bb->backEdges,
                      [](const BasicBlock *bb1, const BasicBlock *bb2) -> bool { return bb1->id < bb2->id; });
            bb->backEdges.erase(unique(bb->backEdges.begin(), bb->backEdges.end()), bb->backEdges.end());

            if (thenb == elseb) {
                // Remove condition from unconditional jumps
                bb->bexit.cond = LocalRef::unconditional();
            }
            if (thenb == elseb && thenb != cfg.deadBlock() && thenb != bb &&
                bb->rubyBlockId == thenb->rubyBlockId) { // can be squashed togather
                if (thenb->backEdges.size() == 1 && thenb->outerLoops == bb->outerLoops) {
                    bb->exprs.insert(bb->exprs.end(), make_move_iterator(thenb->exprs.begin()),
                                     make_move_iterator(thenb->exprs.end()));
                    thenb->backEdges.clear();
                    bb->bexit.cond.variable = thenb->bexit.cond.variable;
                    bb->bexit.thenb = thenb->bexit.thenb;
                    bb->bexit.elseb = thenb->bexit.elseb;
                    bb->bexit.thenb->backEdges.emplace_back(bb);
                    if (bb->bexit.thenb != bb->bexit.elseb) {
                        bb->bexit.elseb->backEdges.emplace_back(bb);
                    }
                    changed = true;
                    sanityCheck(ctx, cfg);
                    continue;
                } else if (thenb->bexit.cond.variable != LocalRef::blockCall() && thenb->exprs.empty()) {
                    // Don't remove block headers
                    bb->bexit.cond.variable = thenb->bexit.cond.variable;
                    bb->bexit.thenb = thenb->bexit.thenb;
                    bb->bexit.elseb = thenb->bexit.elseb;
                    thenb->backEdges.erase(remove(thenb->backEdges.begin(), thenb->backEdges.end(), bb),
                                           thenb->backEdges.end());
                    bb->bexit.thenb->backEdges.emplace_back(bb);
                    if (bb->bexit.thenb != bb->bexit.elseb) {
                        bb->bexit.elseb->backEdges.emplace_back(bb);
                    }
                    changed = true;
                    sanityCheck(ctx, cfg);
                    continue;
                }
            }
            if (thenb != cfg.deadBlock() && bb->rubyBlockId == thenb->rubyBlockId && thenb->exprs.empty() &&
                thenb->bexit.thenb == thenb->bexit.elseb && bb->bexit.thenb != thenb->bexit.thenb) {
                // shortcut then
                bb->bexit.thenb = thenb->bexit.thenb;
                thenb->bexit.thenb->backEdges.emplace_back(bb);
                thenb->backEdges.erase(remove(thenb->backEdges.begin(), thenb->backEdges.end(), bb),
                                       thenb->backEdges.end());
                changed = true;
                sanityCheck(ctx, cfg);
                continue;
            }
            if (elseb != cfg.deadBlock() && bb->rubyBlockId == thenb->rubyBlockId && elseb->exprs.empty() &&
                elseb->bexit.thenb == elseb->bexit.elseb && bb->bexit.elseb != elseb->bexit.elseb) {
                // shortcut else
                sanityCheck(ctx, cfg);
                bb->bexit.elseb = elseb->bexit.elseb;
                bb->bexit.elseb->backEdges.emplace_back(bb);
                elseb->backEdges.erase(remove(elseb->backEdges.begin(), elseb->backEdges.end(), bb),
                                       elseb->backEdges.end());
                changed = true;
                sanityCheck(ctx, cfg);
                continue;
            }
            ++it;
        }
    }
}

void CFGBuilder::sanityCheck(core::Context ctx, CFG &cfg) {
    if (!debug_mode) {
        return;
    }
    for (auto &bb : cfg.basicBlocks) {
        for (auto parent : bb->backEdges) {
            ENFORCE(parent->bexit.thenb == bb.get() || parent->bexit.elseb == bb.get(),
                    "parent is not aware of a child");
        }
        if (bb.get() == cfg.deadBlock()) {
            continue;
        }
        if (bb.get() != cfg.entry()) {
            ENFORCE((bb->flags & CFG::WAS_JUMP_DESTINATION) != 0, "block {} was never linked into cfg", bb->id);
        }
        auto thenFnd = absl::c_find(bb->bexit.thenb->backEdges, bb.get());
        auto elseFnd = absl::c_find(bb->bexit.elseb->backEdges, bb.get());
        ENFORCE(thenFnd != bb->bexit.thenb->backEdges.end(), "backedge unset for thenb");
        ENFORCE(elseFnd != bb->bexit.elseb->backEdges.end(), "backedge unset for elseb");
    }
}

LocalRef maybeDealias(core::Context ctx, CFG &cfg, LocalRef what, UnorderedMap<LocalRef, LocalRef> &aliases) {
    if (what.isSyntheticTemporary(ctx, cfg)) {
        auto fnd = aliases.find(what);
        if (fnd != aliases.end()) {
            return fnd->second;
        }
    }
    return what;
}

/**
 * Remove aliases from CFG. Why does this need a separate pass?
 * because `a.foo(a = "2", if (...) a = true; else a = null; end)`
 */
void CFGBuilder::dealias(core::Context ctx, CFG &cfg) {
    vector<UnorderedMap<LocalRef, LocalRef>> outAliases;
    outAliases.resize(cfg.maxBasicBlockId);

    for (auto it = cfg.forwardsTopoSort.rbegin(); it != cfg.forwardsTopoSort.rend(); ++it) {
        auto &bb = *it;
        if (bb == cfg.deadBlock()) {
            continue;
        }
        auto &current = outAliases[bb->id];

        if (!bb->backEdges.empty()) {
            // Take the intersection of all of the back edges' aliases.
            current = outAliases[bb->backEdges[0]->id];
            for (auto i = 1; i < bb->backEdges.size(); i++) {
                auto *parent = bb->backEdges[i];
                const auto &other = outAliases[parent->id];
                for (auto it = current.begin(); it != current.end(); /* nothing */) {
                    auto &el = *it;
                    auto fnd = other.find(el.first);
                    if (fnd != other.end()) {
                        if (fnd->second != el.second) {
                            current.erase(it++);
                        } else {
                            ++it;
                        }
                    } else {
                        // note: this is correct but too conservative. In particular for loop headers
                        current.erase(it++);
                    }
                }
            }
        }

        // Overapproximation of the set of variables that have aliases.
        // Will have false positives, but no false negatives. Avoids an expensive inner loop below.
        UnorderedSet<LocalRef> mayHaveAlias;
        for (auto &alias : current) {
            mayHaveAlias.insert(alias.second);
        }

        for (Binding &bind : bb->exprs) {
            if (auto *i = cast_instruction<Ident>(bind.value.get())) {
                i->what = maybeDealias(ctx, cfg, i->what, current);
            }
            if (mayHaveAlias.contains(bind.bind.variable)) {
                /* invalidate a stale record (uncommon) */
                for (auto it = current.begin(); it != current.end(); /* nothing */) {
                    if (it->second == bind.bind.variable) {
                        current.erase(it++);
                    } else {
                        ++it;
                    }
                }
            }

            /* dealias */
            if (!bind.value->isSynthetic) {
                // we don't allow dealiasing values into synthetic instructions
                // as otherwise it fools dead code analysis.
                if (auto *v = cast_instruction<Ident>(bind.value.get())) {
                    v->what = maybeDealias(ctx, cfg, v->what, current);
                } else if (auto *v = cast_instruction<Send>(bind.value.get())) {
                    v->recv = maybeDealias(ctx, cfg, v->recv.variable, current);
                    for (auto &arg : v->args) {
                        arg = maybeDealias(ctx, cfg, arg.variable, current);
                    }
                } else if (auto *v = cast_instruction<TAbsurd>(bind.value.get())) {
                    v->what = maybeDealias(ctx, cfg, v->what.variable, current);
                } else if (auto *v = cast_instruction<Return>(bind.value.get())) {
                    v->what = maybeDealias(ctx, cfg, v->what.variable, current);
                }
            }

            // record new aliases
            if (auto *i = cast_instruction<Ident>(bind.value.get())) {
                current[bind.bind.variable] = i->what;
                mayHaveAlias.insert(i->what);
            }
        }
        if (bb->bexit.cond.variable != LocalRef::unconditional()) {
            bb->bexit.cond = maybeDealias(ctx, cfg, bb->bexit.cond.variable, current);
        }
    }
}

void CFGBuilder::markLoopHeaders(core::Context ctx, CFG &cfg) {
    for (unique_ptr<BasicBlock> &bb : cfg.basicBlocks) {
        for (auto *parent : bb->backEdges) {
            if (parent->outerLoops < bb->outerLoops) {
                bb->flags |= CFG::LOOP_HEADER;
                continue;
            }
        }
    }
}
void CFGBuilder::removeDeadAssigns(core::Context ctx, const CFG::ReadsAndWrites &RnW, CFG &cfg) {
    if (!ctx.state.lspQuery.isEmpty()) {
        return;
    }

    Timer timeit(ctx.state.tracer(), "removeDeadAssigns");
    for (auto &it : cfg.basicBlocks) {
        /* remove dead variables */
        for (auto expIt = it->exprs.begin(); expIt != it->exprs.end(); /* nothing */) {
            Binding &bind = *expIt;
            if (bind.bind.variable.isAliasForGlobal(ctx, cfg)) {
                ++expIt;
                continue;
            }

            bool wasRead = RnW.readsSet[it->id].contains(bind.bind.variable.id()); // read in the same block

            if (!wasRead) {
                // args are in LocalRef variable order
                auto argIt = absl::c_lower_bound(it->bexit.thenb->args, bind.bind, [](auto &a, auto &b) -> bool {
                    return a.variable.id() < b.variable.id();
                });
                if (argIt != it->bexit.thenb->args.end() && argIt->variable == bind.bind.variable) {
                    wasRead = true;
                }
            }
            if (!wasRead) {
                // args are in LocalRef variable order
                auto argIt = absl::c_lower_bound(it->bexit.elseb->args, bind.bind, [](auto &a, auto &b) -> bool {
                    return a.variable.id() < b.variable.id();
                });
                if (argIt != it->bexit.elseb->args.end() && argIt->variable == bind.bind.variable) {
                    wasRead = true;
                }
            }
            if (!wasRead) {
                // These are all instructions with no side effects, which can be
                // deleted if the assignment is dead. It would be slightly
                // shorter to list the converse set -- those which *do* have
                // side effects -- but doing it this way is more robust to us
                // adding more instruction types in the future.
                if (isa_instruction<Ident>(bind.value.get()) || isa_instruction<Literal>(bind.value.get()) ||
                    isa_instruction<LoadSelf>(bind.value.get()) || isa_instruction<LoadArg>(bind.value.get()) ||
                    isa_instruction<LoadYieldParams>(bind.value.get())) {
                    expIt = it->exprs.erase(expIt);
                } else {
                    ++expIt;
                }
            } else {
                ++expIt;
            }
        }
    }
}

void CFGBuilder::computeMinMaxLoops(core::Context ctx, const CFG::ReadsAndWrites &RnW, CFG &cfg) {
    for (const auto &bb : cfg.basicBlocks) {
        if (bb.get() == cfg.deadBlock()) {
            continue;
        }

        for (auto local : RnW.reads[bb->id]) {
            auto curMin = cfg.minLoops[local];
            if (curMin > bb->outerLoops) {
                curMin = bb->outerLoops;
            }
            cfg.minLoops[local] = curMin;
        }
    }
    for (const auto &bb : cfg.basicBlocks) {
        if (bb.get() == cfg.deadBlock()) {
            continue;
        }

        for (const auto &expr : bb->exprs) {
            auto what = expr.bind.variable;
            int curMin = cfg.minLoops[what.id()];
            int curMax = cfg.maxLoopWrite[what.id()];
            if (curMin > bb->outerLoops) {
                curMin = bb->outerLoops;
            }
            if (curMax < bb->outerLoops) {
                curMax = bb->outerLoops;
            }
            cfg.minLoops[what.id()] = curMin;
            cfg.maxLoopWrite[what.id()] = curMax;
        }
    }
}

void CFGBuilder::fillInBlockArguments(core::Context ctx, const CFG::ReadsAndWrites &RnW, CFG &cfg) {
    // Dmitry's algorithm for adding basic block arguments
    // I don't remember this version being described in any book.
    //
    // Compute two upper bounds:
    //  - one by accumulating all reads on the reverse graph
    //  - one by accumulating all writes on direct graph
    //
    //  every node gets the intersection between two sets suggested by those overestimations.
    //
    // This solution is  (|BB| + |symbols-mentioned|) * (|cycles|) + |answer_size| in complexity.
    // making this quadratic in anything will be bad.

    const vector<vector<int>> &readsByBlock = RnW.reads;
    const vector<vector<int>> &writesByBlock = RnW.writes;
    const vector<vector<int>> &deadByBlock = RnW.dead;

    // iterate over basic blocks in reverse and found upper bounds on what could a block need.
    vector<vector<int>> upperBounds1;
    bool changed = true;
    {
        Timer timeit(ctx.state.tracer(), "upperBounds1");
        upperBounds1 = readsByBlock;
        while (changed) {
            changed = false;
            for (BasicBlock *bb : cfg.forwardsTopoSort) {
                auto &upperBoundsForBlock = upperBounds1[bb->id];
                const auto sz = upperBoundsForBlock.size();
                if (bb->bexit.thenb != cfg.deadBlock()) {
                    SortedVectorHelpers::setUnionInplace(upperBoundsForBlock, upperBounds1[bb->bexit.thenb->id]);
                }
                if (bb->bexit.elseb != cfg.deadBlock()) {
                    SortedVectorHelpers::setUnionInplace(upperBoundsForBlock, upperBounds1[bb->bexit.elseb->id]);
                }

                // Any variable that we write and do not read is dead on entry to
                // this block, and we do not require it.
                const auto &deadForBlock = deadByBlock[bb->id];
                if (!deadForBlock.empty()) {
                    vector<int> toRemove;
                    for (auto local : deadForBlock) {
                        // TODO(nelhage) We can't erase for variables inside loops, due
                        // to how our "pinning" type inference works. We can remove this
                        // inner condition when we get a better type inference
                        // algorithm.
                        if (bb->outerLoops <= cfg.minLoops[local]) {
                            toRemove.emplace_back(local);
                        }
                    }
                    SortedVectorHelpers::setDifferenceInplace(upperBoundsForBlock, toRemove);
                }

                changed = changed || (upperBoundsForBlock.size() != sz);
            }
        }
    }

    vector<vector<int>> upperBounds2(cfg.maxBasicBlockId);

    changed = true;
    {
        Timer timeit(ctx.state.tracer(), "upperBounds2");
        while (changed) {
            changed = false;
            for (auto it = cfg.forwardsTopoSort.rbegin(); it != cfg.forwardsTopoSort.rend(); ++it) {
                BasicBlock *bb = *it;
                auto &upperBoundsForBlock = upperBounds2[bb->id];
                const auto sz = upperBoundsForBlock.size();
                for (BasicBlock *edge : bb->backEdges) {
                    if (edge != cfg.deadBlock()) {
                        SortedVectorHelpers::setUnionInplace(upperBoundsForBlock, writesByBlock[edge->id]);
                        SortedVectorHelpers::setUnionInplace(upperBoundsForBlock, upperBounds2[edge->id]);
                    }
                }

                changed = changed || sz != upperBoundsForBlock.size();
            }
        }
    }
    {
        Timer timeit(ctx.state.tracer(), "upperBoundsMerge");
        /** Combine two upper bounds */
        for (auto &it : cfg.basicBlocks) {
            const auto &set1 = upperBounds1[it->id];
            const auto &set2 = upperBounds2[it->id];
            auto set1It = set1.begin();
            auto set2It = set2.begin();
            // Note: The loop enqueues arguments in sorted order. We assume that args is empty so we don't need to sort.
            ENFORCE_NO_TIMER(it->args.empty());
            while (set1It != set1.end() && set2It != set2.end()) {
                const auto set1El = *set1It;
                const auto set2El = *set2It;
                if (set1El == set2El) {
                    it->args.emplace_back(set1El);
                    set1It++;
                    set2It++;
                } else if (set1El < set2El) {
                    set1It++;
                } else {
                    set2It++;
                }
            }
            // it->args is now sorted in LocalRef ID order.
            ENFORCE(absl::c_is_sorted(it->args,
                                      [](auto &a, auto &b) -> bool { return a.variable.id() < b.variable.id(); }));
            histogramInc("cfgbuilder.blockArguments", it->args.size());
        }
    }
}

int CFGBuilder::topoSortFwd(vector<BasicBlock *> &target, int nextFree, BasicBlock *currentBB) {
    // ENFORCE(!marked[currentBB]) // graph is cyclic!
    if (currentBB->fwdId != -1) {
        return nextFree;
    } else {
        currentBB->fwdId = -2;
        if (currentBB->bexit.thenb->outerLoops > currentBB->bexit.elseb->outerLoops) {
            nextFree = topoSortFwd(target, nextFree, currentBB->bexit.elseb);
            nextFree = topoSortFwd(target, nextFree, currentBB->bexit.thenb);
        } else {
            nextFree = topoSortFwd(target, nextFree, currentBB->bexit.thenb);
            nextFree = topoSortFwd(target, nextFree, currentBB->bexit.elseb);
        }
        target[nextFree] = currentBB;
        currentBB->fwdId = nextFree;
        return nextFree + 1;
    }
}
} // namespace sorbet::cfg
