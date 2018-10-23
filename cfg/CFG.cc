#include "cfg/CFG.h"
#include <algorithm>
#include <unordered_set>

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
    fmt::memory_buffer buf;
    string symbolName = this->symbol.data(ctx)->fullName(ctx);
    fmt::format_to(buf,
                   "subgraph \"cluster_{}\" {{\n"
                   "    label = \"{}\";\n"
                   "    color = blue;\n"
                   "    \"bb{}_0\" [shape = invhouse];\n"
                   "    \"bb{}_1\" [shape = parallelogram];\n\n",
                   symbolName, symbolName, symbolName, symbolName);
    for (auto &basicBlock : this->basicBlocks) {
        auto text = basicBlock->toString(ctx);
        auto lines = absl::StrSplit(text, "\n");

        fmt::format_to(
            buf,
            "    \"bb{}_{}\" [\n"
            "        label = \"{}\\l\"\n"
            "    ];\n\n"
            "    \"bb{}_{}\" -> \"bb{}_{}\" [style=\"bold\"];\n",
            symbolName, basicBlock->id,
            fmt::map_join(lines.begin(), lines.end(), "\\l", [](auto line) -> string { return absl::CEscape(line); }),
            symbolName, basicBlock->id, symbolName, basicBlock->bexit.thenb->id);

        if (basicBlock->bexit.thenb != basicBlock->bexit.elseb) {
            fmt::format_to(buf, "    \"bb{}_{}\" -> \"bb{}_{}\" [style=\"tapered\"];\n\n", symbolName, basicBlock->id,
                           symbolName, basicBlock->bexit.elseb->id);
        }
    }
    fmt::format_to(buf, "}}");
    return to_string(buf);
}

string BasicBlock::toString(core::Context ctx) {
    fmt::memory_buffer buf;
    fmt::format_to(buf, "block[id={}]({})\n", this->id,
                   fmt::map_join(this->args.begin(), this->args.end(),
                                 ", ", [&](const auto &arg) -> auto { return arg.toString(ctx); }));

    if (this->outerLoops > 0) {
        fmt::format_to(buf, "outerLoops: {}\n", this->outerLoops);
    }
    for (Binding &exp : this->exprs) {
        if (isa_instruction<DebugEnvironment>(exp.value.get())) {
            fmt::format_to(buf, "{}", exp.value->toString(ctx));
            continue;
        }
        fmt::format_to(buf, "{} = {}\n", exp.bind.toString(ctx), exp.value->toString(ctx));
    }
    if (this->bexit.cond.variable.exists()) {
        fmt::format_to(buf, "{}", this->bexit.cond.toString(ctx));
    } else {
        fmt::format_to(buf, "<unconditional>");
    }
    return to_string(buf);
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
