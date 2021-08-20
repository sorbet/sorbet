// These violate our poisons so have to happen first
#include "llvm/IR/IRBuilder.h"

#include "cfg/CFG.h"

#include "compiler/Core/CompilerState.h"
#include "compiler/IREmitter/IREmitterContext.h"
#include "compiler/IREmitter/IREmitterHelpers.h"
#include "compiler/IREmitter/MethodCallContext.h"
#include "compiler/IREmitter/Payload.h"

namespace sorbet::compiler {

MethodCallContext MethodCallContext::create(CompilerState &cs, llvm::IRBuilderBase &builder,
                                            const IREmitterContext &irctx, int rubyBlockId, cfg::Send *send,
                                            std::optional<int> blk) {
    MethodCallContext ret{cs, builder, irctx, rubyBlockId, send, blk};

    auto *func = builder.GetInsertBlock()->getParent();
    ret.sendEntry = llvm::BasicBlock::Create(cs, "sendEntry", func);
    ret.sendContinuation = llvm::BasicBlock::Create(cs, "sendContinuation", func);

    builder.CreateBr(ret.sendEntry);
    builder.SetInsertPoint(ret.sendContinuation);

    return ret;
}

void MethodCallContext::finalize() {
    ENFORCE(!this->isFinalized);

    auto saved = builder.saveIP();
    this->builder.SetInsertPoint(this->sendEntry);
    this->builder.CreateBr(this->sendContinuation);
    this->builder.restoreIP(saved);

    this->isFinalized = true;
}

void MethodCallContext::initArgsAndCache() {
    ENFORCE(!this->isFinalized);

    auto saved = this->builder.saveIP();

    this->builder.SetInsertPoint(this->sendEntry);
    auto [stack, keywords, cacheFlags] = IREmitterHelpers::buildSendArgs(*this, this->send->recv.variable, 0);
    this->stack = stack;

    auto methodName = std::string(this->send->fun.shortName(this->cs));
    this->inlineCache =
        IREmitterHelpers::makeInlineCache(this->cs, this->builder, methodName, cacheFlags, stack.size(), keywords);

    builder.restoreIP(saved);
}

llvm::Value *MethodCallContext::varGetRecv() {
    if (this->recv == nullptr) {
        ENFORCE(!this->isFinalized);

        auto saved = builder.saveIP();
        this->builder.SetInsertPoint(this->sendEntry);
        this->recv =
            Payload::varGet(this->cs, this->send->recv.variable, this->builder, this->irctx, this->rubyBlockId);
        this->builder.restoreIP(saved);
    }

    return this->recv;
}

llvm::Function *MethodCallContext::blkAsFunction() const {
    if (!blk.has_value()) {
        return nullptr;
    }

    return irctx.rubyBlocks2Functions[*blk];
}

llvm::Value *MethodCallContext::getInlineCache() {
    if (this->inlineCache == nullptr) {
        this->initArgsAndCache();
    }

    return this->inlineCache;
}

void MethodCallContext::emitMethodSearch() {
    if (this->methodSearchPerformed) {
        return;
    }

    ENFORCE(!this->isFinalized);

    auto *cache = this->getInlineCache();

    this->builder.CreateCall(cs.getFunction("sorbet_vmMethodSearch"), {cache, recv});
    this->methodSearchPerformed = true;
}

bool MethodCallContext::hasUntypedArgs() const {
    return absl::c_any_of(this->send->args, [](auto &arg) { return arg.type.isUntyped(); });
}

const std::vector<llvm::Value *> &MethodCallContext::getStackArgs() {
    if (this->inlineCache == nullptr) {
        this->initArgsAndCache();
    }

    return this->stack;
}

} // namespace sorbet::compiler
