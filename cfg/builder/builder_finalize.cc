#include "builder.h"
#include "core/Names/cfg.h"

#include <algorithm> // sort, remove, unique
#include <climits>   // INT_MAX
#include <unordered_map>
#include <unordered_set>
using namespace std;

namespace ruby_typer {
namespace cfg {

void CFGBuilder::simplify(core::Context ctx, CFG &cfg) {
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
                    thenb->backEdges.erase(std::remove(thenb->backEdges.begin(), thenb->backEdges.end(), bb),
                                           thenb->backEdges.end());
                    if (elseb != thenb) {
                        elseb->backEdges.erase(std::remove(elseb->backEdges.begin(), elseb->backEdges.end(), bb),
                                               elseb->backEdges.end());
                    }
                    it = cfg.basicBlocks.erase(it);
                    cfg.forwardsTopoSort.erase(
                        std::remove(cfg.forwardsTopoSort.begin(), cfg.forwardsTopoSort.end(), bb),
                        cfg.forwardsTopoSort.end());
                    changed = true;
                    sanityCheck(ctx, cfg);
                    continue;
                } else {
                    sort(bb->backEdges.begin(), bb->backEdges.end(),
                         [](const BasicBlock *bb1, const BasicBlock *bb2) -> bool { return bb1->id < bb2->id; });
                    bb->backEdges.erase(unique(bb->backEdges.begin(), bb->backEdges.end()), bb->backEdges.end());
                }
            }
            if (thenb == elseb) {
                // Remove condition from unconditional jumps
                bb->bexit.cond = core::LocalVariable::noVariable();
            }
            if (thenb == elseb && thenb != cfg.deadBlock() && thenb != bb) { // can be squashed togather
                if (thenb->backEdges.size() == 1 && thenb->outerLoops == bb->outerLoops) {
                    bb->exprs.insert(bb->exprs.end(), std::make_move_iterator(thenb->exprs.begin()),
                                     std::make_move_iterator(thenb->exprs.end()));
                    thenb->backEdges.clear();
                    bb->bexit = thenb->bexit;
                    bb->bexit.thenb->backEdges.push_back(bb);
                    if (bb->bexit.thenb != bb->bexit.elseb) {
                        bb->bexit.elseb->backEdges.push_back(bb);
                    }
                    changed = true;
                    sanityCheck(ctx, cfg);
                    continue;
                } else if (thenb->bexit.cond != core::LocalVariable::blockCall() && thenb->exprs.empty()) {
                    // Don't remove block headers
                    bb->bexit = thenb->bexit;
                    thenb->backEdges.erase(std::remove(thenb->backEdges.begin(), thenb->backEdges.end(), bb),
                                           thenb->backEdges.end());
                    bb->bexit.thenb->backEdges.push_back(bb);
                    if (bb->bexit.thenb != bb->bexit.elseb) {
                        bb->bexit.elseb->backEdges.push_back(bb);
                    }
                    changed = true;
                    sanityCheck(ctx, cfg);
                    continue;
                }
            }
            if (thenb != cfg.deadBlock() && thenb->exprs.empty() && thenb->bexit.thenb == thenb->bexit.elseb &&
                bb->bexit.thenb != thenb->bexit.thenb) {
                // shortcut then
                bb->bexit.thenb = thenb->bexit.thenb;
                thenb->bexit.thenb->backEdges.push_back(bb);
                thenb->backEdges.erase(std::remove(thenb->backEdges.begin(), thenb->backEdges.end(), bb),
                                       thenb->backEdges.end());
                changed = true;
                sanityCheck(ctx, cfg);
                continue;
            }
            if (elseb != cfg.deadBlock() && elseb->exprs.empty() && elseb->bexit.thenb == elseb->bexit.elseb &&
                bb->bexit.elseb != elseb->bexit.elseb) {
                // shortcut else
                sanityCheck(ctx, cfg);
                bb->bexit.elseb = elseb->bexit.elseb;
                bb->bexit.elseb->backEdges.push_back(bb);
                elseb->backEdges.erase(std::remove(elseb->backEdges.begin(), elseb->backEdges.end(), bb),
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
            ENFORCE((bb->flags & CFG::WAS_JUMP_DESTINATION) != 0, "block ", bb->id, " was never linked into cfg");
        }
        auto thenFnd = std::find(bb->bexit.thenb->backEdges.begin(), bb->bexit.thenb->backEdges.end(), bb.get());
        auto elseFnd = std::find(bb->bexit.elseb->backEdges.begin(), bb->bexit.elseb->backEdges.end(), bb.get());
        ENFORCE(thenFnd != bb->bexit.thenb->backEdges.end(), "backedge unset for thenb");
        ENFORCE(elseFnd != bb->bexit.elseb->backEdges.end(), "backedge unset for elseb");
    }
}

