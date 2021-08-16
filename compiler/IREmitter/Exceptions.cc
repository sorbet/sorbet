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

llvm::Function *getExceptionFunc(CompilerState &cs, const IREmitterContext &irctx, int rubyBlockId) {
    auto present = irctx.rubyBlockType[rubyBlockId] != FunctionType::Unused;
    return present ? irctx.rubyBlocks2Functions[rubyBlockId] : getDoNothing(cs);
}
} // namespace

void IREmitterHelpers::emitExceptionHandlers(CompilerState &cs, llvm::IRBuilderBase &build,
                                             const IREmitterContext &irctx, int rubyBlockId, int bodyRubyBlockId,
                                             cfg::LocalRef exceptionValue) {
    auto &builder = static_cast<llvm::IRBuilder<> &>(build);

    const int handlersRubyBlockId = bodyRubyBlockId + cfg::CFG::HANDLERS_BLOCK_OFFSET;
    const int ensureRubyBlockId = bodyRubyBlockId + cfg::CFG::ENSURE_BLOCK_OFFSET;
    const int elseRubyBlockId = bodyRubyBlockId + cfg::CFG::ELSE_BLOCK_OFFSET;

    auto *currentFunc = irctx.rubyBlocks2Functions[rubyBlockId];
    // TODO: it would be nice if we could detect some of these functions
    // were empty and/or not useful so that we could eliminate dealing with
    // them from the exception handling logic.
    ENFORCE(irctx.rubyBlockType[handlersRubyBlockId] != FunctionType::Unused);
    // ENFORCE(irctx.rubyBlockType[ensureRubyBlockId] != FunctionType::Unused);
    // ENFORCE(irctx.rubyBlockType[elseRubyBlockId] != FunctionType::Unused);
    auto *handlersFunc = getExceptionFunc(cs, irctx, handlersRubyBlockId);
    auto *ensureFunc = getExceptionFunc(cs, irctx, ensureRubyBlockId);
    auto *elseFunc = getExceptionFunc(cs, irctx, elseRubyBlockId);

    // Allocate a place to hold the execution context.  This must go in the function
    // init block, otherwise the alloca might happen inside a loop, which could cause
    // a stack overflow at runtime.
    auto ip = builder.saveIP();
    builder.SetInsertPoint(irctx.functionInitializersByFunction[rubyBlockId]);
    auto *ecType = llvm::StructType::getTypeByName(cs, "struct.rb_execution_context_struct");
    auto *ecPtr = ecType->getPointerTo();
    auto *ecAlloca = builder.CreateAlloca(ecPtr, nullptr, "ecCache");
    builder.restoreIP(ip);

    auto *ec = builder.CreateCall(cs.getFunction("sorbet_getEC"), {}, "ec");
    builder.CreateStore(ec, ecAlloca);

    auto *pc = builder.CreateLoad(irctx.lineNumberPtrsByFunction[rubyBlockId]);
    auto *closure = Payload::buildLocalsOffset(cs);
    auto *cfp = Payload::getCFPForBlock(cs, builder, irctx, rubyBlockId);

    auto [index, level] = Payload::escapedVariableIndexAndLevel(cs, exceptionValue, irctx, bodyRubyBlockId);
    auto *v = builder.CreateCall(cs.getFunction("sorbet_run_exception_handling"),
                                 {ecAlloca, irctx.rubyBlocks2Functions[bodyRubyBlockId], pc, closure, cfp, handlersFunc,
                                  elseFunc, ensureFunc, Payload::retrySingleton(cs, builder, irctx), index, level});

    auto *exceptionContinue = llvm::BasicBlock::Create(cs, "exception-continue", currentFunc);
    auto *exceptionReturn = llvm::BasicBlock::Create(cs, "exception-return", currentFunc);

    auto *notUndef = builder.CreateICmpNE(v, Payload::rubyUndef(cs, builder), "ensureReturnValue");
    builder.CreateCondBr(notUndef, exceptionReturn, exceptionContinue);

    builder.SetInsertPoint(exceptionReturn);
    IREmitterHelpers::emitReturn(cs, builder, irctx, rubyBlockId, v);

    builder.SetInsertPoint(exceptionContinue);
    return;
}

} // namespace sorbet::compiler
