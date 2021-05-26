#ifndef SORBET_COMPILER_IREMITTER_METHODCALLCONTEXT_H
#define SORBET_COMPILER_IREMITTER_METHODCALLCONTEXT_H

#include "compiler/Core/ForwardDeclarations.h"

#include <optional>

namespace sorbet::compiler {

class CompilerState;
struct IREmitterContext;

class MethodCallContext {
    // The LLVM register holding the evaluated receiver.
    // Use `mcctx.varGetRecv(...)` to get or initialize this.
    llvm::Value *recv = nullptr;

public:
    CompilerState &cs;

    llvm::IRBuilderBase &build;

    const IREmitterContext &irctx;

    // See IREmitterContext for a description of rubyBlockId. Primarily used to index into the
    // various vectors in `irctx`.
    int rubyBlockId;

    // The method call being emitted right now.
    cfg::Send *send;

    // The block id associated with this send via core::SendAndBlockLink.
    // `std::nullopt` if no `do ... end` block is provided at the call site.
    std::optional<int> blk;

    // Get the receiver for the send being emitted right now.
    // Use this to avoid accidentally calling Payload::varGet multiple times per one send,
    // duplicating work.
    llvm::Value *varGetRecv();

    // Return the function associated with the block, nullptr if blk is std::nullopt.
    llvm::Function *blkAsFunction() const;

    MethodCallContext(CompilerState &cs, llvm::IRBuilderBase &build, const IREmitterContext &irctx, int rubyBlockId,
                      cfg::Send *send, std::optional<int> blk)
        : cs(cs), build(build), irctx(irctx), rubyBlockId(rubyBlockId), send(send), blk(blk){};
};

} // namespace sorbet::compiler

#endif
