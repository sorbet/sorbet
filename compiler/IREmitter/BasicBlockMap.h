#ifndef SORBET_COMPILER_IREMITTER_BASIC_BLOCK_MAP_H
#define SORBET_COMPILER_IREMITTER_BASIC_BLOCK_MAP_H

#include "core/core.h"

namespace sorbet::compiler {
struct BasicBlockMap {
    core::SymbolRef forMethod;
    std::vector<llvm::BasicBlock *> functionInitializersByFunction;
    std::vector<llvm::BasicBlock *> argumentSetupBlocksByFunction;
    std::vector<llvm::BasicBlock *> userEntryBlockByFunction;
    std::vector<llvm::BasicBlock *> llvmBlocksBySorbetBlocks;
    std::vector<int> basicBlockJumpOverrides;
    std::vector<llvm::AllocaInst *> sendArgArrayByBlock;
    std::vector<llvm::Value *> escapedClosure;
    UnorderedMap<core::LocalVariable, int> escapedVariableIndeces;
    llvm::BasicBlock *sigVerificationBlock;
    llvm::BasicBlock *postProcessBlock;
    std::vector<std::shared_ptr<core::SendAndBlockLink>> blockLinks;
    std::vector<std::vector<core::LocalVariable>> rubyBlockArgs;
    std::vector<llvm::Function *> rubyBlocks2Functions;
    UnorderedMap<core::LocalVariable, llvm::AllocaInst *> llvmVariables;
    std::vector<llvm::AllocaInst *> lineNumberPtrsByFunction;
    std::vector<llvm::AllocaInst *> iseqEncodedPtrsByFunction;
    bool usesBlockArgs;
};
} // namespace sorbet::compiler

#endif
