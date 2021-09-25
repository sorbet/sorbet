#ifndef SORBET_COMPILER_IREMITTER_IREMITTERCONTEXT_H
#define SORBET_COMPILER_IREMITTER_IREMITTERCONTEXT_H

#include "compiler/Core/ForwardDeclarations.h"
#include "core/core.h"

namespace llvm {
class IRBuilderBase;
} // namespace llvm

namespace sorbet::compiler {

class CompilerState;

enum class FunctionType {
    // A normal method
    Method,

    // A file-scope static-init method
    StaticInitFile,

    // A module/class-scope static-init method
    StaticInitModule,

    // Blocks used within a method
    Block,

    // Functions extracted for rescue handlers
    Rescue,

    // Functions extracted for ensure handlers
    Ensure,

    // Functions extracted for the body of an exception
    ExceptionBegin,

    // The type of blocks that aren't reference, and should be skipped during code generation
    Unused,
};

constexpr bool functionTypePushesFrame(FunctionType ty) {
    switch (ty) {
        case FunctionType::Method:
            return false;
        case FunctionType::StaticInitModule:
            return false;
        case FunctionType::StaticInitFile:
            return false;
        case FunctionType::Block:
            return false;
        case FunctionType::Rescue:
            return true;
        case FunctionType::Ensure:
            return true;
        case FunctionType::ExceptionBegin:
            return false;
        case FunctionType::Unused:
            return false;
    }
}

constexpr bool functionTypeNeedsPostprocessing(FunctionType ty) {
    switch (ty) {
        case FunctionType::Method:
            return true;
        case FunctionType::StaticInitModule:
            return true;
        case FunctionType::StaticInitFile:
            return true;
        case FunctionType::Block:
            return false;
        case FunctionType::Rescue:
            return false;
        case FunctionType::Ensure:
            return false;
        case FunctionType::ExceptionBegin:
            return false;
        case FunctionType::Unused:
            return false;
    }
}

struct Alias;

struct BlockArity {
    int min = 0;
    int max = 0;
};

// Contains a bunch of state that gets populated and accessed while emitting IR for a single Ruby method.
//
// Nearly every vector here behaves as a lookup map keyed on cfg::BasicBlock::rubyBlockId (i.e., an ID
// for each Ruby block like `do ...  end` inside a Ruby method, with 0 meaning "outside a Ruby block").
// It might make sense to newtype this someday.
struct IREmitterContext {
    cfg::CFG &cfg;

    // Sorbet uses cfg::Alias to link a local variable to a global construct, like an instance variable or a constant.
    //
    // This mapping is essentially, "if you were about to access a local variable corresponding to an alias,
    // this is the thing you should access instead"
    UnorderedMap<cfg::LocalRef, Alias> aliases;

    // This is a mapping from local ref to the string that represents its symbol value. The mapping is used when
    // building up the static data necessary for keyword arg sends that use the ruby value stack.
    //
    // key: local ref for the symbol keyword arg
    // value: string_view of that keyword arg's contents
    UnorderedMap<cfg::LocalRef, std::string_view> symbols;

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

    // This is the maximum number of argument seen in a given ruby method.
    // TODO(trevor) this could be a vector, allowing for more specific information to be present when checking for stack
    // overflow.
    int maxSendArgCount;

    // For simplicity, all our compiled functions advertise themselves as variadic Ruby methods.
    // One reason why this is simpler is because we can pre-allocate a stack array big enough to
    // hold the argv of the Ruby method call taking the most arguments within this Ruby method or
    // Ruby block, and reuse that stack space when making each Ruby method call.
    //
    // This is a mapping from each Ruby method or block to that AllocaInst.
    //
    // idx: cfg::BasicBlock::rubyBlockId;
    std::vector<llvm::AllocaInst *> sendArgArrayByBlock;

    // TODO(jez) document escapedVariableIndices
    UnorderedMap<cfg::LocalRef, int> escapedVariableIndices;

