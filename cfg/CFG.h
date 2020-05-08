#ifndef SORBET_CFG_H
#define SORBET_CFG_H

#include "core/Context.h"
#include "core/LocalVariable.h"
#include "core/Types.h"
#include <climits>
#include <memory>

#include "cfg/Instructions.h"

//
// This file defines the IR that the inference algorithm operates on.
// A CFG (control flow graph) is a directed graph of "basic blocks" which are
// sequences of instructions which cannot conditionally branch.
//
// The list of valid instructions in a binding of a basic block are defined
// elsewhere.
//

namespace sorbet::cfg {

class BasicBlock;

class BlockExit final {
public:
    VariableUseSite cond;
    BasicBlock *thenb;
    BasicBlock *elseb;
    core::LocOffsets loc;
    bool isCondSet() {
        return cond.variable.exists();
    }
    BlockExit() : cond(), thenb(nullptr), elseb(nullptr){};
};

class Binding final {
public:
    VariableUseSite bind;
    core::LocOffsets loc;

    std::unique_ptr<Instruction> value;

    Binding(core::LocalVariable bind, core::LocOffsets loc, std::unique_ptr<Instruction> value);
    Binding(Binding &&other) = default;
    Binding() = default;

    Binding &operator=(Binding &&) = default;
};

class BasicBlock final {
public:
    std::vector<VariableUseSite> args;
    int id = 0;
    int fwdId = -1;
    int bwdId = -1;
    int flags = 0;
    int outerLoops = 0;
    // Tracks which Ruby block (do ... end) this BasicBlock was generated from.
    // Incremented every time builder_walk sees a new Ruby block while traversing a Ruby method.
    // rubyBlockId == 0 means code at the top-level of this method (outside any Ruby block).
    int rubyBlockId = 0;
    int firstDeadInstructionIdx = -1;
    std::vector<Binding> exprs;
    BlockExit bexit;
    std::vector<BasicBlock *> backEdges;
    BasicBlock() {
        counterInc("basicblocks");
    };

    std::string toString(const core::GlobalState &gs) const;
    std::string showRaw(core::Context ctx) const;
};

class CFGContext;

class CFG final {
    friend class CFGBuilder;
    /**
     * CFG owns all the BasicBlocks, and then they have raw unmanaged pointers to and between each other,
     * because they all have lifetime identical with each other and the CFG.
     */
public:
    core::SymbolRef symbol;
    int maxBasicBlockId = 0;
    int maxRubyBlockId = 0;
    core::FileRef file;
    std::vector<std::unique_ptr<BasicBlock>> basicBlocks;
    /** Blocks in topoligical sort. All parent blocks are earlier than child blocks
     *
     * The name here goes from using forwards or backwards edges as dependencies in topological sort.
     * This in indeed kind-a reverse to what order would node be in: for topological sort with forward edges
     * the entry point is going to be the last node in sorted array.
     */
    std::vector<BasicBlock *> forwardsTopoSort;
    inline BasicBlock *entry() {
        return basicBlocks[0].get();
    }

    BasicBlock *deadBlock() {
        return basicBlocks[1].get();
    };

    // Abbreviated debug output, useful if you already know what you're looking at
    std::string toString(const core::GlobalState &gs) const;
    // Verbose debug output
    std::string showRaw(core::Context ctx) const;

    // Flags
    static constexpr int LOOP_HEADER = 1 << 0;
    static constexpr int WAS_JUMP_DESTINATION = 1 << 1;

    // special minLoops
    static constexpr int MIN_LOOP_FIELD = -1;
    static constexpr int MIN_LOOP_GLOBAL = -2;
    static constexpr int MIN_LOOP_LET = -3;

    UnorderedMap<core::LocalVariable, int> minLoops;
    UnorderedMap<core::LocalVariable, int> maxLoopWrite;

    void sanityCheck(core::Context ctx);

    struct ReadsAndWrites {
        std::vector<UnorderedSet<core::LocalVariable>> reads;
        std::vector<UnorderedSet<core::LocalVariable>> writes;

        // The "dead" set reports, for each block, variables that are *only*
        // read in that block after being written; they are thus dead on entry,
        // which we take advantage of when building dataflow information for
        // inference.
        std::vector<UnorderedSet<core::LocalVariable>> dead;
    };
    ReadsAndWrites findAllReadsAndWrites(core::Context ctx);

private:
    CFG();
    BasicBlock *freshBlock(int outerLoops, int rubyBlockid);
};

} // namespace sorbet::cfg

#endif // SORBET_CFG_H
