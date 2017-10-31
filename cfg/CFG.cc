
#include "CFG.h"
#include <algorithm>
#include <algorithm> // sort
#include <sstream>
#include <unordered_map>
#include <unordered_set>

// helps debugging
template class std::unique_ptr<ruby_typer::cfg::CFG>;

using namespace std;

namespace ruby_typer {
namespace cfg {

void CFG::fillInBlockArguments(ast::Context ctx) {
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
    unordered_map<ast::SymbolRef, unordered_set<BasicBlock *>> reads;
    unordered_map<ast::SymbolRef, unordered_set<BasicBlock *>> writes;

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
            } else if (auto *v = dynamic_cast<New *>(bind.value.get())) {
                for (auto arg : v->args) {
                    reads[arg].insert(bb.get());
                }
            } else if (auto *v = dynamic_cast<Super *>(bind.value.get())) {
                for (auto arg : v->args) {
                    reads[arg].insert(bb.get());
                }
            } else if (auto *v = dynamic_cast<Return *>(bind.value.get())) {
                reads[v->what].insert(bb.get());
            } else if (auto *v = dynamic_cast<Return *>(bind.value.get())) {
                reads[v->what].insert(bb.get());
            } else if (auto *v = dynamic_cast<NamedArg *>(bind.value.get())) {
                reads[v->value].insert(bb.get());
            } else if (auto *v = dynamic_cast<LoadArg *>(bind.value.get())) {
                reads[v->receiver].insert(bb.get());
            }
        }
        if (bb->bexit.cond != ctx.state.defn_cfg_never() && bb->bexit.cond != ctx.state.defn_cfg_always()) {
            reads[bb->bexit.cond].insert(bb.get());
        }
    }

    unordered_map<BasicBlock *, unordered_set<ast::SymbolRef>> reads_by_block;
    unordered_map<BasicBlock *, unordered_set<ast::SymbolRef>> writes_by_block;

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
            reads_by_block[bb].insert(wts.first);
        }
        for (BasicBlock *bb : wts.second) {
            writes_by_block[bb].insert(wts.first);
        }
    }

    // iterate ver basic blocks in reverse and found upper bounds on what could a block need.
    unordered_map<BasicBlock *, unordered_set<ast::SymbolRef>> upper_bounds1;
    bool changed = true;

    while (changed) {
        changed = false;
        for (BasicBlock *bb : this->forwardsTopoSort) {
            int sz = upper_bounds1[bb].size();
            upper_bounds1[bb].insert(reads_by_block[bb].begin(), reads_by_block[bb].end());
            if (bb->bexit.thenb != deadBlock()) {
                upper_bounds1[bb].insert(upper_bounds1[bb->bexit.thenb].begin(), upper_bounds1[bb->bexit.thenb].end());
            }
            if (bb->bexit.elseb != deadBlock()) {
                upper_bounds1[bb].insert(upper_bounds1[bb->bexit.elseb].begin(), upper_bounds1[bb->bexit.elseb].end());
            }
            changed = changed || (upper_bounds1[bb].size() != sz);
        }
    }

    unordered_map<BasicBlock *, unordered_set<ast::SymbolRef>> upper_bounds2;

    changed = true;
    while (changed) {
        changed = false;
        for (auto it = this->backwardsTopoSort.begin(); it != this->backwardsTopoSort.end(); ++it) {
            BasicBlock *bb = *it;
            int sz = upper_bounds2[bb].size();
            upper_bounds2[bb].insert(writes_by_block[bb].begin(), writes_by_block[bb].end());
            for (BasicBlock *edge : bb->backEdges) {
                if (edge != deadBlock()) {
                    upper_bounds2[bb].insert(upper_bounds2[edge].begin(), upper_bounds2[edge].end());
                }
            }
            changed = changed || (upper_bounds2[bb].size() != sz);
        }
    }

    for (auto &it : this->basicBlocks) {
        auto set2 = upper_bounds2[it.get()];

        for (auto el : upper_bounds1[it.get()]) {
            if (set2.find(el) != set2.end()) {
                it->args.push_back(el);
            }
        }
        std::sort(it->args.begin(), it->args.end(),
                  [](ast::SymbolRef a, ast::SymbolRef b) -> bool { return a._id < b._id; });
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
    if ((currentBB->flags & BACKWARD_TOPO_SORT_VISITED)) {
        return nextFree;
    } else {
        currentBB->flags |= BACKWARD_TOPO_SORT_VISITED;
        for (BasicBlock *edge : currentBB->backEdges) {
            nextFree = topoSortBwd(target, nextFree, edge);
        }
        target[nextFree] = currentBB;
        return nextFree + 1;
    }
}

