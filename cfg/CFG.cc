#include "cfg/CFG.h"
#include "absl/strings/escaping.h"
#include "absl/strings/str_split.h"
#include "common/UIntSetForEach.h"
#include "common/sort/sort.h"
#include "common/strings/formatting.h"
#include "common/timers/Timer.h"

// helps debugging
template class std::unique_ptr<sorbet::cfg::CFG>;
template class std::unique_ptr<sorbet::cfg::BasicBlock>;
template class std::vector<sorbet::cfg::BasicBlock *>;

using namespace std;

namespace sorbet::cfg {

CFG::ReadsAndWrites::ReadsAndWrites(uint32_t maxBasicBlockId, uint32_t numLocalVariables)
    : reads(maxBasicBlockId, UIntSet(numLocalVariables)), writes(maxBasicBlockId, UIntSet(numLocalVariables)),
      dead(maxBasicBlockId, UIntSet(numLocalVariables)) {}

CFG::UnfreezeCFGLocalVariables::UnfreezeCFGLocalVariables(CFG &cfg) : cfg(cfg) {
    this->cfg.localVariablesFrozen = false;
}

CFG::UnfreezeCFGLocalVariables::~UnfreezeCFGLocalVariables() {
    this->cfg.localVariablesFrozen = true;
}

int CFG::numLocalVariables() const {
    return this->localVariables.size();
}

BasicBlock *CFG::freshBlock(int outerLoops) {
    int id = this->maxBasicBlockId++;
    auto &r = this->basicBlocks.emplace_back(make_unique<BasicBlock>());
    r->id = id;
    r->outerLoops = outerLoops;
    return r.get();
}

BasicBlock *CFG::freshBlockWithRegion(int outerLoops, int rubyRegionId) {
    int id = this->maxBasicBlockId++;
    auto &r = this->basicBlocks.emplace_back(make_unique<BasicBlock>());
    r->id = id;
    r->outerLoops = outerLoops;
    r->rubyRegionId = rubyRegionId;
    return r.get();
}

void CFG::enterLocalInternal(core::LocalVariable variable, LocalRef &ref) {
    ENFORCE_NO_TIMER(!this->localVariablesFrozen);
    int id = this->localVariables.size();
    this->localVariables.emplace_back(variable);

    // Default values
    this->minLoops.emplace_back(INT_MAX);
    this->maxLoopWrite.emplace_back(0);

    ENFORCE(this->localVariables.size() == this->minLoops.size());
    ENFORCE(this->localVariables.size() == this->maxLoopWrite.size());
    ref = LocalRef(id);
}

LocalRef CFG::enterLocal(core::LocalVariable variable) {
    auto &ref = localVariableToLocalRef[variable];
    if (!ref.exists() && variable.exists()) {
        // ref is an out parameter.
        enterLocalInternal(variable, ref);
    }
    return ref;
}

CFG::CFG() {
    freshBlock(0); // entry;
    freshBlock(0); // dead code;
    deadBlock()->bexit.elseb = deadBlock();
    deadBlock()->bexit.thenb = deadBlock();
    deadBlock()->bexit.cond.variable = LocalRef::unconditional();

    UnfreezeCFGLocalVariables unfreezeVars(*this);
    // Enter a few fixed local variables
    // noVariable is special because it doesn't 'exist'.
    this->enterLocalInternal(core::LocalVariable::noVariable(),
                             localVariableToLocalRef[core::LocalVariable::noVariable()]);

    LocalRef blockCall = this->enterLocal(core::LocalVariable::blockCall());
    ENFORCE(blockCall == LocalRef::blockCall());

    LocalRef selfVariable = this->enterLocal(core::LocalVariable::selfVariable());
    ENFORCE(selfVariable == LocalRef::selfVariable());

    LocalRef unconditional = this->enterLocal(core::LocalVariable::unconditional());
    ENFORCE(unconditional == LocalRef::unconditional());

    LocalRef finalReturn = this->enterLocal(core::LocalVariable(core::Names::finalReturn(), 0));
    ENFORCE(finalReturn == LocalRef::finalReturn());
}

CFG::ReadsAndWrites CFG::findAllReadsAndWrites(core::Context ctx) {
    Timer timeit(ctx.state.tracer(), "findAllReadsAndWrites");
    CFG::ReadsAndWrites target(maxBasicBlockId, numLocalVariables());

    for (unique_ptr<BasicBlock> &bb : this->basicBlocks) {
        auto &blockWrites = target.writes[bb->id];
        auto &blockReads = target.reads[bb->id];
        auto &blockDead = target.dead[bb->id];
        for (Binding &bind : bb->exprs) {
            blockWrites.add(bind.bind.variable.id());
            /*
             * When we write to an alias, we rely on the type information being
             * propagated through block arguments from the point of
             * assignment. Treating every write as also reading from the
             * variable serves to represent this.
             */
            if (bind.bind.variable.isAliasForGlobal(ctx, *this) && cast_instruction<Alias>(bind.value) == nullptr) {
                blockReads.add(bind.bind.variable.id());
            }

            if (auto v = cast_instruction<Ident>(bind.value)) {
                blockReads.add(v->what.id());
            } else if (auto v = cast_instruction<Send>(bind.value)) {
                blockReads.add(v->recv.variable.id());
                for (auto &arg : v->args) {
                    blockReads.add(arg.variable.id());
                }
            } else if (auto v = cast_instruction<TAbsurd>(bind.value)) {
                blockReads.add(v->what.variable.id());
            } else if (auto v = cast_instruction<Return>(bind.value)) {
                blockReads.add(v->what.variable.id());
            } else if (auto v = cast_instruction<BlockReturn>(bind.value)) {
                blockReads.add(v->what.variable.id());
            } else if (auto v = cast_instruction<Cast>(bind.value)) {
                blockReads.add(v->value.variable.id());
            } else if (auto v = cast_instruction<LoadSelf>(bind.value)) {
                blockReads.add(v->fallback.id());
            } else if (auto v = cast_instruction<SolveConstraint>(bind.value)) {
                blockReads.add(v->send.id());
            } else if (auto v = cast_instruction<YieldLoadArg>(bind.value)) {
                blockReads.add(v->yieldParam.variable.id());
            } else if (auto v = cast_instruction<KeepAlive>(bind.value)) {
                blockReads.add(v->what.id());
            }

            if (!blockReads.contains(bind.bind.variable.id())) {
                blockDead.add(bind.bind.variable.id());
            }
        }
        ENFORCE(bb->bexit.cond.variable.exists());
        if (bb->bexit.cond.variable != LocalRef::unconditional()) {
            blockReads.add(bb->bexit.cond.variable.id());
        }
    }

    vector<pair<int, int>> usageCounts(this->numLocalVariables());

    {
        Timer timeit(ctx.state.tracer(), "privates1");

        UIntSet blockReadsAndWrites(this->numLocalVariables());
        for (auto blockId = 0; blockId < maxBasicBlockId; blockId++) {
            blockReadsAndWrites.overwriteWithUnion(target.reads[blockId], target.writes[blockId]);
            blockReadsAndWrites.forEach([&usageCounts, blockId](uint32_t local) -> void {
                if (usageCounts[local].first == 0) {
                    usageCounts[local].second = blockId;
                }
                usageCounts[local].first += 1;
            });
        }
    }
    {
        Timer timeit(ctx.state.tracer(), "privates2");
        auto local = 0;
        vector<UIntSet> writesToRemove(maxBasicBlockId, UIntSet(numLocalVariables()));
        for (const auto &usages : usageCounts) {
            if (usages.first == 1) {
                writesToRemove[usages.second].add(local);
            }
            local++;
        }
        auto blockId = 0;
        for (const auto &blockWritesToRemove : writesToRemove) {
            target.writes[blockId].remove(blockWritesToRemove);
            blockId++;
        }
    }

    return target;
}

void CFG::sanityCheck(core::Context ctx) {
    if constexpr (!debug_mode) {
        return;
    }

    for (auto &bb : this->basicBlocks) {
        ENFORCE(bb->bexit.isCondSet(), "Block exit condition left unset for block {}", bb->toString(ctx, *this));

        if (bb.get() == deadBlock()) {
            continue;
        }

        auto thenCount = absl::c_count(bb->bexit.thenb->backEdges, bb.get());
        auto elseCount = absl::c_count(bb->bexit.elseb->backEdges, bb.get());
        ENFORCE(thenCount == 1, "bb id={}; then has {} back edges", bb->id, thenCount);
        ENFORCE(elseCount == 1, "bb id={}; else has {} back edges", bb->id, elseCount);
        if (bb->bexit.thenb == bb->bexit.elseb) {
            ENFORCE(bb->bexit.cond.variable == LocalRef::unconditional());
        } else {
            ENFORCE(bb->bexit.cond.variable.exists());
            ENFORCE(bb->bexit.cond.variable != LocalRef::unconditional());
        }
    }
}

string CFG::toString(const core::GlobalState &gs) const {
    fmt::memory_buffer buf;
    string symbolName = this->symbol.showFullName(gs);
    fmt::format_to(std::back_inserter(buf),
                   "subgraph \"cluster_{}\" {{\n"
                   "    label = \"{}\";\n"
                   "    color = blue;\n\n",
                   symbolName, symbolName, symbolName, symbolName);
    for (auto &basicBlock : this->basicBlocks) {
        auto text = basicBlock->toString(gs, *this);
        auto lines = absl::StrSplit(text, "\n");

        auto shape = basicBlock->id == 0 ? "cds" : basicBlock->id == 1 ? "parallelogram" : "rectangle";
        // whole block red if whole block is dead
        auto color = basicBlock->firstDeadInstructionIdx == 0 ? "red" : "black";
        fmt::format_to(std::back_inserter(buf),
                       "    \"bb{}_{}\" [\n"
                       "        shape = {};\n"
                       "        color = {};\n"
                       "        label = \"{}\\l\"\n"
                       "    ];\n\n"
                       "    \"bb{}_{}\" -> \"bb{}_{}\" [style=\"bold\"];\n",
                       symbolName, basicBlock->id, shape, color,
                       fmt::map_join(lines, "\\l", [](auto line) -> string { return absl::CEscape(line); }), symbolName,
                       basicBlock->id, symbolName, basicBlock->bexit.thenb->id);

        if (basicBlock->bexit.thenb != basicBlock->bexit.elseb) {
            fmt::format_to(std::back_inserter(buf), "    \"bb{}_{}\" -> \"bb{}_{}\" [style=\"tapered\"];\n\n",
                           symbolName, basicBlock->id, symbolName, basicBlock->bexit.elseb->id);
        }
    }
    fmt::format_to(std::back_inserter(buf), "}}");
    return to_string(buf);
}

string CFG::toTextualString(const core::GlobalState &gs) const {
    fmt::memory_buffer buf;
    string symbolName = this->symbol.showFullName(gs);
    fmt::format_to(std::back_inserter(buf), "method {} {{\n\n", symbolName);
    for (auto &basicBlock : this->basicBlocks) {
        if (!basicBlock->backEdges.empty()) {
            fmt::format_to(std::back_inserter(buf), "# backedges\n");
            for (auto *backEdge : basicBlock->backEdges) {
                fmt::format_to(std::back_inserter(buf), "# - bb{}\n", backEdge->id);
            }
        }

        fmt::format_to(std::back_inserter(buf), "{}\n", basicBlock->toTextualString(gs, *this));
    }
    fmt::format_to(std::back_inserter(buf), "}}");
    return to_string(buf);
}

string CFG::showRaw(core::Context ctx) const {
    fmt::memory_buffer buf;
    string symbolName = this->symbol.showFullName(ctx);
    fmt::format_to(std::back_inserter(buf),
                   "subgraph \"cluster_{}\" {{\n"
                   "    label = \"{}\";\n"
                   "    color = blue;\n\n",
                   symbolName, symbolName);
    for (auto &basicBlock : this->basicBlocks) {
        auto text = basicBlock->showRaw(ctx, *this);
        auto lines = absl::StrSplit(text, "\n");

        auto shape = basicBlock->id == 0 ? "cds" : basicBlock->id == 1 ? "parallelogram" : "rectangle";
        // whole block red if whole block is dead
        auto color = basicBlock->firstDeadInstructionIdx == 0 ? "red" : "black";
        fmt::format_to(std::back_inserter(buf),
                       "    \"bb{}_{}\" [\n"
                       "        shape = {};\n"
                       "        color = {};\n"
                       "        label = \"{}\\l\"\n"
                       "    ];\n\n"
                       "    \"bb{}_{}\" -> \"bb{}_{}\" [style=\"bold\"];\n",
                       symbolName, basicBlock->id, shape, color,
                       fmt::map_join(lines, "\\l", [](auto line) -> string { return absl::CEscape(line); }), symbolName,
                       basicBlock->id, symbolName, basicBlock->bexit.thenb->id);

        if (basicBlock->bexit.thenb != basicBlock->bexit.elseb) {
            fmt::format_to(std::back_inserter(buf), "    \"bb{}_{}\" -> \"bb{}_{}\" [style=\"tapered\"];\n\n",
                           symbolName, basicBlock->id, symbolName, basicBlock->bexit.elseb->id);
        }
    }
    fmt::format_to(std::back_inserter(buf), "}}");
    return to_string(buf);
}

namespace {
const cfg::Send *looksLikeUpdateKnowledgeSend(const cfg::CFG &inWhat, const cfg::Binding &bind, LocalRef expectedRecv) {
    if (bind.bind.variable != expectedRecv) {
        return nullptr;
    }

    auto send = cfg::cast_instruction<cfg::Send>(bind.value);
    if (send == nullptr) {
        return nullptr;
    }

    if (!send->recv.variable.isSyntheticTemporary(inWhat)) {
        // Conservative heuristic. If the receiver is not synthetic, then the update knowledge
        // method was not computed by some sub expression but instead called directly on a
        // Ruby-level variable. Therefore it wouldn't be useful to report a "maybe factor this to a
        // variable" error for this call site.
        return nullptr;
    }

    if (!send->fun.isUpdateKnowledgeName()) {
        return nullptr;
    }

    return send;
}

} // namespace

optional<BasicBlock::BlockExitCondInfo> BasicBlock::maybeGetUpdateKnowledgeReceiver(const cfg::CFG &inWhat) const {
    if (this->exprs.empty()) {
        return nullopt;
    }

    // Conservative heuristic, maybe we can make this smarter
    // (Currently, only detect cases where the branch condition was the last thing computed)
    auto &lastBinding = this->exprs.back();
    auto send = looksLikeUpdateKnowledgeSend(inWhat, lastBinding, this->bexit.cond.variable);
    if (send == nullptr) {
        return nullopt;
    }

    if (send->fun == core::Names::bang() && this->exprs.size() >= 2) {
        // Heuristic, because it's overwhelmingly common to see `!x.foo.nil?`, in which case the
        // relevant receiver is the second-last, not the last, send binding.
        auto bangRecv = send->recv.variable;
        auto &secondLastBinding = this->exprs[this->exprs.size() - 2];
        auto secondLastSend = looksLikeUpdateKnowledgeSend(inWhat, secondLastBinding, bangRecv);

        if (secondLastSend != nullptr) {
            send = secondLastSend;
        }
    }

    // VariableUseSite copy constructor is deleted, have to manually make a shallow copy
    auto recv = VariableUseSite{send->recv.variable, send->recv.type};
    return make_optional<BlockExitCondInfo>(move(recv), send->receiverLoc, send->fun);
}

string BasicBlock::toString(const core::GlobalState &gs, const CFG &cfg) const {
    fmt::memory_buffer buf;
    fmt::format_to(std::back_inserter(buf), "block[id={}]({})\n", this->id,
                   fmt::map_join(
                       this->args, ", ", [&](const auto &arg) -> auto{ return arg.toString(gs, cfg); }));

    if (this->outerLoops > 0) {
        fmt::format_to(std::back_inserter(buf), "outerLoops: {}\n", this->outerLoops);
    }
    for (const Binding &exp : this->exprs) {
        fmt::format_to(std::back_inserter(buf), "{} = {}\n", exp.bind.toString(gs, cfg), exp.value.toString(gs, cfg));
    }
    fmt::format_to(std::back_inserter(buf), "{}", this->bexit.cond.toString(gs, cfg));
    return to_string(buf);
}

string BasicBlock::toTextualString(const core::GlobalState &gs, const CFG &cfg) const {
    fmt::memory_buffer buf;
    fmt::format_to(std::back_inserter(buf), "bb{}[firstDead={}]({}):\n", this->id, this->firstDeadInstructionIdx,
                   fmt::map_join(
                       this->args, ", ", [&](const auto &arg) -> auto{ return arg.toString(gs, cfg); }));

    if (this->outerLoops > 0) {
        fmt::format_to(std::back_inserter(buf), "    # outerLoops: {}\n", this->outerLoops);
    }
    if (this->rubyRegionId > 0) {
        fmt::format_to(std::back_inserter(buf), "    # rubyRegionId: {}\n", this->rubyRegionId);
    }
    for (const Binding &exp : this->exprs) {
        fmt::format_to(std::back_inserter(buf), "    {} = {}\n", exp.bind.toString(gs, cfg),
                       exp.value.toString(gs, cfg));
    }

    if (this->bexit.thenb == this->bexit.elseb) {
        fmt::format_to(std::back_inserter(buf), "    {} -> bb{}\n", this->bexit.cond.variable.toString(gs, cfg),
                       this->bexit.thenb->id);
    } else {
        // nullptr can sometimes happen in valid use cases. One concrete case: `<get-current-exception>` is an
        // instruction designed to support exception handling in the Sorbet Compiler. It always comes in a
        // `rescue` block, but the `rescue` block might unconditionally raise before the inferencer assigns a type
        // to the binding containing the `<get-current-exception>` instruction. The compiler doesn't
        // actually care about the type of the bexit.cond, so this is acceptable. It does mean that
        // we have to check whether the type is set here before attempting to print something.
        auto condType = this->bexit.cond.type == nullptr ? "<nullptr>" : this->bexit.cond.type.show(gs);
        fmt::format_to(std::back_inserter(buf), "    {} -> ({} ? bb{} : bb{})\n",
                       this->bexit.cond.variable.toString(gs, cfg), condType, this->bexit.thenb->id,
                       this->bexit.elseb->id);
    }
    return to_string(buf);
}

string BasicBlock::showRaw(const core::GlobalState &gs, const CFG &cfg) const {
    fmt::memory_buffer buf;
    fmt::format_to(std::back_inserter(buf), "block[id={}]({})\n", this->id,
                   fmt::map_join(
                       this->args, ", ", [&](const auto &arg) -> auto{ return arg.showRaw(gs, cfg); }));

    if (this->outerLoops > 0) {
        fmt::format_to(std::back_inserter(buf), "outerLoops: {}\n", this->outerLoops);
    }
    for (const Binding &exp : this->exprs) {
        fmt::format_to(std::back_inserter(buf), "Binding {{\n&nbsp;bind = {},\n&nbsp;value = {},\n}}\n",
                       exp.bind.showRaw(gs, cfg, 1), exp.value.showRaw(gs, cfg, 1));
    }
    fmt::format_to(std::back_inserter(buf), "{}", this->bexit.cond.showRaw(gs, cfg));
    return to_string(buf);
}

Binding::Binding(LocalRef bind, core::LocOffsets loc, InstructionPtr value)
    : bind(bind), loc(loc), value(std::move(value)) {}

} // namespace sorbet::cfg
