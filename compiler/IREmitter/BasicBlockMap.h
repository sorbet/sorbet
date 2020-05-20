#ifndef SORBET_COMPILER_IREMITTER_BASIC_BLOCK_MAP_H
#define SORBET_COMPILER_IREMITTER_BASIC_BLOCK_MAP_H

#include "core/core.h"

namespace sorbet::compiler {

// We probably want to rename this something more like IREmitterContext, because that's what it's grown into being.
//
// Contains a bunch of state that gets populated and accessed while emitting IR for a single Ruby method.
//
// Nearly every vector here behaves as a lookup map keyed on cfg::BasicBlock::rubyBlockId (i.e., an ID
// for each Ruby block like `do ...  end` inside a Ruby method, with 0 meaning "outside a Ruby block").
// It might make sense to newtype this someday.
struct BasicBlockMap {
    core::SymbolRef forMethod;

    // Contains llvm::BasicBlocks (insertion points) to hold code for a Ruby method or block that runs first,
    // before starting to execute the user-written body.
    //
    // idx: cfg::BasicBlock::rubyBlockId
    std::vector<llvm::BasicBlock *> functionInitializersByFunction;

    // The insertion points for code that loads and validates arguments, one for each Ruby method or Ruby block.
    //
    // These insertions points hold that converts from arguments given to us via Ruby's C extension API interface
    // into the LLVM IR that lets our compiled code interact with them.
    //
    // For simplicitly, all our compiled functions advertise themselves as variadic Ruby methods even when
    // the original Ruby method had a fixed arity, so there ends up being a fair deal of argument checking logic.
    // In particular, there are many llvm::BasicBlocks involved in arguments--this map holds just the starting point.
    //
    // idx: cfg::BasicBlock::rubyBlockId
    std::vector<llvm::BasicBlock *> argumentSetupBlocksByFunction;

    // The insertion points for the body code a user wrote inside a Ruby method or Ruby block.
    //
    // idx: cfg::BasicBlock::rubyBlockId
    std::vector<llvm::BasicBlock *> userEntryBlockByFunction;

    // A mapping from cfg::BasicBlock -> llvm::BasicBlock
    //
    // idx: cfg::BasicBlock::id
    std::vector<llvm::BasicBlock *> llvmBlocksBySorbetBlocks;

    // Sorbet sometimes generates extra cfg::BasicBlocks that make typechecking easier (for example, to model
    // the fact that a block might never run, or might run once, or might run many times). Those made up blocks
    // should never be branched to so SorbetLLVM needs to maintain a mapping which blocks to essentially skip over.
    //
    // idx: cfg::BasicBlock::id
    // val: cfg::BasicBlock::id
    std::vector<int> basicBlockJumpOverrides;

    // Sorbet will simplify and compact the basicBlocks vector inside the CFG. As a result, the mapping from basic block
    // id back to basic block is broken. Currently the only thing we need this mapping for is to identify the ruby block
    // that a jump override corresponds to, so this data can just exist along-side the jump overrides.
    //
    // idx: cfg::BasicBlock::id
    // val: cfg::BasicBlock::rubyBlockId
    std::vector<int> basicBlockRubyBlockId;

    // For simplicity, all our compiled functions advertise themselves as variadic Ruby methods.
    // One reason why this is simpler is because we can pre-allocate a stack array big enough to
    // hold the argv of the Ruby method call taking the most arguments within this Ruby method or
    // Ruby block, and reuse that stack space when making each Ruby method call.
    //
    // This is a mapping from each Ruby method or block to that AllocaInst.
    //
    // idx: cfg::BasicBlock::rubyBlockId;
    std::vector<llvm::AllocaInst *> sendArgArrayByBlock;

    // TODO(jez) document escapedClosure
    std::vector<llvm::Value *> escapedClosure;

    // TODO(jez) document escapedVariableIndices
    UnorderedMap<core::LocalVariable, int> escapedVariableIndices;

    // Every local variable (including arguments) shows up as either an index into the closure (escapedVariableIndices)
    // or something that was explicitly stack allocated.
    //
    // This mapping holds the latter: locals that don't come from the closure.
    UnorderedMap<core::LocalVariable, llvm::AllocaInst *> llvmVariables;

    // Verifies arguments against the types dictated by the SymbolRef for this method.
    // Only one llvm::BasicBlock because Ruby blocks don't get their arg types checked--only Ruby methods.
    //
    // Compiled sig verification does not rely on sorbet-runtime at all, and thus can be much faster.
    // In particular, Ruby methods compiled by SorbetLLVM don't appear to have been wrapped and redefined the same way
    // sorbet-runtime methods do.
    llvm::BasicBlock *sigVerificationBlock;

    // Insertion point for code that runs at the end of a Ruby method (i.e., where returns go)
    // This handles return type checking, among other things.
    llvm::BasicBlock *postProcessBlock;

    // idx: cfg::BasicBlock::rubyBlockId
    // val: The SendAndBlockLink for that block (each Ruby block correspondes to one cfg::Send)
    std::vector<std::shared_ptr<core::SendAndBlockLink>> blockLinks;

    // idx: cfg::BasicBlock::rubyBlockId
    // val: The arguments for that Ruby method or block.
    std::vector<std::vector<core::LocalVariable>> rubyBlockArgs;

    // Each Ruby method and Ruby block gets compiled to an llvm::Function so that it can be called
    // directly like any other C function.
    //
    // idx: cfg::BasicBlock::rubyBlockId
    std::vector<llvm::Function *> rubyBlocks2Functions;

    // TODO(jez) document lineNumberPtrsByFunction
    std::vector<llvm::AllocaInst *> lineNumberPtrsByFunction;

    // TODO(jez) document iseqEncodedPtrsByFunction
    std::vector<llvm::AllocaInst *> iseqEncodedPtrsByFunction;

    // TODO(jez) usesBlockArgs
    bool usesBlockArgs;

    // Non-zero for basic-blocks that originally jumped to exception-handling code.
    // idx: block id
    // val: the rubyBlockId for the body of the exception block
    std::vector<int> exceptionBlockHeader;

    // Mapping from ruby block id to llvm dead block.
    std::vector<llvm::BasicBlock *> deadBlockMapping;

    // Mapping from ruby block id to llvm block exit blocks. These blocks are used for the case where a transition
    // between a basic block in a ruby block exists, and transitions to a node in a different ruby block (that isn't the
    // dead block).
    std::vector<llvm::BasicBlock *> blockExitMapping;
};

} // namespace sorbet::compiler

#endif
