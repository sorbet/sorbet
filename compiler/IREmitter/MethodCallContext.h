#ifndef SORBET_COMPILER_IREMITTER_METHODCALLCONTEXT_H
#define SORBET_COMPILER_IREMITTER_METHODCALLCONTEXT_H

#include "compiler/Core/ForwardDeclarations.h"

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

    // The compiled function (like `func_A.main$block_1`) associated with this send via the
    // core::SendAndBlockLink. `nullptr` if no `do ... end` block provided at the call site.
    llvm::Function *blk;

    // Get the receiver for the send being emitted right now.
    // Use this to avoid accidentally calling Payload::varGet multiple times per one send,
    // duplicating work.
    llvm::Value *varGetRecv();

    MethodCallContext(CompilerState &cs, llvm::IRBuilderBase &build, const IREmitterContext &irctx, int rubyBlockId,
                      cfg::Send *send, llvm::Function *blk)
        : cs(cs), build(build), irctx(irctx), rubyBlockId(rubyBlockId), send(send), blk(blk){};
};

} // namespace sorbet::compiler

#endif
