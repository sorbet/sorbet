#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Type.h"

#include "cfg/CFG.h"
#include "compiler/Core/CompilerState.h"
#include "compiler/IREmitter/BasicBlockMap.h"
#include "compiler/IREmitter/IREmitterHelpers.h"
#include "compiler/IREmitter/Payload.h"

namespace sorbet::compiler {

namespace {

class ExceptionState {
    CompilerState &cs;
    llvm::IRBuilder<> &builder;
    const BasicBlockMap &blockMap;
    UnorderedMap<core::LocalVariable, Alias> &aliases;
    const int rubyBlockId;
    const int bodyRubyBlockId;
    const int handlersRubyBlockId;
    const int ensureRubyBlockId;
    const int elseRubyBlockId;
    core::LocalVariable exceptionValue;
    llvm::Function *currentFunc;
    llvm::Value *closure;

    llvm::Value *pc = nullptr;
    llvm::Value *iseq_encoded = nullptr;
    llvm::Value *nil = nullptr;
    llvm::Value *undef = nullptr;
    llvm::Function *doNothing = nullptr;
    llvm::Value *previousException = nullptr;
    llvm::BasicBlock *exceptionEntry = nullptr;

    ExceptionState(CompilerState &cs, llvm::IRBuilderBase &builder, const BasicBlockMap &blockMap,
                   UnorderedMap<core::LocalVariable, Alias> &aliases, int rubyBlockId, int bodyRubyBlockId,
                   core::LocalVariable exceptionValue)
        : cs(cs), builder(static_cast<llvm::IRBuilder<> &>(builder)), blockMap(blockMap), aliases(aliases),
          rubyBlockId(rubyBlockId), bodyRubyBlockId(bodyRubyBlockId),
          handlersRubyBlockId(bodyRubyBlockId + cfg::CFG::HANDLERS_BLOCK_OFFSET),
          ensureRubyBlockId(bodyRubyBlockId + cfg::CFG::ENSURE_BLOCK_OFFSET),
          elseRubyBlockId(bodyRubyBlockId + cfg::CFG::ELSE_BLOCK_OFFSET), exceptionValue(exceptionValue),
          currentFunc(blockMap.rubyBlocks2Functions[rubyBlockId]), closure(blockMap.escapedClosure[rubyBlockId]) {}

public:
    // Setup the context for compiling exception-handling code, and bring some needed constants into scope.
    static ExceptionState setup(CompilerState &cs, llvm::IRBuilderBase &builder, const BasicBlockMap &blockMap,
                                UnorderedMap<core::LocalVariable, Alias> &aliases, int rubyBlockId, int bodyRubyBlockId,
                                core::LocalVariable exceptionValue) {
        ExceptionState state(cs, builder, blockMap, aliases, rubyBlockId, bodyRubyBlockId, exceptionValue);

        // Setup local environment
        state.pc = state.builder.CreateLoad(state.blockMap.lineNumberPtrsByFunction[state.rubyBlockId]);
        state.iseq_encoded = state.builder.CreateLoad(state.blockMap.iseqEncodedPtrsByFunction[state.rubyBlockId]);
        state.nil = Payload::rubyNil(state.cs, state.builder);
        state.undef = Payload::rubyUndef(state.cs, state.builder);
        state.doNothing = cs.module->getFunction("sorbet_blockReturnUndef");

        // Store the last exception state
        state.previousException =
            state.builder.CreateCall(cs.module->getFunction("rb_errinfo"), {}, "previousException");

        // Setup the exception handling entry block
        state.exceptionEntry = llvm::BasicBlock::Create(cs, "exception-entry", state.currentFunc);
        state.builder.CreateBr(state.exceptionEntry);
        state.builder.SetInsertPoint(state.exceptionEntry);

        // Clear out the variable that we store the current exception in
        Payload::varSet(cs, exceptionValue, state.nil, builder, blockMap, aliases, rubyBlockId);

        return state;
    }

    llvm::Function *getEnsure() const {
        auto ensurePresent = blockMap.rubyBlockType[ensureRubyBlockId] != FunctionType::Unused;
        return ensurePresent ? blockMap.rubyBlocks2Functions[ensureRubyBlockId] : doNothing;
    }

    llvm::Function *getElse() const {
        auto elsePresent = blockMap.rubyBlockType[elseRubyBlockId] != FunctionType::Unused;
        return elsePresent ? blockMap.rubyBlocks2Functions[elseRubyBlockId] : doNothing;
    }

