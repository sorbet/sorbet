#include "CFG.h"
#include <algorithm>
#include <sstream>
#include <unordered_set>

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

BasicBlock *CFG::freshBlock(int outerLoops, BasicBlock *from) {
    if (from != nullptr && from == deadBlock()) {
        return from;
    }
    int id = this->maxBasicBlockId++;
    this->basicBlocks.emplace_back(new BasicBlock());
    BasicBlock *r = this->basicBlocks.back().get();
    r->id = id;
    if (from != nullptr) {
        r->backEdges.push_back(from);
    }
    r->outerLoops = outerLoops;
    return r;
}

CFG::CFG() {
    freshBlock(0, nullptr); // entry;
    freshBlock(0, nullptr); // dead code;
    deadBlock()->bexit.elseb = deadBlock();
    deadBlock()->bexit.thenb = deadBlock();
    deadBlock()->bexit.cond = core::NameRef::noName();
}

CFG::ReadsAndWrites CFG::findAllReadsAndWrites() {
    unordered_map<core::LocalVariable, unordered_set<BasicBlock *>> reads;
    unordered_map<core::LocalVariable, unordered_set<BasicBlock *>> writes;
    for (unique_ptr<BasicBlock> &bb : this->basicBlocks) {
        for (Binding &bind : bb->exprs) {
            writes[bind.bind].insert(bb.get());
            if (auto *v = cast_instruction<Ident>(bind.value.get())) {
                reads[v->what].insert(bb.get());
            } else if (auto *v = cast_instruction<Send>(bind.value.get())) {
                reads[v->recv].insert(bb.get());
                for (auto arg : v->args) {
                    reads[arg].insert(bb.get());
                }
            } else if (auto *v = cast_instruction<Return>(bind.value.get())) {
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
    return CFG::ReadsAndWrites{move(reads), move(writes)};
}

void CFG::sanityCheck(core::Context ctx) {
    if (!debug_mode)
        return;

    for (auto &bb : this->basicBlocks) {
        ENFORCE(bb->bexit.isCondSet(), "Block exit condition left unset for block " + bb->toString(ctx));
    }

    // check that synthetic variable that is read is ever written to.
    ReadsAndWrites RnW = CFG::findAllReadsAndWrites();
    for (auto &el : RnW.reads) {
        core::Name &nm = el.first.name.name(ctx);
        if (nm.kind != core::NameKind::UNIQUE || nm.unique.uniqueNameKind != core::UniqueNameKind::CFG)
            continue;
        //        ENFORCE(writes.find(el.first) != writes.end());
    }
}

string CFG::toString(core::Context ctx) {
    stringstream buf;
    string symbolName = this->symbol.info(ctx).fullName(ctx);
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
            escaped << Strings::escapeCString(line);
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

string BasicBlock::toString(core::Context ctx) {
    stringstream buf;
    buf << "block[id=" << this->id << "](";
    bool first = true;
    for (core::LocalVariable arg : this->args) {
        if (!first) {
            buf << ", ";
        }
        first = false;
        buf << arg.name.name(ctx).toString(ctx);
    }
    buf << ")" << endl;
    if (this->outerLoops > 0) {
        buf << "outerLoops: " << this->outerLoops << endl;
    }
    for (Binding &exp : this->exprs) {
        if (dynamic_cast<DebugEnvironment *>(exp.value.get())) {
            buf << exp.value->toString(ctx);
            continue;
        }
        buf << exp.bind.name.name(ctx).toString(ctx) << " = " << exp.value->toString(ctx);
        if (exp.tpe) {
            buf << " : " << exp.tpe->toString(ctx);
        }
        buf << endl;
    }
    if (this->bexit.cond.exists()) {
        buf << this->bexit.cond.name.name(ctx).toString(ctx);
    } else {
        buf << "<unconditional>";
    }
    return buf.str();
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
