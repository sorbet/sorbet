#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Type.h"

#include "cfg/CFG.h"
#include "compiler/Core/CompilerState.h"
#include "compiler/IREmitter/BasicBlockMap.h"
#include "compiler/IREmitter/IREmitterHelpers.h"
#include "compiler/IREmitter/Payload.h"

namespace sorbet::compiler {

void IREmitterHelpers::emitExceptionHandlers(CompilerState &cs, llvm::IRBuilderBase &build,
                                             const BasicBlockMap &blockMap,
                                             UnorderedMap<core::LocalVariable, Alias> &aliases, int rubyBlockId,
                                             int bodyRubyBlockId, core::LocalVariable exceptionValue) {
    llvm::IRBuilder<> &builder = static_cast<llvm::IRBuilder<> &>(build);
    auto *closure = blockMap.escapedClosure[rubyBlockId];
    auto *currentFunc = blockMap.rubyBlocks2Functions[rubyBlockId];
    auto *nil = Payload::rubyNil(cs, build);

    auto *pc = builder.CreateLoad(blockMap.lineNumberPtrsByFunction[rubyBlockId]);
    auto *iseq_encoded = builder.CreateLoad(blockMap.iseqEncodedPtrsByFunction[rubyBlockId]);

    // 1. Write a nil to `exceptionValue` to make sure that it's properly initialized, and fetch the current exception
    // value so that we can restore it later.
    Payload::varSet(cs, exceptionValue, nil, builder, blockMap, aliases, rubyBlockId);
    auto *previousException = builder.CreateCall(cs.module->getFunction("rb_errinfo"), {}, "previousException");

    // 2. wrap the call to the body block in `rb_rescue2`, with the rescue handler writing out the exception value to
    // `exceptionValue`
    auto *bodyFunction = blockMap.rubyBlocks2Functions[bodyRubyBlockId];
    auto *exceptionResultPtr = builder.CreateAlloca(llvm::Type::getInt64Ty(cs), nullptr, "exceptionValue");
    auto *exceptionRaised =
        builder.CreateCall(cs.module->getFunction("sorbet_try"),
                           {bodyFunction, pc, iseq_encoded, closure, exceptionResultPtr}, "exceptionRaised");
    auto *exceptionResult = builder.CreateLoad(exceptionResultPtr);
    Payload::varSet(cs, exceptionValue, exceptionResult, builder, blockMap, aliases, rubyBlockId);

    // 3. if `exceptionValue` is non-nil, call the `handlers` block, otherwise call the `else` block. If there is an
    // ensure block present in the CFG, wrap this call in `rb_ensure`.
    auto handlersRubyBlockId = bodyRubyBlockId + 1;
    auto ensureRubyBlockId = bodyRubyBlockId + 2;
    auto elseRubyBlockId = bodyRubyBlockId + 3;

    auto *handlersFunction = blockMap.rubyBlocks2Functions[handlersRubyBlockId];

    // We use this block function in place of any user code when else or ensure are missing.
    auto *doNothing = cs.module->getFunction("sorbet_blockReturnNil");

    bool elsePresent = nullptr != blockMap.userEntryBlockByFunction[elseRubyBlockId];
    auto *elseFunction = elsePresent ? blockMap.rubyBlocks2Functions[elseRubyBlockId] : doNothing;

    bool ensurePresent = nullptr != blockMap.userEntryBlockByFunction[ensureRubyBlockId];
    auto *ensureFunction = ensurePresent ? blockMap.rubyBlocks2Functions[ensureRubyBlockId] : doNothing;

    // Run the handler with sorbet_ensure, so that we always cleanup the VM exception state, and run the ensure block.
    auto *handler = builder.CreateSelect(exceptionRaised, handlersFunction, elseFunction, "handler");
    builder.CreateCall(cs.module->getFunction("sorbet_ensure"),
                       {handler, ensureFunction, previousException, exceptionResult, pc, iseq_encoded, closure});

    // 4. Re-raise the exception value if it wasn't handled.
    {
        auto *raiseBlock = llvm::BasicBlock::Create(cs, "raise-unhandled", currentFunc);
        auto *exitBlock = llvm::BasicBlock::Create(cs, "exit-exception-handling", currentFunc);

        auto *exn = Payload::varGet(cs, exceptionValue, builder, blockMap, aliases, rubyBlockId);
        auto *reRaise = builder.CreateICmpNE(exn, nil, "exceptionNotHandled");

        builder.CreateCondBr(reRaise, raiseBlock, exitBlock);

        builder.SetInsertPoint(raiseBlock);
        builder.CreateCall(cs.module->getFunction("rb_exc_raise"), {exn});
        builder.CreateUnreachable();

        builder.SetInsertPoint(exitBlock);
    }

    return;
}

} // namespace sorbet::compiler