    // When arguments have defaults, the use of the default is guarded by a call to ArgPresent. The ArgPresent variables
    // are initialized during setupArguments, but need to be available by argument index.
    //
    // outer idx: the ruby block id that defines these arguments
    // idx: Argument index into the method arguments
    // val: The local ref that holds the result of the ArgPresent instruction, or cfg::LocalRef::noVariable if the
    //      argument does not have a default value.
    std::vector<std::vector<cfg::LocalRef>> argPresentVariables;

    // Every local variable (including arguments) shows up as either an index into the closure (escapedVariableIndices)
    // or something that was explicitly stack allocated.
    //
    // This mapping holds the latter: locals that don't come from the closure.
    UnorderedMap<cfg::LocalRef, llvm::AllocaInst *> llvmVariables;

    // `self` is retrieved at the entry point of each Ruby block and stashed here.
    //
    // idx: cfg::BasicBlock::rubyBlockId
    // val: alloca for `self`
    UnorderedMap<int, llvm::AllocaInst *> selfVariables;

    // Insertion point for code that runs at the end of a Ruby method (i.e., where returns go)
    // This handles return type checking, among other things.
    llvm::BasicBlock *postProcessBlock;

    // idx: cfg::BasicBlock::rubyBlockId
    // val: The SendAndBlockLink for that block (each Ruby block correspondes to one cfg::Send)
    std::vector<std::shared_ptr<core::SendAndBlockLink>> blockLinks;

    // idx: cfg::BasicBlock::rubyBlockId
    // val: The arguments for that Ruby method or block.
    std::vector<std::vector<cfg::LocalRef>> rubyBlockArgs;

    // Each Ruby method and Ruby block gets compiled to an llvm::Function so that it can be called
    // directly like any other C function.
    //
    // idx: cfg::BasicBlock::rubyBlockId
    std::vector<llvm::Function *> rubyBlocks2Functions;

    // The type of each function that we're going to generate.
    //
    // idx: cfg::BasicBlock::rubyBlockId
    std::vector<FunctionType> rubyBlockType;

    // The parent block id of each function being generated. Used for constructing the parent relationship when
    // allocating iseq values for block/exception functions.
    //
    // idx: cfg::BasicBlock::rubyBlockId
    // val: cfg::BasicBlock::rubyBlockId
    std::vector<int> rubyBlockParent;

    // The level of a ruby block corresponds to the nesting depth of non-method stack frames when it's called. So for a
    // block at the top-level of the method, the value would be 1.
    //
    // idx: cfg::BasicBlock::rubyBlockId
    // val: the number of scopes traversed to get back to the top-level method frame at runtime
    std::vector<int> rubyBlockLevel;

    // TODO(jez) document lineNumberPtrsByFunction
    std::vector<llvm::AllocaInst *> lineNumberPtrsByFunction;

    // Stores the control frame (GET_EC()->cfp) for ordinary ruby blocks.
    //
    // idx: cfg::BasicBlock::rubyBlockId
    UnorderedMap<int, llvm::AllocaInst *> blockControlFramePtrs;

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

    // Mappinf from ruby block id to debug info scope.
    std::vector<llvm::DISubprogram *> blockScopes;

    // Records which ruby blocks use `break`, as that will forbid us from emitting type assertions on intrinsics that
    // they are used with.
    //
    // idx: cfg::BasicBlock::rubyBlockId
    // val: true when the block uses `break`
    std::vector<bool> blockUsesBreak;

    // Whether an explicit return statement ever appears in a context that has a
    // Ruby block as an ancestor.
    bool hasReturnAcrossBlock;

    // Mapping from ruby block id to the name for the block's iseq.
    std::vector<std::optional<std::string>> rubyBlockLocationNames;

    // Mapping from ruby block id to min/max arguments that block takes. As this is only used for block allocations,
    // it's {0, 0} for every ruby block id that is not a FunctionType::Block.
    std::vector<BlockArity> rubyBlockArity;

    static IREmitterContext getSorbetBlocks2LLVMBlockMapping(CompilerState &cs, cfg::CFG &cfg, const ast::MethodDef &md,
                                                             llvm::Function *mainFunc);
};

} // namespace sorbet::compiler

#endif
