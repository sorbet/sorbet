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

class ExceptionState {
    CompilerState &cs;
    llvm::IRBuilder<> &builder;
    const IREmitterContext &irctx;
    const int rubyBlockId;
    const int bodyRubyBlockId;
    const int handlersRubyBlockId;
    const int ensureRubyBlockId;
    const int elseRubyBlockId;
    cfg::LocalRef exceptionValue;
    llvm::Function *currentFunc;

    llvm::Value *previousException = nullptr;
    llvm::Value *exceptionResultPtr = nullptr;

    llvm::BasicBlock *exceptionTagStack = nullptr;
    llvm::BasicBlock *exceptionEntry = nullptr;
    llvm::BasicBlock *bodyReturn = nullptr;
    llvm::BasicBlock *handlersBlock = nullptr;
    llvm::BasicBlock *ensureBlock = nullptr;
    llvm::BasicBlock *exceptionContinue = nullptr;
    llvm::BasicBlock *exceptionReturn = nullptr;
    llvm::PHINode *ensureValuePhi = nullptr;
    llvm::PHINode *ensureFromBodyPhi = nullptr;
    // We need this because we're going to compute a possible exception to throw
    // after running handlers, but we're going to use that possible exception in
    // a block that is not dominated by the block in which we compute the
    // exception.  So we need this somewhat useless phi node to correctly pass
    // the exception down to the place that needs it.
    llvm::PHINode *previousExceptionPhi = nullptr;

    // setjmp/longjmp bits for ensuring that we execute ensure.

    // Dummy value for the result of executing the exception handling region.
    // Normally this would come from the body or one of the handlers, but since
    // we have a path to the ensure handler that doesn't go through either one
    // of those (i.e. longjmp'ing to this->tag->buf), we need a value that we
    // can pass to the appropriate phi node.
    llvm::Value *undefFromRegion = nullptr;
    // A struct rb_vm_tag.
    llvm::Value *tag = nullptr;
    // Return value from sorbet_initializeTag.
    llvm::Value *tagState = nullptr;