void CFG::fillInTopoSorts(ast::Context ctx) {
    auto &target1 = this->forwardsTopoSort;
    target1.resize(this->basicBlocks.size());
    this->topoSortFwd(target1, 0, this->entry());

    auto &target2 = this->backwardsTopoSort;
    target2.resize(this->basicBlocks.size());
    this->topoSortBwd(target2, 0, this->deadBlock());
    return;
}

void jumpToDead(BasicBlock *from, CFG &inWhat);

unique_ptr<CFG> CFG::buildFor(ast::Context ctx, ast::MethodDef &md) {
    unique_ptr<CFG> res(new CFG); // private constructor
    res->symbol = md.symbol;
    ast::SymbolRef retSym = ctx.state.newTemporary(ast::UniqueNameKind::CFG, ast::Names::returnMethodTemp(), md.symbol);
    auto cont = res->walk(ctx, md.rhs.get(), res->entry(), *res.get(), retSym, 0);
    ast::SymbolRef retSym1 =
        ctx.state.newTemporary(ast::UniqueNameKind::CFG, ast::Names::returnMethodTemp(), md.symbol);

    cont->exprs.emplace_back(retSym1, make_unique<Return>(retSym)); // dead assign.
    jumpToDead(cont, *res.get());

    res->fillInTopoSorts(ctx);
    res->fillInBlockArguments(ctx);
    return res;
}

BasicBlock *CFG::freshBlock(int outerLoops) {
    this->basicBlocks.emplace_back(new BasicBlock());
    BasicBlock *r = this->basicBlocks.back().get();
    r->outerLoops = outerLoops;
    return r;
}

CFG::CFG() {
    freshBlock(0); // entry;
    freshBlock(0); // dead code;
    deadBlock()->bexit.elseb = deadBlock();
    deadBlock()->bexit.thenb = deadBlock();
    deadBlock()->bexit.cond = ast::GlobalState::defn_cfg_never();
}

void conditionalJump(BasicBlock *from, ast::SymbolRef cond, BasicBlock *thenb, BasicBlock *elseb, CFG &inWhat) {
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
        from->bexit.cond = ast::GlobalState::defn_cfg_always();
        from->bexit.elseb = to;
        from->bexit.thenb = to;
        to->backEdges.push_back(from);
    }
}

void jumpToDead(BasicBlock *from, CFG &inWhat) {
    auto *db = inWhat.deadBlock();
    if (from != db) {
        Error::check(!from->bexit.cond.exists());
        from->bexit.cond = ast::GlobalState::defn_cfg_never();
        from->bexit.elseb = db;
        from->bexit.thenb = db;
        db->backEdges.push_back(from);
    }
}

/** Convert `what` into a cfg, by starting to evaluate it in `current` inside method defined by `inWhat`.
 * store result of evaluation into `target`. Returns basic block in which evaluation should proceed.
 */