core::LocalVariable maybeDealias(core::Context ctx, core::LocalVariable what,
                                 unordered_map<core::LocalVariable, core::LocalVariable> &aliases) {
    if (what.isSyntheticTemporary(ctx)) {
        auto fnd = aliases.find(what);
        if (fnd != aliases.end()) {
            return fnd->second;
        } else {
            return what;
        }
    } else {
        return what;
    }
}

/**
 * Remove aliases from CFG. Why does this need a separate pass?
 * because `a.foo(a = "2", if (...) a = true; else a = null; end)`
 */
void CFGBuilder::dealias(core::Context ctx, CFG &cfg) {
    vector<unordered_map<core::LocalVariable, core::LocalVariable>> outAliases;

    outAliases.resize(cfg.maxBasicBlockId);
    for (auto it = cfg.forwardsTopoSort.rbegin(); it != cfg.forwardsTopoSort.rend(); ++it) {
        auto &bb = *it;
        if (bb == cfg.deadBlock()) {
            continue;
        }
        unordered_map<core::LocalVariable, core::LocalVariable> &current = outAliases[bb->id];
        if (!bb->backEdges.empty()) {
            current = outAliases[bb->backEdges[0]->id];
        }

        for (BasicBlock *parent : bb->backEdges) {
            unordered_map<core::LocalVariable, core::LocalVariable> other = outAliases[parent->id];
            for (auto it = current.begin(); it != current.end(); /* nothing */) {
                auto &el = *it;
                auto fnd = other.find(el.first);
                if (fnd != other.end()) {
                    if (fnd->second != el.second) {
                        it = current.erase(it);
                    } else {
                        ++it;
                    }
                } else {
                    it = current.erase(it); // note: this is correct but to conservative. In particular for loop headers
                }
            }
        }

        for (Binding &bind : bb->exprs) {
            if (auto *i = cast_instruction<Ident>(bind.value.get())) {
                i->what = maybeDealias(ctx, i->what, current);
            }
            /* invalidate a stale record */
            for (auto it = current.begin(); it != current.end(); /* nothing */) {
                if (it->second == bind.bind) {
                    it = current.erase(it);
                } else {
                    ++it;
                }
            }
            /* dealias */
            if (auto *v = cast_instruction<Ident>(bind.value.get())) {
                v->what = maybeDealias(ctx, v->what, current);
            } else if (auto *v = cast_instruction<Send>(bind.value.get())) {
                v->recv = maybeDealias(ctx, v->recv, current);
                for (auto &arg : v->args) {
                    arg = maybeDealias(ctx, arg, current);
                }
            } else if (auto *v = cast_instruction<Return>(bind.value.get())) {
                v->what = maybeDealias(ctx, v->what, current);
            }

            // record new aliases
            if (auto *i = cast_instruction<Ident>(bind.value.get())) {
                current[bind.bind] = i->what;
            }
        }
        if (bb->bexit.cond.exists()) {
            bb->bexit.cond = maybeDealias(ctx, bb->bexit.cond, current);
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
    for (auto &it : cfg.basicBlocks) {
        /* remove dead variables */
        for (auto expIt = it->exprs.begin(); expIt != it->exprs.end(); /* nothing */) {
            Binding &bind = *expIt;
            if (bind.bind.isAliasForGlobal(ctx)) {
                ++expIt;
                continue;
            }

            auto fnd = RnW.reads.find(bind.bind);
            if (fnd == RnW.reads.end()) {
                // These are all instructions with no side effects, which can be
                // deleted if the assignment is dead. It would be slightly
                // shorter to list the converse set -- those which *do* have
                // side effects -- but doing it this way is more robust to us
                // adding more instruction types in the future.
                if (isa_instruction<Ident>(bind.value.get()) || isa_instruction<ArraySplat>(bind.value.get()) ||
                    isa_instruction<HashSplat>(bind.value.get()) || isa_instruction<Literal>(bind.value.get()) ||
                    isa_instruction<Self>(bind.value.get()) || isa_instruction<LoadArg>(bind.value.get()) ||
                    isa_instruction<LoadYieldParam>(bind.value.get())) {
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
    for (auto &pair : RnW.reads) {
        core::LocalVariable what = pair.first;
        const unordered_set<BasicBlock *> &where = pair.second;
        auto fnd = cfg.minLoops.insert({what, INT_MAX});
        int &min = (*(fnd.first)).second;
        for (const BasicBlock *bb : where) {
            if (min > bb->outerLoops) {
                min = bb->outerLoops;
            }
        }
    }

    for (auto &pair : RnW.writes) {
        core::LocalVariable what = pair.first;
        const unordered_set<BasicBlock *> &where = pair.second;
        auto fndMn = cfg.minLoops.insert({what, INT_MAX}); // note: this will NOT overrwite existing value
        auto fndMx = cfg.maxLoopWrite.insert({what, 0});
        int &min = (*(fndMn.first)).second;
        int &max = (*(fndMx.first)).second;
        for (const BasicBlock *bb : where) {
            if (min > bb->outerLoops) {
                min = bb->outerLoops;
            }
            if (max < bb->outerLoops) {
                max = bb->outerLoops;
            }
        }
    }
}

void CFGBuilder::fillInBlockArguments(core::Context ctx, CFG::ReadsAndWrites &RnW, CFG &cfg) {
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

    vector<unordered_set<core::LocalVariable>> reads_by_block(cfg.maxBasicBlockId);
    vector<unordered_set<core::LocalVariable>> writes_by_block(cfg.maxBasicBlockId);
    vector<unordered_set<core::LocalVariable>> kills_by_block(cfg.maxBasicBlockId);

    for (auto &rds : RnW.reads) {
        auto &wts = RnW.writes[rds.first];
        core::histogramInc("cfgbuilder.readsPerBlock", rds.second.size());
        if (rds.second.size() == 1 && wts.size() == 1 && *(rds.second.begin()) == *(wts.begin())) {
            wts.clear();
            rds.second.clear(); // remove symref that never escapes a block.
        } else if (wts.empty()) {
            rds.second.clear();
        }
    }

    for (auto &wts : RnW.writes) {
        core::histogramInc("cfgbuilder.writesPerBlock", wts.second.size());
        auto &rds = RnW.reads[wts.first];
        if (rds.empty()) {
            wts.second.clear();
        }
        for (BasicBlock *bb : rds) {
            reads_by_block[bb->id].insert(wts.first);
        }
        for (BasicBlock *bb : wts.second) {
            writes_by_block[bb->id].insert(wts.first);
        }
    }

    for (auto &kills : RnW.kills) {
        for (BasicBlock *bb : kills.second) {
            kills_by_block[bb->id].insert(kills.first);
        }
    }

    // iterate ver basic blocks in reverse and found upper bounds on what could a block need.
    vector<unordered_set<core::LocalVariable>> upper_bounds1(cfg.maxBasicBlockId);
    bool changed = true;

    while (changed) {
        changed = false;
        for (BasicBlock *bb : cfg.forwardsTopoSort) {
            int sz = upper_bounds1[bb->id].size();
            upper_bounds1[bb->id].insert(reads_by_block[bb->id].begin(), reads_by_block[bb->id].end());
            if (bb->bexit.thenb != cfg.deadBlock()) {
                upper_bounds1[bb->id].insert(upper_bounds1[bb->bexit.thenb->id].begin(),
                                             upper_bounds1[bb->bexit.thenb->id].end());
            }
            if (bb->bexit.elseb != cfg.deadBlock()) {
                upper_bounds1[bb->id].insert(upper_bounds1[bb->bexit.elseb->id].begin(),
                                             upper_bounds1[bb->bexit.elseb->id].end());
            }
            // Any variable that we write and do not read is dead on entry to
            // this block, and we do not require it.
            for (auto kill : kills_by_block[bb->id]) {
                // TODO(nelhage) We can't erase for variables inside loops, due
                // to how our "pinning" type inference works. We can remove this
                // inner condition when we get a better type inference
                // algorithm.
                if (bb->outerLoops <= cfg.minLoops[kill]) {
                    upper_bounds1[bb->id].erase(kill);
                }
            }

            changed = changed || (upper_bounds1[bb->id].size() != sz);
        }
    }

    vector<unordered_set<core::LocalVariable>> upper_bounds2(cfg.maxBasicBlockId);

    changed = true;
    while (changed) {
        changed = false;
        for (auto it = cfg.forwardsTopoSort.rbegin(); it != cfg.forwardsTopoSort.rend(); ++it) {
            BasicBlock *bb = *it;
            int sz = upper_bounds2[bb->id].size();
            for (BasicBlock *edge : bb->backEdges) {
                if (edge != cfg.deadBlock()) {
                    upper_bounds2[bb->id].insert(writes_by_block[edge->id].begin(), writes_by_block[edge->id].end());
                    upper_bounds2[bb->id].insert(upper_bounds2[edge->id].begin(), upper_bounds2[edge->id].end());
                }
            }
            changed = changed || (upper_bounds2[bb->id].size() != sz);
        }
    }

    /** Combine two upper bounds */
    for (auto &it : cfg.basicBlocks) {
        auto set2 = upper_bounds2[it->id];

        int set1Sz = set2.size();
        int set2Sz = upper_bounds1[it->id].size();
        it->args.reserve(set1Sz > set2Sz ? set2Sz : set1Sz);
        for (auto el : upper_bounds1[it->id]) {
            if (set2.find(el) != set2.end()) {
                it->args.push_back(el);
            }
        }
        sort(it->args.begin(), it->args.end());
        core::histogramInc("cfgbuilder.blockArguments", it->args.size());
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
} // namespace cfg
} // namespace ruby_typer
