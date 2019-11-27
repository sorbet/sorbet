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
#include "compiler/Core/CompilerState.h"
#include "compiler/Errors/Errors.h"
#include "compiler/IREmitter/BasicBlockMap.h"
#include "compiler/IREmitter/IREmitter.h"
#include "compiler/IREmitter/IREmitterHelpers.h"
#include "compiler/IREmitter/NameBasedIntrinsics.h"
#include "compiler/IREmitter/Payload.h"
#include "compiler/IREmitter/SymbolBasedIntrinsicMethod.h"
#include "compiler/Names/Names.h"
#include <string_view>

using namespace std;
namespace sorbet::compiler {
namespace {
llvm::IRBuilder<> &builderCast(llvm::IRBuilderBase &builder) {
    return static_cast<llvm::IRBuilder<> &>(builder);
};

}; // namespace

llvm::Value *IREmitterHelpers::emitMethodCall(CompilerState &cs, llvm::IRBuilderBase &build, cfg::Send *i,
                                              const BasicBlockMap &blockMap,
                                              UnorderedMap<core::LocalVariable, Alias> &aliases, int rubyBlockId) {
    for (auto symbolBasedIntrinsic : SymbolBasedIntrinsicMethod::definedIntrinsics()) {
        if (absl::c_linear_search(symbolBasedIntrinsic->applicableMethods(cs), i->fun)) {
            auto potentialClasses = symbolBasedIntrinsic->applicableClasses(cs);
            for (auto &c : potentialClasses) {
                if (i->recv.type->derivesFrom(cs, c)) {
                    auto methodName = i->fun.data(cs)->shortName(cs);
                    llvm::StringRef methodNameRef(methodName.data(), methodName.size());
                    auto &builder = builderCast(build);
                    auto recv = Payload::varGet(cs, i->recv.variable, builder, blockMap, aliases, rubyBlockId);

                    auto typeTest = Payload::typeTest(cs, builder, recv, core::make_type<core::ClassType>(c));

                    auto afterSend = llvm::BasicBlock::Create(cs, llvm::Twine("afterSymCallIntrinsic_") + methodNameRef,
                                                              builder.GetInsertBlock()->getParent());
                    auto slowPath = llvm::BasicBlock::Create(cs, llvm::Twine("slowSymCallIntrinsic_") + methodNameRef,
                                                             builder.GetInsertBlock()->getParent());
                    auto fastPath = llvm::BasicBlock::Create(cs, llvm::Twine("fastSymCallIntrinsic_") + methodNameRef,
                                                             builder.GetInsertBlock()->getParent());
                    builder.CreateCondBr(Payload::setExpectedBool(cs, builder, typeTest, true), fastPath, slowPath);
                    builder.SetInsertPoint(fastPath);
                    auto fastPathRes = symbolBasedIntrinsic->makeCall(cs, i, build, blockMap, aliases, rubyBlockId);
                    auto fastPathEnd = builder.GetInsertBlock();
                    builder.CreateBr(afterSend);
                    builder.SetInsertPoint(slowPath);
                    auto slowPathRes =
                        IREmitterHelpers::emitMethodCallViaRubyVM(cs, build, i, blockMap, aliases, rubyBlockId);
                    auto slowPathEnd = builder.GetInsertBlock();
                    builder.CreateBr(afterSend);
                    builder.SetInsertPoint(afterSend);
                    auto phi =
                        builder.CreatePHI(builder.getInt64Ty(), 2, llvm::Twine("symIntrinsicRawPhi_") + methodNameRef);
                    phi->addIncoming(fastPathRes, fastPathEnd);
                    phi->addIncoming(slowPathRes, slowPathEnd);
                    return phi;
                }
            }
        };
    };
    for (auto nameBasedIntrinsic : NameBasedIntrinsicMethod::definedIntrinsics()) {
        if (absl::c_linear_search(nameBasedIntrinsic->applicableMethods(cs), i->fun)) {
            return nameBasedIntrinsic->makeCall(cs, i, build, blockMap, aliases, rubyBlockId);
        }
    }

    auto &recvType = i->recv.type;
    core::SymbolRef recvClass = core::Symbols::noSymbol();
    if (auto ct = core::cast_type<core::ClassType>(recvType.get())) {
        recvClass = ct->symbol;
    } else if (auto at = core::cast_type<core::AppliedType>(recvType.get())) {
        recvClass = at->klass;
    }

    if (recvClass.exists()) {
        auto funSym = recvClass.data(cs)->findMember(cs, i->fun);
        if (funSym.exists() && funSym.data(cs)->isFinalMethod()) {
            auto llvmFunc = IREmitterHelpers::lookupFunction(cs, funSym);
            if (llvmFunc != nullptr) {
                auto methodName = i->fun.data(cs)->shortName(cs);
                llvm::StringRef methodNameRef(methodName.data(), methodName.size());
                auto &builder = builderCast(build);
                auto recv = Payload::varGet(cs, i->recv.variable, builder, blockMap, aliases, rubyBlockId);

                auto typeTest = Payload::typeTest(cs, builder, recv, core::make_type<core::ClassType>(recvClass));

                auto afterSend = llvm::BasicBlock::Create(cs, llvm::Twine("afterCallFinal_") + methodNameRef,
                                                          builder.GetInsertBlock()->getParent());
                auto slowPath = llvm::BasicBlock::Create(cs, llvm::Twine("slowCallFinal_") + methodNameRef,
                                                         builder.GetInsertBlock()->getParent());
                auto fastPath = llvm::BasicBlock::Create(cs, llvm::Twine("fastCallFinal_") + methodNameRef,
                                                         builder.GetInsertBlock()->getParent());
                builder.CreateCondBr(Payload::setExpectedBool(cs, builder, typeTest, true), fastPath, slowPath);
                builder.SetInsertPoint(fastPath);
                auto fastPathRes =
                    IREmitterHelpers::emitMethodCallDirrect(cs, build, funSym, i, blockMap, aliases, rubyBlockId);
                auto fastPathEnd = builder.GetInsertBlock();
                builder.CreateBr(afterSend);
                builder.SetInsertPoint(slowPath);
                auto slowPathRes =
                    IREmitterHelpers::emitMethodCallViaRubyVM(cs, build, i, blockMap, aliases, rubyBlockId);
                auto slowPathEnd = builder.GetInsertBlock();
                builder.CreateBr(afterSend);
                builder.SetInsertPoint(afterSend);
                auto phi = builder.CreatePHI(builder.getInt64Ty(), 2, llvm::Twine("finalCallPhi_") + methodNameRef);
                phi->addIncoming(fastPathRes, fastPathEnd);
                phi->addIncoming(slowPathRes, slowPathEnd);
                return phi;
            }
        }
    }

    return IREmitterHelpers::emitMethodCallViaRubyVM(cs, build, i, blockMap, aliases, rubyBlockId);
}

llvm::Value *IREmitterHelpers::emitMethodCallDirrect(CompilerState &cs, llvm::IRBuilderBase &build,
                                                     core::SymbolRef funSym, cfg::Send *i,
                                                     const BasicBlockMap &blockMap,
                                                     UnorderedMap<core::LocalVariable, Alias> &aliases,
                                                     int rubyBlockId) {
    auto &builder = builderCast(build);
    auto llvmFunc = IREmitterHelpers::lookupFunction(cs, funSym);
    ENFORCE(llvmFunc != nullptr);
    // TODO: insert type guard
    {
        // fill in args
        int argId = -1;
        for (auto &arg : i->args) {
            argId += 1;
            llvm::Value *indices[] = {llvm::ConstantInt::get(cs, llvm::APInt(32, 0, true)),
                                      llvm::ConstantInt::get(cs, llvm::APInt(64, argId, true))};
            auto var = Payload::varGet(cs, arg.variable, builder, blockMap, aliases, rubyBlockId);
            builder.CreateStore(var,
                                builder.CreateGEP(blockMap.sendArgArrayByBlock[rubyBlockId], indices, "callArgsAddr"));
        }
    }
    llvm::Value *indices[] = {llvm::ConstantInt::get(cs, llvm::APInt(64, 0, true)),
                              llvm::ConstantInt::get(cs, llvm::APInt(64, 0, true))};

    auto var = Payload::varGet(cs, i->recv.variable, builder, blockMap, aliases, rubyBlockId);
    builder.CreateCall(cs.module->getFunction("sorbet_checkStack"), {});
    llvm::Value *rawCall =
        builder.CreateCall(llvmFunc,
                           {llvm::ConstantInt::get(cs, llvm::APInt(32, i->args.size(), true)),
                            builder.CreateGEP(blockMap.sendArgArrayByBlock[rubyBlockId], indices), var},
                           "directSendResult");
    return rawCall;
}

llvm::Value *IREmitterHelpers::emitMethodCallViaRubyVM(CompilerState &cs, llvm::IRBuilderBase &build, cfg::Send *i,
                                                       const BasicBlockMap &blockMap,
                                                       UnorderedMap<core::LocalVariable, Alias> &aliases,
                                                       int rubyBlockId) {
    auto &builder = builderCast(build);
    auto str = i->fun.data(cs)->shortName(cs);
    auto rawId = Payload::idIntern(cs, builder, str);

    // fill in args
    {
        int argId = -1;
        for (auto &arg : i->args) {
            argId += 1;
            llvm::Value *indices[] = {llvm::ConstantInt::get(cs, llvm::APInt(32, 0, true)),
                                      llvm::ConstantInt::get(cs, llvm::APInt(64, argId, true))};
            auto var = Payload::varGet(cs, arg.variable, builder, blockMap, aliases, rubyBlockId);
            builder.CreateStore(var,
                                builder.CreateGEP(blockMap.sendArgArrayByBlock[rubyBlockId], indices, "callArgsAddr"));
        }
    }
    llvm::Value *indices[] = {llvm::ConstantInt::get(cs, llvm::APInt(64, 0, true)),
                              llvm::ConstantInt::get(cs, llvm::APInt(64, 0, true))};

    // TODO(perf): call
    // https://github.com/ruby/ruby/blob/3e3cc0885a9100e9d1bfdb77e136416ec803f4ca/internal.h#L2372
    // to get inline caching.
    // before this, perf will not be good
    auto var = Payload::varGet(cs, i->recv.variable, builder, blockMap, aliases, rubyBlockId);
    llvm::Value *rawCall;
    if (i->link != nullptr) {
        // this send has a block!
        rawCall = builder.CreateCall(cs.module->getFunction("sorbet_callFuncBlock"),
                                     {var, rawId, llvm::ConstantInt::get(cs, llvm::APInt(32, i->args.size(), true)),
                                      builder.CreateGEP(blockMap.sendArgArrayByBlock[rubyBlockId], indices),
                                      blockMap.rubyBlocks2Functions[i->link->rubyBlockId],
                                      blockMap.escapedClosure[rubyBlockId]},
                                     "rawSendResult");

    } else {
        rawCall = builder.CreateCall(cs.module->getFunction("sorbet_callFunc"),
                                     {var, rawId, llvm::ConstantInt::get(cs, llvm::APInt(32, i->args.size(), true)),
                                      builder.CreateGEP(blockMap.sendArgArrayByBlock[rubyBlockId], indices)},
                                     "rawSendResult");
    }
    return rawCall;
};
} // namespace sorbet::compiler
