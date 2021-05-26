// These violate our poisons so have to happen first
#include "llvm/IR/IRBuilder.h"

#include "cfg/CFG.h"

#include "compiler/IREmitter/IREmitterContext.h"
#include "compiler/IREmitter/MethodCallContext.h"
#include "compiler/IREmitter/Payload.h"

namespace sorbet::compiler {

namespace {
llvm::IRBuilder<> &builderCast(llvm::IRBuilderBase &builder) {
    return static_cast<llvm::IRBuilder<> &>(builder);
};
} // namespace

llvm::Value *MethodCallContext::varGetRecv() {
    if (this->recv == nullptr) {
        auto &builder = builderCast(this->build);
        this->recv = Payload::varGet(this->cs, this->send->recv.variable, builder, this->irctx, this->rubyBlockId);
    }

    return this->recv;
}

llvm::Function *MethodCallContext::blkAsFunction() const {
    if (!blk.has_value()) {
        return nullptr;
    }

    return irctx.rubyBlocks2Functions[*blk];
}

} // namespace sorbet::compiler
