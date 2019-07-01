#include "cfg/CFG.h"
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
    Timer timeit(ctx.state.tracer(), "findAllReadsAndWrites");
    CFG::ReadsAndWrites target;
    target.reads.resize(maxBasicBlockId);
    target.writes.resize(maxBasicBlockId);
    target.dead.resize(maxBasicBlockId);
    vector<UnorderedSet<core::LocalVariable>> readsAndWrites(maxBasicBlockId);

    for (unique_ptr<BasicBlock> &bb : this->basicBlocks) {
        auto &blockWrites = target.writes[bb->id];
        auto &blockReads = target.reads[bb->id];
        auto &blockDead = target.dead[bb->id];
        auto &blockReadsAndWrites = readsAndWrites[bb->id];
        for (Binding &bind : bb->exprs) {
            blockWrites.insert(bind.bind.variable);
            blockReadsAndWrites.insert(bind.bind.variable);
            /*
             * When we write to an alias, we rely on the type information being
             * propagated through block arguments from the point of
             * assignment. Treating every write as also reading from the
             * variable serves to represent this.
             */
            if (bind.bind.variable.isAliasForGlobal(ctx) && cast_instruction<Alias>(bind.value.get()) == nullptr) {
                blockReads.insert(bind.bind.variable);
            }

            if (auto *v = cast_instruction<Ident>(bind.value.get())) {
                blockReads.insert(v->what);
                blockReadsAndWrites.insert(v->what);
            } else if (auto *v = cast_instruction<Send>(bind.value.get())) {
                blockReads.insert(v->recv.variable);
                blockReadsAndWrites.insert(v->recv.variable);
                for (auto &arg : v->args) {
                    blockReads.insert(arg.variable);
                    blockReadsAndWrites.insert(arg.variable);
                }
            } else if (auto *v = cast_instruction<Return>(bind.value.get())) {
                blockReads.insert(v->what.variable);
                blockReadsAndWrites.insert(v->what.variable);
            } else if (auto *v = cast_instruction<BlockReturn>(bind.value.get())) {
                blockReads.insert(v->what.variable);
                blockReadsAndWrites.insert(v->what.variable);
            } else if (auto *v = cast_instruction<Cast>(bind.value.get())) {
                blockReads.insert(v->value.variable);
                blockReadsAndWrites.insert(v->value.variable);
            } else if (auto *v = cast_instruction<LoadSelf>(bind.value.get())) {
                blockReads.insert(v->fallback);
                blockReadsAndWrites.insert(v->fallback);
            }

            auto fnd = blockReads.find(bind.bind.variable);
            if (fnd == blockReads.end()) {
                blockDead.insert(bind.bind.variable);
            }
        }
        if (bb->bexit.cond.variable.exists()) {
            blockReads.insert(bb->bexit.cond.variable);
            blockReadsAndWrites.insert(bb->bexit.cond.variable);
        }
    }
    UnorderedMap<core::LocalVariable, pair<int, int>> usageCounts;

    {
        Timer timeit(ctx.state.tracer(), "privates1");

        for (auto blockId = 0; blockId < maxBasicBlockId; blockId++) {
            for (auto &el : readsAndWrites[blockId]) {
                usageCounts.try_emplace(el, make_pair(0, blockId)).first->second.first += 1;
            }
        }
    }
    {
        Timer timeit(ctx.state.tracer(), "privates2");
        for (const auto &[local, usages] : usageCounts) {
            if (usages.first == 1) {
                target.writes[usages.second].erase(local);
            }
        }
    }

    return target;
}

void CFG::sanityCheck(core::Context ctx) {
    if (!debug_mode) {
        return;
    }

    for (auto &bb : this->basicBlocks) {
        ENFORCE(bb->bexit.isCondSet(), "Block exit condition left unset for block {}", bb->toString(ctx));

        if (bb.get() == deadBlock()) {
            continue;
        }

        auto thenCount = absl::c_count(bb->bexit.thenb->backEdges, bb.get());
        auto elseCount = absl::c_count(bb->bexit.elseb->backEdges, bb.get());
        ENFORCE(thenCount == 1, "bb id={}; then has {} back edges", bb->id, thenCount);
        ENFORCE(elseCount == 1, "bb id={}; else has {} back edges", bb->id, elseCount);
        if (bb->bexit.thenb == bb->bexit.elseb) {
            ENFORCE(!bb->bexit.cond.variable.exists());
        } else {
            ENFORCE(bb->bexit.cond.variable.exists());
        }
    }
}

string CFG::toString(core::Context ctx) {
    fmt::memory_buffer buf;
    string symbolName = this->symbol.data(ctx)->showFullName(ctx);
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
    fmt::format_to(
        buf, "block[id={}]({})\n", this->id,
        fmt::map_join(
            this->args.begin(), this->args.end(), ", ", [&](const auto &arg) -> auto { return arg.toString(ctx); }));

    if (this->outerLoops > 0) {
        fmt::format_to(buf, "outerLoops: {}\n", this->outerLoops);
    }
    for (Binding &exp : this->exprs) {
        fmt::format_to(buf, "{} = {}\n", exp.bind.toString(ctx), exp.value->toString(ctx));
    }
    if (this->bexit.cond.variable.exists()) {
        fmt::format_to(buf, "{}", this->bexit.cond.toString(ctx));
    } else {
        fmt::format_to(buf, "<unconditional>");
    }
    return to_string(buf);
}

Binding::Binding(core::LocalVariable bind, core::Loc loc, unique_ptr<Instruction> value)
    : bind(bind), loc(loc), value(std::move(value)) {}

bool CFG::shouldExport(const core::GlobalState &gs) const {
    // Only export CFGs whose owner mixes in T::CFGExport
    return symbol.data(gs)->owner.dataAllowingNone(gs)->derivesFrom(gs, core::Symbols::T_CFGExport());
}

} // namespace sorbet::cfg