    llvm::Function *getHandlers() const {
        return blockMap.rubyBlocks2Functions[handlersRubyBlockId];
    }

    llvm::Value *sorbetEnsure(llvm::Value *handler, llvm::Value *exceptionResult) {
        return builder.CreateCall(
            cs.module->getFunction("sorbet_ensure"),
            {handler, getEnsure(), previousException, exceptionResult, pc, iseq_encoded, closure});
    }

    void restorePreviousException() {
        builder.CreateCall(cs.module->getFunction("rb_set_errinfo"), {previousException});
    }

    enum class Ensure {
        Needed,
        AlreadyCalled,
    };

    // If a non-undef return value was present, this means that a `return` happened. In this case, an exception
    // couldn't have happened, so restore the previous exception and return.
    void handleEarlyReturn(Ensure es, llvm::Value *handlerReturnValue) {
        auto *returnBlock = llvm::BasicBlock::Create(cs, "exception-return", currentFunc);
        auto *continueBlock = llvm::BasicBlock::Create(cs, "exception-continue", currentFunc);
        auto *earlyReturn = builder.CreateICmpNE(handlerReturnValue, undef, "earlyReturn");
        builder.CreateCondBr(earlyReturn, returnBlock, continueBlock);

        builder.SetInsertPoint(returnBlock);
        switch (es) {
            case Ensure::Needed: {
                auto *res = sorbetEnsure(doNothing, nil);
                auto *notUndef = builder.CreateICmpNE(res, undef, "ensureReturnValue");
                builder.CreateRet(builder.CreateSelect(notUndef, res, handlerReturnValue));
                break;
            }

            case Ensure::AlreadyCalled:
                builder.CreateRet(handlerReturnValue);
                break;
        }

        builder.SetInsertPoint(continueBlock);
    }

    // Returns the exception value that was raised.
    llvm::Value *runBody() {
        // Call the body, using sorbet_try to catch any exceptions raised and communicate them out via
        // exceptionResultPtr.
        auto *bodyFunction = blockMap.rubyBlocks2Functions[bodyRubyBlockId];
        auto *exceptionResultPtr = builder.CreateAlloca(llvm::Type::getInt64Ty(cs), nullptr, "exceptionValue");
        auto *bodyResult =
            builder.CreateCall(cs.module->getFunction("sorbet_try"),
                               {bodyFunction, pc, iseq_encoded, closure, exceptionResultPtr}, "bodyResult");

        handleEarlyReturn(Ensure::Needed, bodyResult);

        // Update the exceptionValue closure variable
        auto *exceptionResult = builder.CreateLoad(exceptionResultPtr);
        Payload::varSet(cs, exceptionValue, exceptionResult, builder, blockMap, aliases, rubyBlockId);

        return exceptionResult;
    }

    void runRescueElseEnsure(llvm::Value *exceptionResult) {
        auto *handlersFunction = getHandlers();
        auto *elseFunction = getElse();

        // Run the handler with sorbet_ensure, so that we always cleanup the VM exception state, and run the ensure
        // block.
        auto *exceptionRaised = builder.CreateICmpNE(exceptionResult, nil);
        auto *handler = builder.CreateSelect(exceptionRaised, handlersFunction, elseFunction, "handler");
        auto *res = sorbetEnsure(handler, exceptionResult);

        handleEarlyReturn(Ensure::AlreadyCalled, res);
    }

    // If no rescue clause handled the exception, the exceptionValue will contain a non-nil value. Test for that at the
    // end of exception handling, conditionally re-raising it to the outer context.
    void raiseUnhandledException() {
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
};

} // namespace

void IREmitterHelpers::emitExceptionHandlers(CompilerState &cs, llvm::IRBuilderBase &build,
                                             const BasicBlockMap &blockMap,
                                             UnorderedMap<core::LocalVariable, Alias> &aliases, int rubyBlockId,
                                             int bodyRubyBlockId, core::LocalVariable exceptionValue) {
    auto state = ExceptionState::setup(cs, build, blockMap, aliases, rubyBlockId, bodyRubyBlockId, exceptionValue);

    auto exceptionResult = state.runBody();
    state.runRescueElseEnsure(exceptionResult);
    state.raiseUnhandledException();

    return;
}

} // namespace sorbet::compiler
