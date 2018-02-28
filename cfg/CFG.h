#ifndef SRUBY_CFG_H
#define SRUBY_CFG_H

#include "core/core.h"
#include <climits>
#include <memory>
#include <unordered_map>
#include <unordered_set>

#include "Instructions.h"

namespace ruby_typer {
namespace cfg {

class BasicBlock;

class BlockExit final {
public:
    core::LocalVariable cond;
    BasicBlock *thenb;
    BasicBlock *elseb;
    core::Loc loc;
    bool isCondSet() {
        return cond._name.id() >= 0;
    }
    BlockExit() : cond(), thenb(nullptr), elseb(nullptr){};
};

class Binding final {
public:
    core::LocalVariable bind;
    core::Loc loc;

    std::unique_ptr<Instruction> value;
    std::shared_ptr<core::Type> tpe;

    Binding(core::LocalVariable bind, core::Loc loc, std::unique_ptr<Instruction> value);
    Binding(Binding &&other) = default;
    Binding() = default;

    Binding &operator=(Binding &&) = default;
};

class BasicBlock final {
public:
    std::vector<core::LocalVariable> args;
    int id = 0;
    int fwdId = -1;
    int bwdId = -1;
    int flags = 0;
    int outerLoops = 0;
    std::vector<Binding> exprs;
    BlockExit bexit;
    std::vector<BasicBlock *> backEdges;
    BasicBlock() {
        counterInc("basicblocks");
    };

    std::string toString(core::Context ctx);
    void recordAnnotations(core::Context ctx);
    core::Loc loc();
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

    std::string toString(core::Context ctx);
    void recordAnnotations(core::Context ctx);

    // Flags
    static constexpr int LOOP_HEADER = 1 << 0;
    static constexpr int WAS_JUMP_DESTINATION = 1 << 1;

    // special minLoops
    static constexpr int MIN_LOOP_FIELD = -1;
    static constexpr int MIN_LOOP_GLOBAL = -2;
    static constexpr int MIN_LOOP_LET = -3;

    std::unordered_map<core::LocalVariable, int> minLoops;
    std::unordered_map<core::LocalVariable, int> maxLoopWrite;

    void sanityCheck(core::Context ctx);

    struct ReadsAndWrites {
        std::unordered_map<core::LocalVariable, std::unordered_set<BasicBlock *>> reads;
        std::unordered_map<core::LocalVariable, std::unordered_set<BasicBlock *>> writes;
        std::unordered_map<core::LocalVariable, std::unordered_set<BasicBlock *>> kills;
    };
    ReadsAndWrites findAllReadsAndWrites(core::Context ctx);

private:
    CFG();
    BasicBlock *freshBlock(int outerLoops);
};

} // namespace cfg
} // namespace ruby_typer

#endif // SRUBY_CFG_H
