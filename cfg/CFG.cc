#include "CFG.h"
#include <algorithm>
#include <sstream>
#include <unordered_set>

#include "absl/strings/escaping.h"
#include "absl/strings/str_split.h"

// helps debugging
template class std::unique_ptr<ruby_typer::cfg::CFG>;
template class std::unique_ptr<ruby_typer::cfg::BasicBlock>;

using namespace std;

namespace ruby_typer {
namespace cfg {

int CFG::FORWARD_TOPO_SORT_VISITED = 1 << 0;
int CFG::BACKWARD_TOPO_SORT_VISITED = 1 << 1;
int CFG::LOOP_HEADER = 1 << 2;
int CFG::WAS_JUMP_DESTINATION = 1 << 3;

BasicBlock *CFG::freshBlock(int outerLoops) {
    int id = this->maxBasicBlockId++;
    this->basicBlocks.emplace_back(new BasicBlock());
    BasicBlock *r = this->basicBlocks.back().get();
    r->id = id;
    r->outerLoops = outerLoops;
    return r;
}

CFG::CFG() {
    freshBlock(0); // entry;
    freshBlock(0); // dead code;
    deadBlock()->bexit.elseb = deadBlock();
    deadBlock()->bexit.thenb = deadBlock();
    deadBlock()->bexit.cond = core::LocalVariable::noVariable();
}

CFG::ReadsAndWrites CFG::findAllReadsAndWrites(core::Context ctx) {
    unordered_map<core::LocalVariable, unordered_set<BasicBlock *>> reads;
    unordered_map<core::LocalVariable, unordered_set<BasicBlock *>> writes;
    unordered_map<core::LocalVariable, unordered_set<BasicBlock *>> kills;

    for (unique_ptr<BasicBlock> &bb : this->basicBlocks) {
        for (Binding &bind : bb->exprs) {
            writes[bind.bind].insert(bb.get());
            auto fnd = reads.find(bind.bind);
            if (fnd == reads.end() || fnd->second.count(bb.get()) == 0) {
                kills[bind.bind].insert(bb.get());
            }

            /*
             * When we write to an alias, we rely on the type information being
             * propagated through block arguments from the point of
             * assignment. Treating every write as also reading from the
             * variable serves to represent this.
             */
            if (bind.bind.isAliasForGlobal(ctx) && cast_instruction<Alias>(bind.value.get()) == nullptr) {
                reads[bind.bind].insert(bb.get());
            }

            if (auto *v = cast_instruction<Ident>(bind.value.get())) {
                reads[v->what].insert(bb.get());
            } else if (auto *v = cast_instruction<Send>(bind.value.get())) {
                reads[v->recv].insert(bb.get());
                for (auto arg : v->args) {
                    reads[arg].insert(bb.get());
                }
            } else if (auto *v = cast_instruction<Return>(bind.value.get())) {
                reads[v->what].insert(bb.get());
            } else if (auto *v = cast_instruction<BlockReturn>(bind.value.get())) {
                reads[v->what].insert(bb.get());
            } else if (auto *v = cast_instruction<LoadArg>(bind.value.get())) {
                reads[v->receiver].insert(bb.get());
            } else if (auto *v = cast_instruction<Cast>(bind.value.get())) {
                reads[v->value].insert(bb.get());
            }
        }
        if (bb->bexit.cond.exists()) {
            reads[bb->bexit.cond].insert(bb.get());
        }
    }
    return CFG::ReadsAndWrites{move(reads), move(writes), move(kills)};
}

void CFG::sanityCheck(core::Context ctx) {
    if (!debug_mode) {
        return;
    }

    for (auto &bb : this->basicBlocks) {
        ENFORCE(bb->bexit.isCondSet(), "Block exit condition left unset for block " + bb->toString(ctx));
    }

    // check that synthetic variable that is read is ever written to.
    ReadsAndWrites RnW = CFG::findAllReadsAndWrites(ctx);
    for (auto &el : RnW.reads) {
        if (!el.first.isSyntheticTemporary(ctx)) {
            continue;
        }
        //        ENFORCE(writes.find(el.first) != writes.end());
    }
}

string CFG::toString(const core::Context ctx) {
    stringstream buf;
    string symbolName = this->symbol.data(ctx).fullName(ctx);
    buf << "subgraph \"cluster_" << symbolName << "\" {" << endl;
    buf << "    label = \"" << symbolName << "\";" << endl;
    buf << "    color = blue;" << endl;
    buf << "    \"bb" << symbolName << "_0\" [shape = invhouse];" << endl;
    buf << "    \"bb" << symbolName << "_1\" [shape = parallelogram];" << endl << endl;
    for (auto &basicBlock : this->basicBlocks) {
        auto text = basicBlock->toString(ctx);
        auto lines = absl::StrSplit(text, "\n");
        stringstream escaped;
        bool first = true;
        for (auto &line : lines) {
            if (!first) {
                escaped << endl;
            }
            first = false;
            escaped << absl::CEscape(line);
        }
        buf << "    \"bb" << symbolName << "_" << basicBlock->id << "\" [label = \"" << escaped.str() << "\"];" << endl
            << endl;
        buf << "    \"bb" << symbolName << "_" << basicBlock->id << "\" -> \"bb" << symbolName << "_"
            << basicBlock->bexit.thenb->id << "\" [style=\"bold\"];" << endl;
        if (basicBlock->bexit.thenb != basicBlock->bexit.elseb) {
            buf << "    \"bb" << symbolName << "_" << basicBlock->id << "\" -> \"bb" << symbolName << "_"
                << basicBlock->bexit.elseb->id << "\" [style=\"tapered\"];" << endl
                << endl;
        }
    }
    buf << "}";
    return buf.str();
}

string BasicBlock::toString(const core::Context ctx) {
    stringstream buf;
    buf << "block[id=" << this->id << "](";
    bool first = true;
    for (core::LocalVariable arg : this->args) {
        if (!first) {
            buf << ", ";
        }
        first = false;
        buf << arg.toString(ctx);
    }
    buf << ")" << endl;
    if (this->outerLoops > 0) {
        buf << "outerLoops: " << this->outerLoops << endl;
    }
    for (Binding &exp : this->exprs) {
        if (dynamic_cast<DebugEnvironment *>(exp.value.get()) != nullptr) {
            buf << exp.value->toString(ctx);
            continue;
        }
        buf << exp.bind.toString(ctx) << " = " << exp.value->toString(ctx);
        if (exp.tpe) {
            buf << " : " << exp.tpe->toString(ctx);
        }
        buf << endl;
    }
    if (this->bexit.cond.exists()) {
        buf << this->bexit.cond.toString(ctx);
    } else {
        buf << "<unconditional>";
    }
    return buf.str();
}

void CFG::recordAnnotations(core::Context ctx) {
    for (auto &basicBlock : this->basicBlocks) {
        basicBlock->recordAnnotations(ctx);
    }
}

void BasicBlock::recordAnnotations(core::Context ctx) {
    // Exit out if there was no user written code
    if (this->exprs.size() <= 1) {
        return;
    }

    for (Binding &exp : this->exprs) {
        if (auto debugEnv = dynamic_cast<DebugEnvironment *>(exp.value.get())) {
            ctx.state.addAnnotation(exp.loc, exp.value->toString(ctx), debugEnv->pos);
        }
    }
}

core::Loc BasicBlock::loc() {
    core::Loc loc;
    if (!this->exprs.empty()) {
        loc = loc.join(this->exprs.front().loc);
        loc = loc.join(this->exprs.back().loc);
    }
    loc = loc.join(this->bexit.loc);
    return loc;
}

Binding::Binding(core::LocalVariable bind, core::Loc loc, unique_ptr<Instruction> value)
    : bind(bind), loc(loc), value(move(value)) {}

} // namespace cfg
} // namespace ruby_typer
