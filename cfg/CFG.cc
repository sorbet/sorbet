#include "cfg/CFG.h"
#include <algorithm>
#include <sstream>
#include <unordered_set>

#include "absl/algorithm/container.h"
#include "absl/strings/escaping.h"
#include "absl/strings/str_split.h"

// helps debugging
template class std::unique_ptr<sorbet::cfg::CFG>;
template class std::unique_ptr<sorbet::cfg::BasicBlock>;
template class std::vector<sorbet::cfg::BasicBlock *>;

using namespace std;

namespace sorbet::cfg {

BasicBlock *CFG::freshBlock(int outerLoops) {
    int id = this->maxBasicBlockId++;
    auto &r = this->basicBlocks.emplace_back(make_unique<BasicBlock>());
    r->id = id;
    r->outerLoops = outerLoops;
    return r.get();
}

CFG::CFG() {
    freshBlock(0); // entry;
    freshBlock(0); // dead code;
    deadBlock()->bexit.elseb = deadBlock();
    deadBlock()->bexit.thenb = deadBlock();
    deadBlock()->bexit.cond.variable = core::LocalVariable::noVariable();
}

CFG::ReadsAndWrites CFG::findAllReadsAndWrites(core::Context ctx) {
    UnorderedMap<core::LocalVariable, unordered_set<BasicBlock *>> reads;
    UnorderedMap<core::LocalVariable, unordered_set<BasicBlock *>> writes;
    UnorderedMap<core::LocalVariable, unordered_set<BasicBlock *>> dead;

    for (unique_ptr<BasicBlock> &bb : this->basicBlocks) {
        for (Binding &bind : bb->exprs) {
            writes[bind.bind.variable].insert(bb.get());
            /*
             * When we write to an alias, we rely on the type information being
             * propagated through block arguments from the point of
             * assignment. Treating every write as also reading from the
             * variable serves to represent this.
             */
            if (bind.bind.variable.isAliasForGlobal(ctx) && cast_instruction<Alias>(bind.value.get()) == nullptr) {
                reads[bind.bind.variable].insert(bb.get());
            }

            if (auto *v = cast_instruction<Ident>(bind.value.get())) {
                reads[v->what].insert(bb.get());
            } else if (auto *v = cast_instruction<Send>(bind.value.get())) {
                reads[v->recv.variable].insert(bb.get());
                for (auto &arg : v->args) {
                    reads[arg.variable].insert(bb.get());
                }
            } else if (auto *v = cast_instruction<Return>(bind.value.get())) {
                reads[v->what.variable].insert(bb.get());
            } else if (auto *v = cast_instruction<BlockReturn>(bind.value.get())) {
                reads[v->what.variable].insert(bb.get());
            } else if (auto *v = cast_instruction<LoadArg>(bind.value.get())) {
                reads[v->receiver.variable].insert(bb.get());
            } else if (auto *v = cast_instruction<Cast>(bind.value.get())) {
                reads[v->value.variable].insert(bb.get());
            }

            auto fnd = reads.find(bind.bind.variable);
            if (fnd == reads.end() || fnd->second.count(bb.get()) == 0) {
                dead[bind.bind.variable].insert(bb.get());
            }
        }
        if (bb->bexit.cond.variable.exists()) {
            reads[bb->bexit.cond.variable].insert(bb.get());
        }
    }
    return CFG::ReadsAndWrites{move(reads), move(writes), move(dead)};
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

        auto thenCount = absl::c_count(bb->bexit.thenb->backEdges, bb.get());
        auto elseCount = absl::c_count(bb->bexit.elseb->backEdges, bb.get());
        ENFORCE(thenCount == 1, "bb id=", bb->id, "; then has ", thenCount, " back edges");
        ENFORCE(elseCount == 1, "bb id=", bb->id, "; else has ", elseCount, " back edges");
        if (bb->bexit.thenb == bb->bexit.elseb) {
            ENFORCE(!bb->bexit.cond.variable.exists());
        } else {
            ENFORCE(bb->bexit.cond.variable.exists());
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
    string symbolName = this->symbol.data(ctx)->fullName(ctx);
    buf << "subgraph \"cluster_" << symbolName << "\" {" << '\n';
    buf << "    label = \"" << symbolName << "\";" << '\n';
    buf << "    color = blue;" << '\n';
    buf << "    \"bb" << symbolName << "_0\" [shape = invhouse];" << '\n';
    buf << "    \"bb" << symbolName << "_1\" [shape = parallelogram];" << '\n' << '\n';
    for (auto &basicBlock : this->basicBlocks) {
        auto text = basicBlock->toString(ctx);
        auto lines = absl::StrSplit(text, '\n');
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
        while ((start_pos = label.find('\n', start_pos)) != string::npos) {
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
    for (const cfg::VariableUseSite &arg : this->args) {
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
        buf << '\n';
    }
    if (this->bexit.cond.variable.exists()) {
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

} // namespace sorbet::cfg