BasicBlock *CFG::walk(ast::Context ctx, ast::Statement *what, BasicBlock *current, CFG &inWhat, ast::SymbolRef target, int loops) {
    /** Try to pay additional attention not to duplicate any part of tree.
     * Though this may lead to more effictient and a better CFG if it was to be actually compiled into code
     * This will lead to duplicate typechecking and may lead to exponential explosion of typechecking time
     * for some code snippets. */
    Error::check(!current->bexit.cond.exists());

    BasicBlock *ret = nullptr;
    typecase(what,
             [&](ast::While *a) {
                 auto headerBlock = inWhat.freshBlock(loops + 1);
                 unconditionalJump(current, headerBlock, inWhat);

                 ast::SymbolRef condSym =
                     ctx.state.newTemporary(ast::UniqueNameKind::CFG, ast::Names::whileTemp(), inWhat.symbol);
                 auto headerEnd = walk(ctx, a->cond.get(), headerBlock, inWhat, condSym, loops + 1);
                 auto bodyBlock = inWhat.freshBlock(loops + 1);
                 auto continueBlock = inWhat.freshBlock(loops);
                 conditionalJump(headerEnd, condSym, bodyBlock, continueBlock, inWhat);
                 // finishHeader
                 ast::SymbolRef bodySym =
                     ctx.state.newTemporary(ast::UniqueNameKind::CFG, ast::Names::statTemp(), inWhat.symbol);

                 auto body = walk(ctx, a->body.get(), bodyBlock, inWhat, bodySym, loops + 1);
                 unconditionalJump(body, headerBlock, inWhat);

                 continueBlock->exprs.emplace_back(target, make_unique<Nil>());
                 ret = continueBlock;
             },
             [&](ast::Return *a) {
                 ast::SymbolRef retSym =
                     ctx.state.newTemporary(ast::UniqueNameKind::CFG, ast::Names::returnTemp(), inWhat.symbol);
                 auto cont = walk(ctx, a->expr.get(), current, inWhat, retSym, loops);
                 cont->exprs.emplace_back(target, make_unique<Return>(retSym)); // dead assign.
                 jumpToDead(cont, inWhat);
                 ret = deadBlock();
             },
             [&](ast::If *a) {
                 ast::SymbolRef ifSym =
                     ctx.state.newTemporary(ast::UniqueNameKind::CFG, ast::Names::ifTemp(), inWhat.symbol);
                 Error::check(ifSym.exists());
                 auto thenBlock = inWhat.freshBlock(loops);
                 auto elseBlock = inWhat.freshBlock(loops);
                 auto cont = walk(ctx, a->cond.get(), current, inWhat, ifSym, loops);
                 conditionalJump(cont, ifSym, thenBlock, elseBlock, inWhat);

                 auto thenEnd = walk(ctx, a->thenp.get(), thenBlock, inWhat, target, loops);
                 auto elseEnd = walk(ctx, a->elsep.get(), elseBlock, inWhat, target, loops);
                 if (thenEnd != deadBlock() || elseEnd != deadBlock()) {
                     if (thenEnd == deadBlock()) {
                         ret = elseEnd;
                     } else if (thenEnd == deadBlock()) {
                         ret = thenEnd;
                     } else {
                         ret = freshBlock(loops);
                         unconditionalJump(thenEnd, ret, inWhat);
                         unconditionalJump(elseEnd, ret, inWhat);
                     }
                 } else {
                     ret = deadBlock();
                 }
             },
             [&](ast::IntLit *a) {
                 current->exprs.emplace_back(target, make_unique<IntLit>(a->value));
                 ret = current;
             },
             [&](ast::FloatLit *a) {
                 current->exprs.emplace_back(target, make_unique<FloatLit>(a->value));
                 ret = current;
             },
             [&](ast::StringLit *a) {
                 current->exprs.emplace_back(target, make_unique<StringLit>(a->value));
                 ret = current;
             },
             [&](ast::BoolLit *a) {
                 current->exprs.emplace_back(target, make_unique<BoolLit>(a->value));
                 ret = current;
             },
             [&](ast::ConstantLit *a) {
                 current->exprs.emplace_back(target, make_unique<ConstantLit>(a->cnst));
                 ret = current;
             },
             [&](ast::Ident *a) {
                 current->exprs.emplace_back(target, make_unique<Ident>(a->symbol));
                 ret = current;
             },
             [&](ast::Self *a) {
                 current->exprs.emplace_back(target, make_unique<Self>(a->claz));
                 ret = current;
             },
             [&](ast::Assign *a) {
                 auto lhsIdent = dynamic_cast<ast::Ident *>(a->lhs.get());
                 Error::check(lhsIdent != nullptr);
                 auto rhsCont = walk(ctx, a->rhs.get(), current, inWhat, lhsIdent->symbol, loops);
                 rhsCont->exprs.emplace_back(target, make_unique<Ident>(lhsIdent->symbol));
                 ret = rhsCont;
             },
             [&](ast::InsSeq *a) {
                 for (auto &exp : a->stats) {
                     ast::SymbolRef temp =
                         ctx.state.newTemporary(ast::UniqueNameKind::CFG, ast::Names::statTemp(), inWhat.symbol);
                     current = walk(ctx, exp.get(), current, inWhat, temp, loops);
                 }
                 ret = walk(ctx, a->expr.get(), current, inWhat, target, loops);
             },
             [&](ast::Send *s) {
                 ast::SymbolRef recv;
                 if (auto *i = dynamic_cast<ast::Ident *>(s->recv.get())) {
                     recv = i->symbol;
                 } else {
                     recv = ctx.state.newTemporary(ast::UniqueNameKind::CFG, ast::Names::statTemp(), inWhat.symbol);
                     current = walk(ctx, s->recv.get(), current, inWhat, recv, loops);
                 }

                 vector<ast::SymbolRef> args;
                 for (auto &exp : s->args) {
                     ast::SymbolRef temp;
                     if (auto *i = dynamic_cast<ast::Ident *>(exp.get())) {
                         temp = i->symbol;
                     } else {
                         temp = ctx.state.newTemporary(ast::UniqueNameKind::CFG, ast::Names::statTemp(), inWhat.symbol);
                         current = walk(ctx, exp.get(), current, inWhat, temp, loops);
                     }
                     args.push_back(temp);
                 }

                 if (s->block != nullptr) {
                     auto headerBlock = inWhat.freshBlock(loops + 1);
                     auto postBlock = inWhat.freshBlock(loops);
                     auto bodyBlock = inWhat.freshBlock(loops + 1);

                     for (int i = 0; i < s->block->args.size(); ++i) {
                         auto &arg = s->block->args[i];

                         if (auto id = dynamic_cast<ast::Ident *>(arg.get())) {
                             headerBlock->exprs.emplace_back(id->symbol, make_unique<LoadArg>(recv, s->fun, i));
                         } else {
                             // TODO(nelhage): this will be an error once the namer
                             // is more complete and turns all args into Ident
                         }
                     }

                     unconditionalJump(current, headerBlock, inWhat);

                     conditionalJump(headerBlock, ctx.state.defn_cfg_block_call(), bodyBlock, postBlock, inWhat);

                     // TODO: handle block arguments somehow??
                     ast::SymbolRef blockrv =
                         ctx.state.newTemporary(ast::UniqueNameKind::CFG, ast::Names::blockReturnTemp(), inWhat.symbol);
                     auto blockLast = walk(ctx, s->block->body.get(), bodyBlock, inWhat, blockrv, loops + 1);

                     unconditionalJump(blockLast, headerBlock, inWhat);

                     current = postBlock;
                 }

                 current->exprs.emplace_back(target, make_unique<Send>(recv, s->fun, args));
                 ret = current;
             },

             [&](ast::Statement *n) {
                 current->exprs.emplace_back(target, make_unique<NotSupported>(""));
                 ret = current;
             },

             [&](ast::Block *a) { Error::raise("should never encounter a bare Block"); });

    /*[&](ast::Break *a) {}, [&](ast::Next *a) {},*/
    // For, Next, Rescue,
    // Symbol, Send, New, Super, NamedArg, Hash, Array,
    // ArraySplat, HashAplat, Block,
    Error::check(ret != nullptr);
    return ret;
}

