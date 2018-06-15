#include "CFG.h"
#include <algorithm>
#include <sstream>
#include <unordered_set>

#include "absl/strings/escaping.h"
#include "absl/strings/str_split.h"

// helps debugging
template class std::unique_ptr<sorbet::cfg::CFG>;
template class std::unique_ptr<sorbet::cfg::BasicBlock>;
template class std::vector<sorbet::cfg::BasicBlock *>;

using namespace std;

namespace sorbet {
namespace cfg {

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

        if (bb.get() == deadBlock()) {
            continue;
        }

        auto thenCount = count(bb->bexit.thenb->backEdges.begin(), bb->bexit.thenb->backEdges.end(), bb.get());
        auto elseCount = count(bb->bexit.elseb->backEdges.begin(), bb->bexit.elseb->backEdges.end(), bb.get());
        ENFORCE(thenCount == 1, "bb id=", bb->id, "; then has ", thenCount, " back edges");
        ENFORCE(elseCount == 1, "bb id=", bb->id, "; else has ", elseCount, " back edges");
        if (bb->bexit.thenb == bb->bexit.elseb) {
            ENFORCE(!bb->bexit.cond.exists());
        } else {
            ENFORCE(bb->bexit.cond.exists());
        }
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

string CFG::toString(core::Context ctx) {
    stringstream buf;
    string symbolName = this->symbol.data(ctx).fullName(ctx);
    buf << "subgraph \"cluster_" << symbolName << "\" {" << '\n';
    buf << "    label = \"" << symbolName << "\";" << '\n';
    buf << "    color = blue;" << '\n';
    buf << "    \"bb" << symbolName << "_0\" [shape = invhouse];" << '\n';
    buf << "    \"bb" << symbolName << "_1\" [shape = parallelogram];" << '\n' << '\n';
    for (auto &basicBlock : this->basicBlocks) {
        auto text = basicBlock->toString(ctx);
        auto lines = absl::StrSplit(text, "\n");
        stringstream escaped;
        bool first = true;
        for (auto &line : lines) {
            if (!first) {
                escaped << '\n';
            }
            first = false;
            escaped << absl::CEscape(line);
        }
        string label = escaped.str();
        size_t start_pos = 0;
        while ((start_pos = label.find("\n", start_pos)) != string::npos) {
            label.replace(start_pos, 1, "\\l");
            start_pos += 2;
        }
        label += "\\l";

        buf << "    \"bb" << symbolName << "_" << basicBlock->id << "\" [" << '\n';
        buf << "        label = \"" << label << "\"" << '\n';
        buf << "    ];" << '\n' << '\n';
        buf << "    \"bb" << symbolName << "_" << basicBlock->id << "\" -> \"bb" << symbolName << "_"
            << basicBlock->bexit.thenb->id << R"(" [style="bold"];)" << '\n';
        if (basicBlock->bexit.thenb != basicBlock->bexit.elseb) {
            buf << "    \"bb" << symbolName << "_" << basicBlock->id << "\" -> \"bb" << symbolName << "_"
                << basicBlock->bexit.elseb->id << R"(" [style="tapered"];)" << '\n'
                << '\n';
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
        buf << arg.toString(ctx);
    }
    buf << ")" << '\n';
    if (this->outerLoops > 0) {
        buf << "outerLoops: " << this->outerLoops << '\n';
    }
    for (Binding &exp : this->exprs) {
        if (isa_instruction<DebugEnvironment>(exp.value.get())) {
            buf << exp.value->toString(ctx);
            continue;
        }
        buf << exp.bind.toString(ctx) << " = " << exp.value->toString(ctx);
        if (exp.tpe) {
            buf << " : " << exp.tpe->toString(ctx);
        }
        buf << '\n';
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
        if (auto debugEnv = cast_instruction<DebugEnvironment>(exp.value.get())) {
            ctx.state.addAnnotation(exp.loc, exp.value->toString(ctx), debugEnv->pos);
        }
    }
}

core::Loc BasicBlock::loc() const {
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
} // namespace sorbet
