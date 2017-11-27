#include "builder.h"

#include <algorithm> // sort
#include <climits>   // INT_MAX
#include <unordered_map>
#include <unordered_set>
using namespace std;

namespace ruby_typer {
namespace cfg {

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

    outAliases.resize(cfg.basicBlocks.size());
    for (BasicBlock *bb : cfg.backwardsTopoSort) {
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
            if (auto *i = dynamic_cast<Ident *>(bind.value.get())) {
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
            if (auto *v = dynamic_cast<Ident *>(bind.value.get())) {
                v->what = maybeDealias(ctx, v->what, current);
            } else if (auto *v = dynamic_cast<Send *>(bind.value.get())) {
                v->recv = maybeDealias(ctx, v->recv, current);
                for (auto &arg : v->args) {
                    arg = maybeDealias(ctx, arg, current);
                }
            } else if (auto *v = dynamic_cast<Return *>(bind.value.get())) {
                v->what = maybeDealias(ctx, v->what, current);
            } else if (auto *v = dynamic_cast<NamedArg *>(bind.value.get())) {
                v->value = maybeDealias(ctx, v->value, current);
            }

            // record new aliases
            if (auto *i = dynamic_cast<Ident *>(bind.value.get())) {
                current[bind.bind] = i->what;
            }
        }
        if (bb->bexit.cond.exists()) {
            bb->bexit.cond = maybeDealias(ctx, bb->bexit.cond, current);
        }
    }
}

void CFGBuilder::fillInBlockArguments(core::Context ctx, CFG &cfg) {
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
    unordered_map<core::LocalVariable, unordered_set<BasicBlock *>> reads;
    unordered_map<core::LocalVariable, unordered_set<BasicBlock *>> writes;

    for (unique_ptr<BasicBlock> &bb : cfg.basicBlocks) {
        for (Binding &bind : bb->exprs) {
            writes[bind.bind].insert(bb.get());
            if (auto *v = dynamic_cast<Ident *>(bind.value.get())) {
                reads[v->what].insert(bb.get());
            } else if (auto *v = dynamic_cast<Send *>(bind.value.get())) {
                reads[v->recv].insert(bb.get());
                for (auto arg : v->args) {
                    reads[arg].insert(bb.get());
                }
            } else if (auto *v = dynamic_cast<Return *>(bind.value.get())) {
                reads[v->what].insert(bb.get());
            } else if (auto *v = dynamic_cast<NamedArg *>(bind.value.get())) {
                reads[v->value].insert(bb.get());
            } else if (auto *v = dynamic_cast<LoadArg *>(bind.value.get())) {
                reads[v->receiver].insert(bb.get());
            }
        }
        if (bb->bexit.cond.exists()) {
            reads[bb->bexit.cond].insert(bb.get());
        }
    }

    for (auto &pair : reads) {
        core::LocalVariable what = pair.first;
        unordered_set<BasicBlock *> &where = pair.second;
        auto fnd = cfg.minLoops.insert({what, INT_MAX});
        int &min = (*(fnd.first)).second;
        for (BasicBlock *bb : where) {
            if (min > bb->outerLoops) {
                min = bb->outerLoops;
            }
        }
    }

    for (auto &pair : writes) {
        core::LocalVariable what = pair.first;
        unordered_set<BasicBlock *> &where = pair.second;
        auto fnd = cfg.minLoops.insert({what, INT_MAX});
        int &min = (*(fnd.first)).second;
        for (BasicBlock *bb : where) {
            if (min > bb->outerLoops) {
                min = bb->outerLoops;
            }
        }
    }

    for (auto &it : cfg.basicBlocks) {
        /* remove dead variables */
        for (auto expIt = it->exprs.begin(); expIt != it->exprs.end(); /* nothing */) {
            Binding &bind = *expIt;
            auto fnd = reads.find(bind.bind);
            if (fnd == reads.end()) {
                // This should be !New && !Send && !Return, but I prefer to list explicitly in case we start adding
                // nodes.
                if (dynamic_cast<Ident *>(bind.value.get()) != nullptr ||
                    dynamic_cast<ArraySplat *>(bind.value.get()) != nullptr ||
                    dynamic_cast<HashSplat *>(bind.value.get()) != nullptr ||
                    dynamic_cast<BoolLit *>(bind.value.get()) != nullptr ||
                    dynamic_cast<StringLit *>(bind.value.get()) != nullptr ||
                    dynamic_cast<SymbolLit *>(bind.value.get()) != nullptr ||
                    dynamic_cast<IntLit *>(bind.value.get()) != nullptr ||
                    dynamic_cast<FloatLit *>(bind.value.get()) != nullptr ||
                    dynamic_cast<Self *>(bind.value.get()) != nullptr ||
                    dynamic_cast<LoadArg *>(bind.value.get()) != nullptr ||
                    dynamic_cast<NamedArg *>(bind.value.get()) != nullptr) {
                    expIt = it->exprs.erase(expIt);
                } else {
                    ++expIt;
                }
            } else {
                ++expIt;
            }
        }
    }

    vector<unordered_set<core::LocalVariable>> reads_by_block(cfg.basicBlocks.size());
    vector<unordered_set<core::LocalVariable>> writes_by_block(cfg.basicBlocks.size());

    for (auto &rds : reads) {
        auto &wts = writes[rds.first];
        if (rds.second.size() == 1 && wts.size() == 1 && *(rds.second.begin()) == *(wts.begin())) {
            wts.clear();
            rds.second.clear(); // remove symref that never escapes a block.
        } else if (wts.empty()) {
            rds.second.clear();
        }
    }

    for (auto &wts : writes) {
        auto &rds = reads[wts.first];
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

    // iterate ver basic blocks in reverse and found upper bounds on what could a block need.
    vector<unordered_set<core::LocalVariable>> upper_bounds1(cfg.basicBlocks.size());
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
            changed = changed || (upper_bounds1[bb->id].size() != sz);
        }
    }

