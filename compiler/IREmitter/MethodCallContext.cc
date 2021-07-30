// These violate our poisons so have to happen first
#include "llvm/IR/IRBuilder.h"

#include "cfg/CFG.h"

#include "compiler/Core/CompilerState.h"
#include "compiler/IREmitter/IREmitterContext.h"
#include "compiler/IREmitter/IREmitterHelpers.h"
#include "compiler/IREmitter/MethodCallContext.h"
#include "compiler/IREmitter/Payload.h"

namespace sorbet::compiler {

namespace {
llvm::IRBuilder<> &builderCast(llvm::IRBuilderBase &builder) {
    return static_cast<llvm::IRBuilder<> &>(builder);
};
} // namespace

MethodCallContext MethodCallContext::create(CompilerState &cs, llvm::IRBuilderBase &build,
                                            const IREmitterContext &irctx, int rubyBlockId, cfg::Send *send,
                                            std::optional<int> blk) {
    MethodCallContext ret{cs, build, irctx, rubyBlockId, send, blk};

    auto &builder = builderCast(build);

    auto *func = builderCast(build).GetInsertBlock()->getParent();
    ret.sendEntry = llvm::BasicBlock::Create(cs, "sendEntry", func);
    ret.sendContinuation = llvm::BasicBlock::Create(cs, "sendContinuation", func);

    builder.CreateBr(ret.sendEntry);
    builder.SetInsertPoint(ret.sendContinuation);

    return ret;
}

void MethodCallContext::finalize() {
    ENFORCE(!this->isFinalized);

    auto &builder = builderCast(this->build);

    auto saved = builder.saveIP();
    builder.SetInsertPoint(this->sendEntry);
    builder.CreateBr(this->sendContinuation);
    builder.restoreIP(saved);

    this->isFinalized = true;
}

void MethodCallContext::initArgsAndCache() {
    ENFORCE(!this->isFinalized);

    auto &builder = builderCast(this->build);

    auto saved = builder.saveIP();

    builder.SetInsertPoint(this->sendEntry);
    auto [stack, keywords, cacheFlags] = IREmitterHelpers::buildSendArgs(*this, this->send->recv.variable, 0);
    this->stack = stack;

    auto methodName = std::string(this->send->fun.shortName(this->cs));
    this->inlineCache =
        IREmitterHelpers::makeInlineCache(this->cs, this->build, methodName, cacheFlags, stack.size(), keywords);

    builder.restoreIP(saved);
}

llvm::Value *MethodCallContext::varGetRecv() {
    if (this->recv == nullptr) {
        ENFORCE(!this->isFinalized);

        auto &builder = builderCast(this->build);

        auto saved = builder.saveIP();
        builder.SetInsertPoint(this->sendEntry);
        this->recv = Payload::varGet(this->cs, this->send->recv.variable, builder, this->irctx, this->rubyBlockId);
        builder.restoreIP(saved);
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

    auto &builder = builderCast(this->build);

    builder.CreateCall(cs.getFunction("sorbet_vmMethodSearch"), {cache, recv});
    this->methodSearchPerformed = true;
}

const std::vector<llvm::Value *> &MethodCallContext::getStackArgs() {
    if (this->inlineCache == nullptr) {
        this->initArgsAndCache();
    }

    return this->stack;
}

} // namespace sorbet::compiler