    ExceptionState(CompilerState &cs, llvm::IRBuilderBase &builder, const IREmitterContext &irctx, int rubyBlockId,
                   int bodyRubyBlockId, cfg::LocalRef exceptionValue)
        : cs(cs), builder(static_cast<llvm::IRBuilder<> &>(builder)), irctx(irctx), rubyBlockId(rubyBlockId),
          bodyRubyBlockId(bodyRubyBlockId), handlersRubyBlockId(bodyRubyBlockId + cfg::CFG::HANDLERS_BLOCK_OFFSET),
          ensureRubyBlockId(bodyRubyBlockId + cfg::CFG::ENSURE_BLOCK_OFFSET),
          elseRubyBlockId(bodyRubyBlockId + cfg::CFG::ELSE_BLOCK_OFFSET), exceptionValue(exceptionValue),
          currentFunc(irctx.rubyBlocks2Functions[rubyBlockId]) {}

public:
    // Setup the context for compiling exception-handling code, and bring some needed constants into scope.
    static ExceptionState setup(CompilerState &cs, llvm::IRBuilderBase &builder, const IREmitterContext &irctx,
                                int rubyBlockId, int bodyRubyBlockId, cfg::LocalRef exceptionValue) {
        ExceptionState state(cs, builder, irctx, rubyBlockId, bodyRubyBlockId, exceptionValue);

        // Allocate a place to store the exception result from sorbet_try. This must go in the function init block,
        // otherwise the allocation might happen inside of a loop, causing a stack overflow.
        auto ip = state.builder.saveIP();
        state.builder.SetInsertPoint(irctx.functionInitializersByFunction[rubyBlockId]);
        state.exceptionResultPtr = state.builder.CreateAlloca(llvm::Type::getInt64Ty(cs), nullptr, "exceptionValue");
        state.tag =
            state.builder.CreateAlloca(llvm::StructType::getTypeByName(state.cs, "struct.rb_vm_tag"), nullptr, "ecTag");
        state.builder.restoreIP(ip);

        // Setup all the blocks we are going to need ahead of time.
        auto *currentFunc = state.currentFunc;
        state.exceptionTagStack = llvm::BasicBlock::Create(cs, "exception-tag-stack", currentFunc);
        state.exceptionEntry = llvm::BasicBlock::Create(cs, "exception-entry", currentFunc);
        state.bodyReturn = llvm::BasicBlock::Create(cs, "exception-body-return", currentFunc);
        state.handlersBlock = llvm::BasicBlock::Create(cs, "exception-body-handlers", currentFunc);
        state.ensureBlock = llvm::BasicBlock::Create(cs, "exception-ensure", currentFunc);
        state.exceptionContinue = llvm::BasicBlock::Create(cs, "exception-continue", currentFunc);
        state.exceptionReturn = llvm::BasicBlock::Create(cs, "exception-return", currentFunc);

        // Store the last exception state
        state.previousException = state.builder.CreateCall(cs.getFunction("rb_errinfo"), {}, "previousException");
        state.builder.CreateBr(state.exceptionTagStack);

        // Call setjmp to record a point that we can unwind to.
        // If we longjmp to this point, we are unwinding through the exception region
        // and we need to ensure that we're going to execute the ensure handler.
        state.builder.SetInsertPoint(state.exceptionTagStack);
        state.undefFromRegion = state.builder.CreateCall(cs.getFunction("sorbet_rubyUndef"), {}, "undefValue");
        state.tagState = state.builder.CreateCall(cs.getFunction("sorbet_initializeTag"), {state.tag}, "tagState");
        // 0 here is doing double-duty as TAG_NONE but also representing a "normal" return
        // from calling setjmp.
        auto *longjmped = state.builder.CreateICmpNE(state.tagState, builder.getInt32(0));
        builder.CreateCondBr(longjmped, state.ensureBlock, state.exceptionEntry);

        // Ensure we're not going to insert the phi nodes yet.
        state.builder.ClearInsertionPoint();
        // Create these phi nodes for later use.
        state.ensureValuePhi = state.builder.CreatePHI(state.builder.getInt64Ty(), 2, "ensureValue");
        state.ensureFromBodyPhi = state.builder.CreatePHI(state.builder.getInt1Ty(), 2, "ensureFromBody");
        state.previousExceptionPhi =
            state.builder.CreatePHI(state.builder.getInt64Ty(), 2, "computedPreviousException");

        // Setup the exception handling entry block
        state.builder.SetInsertPoint(state.exceptionEntry);

        // Clear out the variable that we store the current exception in
        Payload::varSet(cs, exceptionValue, Payload::rubyNil(state.cs, state.builder), builder, irctx, rubyBlockId);

        return state;
    }

    llvm::Function *getDoNothing() const {
        return cs.getFunction("sorbet_blockReturnUndef");
    }

    llvm::Function *getEnsure() const {
        auto ensurePresent = irctx.rubyBlockType[ensureRubyBlockId] != FunctionType::Unused;
        return ensurePresent ? irctx.rubyBlocks2Functions[ensureRubyBlockId] : getDoNothing();
    }

    llvm::Function *getElse() const {
        auto elsePresent = irctx.rubyBlockType[elseRubyBlockId] != FunctionType::Unused;
        return elsePresent ? irctx.rubyBlocks2Functions[elseRubyBlockId] : getDoNothing();
    }

    llvm::Function *getHandlers() const {
        return irctx.rubyBlocks2Functions[handlersRubyBlockId];
    }

    // Fetch the pc, and closure values that are used when calling an exception function.
    tuple<llvm::Value *, llvm::Value *, llvm::Value *> getExceptionArgs() {
        auto *pc = builder.CreateLoad(irctx.lineNumberPtrsByFunction[rubyBlockId]);
        auto *closure = Payload::buildLocalsOffset(cs);
        auto *cfp = Payload::getCFPForBlock(cs, builder, irctx, rubyBlockId);
        return {pc, closure, cfp};
    }

    // Run a function that may raiase exceptions.
    tuple<llvm::Value *, llvm::Value *> sorbetTry(llvm::Function *fun, llvm::Value *exceptionContext) {
        auto [pc, closure, cfp] = getExceptionArgs();
        auto *result = builder.CreateCall(cs.getFunction("sorbet_try"),
                                          {fun, pc, closure, cfp, exceptionContext, exceptionResultPtr}, "result");

        return {result, exceptionResultPtr};
    }

