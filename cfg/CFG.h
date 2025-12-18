#ifndef SORBET_CFG_H
#define SORBET_CFG_H

#include "common/UIntSet.h"
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
    // Returns true when this exit condition represents an unconditional branch.
    bool isUnconditional() const {
        return thenb == elseb;
    }
    BlockExit() : cond(), thenb(nullptr), elseb(nullptr){};
};

class Binding final {
public:
    VariableUseSite bind;
    core::LocOffsets loc;

    InstructionPtr value;

    Binding(LocalRef bind, core::LocOffsets loc, InstructionPtr value);
    Binding(Binding &&other) = default;
    Binding() = default;

    Binding &operator=(Binding &&) = default;
};

class BasicBlock final {
public:
    std::vector<VariableUseSite> args;
    int id = 0;
    struct Flags {
        bool isLoopHeader : 1;
        bool wasJumpDestination : 1;
        // In C++20 we can replace this with bit field initializers
        Flags() : isLoopHeader(false), wasJumpDestination(false) {}
    };
    Flags flags;
    int outerLoops = 0;
    int firstDeadInstructionIdx = -1;
    std::vector<Binding> exprs;
    BlockExit bexit;
    std::vector<BasicBlock *> backEdges;
    BasicBlock() {
        counterInc("basicblocks");
    };

    struct BlockExitCondInfo {
        VariableUseSite recv;
        core::LocOffsets loc;
        core::NameRef fun;
        BlockExitCondInfo(VariableUseSite &&recv, core::LocOffsets loc, core::NameRef fun)
            : recv(std::move(recv)), loc(loc), fun(fun) {}
    };
    // If the `BlockExit::cond` was initialized by a `Send` to a method with a name that
    // updateKnowledge is aware of, return information about the receiver of that `Send`.
    //
    // Should only be used to *improve* existing error messages, not as a reliable source of truth,
    // because the implementation is only a heuristic, and may fail to find a receiver even when it
    // might be otherwise expected to.
    //
    // More specifically, Sorbet's CFG is not SSA, so finding the `Send` that computes the variable
    // used in `BasicBlock::cond` 100% of the time is not easy (maybe not even possible).
    std::optional<BlockExitCondInfo> maybeGetUpdateKnowledgeReceiver(const cfg::CFG &inWhat) const;

    std::string toString(const core::GlobalState &gs, const CFG &cfg) const;
    std::string toTextualString(const core::GlobalState &gs, const CFG &cfg) const;
    std::string showRaw(const core::GlobalState &gs, const CFG &cfg) const;
};

class CFGContext;

class CFG final {
public:
    class UnfreezeCFGLocalVariables final {
        CFG &cfg;

    public:
        UnfreezeCFGLocalVariables(CFG &cfg);

        ~UnfreezeCFGLocalVariables();
    };

    friend class CFGBuilder;
    friend class LocalRef;
    friend class UnfreezeCFGLocalVariables;
    /**
     * CFG owns all the BasicBlocks, and then they have raw unmanaged pointers to and between each other,
     * because they all have lifetime identical with each other and the CFG.
     */
    core::SymbolRef symbol;

    core::NameRef methodName(core::Context ctx) const;

    int maxBasicBlockId = 0;

    /**
     * Get the number of unique local variables in the CFG. Used to size vectors that contain an entry per LocalRef.
     */
    int numLocalVariables() const;

    // Stores MethodDef::loc (not the MethodDef::declLoc).
    //
    // Thus, this usually spans the full body of the method def (recall that sometime methods are
    // created synthetically, not from method defs written in the source).
    core::LocOffsets loc;

    // Stores MethodDef::declLoc
    core::LocOffsets declLoc;

    std::vector<std::unique_ptr<BasicBlock>> basicBlocks;

    // Special loc that corresponds to implicit method return.
    core::LocOffsets implicitReturnLoc;

    /** Blocks in topoligical sort. All parent blocks are earlier than child blocks
     *
     * The name here goes from using forwards or backwards edges as dependencies in topological sort.
     * This in indeed kind-a reverse to what order would node be in: for topological sort with forward edges
     * the entry point is going to be the last node in sorted array.
     *
     * This is a cached post-order traversal of the CFG from the entry node, which is why we traverse it in reverse when
     * typechecking: the reversed post-order traversal ensures that we visit all nodes in data flow order.
     */
    std::vector<BasicBlock *> forwardsTopoSort;
    inline BasicBlock *entry() {
        return basicBlocks[0].get();
    }

    BasicBlock *deadBlock() const {
        return basicBlocks[1].get();
    };

    // Abbreviated debug output in dot format, useful if you already know what you're looking at
    std::string toString(const core::GlobalState &gs) const;
    // As above, but without dot annotations
    std::string toTextualString(const core::GlobalState &gs) const;
    // Verbose debug output
    std::string showRaw(core::Context ctx) const;

    // special minLoops
    static constexpr int MIN_LOOP_FIELD = -1;
    // static constexpr int MIN_LOOP_GLOBAL = -2;
    static constexpr int MIN_LOOP_LET = -3;

    // special ruby region id offsets for exception handling
    static constexpr int HANDLERS_REGION_OFFSET = 1;
    static constexpr int ENSURE_REGION_OFFSET = 2;
    static constexpr int ELSE_REGION_OFFSET = 3;

    void sanityCheck(core::Context ctx);

    class ReadsAndWrites {
    public:
        ReadsAndWrites(uint32_t maxBasicBlockId, uint32_t numLocalVariables);
        std::vector<UIntSet> reads;
        std::vector<UIntSet> writes;

        // The "dead" set reports, for each block, variables that are *only*
        // read in that block after being written; they are thus dead on entry,
        // which we take advantage of when building dataflow information for
        // inference.
        std::vector<UIntSet> dead;
    };
    ReadsAndWrites findAllReadsAndWrites(core::Context ctx);
    LocalRef enterLocal(core::LocalVariable variable);

private:
    CFG();
    BasicBlock *freshBlock(int outerLoops);
    void enterLocalInternal(core::LocalVariable variable, LocalRef &ref);
    std::vector<int> minLoops;
    std::vector<int> maxLoopWrite;
    bool localVariablesFrozen = true;
    /**
     * Maps from LocalRef ID -> LocalVariable. Lets us compactly construct maps involving only the variables included
     * in the CFG.
     */
    std::vector<core::LocalVariable> localVariables;
    /**
     * Map from LocalVariable -> LocalRef. Used to de-dupe variables in localVariables.
     */
    UnorderedMap<core::LocalVariable, LocalRef> localVariableToLocalRef;
};

} // namespace sorbet::cfg

#endif // SORBET_CFG_H