string CFG::toString(ast::Context ctx) {
    stringstream buf;
    buf << "digraph " << this->symbol.info(ctx).name.name(ctx).toString(ctx) << " {" << endl;
    buf << "    bb0 [shape = invhouse];" << endl;
    buf << "    bb1 [shape = parallelogram];" << endl << endl;
    for (int i = 0; i < this->basicBlocks.size(); i++) {
        auto text = this->basicBlocks[i]->toString(ctx);
        buf << "    bb" << i << " [label = \"" << text << "\"];" << endl;
        auto thenI = find_if(this->basicBlocks.begin(), this->basicBlocks.end(),
                             [&](auto &a) { return a.get() == this->basicBlocks[i]->bexit.thenb; });
        auto elseI = find_if(this->basicBlocks.begin(), this->basicBlocks.end(),
                             [&](auto &a) { return a.get() == this->basicBlocks[i]->bexit.elseb; });
        buf << "    bb" << i << " -> bb" << thenI - this->basicBlocks.begin() << ";" << endl;
        if (this->basicBlocks[i]->bexit.cond != ctx.state.defn_cfg_always() &&
            this->basicBlocks[i]->bexit.cond != ctx.state.defn_cfg_never()) {
            buf << "    bb" << i << " -> bb" << elseI - this->basicBlocks.begin() << ";" << endl << endl;
        }
    }
    buf << "}";
    return buf.str();
}