    // Run the ensure clause, overwriting the previous return value that was passed in if it's present.
    llvm::Value *sorbetEnsure(llvm::Value *previousReturnValue) {
        auto [pc, closure, cfp] = getExceptionArgs();
        auto *res = builder.CreateCall(getEnsure(), {pc, closure, cfp}, "ensureResult");
        auto *notUndef = builder.CreateICmpNE(res, Payload::rubyUndef(cs, builder), "ensureReturnValue");
        return builder.CreateSelect(notUndef, res, previousReturnValue);
    }

    void raiseIfNotNil(llvm::Value *exceptionValue) {
        builder.CreateCall(cs.getFunction("sorbet_raiseIfNotNil"), {exceptionValue});
    }

    // Returns the exception value that was raised.
    llvm::Value *runBody() {
        // Call the body, using sorbet_try to catch any exceptions raised and communicate them out via
        // exceptionResultPtr.
        auto *bodyFunction = irctx.rubyBlocks2Functions[bodyRubyBlockId];
        auto [bodyResult, exceptionResultPtr] = sorbetTry(bodyFunction, previousException);

        // Check for an early return from the body.
        auto *earlyReturnBlock = this->bodyReturn;
        auto *continueBlock = this->handlersBlock;
        auto *isReturnValue = builder.CreateICmpNE(bodyResult, Payload::rubyUndef(cs, builder), "isReturnValue");
        builder.CreateCondBr(isReturnValue, earlyReturnBlock, continueBlock);

        // If a non-undef value was returned from the body, there's no possibility that an exception was raised. Run
        // ensure and return the body result.
        builder.SetInsertPoint(earlyReturnBlock);
        builder.CreateCall(cs.getFunction("rb_set_errinfo"), {previousException});
        auto *nil = Payload::rubyNil(cs, builder);
        builder.CreateBr(this->ensureBlock);

        this->ensureValuePhi->addIncoming(bodyResult, earlyReturnBlock);
        this->ensureFromBodyPhi->addIncoming(builder.getTrue(), earlyReturnBlock);
        // This is somewhat bogus, but the value we're passing here will never
        // actually be used, because there's no actual execution path from a
        // no-exceptions-raised body to a maybe-raise-exceptions-from-handler.
        this->previousExceptionPhi->addIncoming(nil, earlyReturnBlock);

        // Update the exceptionValue closure variable to hold the exception raised.
        builder.SetInsertPoint(continueBlock);
        auto *exceptionResult = builder.CreateLoad(exceptionResultPtr);
        Payload::varSet(cs, exceptionValue, exceptionResult, builder, irctx, rubyBlockId);

        return exceptionResult;
    }

    llvm::Value *determinePostRescueExceptionContext(llvm::Value *handlerException) {
        auto *handlerRaised = builder.CreateICmpNE(handlerException, Payload::rubyNil(cs, builder));

        auto *exn = Payload::varGet(cs, exceptionValue, builder, irctx, rubyBlockId);
        auto *exceptionNotHandled = builder.CreateICmpNE(exn, Payload::rubyNil(cs, builder));

        // There are three exceptions that need to be considered when determining the state after a rescue handler has
        // been applied. They're listed in order of preference when being restored for the ensure handler:
        //
        // 1. An exception raised by the rescue block
        // 2. The original exception raised by the body, that wasn't handled by the rescue block
        // 3. The ambient previous exception that was present before exception handling began
        auto *unhandledOrPrevious = builder.CreateSelect(exceptionNotHandled, exn, previousException);
        return builder.CreateSelect(handlerRaised, handlerException, unhandledOrPrevious);
    }

