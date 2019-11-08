// These violate our poisons so have to happen first
#include "llvm/IR/Attributes.h"
#include "llvm/IR/DerivedTypes.h" // FunctionType, StructType
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Verifier.h"
// ^^^ violate our poisons
#include "absl/base/casts.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "cfg/CFG.h"
#include "common/FileOps.h"
#include "common/sort.h"
#include "common/typecase.h"
#include "compiler/Errors/Errors.h"
#include "compiler/IRHelpers/IRHelpers.h"
#include "compiler/LLVMIREmitter/LLVMIREmitter.h"
#include "compiler/LLVMIREmitter/LLVMIREmitterHelpers.h"
#include "compiler/LLVMIREmitter/NameBasedIntrinsics.h"
#include "compiler/LLVMIREmitter/SymbolBasedIntrinsicMethod.h"
#include "compiler/Names/Names.h"
#include <string_view>

using namespace std;
namespace sorbet::compiler {
namespace {
llvm::IRBuilder<> &builderCast(llvm::IRBuilderBase &builder) {
    return static_cast<llvm::IRBuilder<> &>(builder);
};

}; // namespace

llvm::Value *LLVMIREmitterHelpers::emitMethodCall(CompilerState &cs, llvm::IRBuilderBase &build, cfg::Send *i,
                                                  const BasicBlockMap &blockMap,
                                                  UnorderedMap<core::LocalVariable, Alias> &aliases,
                                                  int currentRubyBlockId) {
    for (auto nameBasedIntrinsic : NameBasedIntrinsicMethod::definedIntrinsics()) {
        if (absl::c_linear_search(nameBasedIntrinsic->applicableMethods(cs), i->fun)) {
            return nameBasedIntrinsic->makeCall(cs, i, build, blockMap, aliases, currentRubyBlockId);
        }
    }
    for (auto symbolBasedIntrinsic : SymbolBasedIntrinsicMethod::definedIntrinsics()) {
        if (absl::c_linear_search(symbolBasedIntrinsic->applicableMethods(cs), i->fun)) {
            auto potentialClasses = symbolBasedIntrinsic->applicableClasses(cs);
            for (auto &c : potentialClasses) {
                if (i->recv.type->derivesFrom(cs, c)) {
                    auto &builder = builderCast(build);
                    auto recv = MK::varGet(cs, i->recv.variable, builder, aliases, blockMap, currentRubyBlockId);

                    auto typeTest = MK::createTypeTestU1(cs, builder, recv, core::make_type<core::ClassType>(c));

                    auto afterSend = llvm::BasicBlock::Create(cs, "afterCall", builder.GetInsertBlock()->getParent());
                    auto slowPath = llvm::BasicBlock::Create(cs, "slowCall", builder.GetInsertBlock()->getParent());
                    auto fastPath = llvm::BasicBlock::Create(cs, "slowCall", builder.GetInsertBlock()->getParent());
                    builder.CreateCondBr(MK::setExpectedBool(cs, builder, typeTest, true), fastPath, slowPath);
                    builder.SetInsertPoint(fastPath);
                    auto fastPathRes =
                        symbolBasedIntrinsic->makeCall(cs, i, build, blockMap, aliases, currentRubyBlockId);
                    auto fastPathEnd = builder.GetInsertBlock();
                    builder.CreateBr(afterSend);
                    builder.SetInsertPoint(slowPath);
                    auto slowPathRes = LLVMIREmitterHelpers::emitMethodCallViaRubyVM(cs, build, i, blockMap, aliases,
                                                                                     currentRubyBlockId);
                    auto slowPathEnd = builder.GetInsertBlock();
                    builder.CreateBr(afterSend);
                    builder.SetInsertPoint(afterSend);
                    auto phi = builder.CreatePHI(builder.getInt64Ty(), 2, "sendResSlowFastRaw");
                    phi->addIncoming(fastPathRes, fastPathEnd);
                    phi->addIncoming(slowPathRes, slowPathEnd);
                    return phi;
                }
            }
        };
    };
    return LLVMIREmitterHelpers::emitMethodCallViaRubyVM(cs, build, i, blockMap, aliases, currentRubyBlockId);
}

llvm::Value *LLVMIREmitterHelpers::emitMethodCallViaRubyVM(CompilerState &cs, llvm::IRBuilderBase &build, cfg::Send *i,
                                                           const BasicBlockMap &blockMap,
                                                           UnorderedMap<core::LocalVariable, Alias> &aliases,
                                                           int currentRubyBlockId) {
    auto &builder = builderCast(build);
    auto str = i->fun.data(cs)->shortName(cs);
    auto rawId = MK::getRubyIdFor(cs, builder, str);

    // fill in args
    {
        int argId = -1;
        for (auto &arg : i->args) {
            argId += 1;
            llvm::Value *indices[] = {llvm::ConstantInt::get(cs, llvm::APInt(32, 0, true)),
                                      llvm::ConstantInt::get(cs, llvm::APInt(64, argId, true))};
            auto var = MK::varGet(cs, arg.variable, builder, aliases, blockMap, currentRubyBlockId);
            builder.CreateStore(
                var, builder.CreateGEP(blockMap.sendArgArrayByBlock[currentRubyBlockId], indices, "callArgsAddr"));
        }
    }
    llvm::Value *indices[] = {llvm::ConstantInt::get(cs, llvm::APInt(64, 0, true)),
                              llvm::ConstantInt::get(cs, llvm::APInt(64, 0, true))};

    // TODO(perf): call
    // https://github.com/ruby/ruby/blob/3e3cc0885a9100e9d1bfdb77e136416ec803f4ca/internal.h#L2372
    // to get inline caching.
    // before this, perf will not be good
    auto var = MK::varGet(cs, i->recv.variable, builder, aliases, blockMap, currentRubyBlockId);
    llvm::Value *rawCall;
    if (i->link != nullptr) {
        // this send has a block!
        rawCall = builder.CreateCall(cs.module->getFunction("sorbet_callFuncBlock"),
                                     {var, rawId, llvm::ConstantInt::get(cs, llvm::APInt(32, i->args.size(), true)),
                                      builder.CreateGEP(blockMap.sendArgArrayByBlock[currentRubyBlockId], indices),
                                      blockMap.rubyBlocks2Functions[i->link->rubyBlockId],
                                      blockMap.escapedClosure[currentRubyBlockId]},
                                     "rawSendResult");

    } else {
        rawCall = builder.CreateCall(cs.module->getFunction("sorbet_callFunc"),
                                     {var, rawId, llvm::ConstantInt::get(cs, llvm::APInt(32, i->args.size(), true)),
                                      builder.CreateGEP(blockMap.sendArgArrayByBlock[currentRubyBlockId], indices)},
                                     "rawSendResult");
    }
    return rawCall;
};
} // namespace sorbet::compiler