    vector<unordered_set<core::LocalVariable>> upper_bounds2(cfg.basicBlocks.size());

    changed = true;
    while (changed) {
        changed = false;
        for (auto it = cfg.backwardsTopoSort.begin(); it != cfg.backwardsTopoSort.end(); ++it) {
            BasicBlock *bb = *it;
            int sz = upper_bounds2[bb->id].size();
            upper_bounds2[bb->id].insert(writes_by_block[bb->id].begin(), writes_by_block[bb->id].end());
            for (BasicBlock *edge : bb->backEdges) {
                if (edge != cfg.deadBlock()) {
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
        sort(it->args.begin(), it->args.end(),
             [](core::LocalVariable a, core::LocalVariable b) -> bool { return a.name._id < b.name._id; });
    }

    return;
}

int CFGBuilder::topoSortFwd(vector<BasicBlock *> &target, int nextFree, BasicBlock *currentBB) {
    // Error::check(!marked[currentBB]) // graph is cyclic!
    if ((currentBB->flags & CFG::FORWARD_TOPO_SORT_VISITED) != 0) {
        return nextFree;
    } else {
        currentBB->flags |= CFG::FORWARD_TOPO_SORT_VISITED;
        nextFree = topoSortFwd(target, nextFree, currentBB->bexit.thenb);
        nextFree = topoSortFwd(target, nextFree, currentBB->bexit.elseb);
        target[nextFree] = currentBB;
        return nextFree + 1;
    }
}

int CFGBuilder::topoSortBwd(vector<BasicBlock *> &target, int nextFree, BasicBlock *currentBB) {
    // We're not looking for an arbitrary topo-sort.
    // First of all topo sort does not exist, as the graph has loops.
    // We are looking for a sort that has all outer loops dominating loop headers that dominate loop bodies.
    //
    // This method is a big cache invalidator and should be removed if we become slow
    // Instead we will build this sort the fly during construction of the CFG, but it will make it hard to add new nodes
    // much harder.

    if ((currentBB->flags & CFG::BACKWARD_TOPO_SORT_VISITED) != 0) {
        return nextFree;
    } else {
        currentBB->flags |= CFG::BACKWARD_TOPO_SORT_VISITED;
        int i = 0;
        // iterate over outer loops
        while (i < currentBB->backEdges.size() && currentBB->outerLoops > currentBB->backEdges[i]->outerLoops) {
            nextFree = topoSortBwd(target, nextFree, currentBB->backEdges[i]);
            i++;
        }
        if (i > 0) { // This is a loop header!
            target[nextFree] = currentBB;
            nextFree = nextFree + 1;
            while (i < currentBB->backEdges.size()) {
                nextFree = topoSortBwd(target, nextFree, currentBB->backEdges[i]);
                i++;
            }
        } else {
            while (i < currentBB->backEdges.size()) {
                nextFree = topoSortBwd(target, nextFree, currentBB->backEdges[i]);
                i++;
            }
            target[nextFree] = currentBB;
            nextFree = nextFree + 1;
        }
        return nextFree;
    }
}
} // namespace cfg
} // namespace ruby_typer