    void runRescueElseEnsure(llvm::Value *exceptionResult) {
        auto *handlersFunction = getHandlers();
        auto *elseFunction = getElse();

        // Run the handler with sorbet_handle, so that we cleanup the vm exception state when it exits successfully.
        auto *exceptionRaised = builder.CreateICmpNE(exceptionResult, Payload::rubyNil(cs, builder));
        auto *handler = builder.CreateSelect(exceptionRaised, handlersFunction, elseFunction, "handler");
        auto [handlerResult, handlerExceptionPtr] = sorbetTry(static_cast<llvm::Function *>(handler), exceptionResult);

        // Setup the exception state for the ensure handler
        auto *handlerException = builder.CreateLoad(handlerExceptionPtr);
        auto *exceptionContext = determinePostRescueExceptionContext(handlerException);
        builder.CreateCall(cs.getFunction("rb_set_errinfo"), {exceptionContext});

        // Check the handlerResult for a retry, and branch back to the beginning of exception handling if it's present
        // and we ran the rescue block.
        // NOTE: if retry was returned from the handler, we know that no new exceptions were raised (exceptionContext
        // will be nil), and that the previous exception will have been restored.
        auto *shouldRetry = builder.CreateAnd(
            exceptionRaised,
            builder.CreateICmpEQ(handlerResult, Payload::retrySingleton(cs, builder, irctx), "shouldRetry"));
        builder.CreateCondBr(shouldRetry, exceptionEntry, this->ensureBlock);

        this->ensureValuePhi->addIncoming(handlerResult, this->handlersBlock);
        this->ensureFromBodyPhi->addIncoming(builder.getFalse(), this->handlersBlock);
        this->previousExceptionPhi->addIncoming(exceptionContext, this->handlersBlock);

        this->ensureValuePhi->addIncoming(this->undefFromRegion, this->exceptionTagStack);
        this->ensureFromBodyPhi->addIncoming(builder.getFalse(), this->exceptionTagStack);
        this->previousExceptionPhi->addIncoming(this->previousException, this->exceptionTagStack);

        // The phi nodes are now completely constructed.

        // the handler didn't retry, run ensure and return its value.
        builder.SetInsertPoint(this->ensureBlock);
        builder.Insert(this->ensureValuePhi);
        builder.Insert(this->ensureFromBodyPhi);
        builder.Insert(this->previousExceptionPhi);

        auto *continueBlock = this->exceptionContinue;
        auto *returnBlock = this->exceptionReturn;

        // However we got here, we are done with the entry on the tag stack that
        // we pushed at the start of this process.
        builder.CreateCall(this->cs.getFunction("sorbet_teardownTagForThrowReturn"), {this->tag});

        // Run the ensure block with whatever value we have produced by running
        // the body + applicable handlers.
        auto *ensureResult = sorbetEnsure(this->ensureValuePhi);

        // We might have arrived at running the ensure via unwinding the call stack.
        // Calling this function will handle resuming that unwind if necessary.
        builder.CreateCall(this->cs.getFunction("sorbet_maybeContinueUnwind"), {this->tagState});

        // This compare+branch is a little subtle.  If we returned from the body,
        // running the ensure will choose the ensure's result (if it is not undef)
        // or the body's result.  Therefore, arriving here having executed the
        // body will always choose the return.
        //
        // If some handler has returned a value, then the logic is largely the
        // same as the above.  And if we had an exception, then ensureValuePhi
        // will be undef, so we rely on whether or not the ensure returned a value.
        auto *isReturnValue = builder.CreateICmpNE(ensureResult, Payload::rubyUndef(cs, builder), "isReturnValue");
        builder.CreateCondBr(isReturnValue, returnBlock, continueBlock);

        builder.SetInsertPoint(returnBlock);
        IREmitterHelpers::emitReturn(cs, builder, irctx, rubyBlockId, ensureResult);

        // Re-raise if an exception was raised by the handler
        builder.SetInsertPoint(continueBlock);
        raiseIfNotNil(this->previousExceptionPhi);
    }

    // If no rescue clause handled the exception, the exceptionValue will contain a non-nil value. Test for that at
    // the end of exception handling, conditionally re-raising it to the outer context.
    void raiseUnhandledException() {
        auto *exn = Payload::varGet(cs, exceptionValue, builder, irctx, rubyBlockId);
        raiseIfNotNil(exn);
    }
};

} // namespace

void IREmitterHelpers::emitExceptionHandlers(CompilerState &cs, llvm::IRBuilderBase &build,
                                             const IREmitterContext &irctx, int rubyBlockId, int bodyRubyBlockId,
                                             cfg::LocalRef exceptionValue) {
    auto state = ExceptionState::setup(cs, build, irctx, rubyBlockId, bodyRubyBlockId, exceptionValue);

    auto exceptionResult = state.runBody();
    state.runRescueElseEnsure(exceptionResult);
    state.raiseUnhandledException();

    return;
}

} // namespace sorbet::compiler
