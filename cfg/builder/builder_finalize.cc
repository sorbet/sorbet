#include "cfg/builder/builder.h"
#include "common/UIntSetForEach.h"
#include "common/sort/sort.h"
#include "common/timers/Timer.h"
#include "core/Names.h"

#include <algorithm> // sort, remove, unique
#include <climits>   // INT_MAX
using namespace std;

namespace sorbet::cfg {

void CFGBuilder::simplify(core::Context ctx, CFG &cfg) {
    sanityCheck(ctx, cfg);
    bool changed = true;
    while (changed) {
        changed = false;
        for (auto it = cfg.basicBlocks.begin(); it != cfg.basicBlocks.end(); /*nothing*/) {
            BasicBlock *bb = it->get();
            auto *const thenb = bb->bexit.thenb;
            auto *const elseb = bb->bexit.elseb;

            // Only consider removing nodes that aren't the entry or dead block.
            if (bb != cfg.deadBlock() && bb != cfg.entry()) {
                // When nothing jumps to this block, we can remove it.
                if (bb->backEdges.empty()) {
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

                // These two simplifications only apply on unconditional jumps that:
                //
                // 1. Are not to the dead block
                // 2. Are not a back edge to the parent
                // 3. Do not differ in their outerLoops values
                //
                // The third condition is important to avoid situations where we might duplicate a loop header.
                // Duplicating a loop header will potentially invalidate the cached `forwardsTopoSort` traversal, and
                // throw off inference by causing values to look nilable after loops that don't use them.
                if (thenb != cfg.deadBlock() && thenb != bb && thenb->outerLoops == bb->outerLoops) {
                    // If this is the only block that jumps to `thenb`, we can squish it into `bb` and disconnect it
                    // from the graph.
                    if (thenb->backEdges.size() == 1) {
                        bb->exprs.insert(bb->exprs.end(), make_move_iterator(thenb->exprs.begin()),
                                         make_move_iterator(thenb->exprs.end()));
                        thenb->backEdges.clear();
                        bb->bexit.cond.variable = thenb->bexit.cond.variable;
                        bb->bexit.thenb = thenb->bexit.thenb;
                        bb->bexit.elseb = thenb->bexit.elseb;
                        bb->bexit.loc = thenb->bexit.loc;
                        bb->bexit.thenb->backEdges.emplace_back(bb);
                        if (bb->bexit.thenb != bb->bexit.elseb) {
                            bb->bexit.elseb->backEdges.emplace_back(bb);
                        }
                        changed = true;
                        sanityCheck(ctx, cfg);
                        continue;
                    }

                    // `thenb` is a candidate for having its condition inlined into `bb` when:
                    //
                    // 1. Its condition is not `blockCall`, indicating that the `thenb` branch is actually the block
                    //    body given to a send
                    // 2. Its expressions are empty
                    if (thenb->bexit.cond.variable != LocalRef::blockCall() && thenb->exprs.empty()) {
                        // Don't remove block headers
                        bb->bexit.cond.variable = thenb->bexit.cond.variable;
                        bb->bexit.thenb = thenb->bexit.thenb;
                        bb->bexit.elseb = thenb->bexit.elseb;
                        bb->bexit.loc = thenb->bexit.loc;
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
            }

            // Now we consider shortcutting the `thenb` and `elseb` blocks in turn, which is only valid when the block:
            //
            // 1. Is not the dead block
            // 2. Has no expressions
            // 3. Contains an unconditional jump out
            // 4. Does not jump back to itself.
            //
            // These conditions are very similar to how we decide to inline the bexit of an unconditional jump above,
            // but for when we're looking to prune out intermediate nodes from one path of a conditional jump.

            if (thenb != cfg.deadBlock() && thenb->exprs.empty() && thenb->bexit.isUnconditional() &&
                bb->bexit.thenb != thenb->bexit.thenb) {
                // shortcut then
                bb->bexit.thenb = thenb->bexit.thenb;
                bb->bexit.thenb->backEdges.emplace_back(bb);
                thenb->backEdges.erase(remove(thenb->backEdges.begin(), thenb->backEdges.end(), bb),
                                       thenb->backEdges.end());
                // Update unconditional jumps
                if (thenb == elseb) {
                    bb->bexit.elseb = bb->bexit.thenb;
                }

                changed = true;
                sanityCheck(ctx, cfg);
                continue;
            }

            if (elseb != cfg.deadBlock() && elseb->exprs.empty() && elseb->bexit.isUnconditional() &&
                bb->bexit.elseb != elseb->bexit.elseb) {
                // shortcut else
                sanityCheck(ctx, cfg);
                bb->bexit.elseb = elseb->bexit.elseb;
                bb->bexit.elseb->backEdges.emplace_back(bb);
                elseb->backEdges.erase(remove(elseb->backEdges.begin(), elseb->backEdges.end(), bb),
                                       elseb->backEdges.end());
                // Update unconditional jumps
                if (thenb == elseb) {
                    bb->bexit.thenb = bb->bexit.elseb;
                }

                changed = true;
                sanityCheck(ctx, cfg);
                continue;
            }
            ++it;
        }
    }
}

void CFGBuilder::sanityCheck(core::Context ctx, CFG &cfg) {
    if constexpr (!debug_mode) {
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
            ENFORCE(bb->flags.wasJumpDestination, "block {} was never linked into cfg", bb->id);
        }
        ENFORCE(absl::c_contains(bb->bexit.thenb->backEdges, bb.get()), "backedge unset for thenb");
        ENFORCE(absl::c_contains(bb->bexit.elseb->backEdges, bb.get()), "backedge unset for elseb");
    }
}

LocalRef maybeDealias(core::Context ctx, CFG &cfg, LocalRef what, UnorderedMap<LocalRef, LocalRef> &aliases) {
    if (what.isSyntheticTemporary(cfg)) {
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
        UIntSet mayHaveAlias(cfg.numLocalVariables());
        for (auto &alias : current) {
            mayHaveAlias.add(alias.second.id());
        }

        for (Binding &bind : bb->exprs) {
            if (auto i = cast_instruction<Ident>(bind.value)) {
                i->what = maybeDealias(ctx, cfg, i->what, current);
            }
            if (mayHaveAlias.contains(bind.bind.variable.id())) {
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
            if (!bind.value.isSynthetic()) {
                // we don't allow dealiasing values into synthetic instructions
                // as otherwise it fools dead code analysis.
                if (auto v = cast_instruction<Ident>(bind.value)) {
                    v->what = maybeDealias(ctx, cfg, v->what, current);
                } else if (auto v = cast_instruction<Send>(bind.value)) {
                    v->recv = maybeDealias(ctx, cfg, v->recv.variable, current);
                    for (auto &arg : v->argRefs()) {
                        arg = maybeDealias(ctx, cfg, arg, current);
                    }
                } else if (auto v = cast_instruction<TAbsurd>(bind.value)) {
                    v->what = maybeDealias(ctx, cfg, v->what.variable, current);
                } else if (auto v = cast_instruction<Return>(bind.value)) {
                    v->what = maybeDealias(ctx, cfg, v->what.variable, current);
                }
            }

            // record new aliases
            if (auto i = cast_instruction<Ident>(bind.value)) {
                current[bind.bind.variable] = i->what;
                mayHaveAlias.add(i->what.id());
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
                bb->flags.isLoopHeader = true;
                continue;
            }
        }
    }
}
void CFGBuilder::removeDeadAssigns(core::Context ctx, const CFG::ReadsAndWrites &RnW, CFG &cfg,
                                   const vector<UIntSet> &blockArgs) {
    ENFORCE_NO_TIMER(blockArgs.size() == cfg.maxBasicBlockId);
    if (!ctx.state.lspQuery.isEmpty()) {
        return;
    }

    Timer timeit(ctx.state.tracer(), "removeDeadAssigns");
    for (auto &it : cfg.basicBlocks) {
        /* remove dead variables */
        it->exprs.erase(
            remove_if(it->exprs.begin(), it->exprs.end(),
                      [&ctx, &cfg, &RnW, &blockArgs, &it](auto &bind) -> bool {
                          if (bind.bind.variable.isAliasForGlobal(ctx, cfg)) {
                              return false;
                          }
                          bool wasRead =
                              RnW.reads[it->id].contains(bind.bind.variable.id()) || // read in the same block
                              blockArgs[it->bexit.thenb->id].contains(bind.bind.variable.id()) ||
                              blockArgs[it->bexit.elseb->id].contains(bind.bind.variable.id());

                          if (!wasRead) {
                              // These are all instructions with no side effects, which can be
                              // deleted if the assignment is dead. It would be slightly
                              // shorter to list the converse set -- those which *do* have
                              // side effects -- but doing it this way is more robust to us
                              // adding more instruction types in the future.
                              if (isa_instruction<Ident>(bind.value) || isa_instruction<Literal>(bind.value) ||
                                  isa_instruction<LoadSelf>(bind.value) || isa_instruction<LoadArg>(bind.value) ||
                                  isa_instruction<LoadYieldParams>(bind.value)) {
                                  return true;
                              }
                          }
                          return false;
                      }),
            it->exprs.end());
    }
}

void CFGBuilder::computeMinMaxLoops(core::Context ctx, const CFG::ReadsAndWrites &RnW, CFG &cfg) {
    for (const auto &bb : cfg.basicBlocks) {
        if (bb.get() == cfg.deadBlock()) {
            continue;
        }

        RnW.reads[bb->id].forEach([&cfg, &bb](uint32_t local) {
            auto curMin = cfg.minLoops[local];
            if (curMin > bb->outerLoops) {
                curMin = bb->outerLoops;
            }
            cfg.minLoops[local] = curMin;
        });
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

vector<UIntSet> CFGBuilder::fillInBlockArguments(core::Context ctx, const CFG::ReadsAndWrites &RnW, const CFG &cfg) {
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

    const auto &readsByBlock = RnW.reads;
    const auto &writesByBlock = RnW.writes;
    const auto &deadByBlock = RnW.dead;

    // iterate over basic blocks in reverse and found upper bounds on what could a block need.
    vector<UIntSet> upperBounds1;
    bool changed = true;
    {
        Timer timeit(ctx.state.tracer(), "upperBounds1");
        upperBounds1 = readsByBlock;
        UIntSet toRemove(cfg.numLocalVariables());
        while (changed) {
            changed = false;
            for (BasicBlock *bb : cfg.forwardsTopoSort) {
                auto &upperBoundsForBlock = upperBounds1[bb->id];

                const auto sz = upperBoundsForBlock.size();

                if (bb->bexit.thenb != cfg.deadBlock()) {
                    upperBoundsForBlock.add(upperBounds1[bb->bexit.thenb->id]);
                }
                if (bb->bexit.elseb != cfg.deadBlock()) {
                    upperBoundsForBlock.add(upperBounds1[bb->bexit.elseb->id]);
                }

                // Any variable that we write and do not read is dead on entry to
                // this block, and we do not require it.
                const auto &deadForBlock = deadByBlock[bb->id];
                if (!deadForBlock.empty()) {
                    toRemove.clear();
                    deadForBlock.forEach([&bb, &cfg, &toRemove](uint32_t local) -> void {
                        // TODO(nelhage) We can't erase for variables inside loops, due
                        // to how our "pinning" type inference works. We can remove this
                        // inner condition when we get a better type inference
                        // algorithm.
                        if (bb->outerLoops <= cfg.minLoops[local]) {
                            toRemove.add(local);
                        }
                    });
                    upperBoundsForBlock.remove(toRemove);
                }
                changed = changed || (upperBoundsForBlock.size() != sz);
            }
        }
    }

    vector<UIntSet> upperBounds2(cfg.maxBasicBlockId, UIntSet(cfg.numLocalVariables()));

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
                        upperBoundsForBlock.add(writesByBlock[edge->id], upperBounds2[edge->id]);
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
            // Intentionally mutate upperBounds1 here for return value.
            auto &intersection = upperBounds1[it->id];
            intersection.intersect(upperBounds2[it->id]);
            // Note: forEach enqueues arguments in sorted order. We assume that args is empty so we don't need to sort.
            ENFORCE_NO_TIMER(it->args.empty());
            intersection.forEach([&it](uint32_t local) -> void { it->args.emplace_back(local); });
            // it->args is now sorted in LocalRef ID order.
            ENFORCE(absl::c_is_sorted(it->args,
                                      [](auto &a, auto &b) -> bool { return a.variable.id() < b.variable.id(); }));
            histogramInc("cfgbuilder.blockArguments", it->args.size());
        }
    }
    // upperBounds1 now contains the intersection of upperBounds2 and 1.
    return upperBounds1;
}

namespace {

// The marker for nodes that the post-order traversal has not yet encountered.
constexpr int UNVISITED = -1;

// The marker for nodes that the post-order traversal is processing the children of (this implies that the node is
// somewhere in the work queue).
constexpr int PROCESSING = -2;

struct WorkItem {
    enum class Event { Enter, Exit };

    BasicBlock *block;
    Event event;

    explicit WorkItem(BasicBlock *block) : block{block}, event{Event::Enter} {}
};

} // namespace

vector<int> CFGBuilder::topoSortFwd(vector<BasicBlock *> &target, int numBlocks, BasicBlock *entryBB) {
    ENFORCE(target.empty(), "The output vector must be empty to start with");

    target.reserve(numBlocks);

    vector<int> forwardIndex(numBlocks, UNVISITED);
    vector<WorkItem> work;

    // Arbitrary, could use tuning
    work.reserve(10);

    work.emplace_back(entryBB);

    while (!work.empty()) {
        auto [block, event] = work.back();

        switch (event) {
            case WorkItem::Event::Enter: {
                // Have we seen this node already through another path?
                if (forwardIndex[block->id] != UNVISITED) {
                    work.pop_back();
                    continue;
                }

                // Ensure that we record this node on the way back out, mark it as visited, and enqueue its successors.
                work.back().event = WorkItem::Event::Exit;

                forwardIndex[block->id] = PROCESSING;

                if (forwardIndex[block->bexit.thenb->id] == UNVISITED) {
                    work.emplace_back(block->bexit.thenb);
                }

                if (!block->bexit.isUnconditional() && forwardIndex[block->bexit.elseb->id] == UNVISITED) {
                    work.emplace_back(block->bexit.elseb);
                }

                break;
            }

            case WorkItem::Event::Exit: {
                ENFORCE(forwardIndex[block->id] == PROCESSING);

                forwardIndex[block->id] = target.size();
                target.emplace_back(block);
                work.pop_back();

                break;
            }
        }
    }

    target.shrink_to_fit();

    return forwardIndex;
}
} // namespace sorbet::cfg