string BasicBlock::toString(ast::Context ctx) {
    stringstream buf;
    buf << "(";
    bool first = true;
    for (ast::SymbolRef arg : this->args) {
        if (!first) {
            buf << ", ";
        }
        first = false;
        buf << arg.info(ctx).name.name(ctx).toString(ctx);
    }
    buf << ")\\n";
    if (this->outerLoops > 0) {
        buf << "outerLoops: " << this->outerLoops << "\\n";
    }
    for (auto &exp : this->exprs) {
        buf << exp.bind.info(ctx).name.name(ctx).toString(ctx) << " = " << exp.value->toString(ctx);
        buf << "\\n"; // intentional! graphviz will do interpolation.
    }
    buf << this->bexit.cond.info(ctx).name.name(ctx).toString(ctx);
    return buf.str();
}

Binding::Binding(const ast::SymbolRef &bind, unique_ptr<Instruction> value) : bind(bind), value(move(value)) {}

Return::Return(const ast::SymbolRef &what) : what(what) {}

string Return::toString(ast::Context ctx) {
    return "return " + this->what.info(ctx).name.name(ctx).toString(ctx);
}

New::New(const ast::SymbolRef &klaz, vector<ast::SymbolRef> &args) : klass(klaz), args(move(args)) {}

Send::Send(ast::SymbolRef recv, ast::NameRef fun, std::vector<ast::SymbolRef> &args)
    : recv(recv), fun(fun), args(std::move(args)) {}

string New::toString(ast::Context ctx) {
    stringstream buf;
    buf << "new " << this->klass.info(ctx).name.name(ctx).toString(ctx) << "(";
    bool isFirst = true;
    for (auto arg : this->args) {
        if (!isFirst) {
            buf << " ,";
        }
        isFirst = true;
        buf << arg.info(ctx).name.name(ctx).toString(ctx);
    }
    buf << ")";
    return buf.str();
}

Super::Super(vector<ast::SymbolRef> &args) : args(move(args)) {}

string Super::toString(ast::Context ctx) {
    stringstream buf;
    buf << "super(";
    bool isFirst = true;
    for (auto arg : this->args) {
        if (!isFirst) {
            buf << " ,";
        }
        isFirst = true;
        buf << arg.info(ctx).name.name(ctx).toString(ctx);
    }
    buf << ")";
    return buf.str();
}

FloatLit::FloatLit(float value) : value(value) {}

string FloatLit::toString(ast::Context ctx) {
    return to_string(this->value);
}

IntLit::IntLit(int value) : value(value) {}

string IntLit::toString(ast::Context ctx) {
    return to_string(this->value);
}

Ident::Ident(const ast::SymbolRef &what) : what(what) {}

string Ident::toString(ast::Context ctx) {
    return this->what.info(ctx).name.name(ctx).toString(ctx);
}

string Send::toString(ast::Context ctx) {
    stringstream buf;
    buf << this->recv.info(ctx).name.name(ctx).toString(ctx) << "." << this->fun.name(ctx).toString(ctx) << "(";
    bool isFirst = true;
    for (auto arg : this->args) {
        if (!isFirst) {
            buf << " ,";
        }
        isFirst = true;
        buf << arg.info(ctx).name.name(ctx).toString(ctx);
    }
    buf << ")";
    return buf.str();
}

string StringLit::toString(ast::Context ctx) {
    return this->value.name(ctx).toString(ctx);
}

string BoolLit::toString(ast::Context ctx) {
    if (value) {
        return "true";
    } else {
        return "false";
    }
}

string ConstantLit::toString(ast::Context ctx) {
    return this->cnst.name(ctx).toString(ctx);
}

string Nil::toString(ast::Context ctx) {
    return "nil";
}

string Self::toString(ast::Context ctx) {
    return "self";
}

string LoadArg::toString(ast::Context ctx) {
    stringstream buf;
    buf << "load_arg(";
    buf << this->receiver.info(ctx).name.name(ctx).toString(ctx);
    buf << "#";
    buf << this->method.name(ctx).toString(ctx);
    buf << ", " << this->arg << ")";
    return buf.str();
}

string NotSupported::toString(ast::Context ctx) {
    return "NotSupported(" + why + ")";
}
} // namespace cfg
} // namespace ruby_typer
