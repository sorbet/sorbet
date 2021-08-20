#ifndef SORBET_COMPILER_IREMITTER_METHODCALLCONTEXT_H
#define SORBET_COMPILER_IREMITTER_METHODCALLCONTEXT_H

#include "llvm/IR/IRBuilder.h"

#include "compiler/Core/ForwardDeclarations.h"

#include <optional>
#include <vector>

namespace sorbet::compiler {

class CompilerState;
struct IREmitterContext;

class MethodCallContext {
    llvm::BasicBlock *sendEntry = nullptr;
    llvm::BasicBlock *sendContinuation = nullptr;

    // The LLVM register holding the evaluated receiver.
    // Use `mcctx.varGetRecv(...)` to get or initialize this.
    llvm::Value *recv = nullptr;

    llvm::Value *inlineCache = nullptr;
    std::vector<llvm::Value *> stack{};

    bool isFinalized = false;

    bool methodSearchPerformed = false;

    void initArgsAndCache();

    MethodCallContext(CompilerState &cs, llvm::IRBuilderBase &builder, const IREmitterContext &irctx, int rubyBlockId,
                      cfg::Send *send, std::optional<int> blk)
        : cs(cs), builder(builder), irctx(irctx), rubyBlockId(rubyBlockId), send(send), blk(blk){};

public:
    CompilerState &cs;

    llvm::IRBuilderBase &builder;

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
    //
    // WARNING: this method must be called on a path that will reach all remaning cases for method dispatch, or the
    // receiver will be initialized conditionally. This will cause the generated llvm will fail verification.
    llvm::Value *varGetRecv();

    // Get the inline cache to be used by this call.
    llvm::Value *getInlineCache();

    // Emit a call to `sorbet_vmMethodSearch` when the cache is created.
    //
    // WARNING: this method must be called on a path that will reach all remaning cases for method dispatch, or the
    // generated code will potentially query the inline cache without initializing it.
    void emitMethodSearch();

    // Get the args that would be pushed to the ruby stack (the receiver will be the first element)
    const std::vector<llvm::Value *> &getStackArgs();

    // Return the function associated with the block, nullptr if blk is std::nullopt.
    llvm::Function *blkAsFunction() const;

    // Returns true if any arguments in this send are T.untyped
    bool hasUntypedArgs() const;

    static MethodCallContext create(CompilerState &cs, llvm::IRBuilderBase &build, const IREmitterContext &irctx,
                                    int rubyBlockId, cfg::Send *send, std::optional<int> blk);

    // connect the send entry and the continuation
    void finalize();
};

} // namespace sorbet::compiler

#endif
