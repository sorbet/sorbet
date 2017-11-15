
#include "CFG.h"
#include <algorithm>
#include <algorithm> // sort
#include <climits>   // INT_MAX
#include <sstream>
#include <unordered_map>
#include <unordered_set>

// helps debugging
template class std::unique_ptr<ruby_typer::cfg::CFG>;
template class std::unique_ptr<ruby_typer::cfg::Instruction>;

using namespace std;

namespace ruby_typer {
namespace cfg {

int CFG::FORWARD_TOPO_SORT_VISITED = 1 << 0;
int CFG::BACKWARD_TOPO_SORT_VISITED = 1 << 1;

core::LocalVariable maybeDealias(core::Context ctx, core::LocalVariable what,
                                 unordered_map<core::LocalVariable, core::LocalVariable> &aliases) {
    if (what.isSyntheticTemporary(ctx)) {
        auto fnd = aliases.find(what);
        if (fnd != aliases.end()) {
            return fnd->second;
        } else
            return what;
    } else {
        return what;
    }
}

/**
 * Remove aliases from CFG. Why does this need a separate pass?
 * because `a.foo(a = "2", if (...) a = true; else a = null; end)`
 */
void CFG::dealias(core::Context ctx) {
    vector<unordered_map<core::LocalVariable, core::LocalVariable>> outAliases;

    outAliases.resize(this->basicBlocks.size());
    for (BasicBlock *bb : this->backwardsTopoSort) {
        if (bb == this->deadBlock())
            continue;
        unordered_map<core::LocalVariable, core::LocalVariable> &current = outAliases[bb->id];
        if (bb->backEdges.size() > 0) {
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
                    ++it;
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
    }
}

void CFG::fillInBlockArguments(core::Context ctx) {
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

    for (unique_ptr<BasicBlock> &bb : this->basicBlocks) {
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
        auto fnd = this->minLoops.insert({what, INT_MAX});
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
        auto fnd = this->minLoops.insert({what, INT_MAX});
        int &min = (*(fnd.first)).second;
        for (BasicBlock *bb : where) {
            if (min > bb->outerLoops) {
                min = bb->outerLoops;
            }
        }
    }

    for (auto &it : this->basicBlocks) {
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

    vector<unordered_set<core::LocalVariable>> reads_by_block(this->basicBlocks.size());
    vector<unordered_set<core::LocalVariable>> writes_by_block(this->basicBlocks.size());

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
    vector<unordered_set<core::LocalVariable>> upper_bounds1(this->basicBlocks.size());
    bool changed = true;

    while (changed) {
        changed = false;
        for (BasicBlock *bb : this->forwardsTopoSort) {
            int sz = upper_bounds1[bb->id].size();
            upper_bounds1[bb->id].insert(reads_by_block[bb->id].begin(), reads_by_block[bb->id].end());
            if (bb->bexit.thenb != deadBlock()) {
                upper_bounds1[bb->id].insert(upper_bounds1[bb->bexit.thenb->id].begin(),
                                             upper_bounds1[bb->bexit.thenb->id].end());
            }
            if (bb->bexit.elseb != deadBlock()) {
                upper_bounds1[bb->id].insert(upper_bounds1[bb->bexit.elseb->id].begin(),
                                             upper_bounds1[bb->bexit.elseb->id].end());
            }
            changed = changed || (upper_bounds1[bb->id].size() != sz);
        }
    }

    vector<unordered_set<core::LocalVariable>> upper_bounds2(this->basicBlocks.size());

    changed = true;
    while (changed) {
        changed = false;
        for (auto it = this->backwardsTopoSort.begin(); it != this->backwardsTopoSort.end(); ++it) {
            BasicBlock *bb = *it;
            int sz = upper_bounds2[bb->id].size();
            upper_bounds2[bb->id].insert(writes_by_block[bb->id].begin(), writes_by_block[bb->id].end());
            for (BasicBlock *edge : bb->backEdges) {
                if (edge != deadBlock()) {
                    upper_bounds2[bb->id].insert(upper_bounds2[edge->id].begin(), upper_bounds2[edge->id].end());
                }
            }
            changed = changed || (upper_bounds2[bb->id].size() != sz);
        }
    }

    /** Combine two upper bounds */
    for (auto &it : this->basicBlocks) {
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

int CFG::topoSortFwd(vector<BasicBlock *> &target, int nextFree, BasicBlock *currentBB) {
    // Error::check(!marked[currentBB]) // graph is cyclic!
    if ((currentBB->flags & FORWARD_TOPO_SORT_VISITED)) {
        return nextFree;
    } else {
        currentBB->flags |= FORWARD_TOPO_SORT_VISITED;
        nextFree = topoSortFwd(target, nextFree, currentBB->bexit.thenb);
        nextFree = topoSortFwd(target, nextFree, currentBB->bexit.elseb);
        target[nextFree] = currentBB;
        return nextFree + 1;
    }
}

int CFG::topoSortBwd(vector<BasicBlock *> &target, int nextFree, BasicBlock *currentBB) {
    // We're not looking for an arbitrary topo-sort.
    // First of all topo sort does not exist, as the graph has loops.
    // We are looking for a sort that has all outer loops dominating loop headers that dominate loop bodies.
    //
    // This method is a big cache invalidator and should be removed if we become slow
    // Instead we will build this sort the fly during construction of the CFG, but it will make it hard to add new nodes
    // much harder.

    if ((currentBB->flags & BACKWARD_TOPO_SORT_VISITED)) {
        return nextFree;
    } else {
        currentBB->flags |= BACKWARD_TOPO_SORT_VISITED;
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

void CFG::fillInTopoSorts(core::Context ctx) {
    // needed to find loop headers.
    for (auto &bb : this->basicBlocks) {
        std::sort(bb->backEdges.begin(), bb->backEdges.end(),
                  [](const BasicBlock *a, const BasicBlock *b) -> bool { return a->outerLoops < b->outerLoops; });
    }

    auto &target1 = this->forwardsTopoSort;
    target1.resize(this->basicBlocks.size());
    int count = this->topoSortFwd(target1, 0, this->entry());
    Error::check(count == this->basicBlocks.size());

    auto &target2 = this->backwardsTopoSort;
    target2.resize(this->basicBlocks.size());
    count = this->topoSortBwd(target2, 0, this->deadBlock());
    Error::check(count == this->basicBlocks.size());
    return;
}

void jumpToDead(BasicBlock *from, CFG &inWhat);

unique_ptr<CFG> CFG::buildFor(core::Context ctx, ast::MethodDef &md) {
    unique_ptr<CFG> res(new CFG); // private constructor
    res->symbol = md.symbol;
    core::LocalVariable retSym =
        ctx.state.newTemporary(core::UniqueNameKind::CFG, core::Names::returnMethodTemp(), md.symbol);
    core::LocalVariable selfSym =
        ctx.state.newTemporary(core::UniqueNameKind::CFG, core::Names::selfMethodTemp(), md.symbol);

    BasicBlock *entry = res->entry();

    entry->exprs.emplace_back(selfSym, md.loc, make_unique<Self>(md.symbol.info(ctx).owner));
    auto methodName = md.symbol.info(ctx).name;

    int i = 0;
    std::unordered_map<core::SymbolRef, core::LocalVariable> aliases;
    for (core::SymbolRef argSym : md.symbol.info(ctx).arguments()) {
        core::LocalVariable arg(argSym.info(ctx).name);
        entry->exprs.emplace_back(arg, argSym.info(ctx).definitionLoc, make_unique<LoadArg>(selfSym, methodName, i));
        aliases[argSym] = arg;
        i++;
    }
    auto cont = res->walk(CFGContext(ctx, *res.get(), retSym, 0, nullptr, aliases), md.rhs.get(), entry);
    core::LocalVariable retSym1 =
        ctx.state.newTemporary(core::UniqueNameKind::CFG, core::Names::returnMethodTemp(), md.symbol);

    cont->exprs.emplace_back(retSym1, md.loc, make_unique<Return>(retSym)); // dead assign.
    jumpToDead(cont, *res.get());

    std::vector<Binding> aliasesPrefix;
    for (auto kv : aliases) {
        core::SymbolRef global = kv.first;
        core::LocalVariable local = kv.second;
        res->minLoops[local] = -1;
        if (!global.info(ctx).isMethodArgument()) {
            aliasesPrefix.emplace_back(local, md.symbol.info(ctx).definitionLoc, make_unique<Alias>(global));
        }
    }
    std::sort(aliasesPrefix.begin(), aliasesPrefix.end(),
              [](const Binding &l, const Binding &r) -> bool { return l.bind.name._id < r.bind.name._id; });

    entry->exprs.insert(entry->exprs.begin(), make_move_iterator(aliasesPrefix.begin()),
                        make_move_iterator(aliasesPrefix.end()));

    res->fillInTopoSorts(ctx);
    res->dealias(ctx);
    res->fillInBlockArguments(ctx);
    return res;
}

BasicBlock *CFG::freshBlock(int outerLoops, BasicBlock *from) {
    if (from != nullptr && from == deadBlock()) {
        return from;
    }
    int id = this->basicBlocks.size();
    this->basicBlocks.emplace_back(new BasicBlock());
    BasicBlock *r = this->basicBlocks.back().get();
    r->id = id;
    r->outerLoops = outerLoops;
    return r;
}

CFG::CFG() {
    freshBlock(0, nullptr); // entry;
    freshBlock(0, nullptr); // dead code;
    deadBlock()->bexit.elseb = deadBlock();
    deadBlock()->bexit.thenb = deadBlock();
    deadBlock()->bexit.cond = core::NameRef(0);
}

void conditionalJump(BasicBlock *from, core::LocalVariable cond, BasicBlock *thenb, BasicBlock *elseb, CFG &inWhat) {
    if (from != inWhat.deadBlock()) {
        Error::check(!from->bexit.cond.exists());
        from->bexit.cond = cond;
        from->bexit.thenb = thenb;
        from->bexit.elseb = elseb;
        thenb->backEdges.push_back(from);
        elseb->backEdges.push_back(from);
    }
}

void unconditionalJump(BasicBlock *from, BasicBlock *to, CFG &inWhat) {
    if (from != inWhat.deadBlock()) {
        Error::check(!from->bexit.cond.exists());
        from->bexit.cond = core::NameRef(0);
        from->bexit.elseb = to;
        from->bexit.thenb = to;
        to->backEdges.push_back(from);
    }
}

void jumpToDead(BasicBlock *from, CFG &inWhat) {
    auto *db = inWhat.deadBlock();
    if (from != db) {
        Error::check(!from->bexit.cond.exists());
        from->bexit.cond = core::NameRef(0);
        from->bexit.elseb = db;
        from->bexit.thenb = db;
        db->backEdges.push_back(from);
    }
}

core::LocalVariable global2Local(core::Context ctx, core::SymbolRef what, CFG &inWhat,
                                 std::unordered_map<core::SymbolRef, core::LocalVariable> &aliases) {
    core::LocalVariable &alias = aliases[what];
    if (!alias.exists()) {
        core::Symbol &info = what.info(ctx);
        alias = ctx.state.newTemporary(core::UniqueNameKind::CFG, info.name, inWhat.symbol);
    }
    return alias;
}

/** Convert `what` into a cfg, by starting to evaluate it in `current` inside method defined by `inWhat`.
 * store result of evaluation into `target`. Returns basic block in which evaluation should proceed.
 */
BasicBlock *CFG::walk(CFGContext cctx, ast::Expression *what, BasicBlock *current) {
    /** Try to pay additional attention not to duplicate any part of tree.
     * Though this may lead to more effictient and a better CFG if it was to be actually compiled into code
     * This will lead to duplicate typechecking and may lead to exponential explosion of typechecking time
     * for some code snippets. */
    Error::check(!current->bexit.cond.exists());

    BasicBlock *ret = nullptr;
    typecase(
        what,
        [&](ast::While *a) {
            auto headerBlock = cctx.inWhat.freshBlock(cctx.loops + 1, current);
            unconditionalJump(current, headerBlock, cctx.inWhat);

            core::LocalVariable condSym =
                cctx.ctx.state.newTemporary(core::UniqueNameKind::CFG, core::Names::whileTemp(), cctx.inWhat.symbol);
            auto headerEnd = walk(cctx.withTarget(condSym).withScope(headerBlock), a->cond.get(), headerBlock);
            auto bodyBlock = cctx.inWhat.freshBlock(cctx.loops + 1, headerEnd);
            auto continueBlock = cctx.inWhat.freshBlock(cctx.loops, headerEnd);
            conditionalJump(headerEnd, condSym, bodyBlock, continueBlock, cctx.inWhat);
            // finishHeader
            core::LocalVariable bodySym =
                cctx.ctx.state.newTemporary(core::UniqueNameKind::CFG, core::Names::statTemp(), cctx.inWhat.symbol);

            auto body = walk(cctx.withTarget(bodySym).withScope(headerBlock), a->body.get(), bodyBlock);
            unconditionalJump(body, headerBlock, cctx.inWhat);

            continueBlock->exprs.emplace_back(cctx.target, a->loc, make_unique<Nil>());
            ret = continueBlock;
        },
        [&](ast::Return *a) {
            core::LocalVariable retSym =
                cctx.ctx.state.newTemporary(core::UniqueNameKind::CFG, core::Names::returnTemp(), cctx.inWhat.symbol);
            auto cont = walk(cctx.withTarget(retSym), a->expr.get(), current);
            cont->exprs.emplace_back(cctx.target, a->loc, make_unique<Return>(retSym)); // dead assign.
            jumpToDead(cont, cctx.inWhat);
            ret = deadBlock();
        },
        [&](ast::If *a) {
            core::LocalVariable ifSym =
                cctx.ctx.state.newTemporary(core::UniqueNameKind::CFG, core::Names::ifTemp(), cctx.inWhat.symbol);
            Error::check(ifSym.exists());
            auto cont = walk(cctx.withTarget(ifSym), a->cond.get(), current);
            auto thenBlock = cctx.inWhat.freshBlock(cctx.loops, cont);
            auto elseBlock = cctx.inWhat.freshBlock(cctx.loops, cont);
            conditionalJump(cont, ifSym, thenBlock, elseBlock, cctx.inWhat);

            auto thenEnd = walk(cctx, a->thenp.get(), thenBlock);
            auto elseEnd = walk(cctx, a->elsep.get(), elseBlock);
            if (thenEnd != deadBlock() || elseEnd != deadBlock()) {
                if (thenEnd == deadBlock()) {
                    ret = elseEnd;
                } else if (thenEnd == deadBlock()) {
                    ret = thenEnd;
                } else {
                    ret = freshBlock(cctx.loops, thenEnd); // could be elseEnd
                    unconditionalJump(thenEnd, ret, cctx.inWhat);
                    unconditionalJump(elseEnd, ret, cctx.inWhat);
                }
            } else {
                ret = deadBlock();
            }
        },
        [&](ast::IntLit *a) {
            current->exprs.emplace_back(cctx.target, a->loc, make_unique<IntLit>(a->value));
            ret = current;
        },
        [&](ast::FloatLit *a) {
            current->exprs.emplace_back(cctx.target, a->loc, make_unique<FloatLit>(a->value));
            ret = current;
        },
        [&](ast::StringLit *a) {
            current->exprs.emplace_back(cctx.target, a->loc, make_unique<StringLit>(a->value));
            ret = current;
        },
        [&](ast::BoolLit *a) {
            current->exprs.emplace_back(cctx.target, a->loc, make_unique<BoolLit>(a->value));
            ret = current;
        },
        [&](ast::ConstantLit *a) { Error::raise("Should have been eliminated by namer/resolver"); },
        [&](ast::Ident *a) {
            current->exprs.emplace_back(
                cctx.target, a->loc, make_unique<Ident>(global2Local(cctx.ctx, a->symbol, cctx.inWhat, cctx.aliases)));
            ret = current;
        },
        [&](ast::Local *a) {
            current->exprs.emplace_back(cctx.target, a->loc, make_unique<Ident>(a->localVariable));
            ret = current;
        },
        [&](ast::Self *a) {
            current->exprs.emplace_back(cctx.target, a->loc, make_unique<Self>(a->claz));
            ret = current;
        },
        [&](ast::Assign *a) {
            auto lhsIdent = dynamic_cast<ast::Ident *>(a->lhs.get());
            core::LocalVariable lhs;
            if (lhsIdent != nullptr) {
                lhs = global2Local(cctx.ctx, lhsIdent->symbol, cctx.inWhat, cctx.aliases);
            } else if (auto lhsLocal = dynamic_cast<ast::Local *>(a->lhs.get())) {
                lhs = lhsLocal->localVariable;
            } else {
                // TODO(nelhage): Once namer is complete this should be a
                // fatal error
                // lhs = cctx.ctx.state.defn_todo();
                Error::check(false, "should never be reached");
            }
            auto rhsCont = walk(cctx.withTarget(lhs), a->rhs.get(), current);
            rhsCont->exprs.emplace_back(cctx.target, a->loc, make_unique<Ident>(lhs));
            ret = rhsCont;
        },
        [&](ast::InsSeq *a) {
            for (auto &exp : a->stats) {
                core::LocalVariable temp =
                    cctx.ctx.state.newTemporary(core::UniqueNameKind::CFG, core::Names::statTemp(), cctx.inWhat.symbol);
                current = walk(cctx.withTarget(temp), exp.get(), current);
            }
            ret = walk(cctx, a->expr.get(), current);
        },
        [&](ast::Send *s) {
            core::LocalVariable recv;

            recv = cctx.ctx.state.newTemporary(core::UniqueNameKind::CFG, core::Names::statTemp(), cctx.inWhat.symbol);
            current = walk(cctx.withTarget(recv), s->recv.get(), current);

            vector<core::LocalVariable> args;
            for (auto &exp : s->args) {
                core::LocalVariable temp;
                temp =
                    cctx.ctx.state.newTemporary(core::UniqueNameKind::CFG, core::Names::statTemp(), cctx.inWhat.symbol);
                current = walk(cctx.withTarget(temp), exp.get(), current);

                args.push_back(temp);
            }

            if (s->block != nullptr) {
                auto headerBlock = cctx.inWhat.freshBlock(cctx.loops + 1, current);
                auto postBlock = cctx.inWhat.freshBlock(cctx.loops, headerBlock);
                auto bodyBlock = cctx.inWhat.freshBlock(cctx.loops + 1, headerBlock);
                core::SymbolRef sym = s->block->symbol;
                core::Symbol &info = sym.info(cctx.ctx);

                for (int i = 0; i < s->block->args.size(); ++i) {
                    auto &arg = s->block->args[i];

                    if (auto id = dynamic_cast<ast::Local *>(arg.get())) {
                        core::LocalVariable argLoc = id->localVariable;
                        cctx.aliases[info.argumentsOrMixins[i]] = argLoc;
                        bodyBlock->exprs.emplace_back(argLoc, arg->loc, make_unique<LoadArg>(recv, s->fun, i));
                    } else {
                        Error::check(false, "Should have been removed by namer");
                    }
                }

                conditionalJump(headerBlock, core::LocalVariable(core::Names::blockCall()), bodyBlock, postBlock,
                                cctx.inWhat);

                unconditionalJump(current, headerBlock, cctx.inWhat);

                // TODO: handle block arguments somehow??
                core::LocalVariable blockrv = cctx.ctx.state.newTemporary(
                    core::UniqueNameKind::CFG, core::Names::blockReturnTemp(), cctx.inWhat.symbol);
                auto blockLast = walk(cctx.withTarget(blockrv).withScope(headerBlock), s->block->body.get(), bodyBlock);

                unconditionalJump(blockLast, headerBlock, cctx.inWhat);

                current = postBlock;
            }

            current->exprs.emplace_back(cctx.target, s->loc, make_unique<Send>(recv, s->fun, args));
            ret = current;
        },

        [&](ast::Block *a) { Error::raise("should never encounter a bare Block"); },

        [&](ast::Next *a) {
            core::LocalVariable nextSym =
                cctx.ctx.state.newTemporary(core::UniqueNameKind::CFG, core::Names::returnTemp(), cctx.inWhat.symbol);
            auto afterNext = walk(cctx.withTarget(nextSym), a->expr.get(), current);

            if (cctx.scope == nullptr) {
                cctx.ctx.state.errors.error(a->loc, core::ErrorClass::NoNextScope, "No `do` block around `next`");
                // I guess just keep going into deadcode?
                unconditionalJump(afterNext, deadBlock(), cctx.inWhat);
            } else {
                unconditionalJump(afterNext, cctx.scope, cctx.inWhat);
            }

            ret = deadBlock();
        },

        [&](ast::Expression *n) {
            current->exprs.emplace_back(cctx.target, n->loc, make_unique<NotSupported>(""));
            ret = current;
        });

    /*[&](ast::Break *a) {}, */
    // For, Rescue,
    // Symbol, NamedArg, Hash, Array,
    // ArraySplat, HashAplat, Block,
    Error::check(ret != nullptr);
    return ret;
}

string CFG::toString(core::Context ctx) {
    stringstream buf;
    buf << "subgraph \"cluster_" << this->symbol.info(ctx).fullName(ctx) << "\" {" << endl;
    buf << "    label = \"" << this->symbol.info(ctx).fullName(ctx) << "\";" << endl;
    buf << "    color = blue;" << endl;
    buf << "    bb" << this->symbol._id << "_0 [shape = invhouse];" << endl;
    buf << "    bb" << this->symbol._id << "_1 [shape = parallelogram];" << endl << endl;
    for (int i = 0; i < this->basicBlocks.size(); i++) {
        auto text = this->basicBlocks[i]->toString(ctx);
        buf << "    bb" << this->symbol._id << "_" << this->basicBlocks[i]->id << " [label = \"" << text << "\"];"
            << endl;
        buf << "    bb" << this->symbol._id << "_" << i << " -> bb" << this->symbol._id << "_"
            << this->basicBlocks[i]->bexit.thenb->id << ";" << endl;
        if (this->basicBlocks[i]->bexit.thenb != this->basicBlocks[i]->bexit.elseb) {
            buf << "    bb" << this->symbol._id << "_" << i << " -> bb" << this->symbol._id << "_"
                << this->basicBlocks[i]->bexit.elseb->id << ";" << endl
                << endl;
        }
    }
    buf << "}";
    return buf.str();
}

string BasicBlock::toString(core::Context ctx) {
    stringstream buf;
    buf << "(";
    bool first = true;
    for (core::LocalVariable arg : this->args) {
        if (!first) {
            buf << ", ";
        }
        first = false;
        buf << arg.name.name(ctx).toString(ctx);
    }
    buf << ")\\n";
    if (this->outerLoops > 0) {
        buf << "outerLoops: " << this->outerLoops << "\\n";
    }
    for (Binding &exp : this->exprs) {
        buf << exp.bind.name.name(ctx).toString(ctx) << " = " << exp.value->toString(ctx);
        if (exp.tpe) {
            buf << " : " << Strings::escapeCString(exp.tpe->toString(ctx));
        }
        buf << "\\n"; // intentional! graphviz will do interpolation.
    }
    if (this->bexit.cond.exists()) {
        buf << this->bexit.cond.name.name(ctx).toString(ctx);
    } else {
        buf << "<unconditional>";
    }
    return buf.str();
}

Binding::Binding(core::LocalVariable bind, core::Loc loc, unique_ptr<Instruction> value)
    : bind(bind), loc(loc), value(move(value)) {}

Return::Return(core::LocalVariable what) : what(what) {}

string Return::toString(core::Context ctx) {
    return "return " + this->what.name.name(ctx).toString(ctx);
}

Send::Send(core::LocalVariable recv, core::NameRef fun, vector<core::LocalVariable> &args)
    : recv(recv), fun(fun), args(move(args)) {}

FloatLit::FloatLit(float value) : value(value) {}

string FloatLit::toString(core::Context ctx) {
    return to_string(this->value);
}

IntLit::IntLit(int64_t value) : value(value) {}

string IntLit::toString(core::Context ctx) {
    return to_string(this->value);
}

Ident::Ident(core::LocalVariable what) : what(what) {}

Alias::Alias(core::SymbolRef what) : what(what) {}

string Ident::toString(core::Context ctx) {
    return this->what.name.name(ctx).toString(ctx);
}

string Alias::toString(core::Context ctx) {
    return "alias " + this->what.info(ctx).name.name(ctx).toString(ctx);
}

string Send::toString(core::Context ctx) {
    stringstream buf;
    buf << this->recv.name.name(ctx).toString(ctx) << "." << this->fun.name(ctx).toString(ctx) << "(";
    bool isFirst = true;
    for (auto arg : this->args) {
        if (!isFirst) {
            buf << ", ";
        }
        isFirst = false;
        buf << arg.name.name(ctx).toString(ctx);
    }
    buf << ")";
    return buf.str();
}

string StringLit::toString(core::Context ctx) {
    return this->value.name(ctx).toString(ctx);
}

string BoolLit::toString(core::Context ctx) {
    if (value) {
        return "true";
    } else {
        return "false";
    }
}

string Nil::toString(core::Context ctx) {
    return "nil";
}

string Self::toString(core::Context ctx) {
    return "self";
}

string LoadArg::toString(core::Context ctx) {
    stringstream buf;
    buf << "load_arg(";
    buf << this->receiver.name.name(ctx).toString(ctx);
    buf << "#";
    buf << this->method.name(ctx).toString(ctx);
    buf << ", " << this->arg << ")";
    return buf.str();
}

string NotSupported::toString(core::Context ctx) {
    return "NotSupported(" + why + ")";
}

CFGContext CFGContext::withTarget(core::LocalVariable target) {
    auto ret = CFGContext(*this);
    ret.target = target;
    return ret;
}

CFGContext CFGContext::withScope(BasicBlock *scope) {
    auto ret = CFGContext(*this);
    ret.scope = scope;
    ret.loops += 1;
    return ret;
}

} // namespace cfg
} // namespace ruby_typer
