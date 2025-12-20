#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Type.h"

#include "cfg/CFG.h"
#include "compiler/Core/CompilerState.h"
#include "compiler/IREmitter/IREmitterContext.h"
#include "compiler/IREmitter/IREmitterHelpers.h"
#include "compiler/IREmitter/Payload.h"

using namespace std;

namespace sorbet::compiler {

namespace {
llvm::Function *getDoNothing(CompilerState &cs) {
    return cs.getFunction("sorbet_blockReturnUndef");
}

llvm::Function *getExceptionFunc(CompilerState &cs, const IREmitterContext &irctx, int rubyRegionId) {
    auto present = irctx.rubyBlockType[rubyRegionId] != FunctionType::Unused;
    return present ? irctx.rubyBlocks2Functions[rubyRegionId] : getDoNothing(cs);
}
} // namespace

void IREmitterHelpers::emitExceptionHandlers(CompilerState &cs, llvm::IRBuilderBase &builder,
                                             const IREmitterContext &irctx, int rubyRegionId, int bodyRubyRegionId,
                                             cfg::LocalRef exceptionValue) {
    const int handlersRubyRegionId = bodyRubyRegionId + cfg::CFG::HANDLERS_REGION_OFFSET;
    const int ensureRubyRegionId = bodyRubyRegionId + cfg::CFG::ENSURE_REGION_OFFSET;
    const int elseRubyRegionId = bodyRubyRegionId + cfg::CFG::ELSE_REGION_OFFSET;

    auto *currentFunc = irctx.rubyBlocks2Functions[rubyRegionId];
    // TODO: it would be nice if we could detect some of these functions
    // were empty and/or not useful so that we could eliminate dealing with
    // them from the exception handling logic.
    // Note: For begin/ensure without rescue, handlersRubyRegionId may be Unused.
    // getExceptionFunc handles this by returning a no-op function.
    auto *handlersFunc = getExceptionFunc(cs, irctx, handlersRubyRegionId);
    auto *ensureFunc = getExceptionFunc(cs, irctx, ensureRubyRegionId);
    auto *elseFunc = getExceptionFunc(cs, irctx, elseRubyRegionId);

    auto *ec = builder.CreateCall(cs.getFunction("sorbet_getEC"), {}, "ec");

    auto *lineNumAlloca = irctx.lineNumberPtrsByFunction[rubyRegionId];
    auto *pc = builder.CreateLoad(lineNumAlloca->getAllocatedType(), lineNumAlloca);
    auto *closure = Payload::buildLocalsOffset(cs);
    auto *cfp = Payload::getCFPForBlock(cs, builder, irctx, rubyRegionId);

    auto info = Payload::escapedVariableInfo(cs, exceptionValue, irctx, bodyRubyRegionId);
    auto *exceptionHandlingFunc = cs.getFunction("sorbet_run_exception_handling");
    auto *funcType = exceptionHandlingFunc->getFunctionType();
    // Cast function pointers to the expected types (for LLVM 15 typed pointers)
    auto castFn = [&](llvm::Value *fn, unsigned paramIdx) -> llvm::Value * {
        auto *expectedType = funcType->getParamType(paramIdx);
        if (fn->getType() != expectedType) {
            return builder.CreateBitCast(fn, expectedType);
        }
        return fn;
    };
    auto *v =
        builder.CreateCall(exceptionHandlingFunc,
                           {ec, castFn(irctx.rubyBlocks2Functions[bodyRubyRegionId], 1), pc, closure, cfp,
                            castFn(handlersFunc, 5), castFn(elseFunc, 6), castFn(ensureFunc, 7),
                            Payload::retrySingleton(cs, builder, irctx), info.index, info.level});

    auto *exceptionContinue = llvm::BasicBlock::Create(cs, "exception-continue", currentFunc);
    auto *exceptionReturn = llvm::BasicBlock::Create(cs, "exception-return", currentFunc);

    auto *notUndef = builder.CreateICmpNE(v, Payload::rubyUndef(cs, builder), "ensureReturnValue");
    builder.CreateCondBr(notUndef, exceptionReturn, exceptionContinue);

    builder.SetInsertPoint(exceptionReturn);
    IREmitterHelpers::emitReturn(cs, builder, irctx, rubyRegionId, v);

    builder.SetInsertPoint(exceptionContinue);
    return;
}

} // namespace sorbet::compiler
