// These violate our poisons so have to happen first
#include "llvm/IR/Attributes.h"
#include "llvm/IR/DerivedTypes.h" // FunctionType, StructType
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Verifier.h"

#include "absl/base/casts.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "cfg/CFG.h"
#include "common/FileOps.h"
#include "common/sort.h"
#include "common/typecase.h"
#include "compiler/Core/CompilerState.h"
#include "compiler/Errors/Errors.h"
#include "compiler/IREmitter/IREmitter.h"
#include "compiler/IREmitter/IREmitterContext.h"
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

llvm::Value *tryFinalCall(CompilerState &cs, llvm::IRBuilderBase &build, cfg::Send *send, const IREmitterContext &irctx,
                          int rubyBlockId, llvm::Function *blk) {
    if (blk == nullptr) {
        auto &recvType = send->recv.type;
        core::SymbolRef recvClass = core::Symbols::noSymbol();
        if (auto ct = core::cast_type<core::ClassType>(recvType.get())) {
            recvClass = ct->symbol;
        } else if (auto at = core::cast_type<core::AppliedType>(recvType.get())) {
            recvClass = at->klass;
        }

        if (recvClass.exists()) {
            auto funSym = recvClass.data(cs)->findMember(cs, send->fun);
            if (funSym.exists() && funSym.data(cs)->isFinalMethod()) {
                auto llvmFunc = IREmitterHelpers::lookupFunction(cs, funSym);
                if (llvmFunc == nullptr) {
                    // here we create a dymmy _weak_ forwarder for the function.
                    // if the target function is compiled in the same llvm module, it will take
                    // priority as we wipe method bodies first.
                    // If it's compiled in a different module, runtime linker will chose the
                    // non-weak symbol.
                    // If it will never be compiled, the forwarder will stay
                    llvmFunc = IREmitterHelpers::getOrCreateFunctionWeak(cs, funSym);
                    llvm::IRBuilder funcBuilder(cs);
                    auto bb1 = llvm::BasicBlock::Create(cs, "fwd2interpreted", llvmFunc);
                    auto bb2 = llvm::BasicBlock::Create(cs, "fwd", llvmFunc);
                    funcBuilder.SetInsertPoint(bb2);

                    auto selfVar = llvmFunc->arg_begin() + 2;
                    auto argsCount = llvmFunc->arg_begin();
                    auto argsArray = llvmFunc->arg_begin() + 1;
                    auto cs2 = cs;
                    cs2.functionEntryInitializers = bb1;
                    auto rt = IREmitterHelpers::callViaRubyVMSimple(cs2, funcBuilder, selfVar, argsArray, argsCount,
                                                                    send->fun.data(cs)->shortName(cs));
                    funcBuilder.CreateRet(rt);
                    funcBuilder.SetInsertPoint(bb1);
                    funcBuilder.CreateBr(bb2);
                    ENFORCE(!llvm::verifyFunction(*llvmFunc, &llvm::dbgs()));
                }
                auto methodName = send->fun.data(cs)->shortName(cs);
                llvm::StringRef methodNameRef(methodName.data(), methodName.size());
                auto &builder = builderCast(build);
                auto recv = Payload::varGet(cs, send->recv.variable, builder, irctx, rubyBlockId);

                auto typeTest = Payload::typeTest(cs, builder, recv, core::make_type<core::ClassType>(recvClass));

                auto afterSend = llvm::BasicBlock::Create(cs, llvm::Twine("afterCallFinal_") + methodNameRef,
                                                          builder.GetInsertBlock()->getParent());
                auto slowPath = llvm::BasicBlock::Create(cs, llvm::Twine("slowCallFinal_") + methodNameRef,
                                                         builder.GetInsertBlock()->getParent());
                auto fastPath = llvm::BasicBlock::Create(cs, llvm::Twine("fastCallFinal_") + methodNameRef,
                                                         builder.GetInsertBlock()->getParent());
                builder.CreateCondBr(Payload::setExpectedBool(cs, builder, typeTest, true), fastPath, slowPath);
                builder.SetInsertPoint(fastPath);
                auto fastPathRes = IREmitterHelpers::emitMethodCallDirrect(cs, build, funSym, send, irctx, rubyBlockId);
                auto fastPathEnd = builder.GetInsertBlock();
                builder.CreateBr(afterSend);
                builder.SetInsertPoint(slowPath);
                auto slowPathRes = IREmitterHelpers::emitMethodCallViaRubyVM(cs, build, send, irctx, rubyBlockId, blk);
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

    return IREmitterHelpers::emitMethodCallViaRubyVM(cs, build, send, irctx, rubyBlockId, blk);
}

llvm::Value *tryNameBasedIntrinsic(CompilerState &cs, llvm::IRBuilderBase &build, cfg::Send *send,
                                   const IREmitterContext &irctx, int rubyBlockId, llvm::Function *blk) {
    for (auto nameBasedIntrinsic : NameBasedIntrinsicMethod::definedIntrinsics()) {
        if (absl::c_linear_search(nameBasedIntrinsic->applicableMethods(cs), send->fun) &&
            ((blk == nullptr) || nameBasedIntrinsic->blockHandled == Intrinsics::HandleBlock::Handled)) {
            return nameBasedIntrinsic->makeCall(cs, send, build, irctx, rubyBlockId, blk);
        }
    }
    return tryFinalCall(cs, build, send, irctx, rubyBlockId, blk);
}

llvm::Value *trySymbolBasedIntrinsic(CompilerState &cs, llvm::IRBuilderBase &build, cfg::Send *send,
                                     const IREmitterContext &irctx, int rubyBlockId, llvm::Function *blk) {
    auto &builder = builderCast(build);
    auto remainingType = send->recv.type;
    auto afterSend = llvm::BasicBlock::Create(cs, "afterSend", builder.GetInsertBlock()->getParent());
    auto rememberStart = builder.GetInsertBlock();
    builder.SetInsertPoint(afterSend);
    auto methodName = send->fun.data(cs)->shortName(cs);
    llvm::StringRef methodNameRef(methodName.data(), methodName.size());
    auto phi = builder.CreatePHI(builder.getInt64Ty(), 2, llvm::Twine("symIntrinsicRawPhi_") + methodNameRef);
    builder.SetInsertPoint(rememberStart);
    if (!remainingType->isUntyped()) {
        for (auto symbolBasedIntrinsic : SymbolBasedIntrinsicMethod::definedIntrinsics()) {
            if (absl::c_linear_search(symbolBasedIntrinsic->applicableMethods(cs), send->fun) &&
                (blk == nullptr || symbolBasedIntrinsic->blockHandled == Intrinsics::HandleBlock::Handled)) {
                auto potentialClasses = symbolBasedIntrinsic->applicableClasses(cs);
                for (auto &c : potentialClasses) {
                    auto leftType = core::Types::dropSubtypesOf(cs, remainingType, c);

                    if (leftType != remainingType) {
                        remainingType = leftType;

                        auto clazName = c.data(cs)->name.data(cs)->shortName(cs);
                        llvm::StringRef clazNameRef(clazName.data(), clazName.size());

                        auto alternative = llvm::BasicBlock::Create(
                            cs, llvm::Twine("alternativeCallIntrinsic_") + clazNameRef + "_" + methodNameRef,
                            builder.GetInsertBlock()->getParent());
                        auto fastPath = llvm::BasicBlock::Create(
                            cs, llvm::Twine("fastSymCallIntrinsic_") + clazNameRef + "_" + methodNameRef,
                            builder.GetInsertBlock()->getParent());

                        llvm::Value *typeTest;
                        if (symbolBasedIntrinsic->skipReceiverTypeTest()) {
                            typeTest = builder.getInt1(true);
                        } else {
                            auto recv = Payload::varGet(cs, send->recv.variable, builder, irctx, rubyBlockId);
                            typeTest = Payload::typeTest(cs, builder, recv, core::make_type<core::ClassType>(c));
                        }

                        builder.CreateCondBr(Payload::setExpectedBool(cs, builder, typeTest, true), fastPath,
                                             alternative);
                        builder.SetInsertPoint(fastPath);
                        auto fastPathRes = symbolBasedIntrinsic->makeCall(cs, send, build, irctx, rubyBlockId, blk);
                        auto fastPathEnd = builder.GetInsertBlock();
                        builder.CreateBr(afterSend);
                        phi->addIncoming(fastPathRes, fastPathEnd);
                        builder.SetInsertPoint(alternative);
                    }
                }
            }
        }
    }
    auto slowPathRes = tryNameBasedIntrinsic(cs, build, send, irctx, rubyBlockId, blk);
    auto slowPathEnd = builder.GetInsertBlock();
    builder.CreateBr(afterSend);
    builder.SetInsertPoint(afterSend);
    phi->addIncoming(slowPathRes, slowPathEnd);
    return phi;
}

} // namespace
llvm::Value *IREmitterHelpers::emitMethodCall(CompilerState &cs, llvm::IRBuilderBase &build, cfg::Send *send,
                                              const IREmitterContext &irctx, int rubyBlockId) {
    llvm::Function *blk = nullptr;
    if (send->link != nullptr) {
        blk = irctx.rubyBlocks2Functions[send->link->rubyBlockId];
    }

    return trySymbolBasedIntrinsic(cs, build, send, irctx, rubyBlockId, blk);
}

llvm::Value *IREmitterHelpers::fillSendArgArray(CompilerState &cs, llvm::IRBuilderBase &build,
                                                const IREmitterContext &irctx, int rubyBlockId,
                                                const InlinedVector<cfg::VariableUseSite, 2> &args,
                                                const std::size_t offset, const std::size_t length) {
    ENFORCE(offset + length <= args.size(), "Invalid range given to fillSendArgArray");

    auto &builder = builderCast(build);
    if (length == 0) {
        return llvm::Constant::getNullValue(llvm::Type::getInt64PtrTy(cs));
    }

    // fill in args
    {
        int argId = 0;
        auto it = args.begin() + offset;
        for (; argId < length; argId += 1, ++it) {
            llvm::Value *indices[] = {llvm::ConstantInt::get(cs, llvm::APInt(32, 0, true)),
                                      llvm::ConstantInt::get(cs, llvm::APInt(64, argId, true))};
            auto var = Payload::varGet(cs, it->variable, builder, irctx, rubyBlockId);
            builder.CreateStore(var,
                                builder.CreateGEP(irctx.sendArgArrayByBlock[rubyBlockId], indices, "callArgsAddr"));
        }
    }

    llvm::Value *indices[] = {llvm::ConstantInt::get(cs, llvm::APInt(64, 0, true)),
                              llvm::ConstantInt::get(cs, llvm::APInt(64, 0, true))};

    return builder.CreateGEP(irctx.sendArgArrayByBlock[rubyBlockId], indices);
}

llvm::Value *IREmitterHelpers::emitMethodCallDirrect(CompilerState &cs, llvm::IRBuilderBase &build,
                                                     core::SymbolRef funSym, cfg::Send *send,
                                                     const IREmitterContext &irctx, int rubyBlockId) {
    auto &builder = builderCast(build);
    auto llvmFunc = IREmitterHelpers::lookupFunction(cs, funSym);
    ENFORCE(llvmFunc != nullptr);
    // TODO: insert type guard

    auto args = IREmitterHelpers::fillSendArgArray(cs, builder, irctx, rubyBlockId, send->args);
    auto var = Payload::varGet(cs, send->recv.variable, builder, irctx, rubyBlockId);
    builder.CreateCall(cs.module->getFunction("sorbet_checkStack"), {});
    llvm::Value *rawCall =
        builder.CreateCall(llvmFunc, {llvm::ConstantInt::get(cs, llvm::APInt(32, send->args.size(), true)), args, var},
                           "directSendResult");
    return rawCall;
}

llvm::Value *IREmitterHelpers::emitMethodCallViaRubyVM(CompilerState &cs, llvm::IRBuilderBase &build, cfg::Send *send,
                                                       const IREmitterContext &irctx, int rubyBlockId,
                                                       llvm::Function *blk) {
    auto &builder = builderCast(build);
    auto str = send->fun.data(cs)->shortName(cs);

    // fill in args
    auto args = IREmitterHelpers::fillSendArgArray(cs, builder, irctx, rubyBlockId, send->args);

    // TODO(perf): call
    // https://github.com/ruby/ruby/blob/3e3cc0885a9100e9d1bfdb77e136416ec803f4ca/internal.h#L2372
    // to get inline caching.
    // before this, perf will not be good
    auto var = Payload::varGet(cs, send->recv.variable, builder, irctx, rubyBlockId);
    if (send->link != nullptr) {
        // this send has a block!
        auto rawId = Payload::idIntern(cs, builder, str);
        return builder.CreateCall(cs.module->getFunction("sorbet_callFuncBlock"),
                                  {var, rawId, llvm::ConstantInt::get(cs, llvm::APInt(32, send->args.size(), true)),
                                   args, blk, irctx.escapedClosure[rubyBlockId]},
                                  "rawSendResult");

    } else if (send->fun == core::Names::super()) {
        return builder.CreateCall(cs.module->getFunction("sorbet_callSuper"),
                                  {llvm::ConstantInt::get(cs, llvm::APInt(32, send->args.size(), true)), args},
                                  "rawSendResult");
    } else {
        return callViaRubyVMSimple(cs, build, var, args,

                                   llvm::ConstantInt::get(cs, llvm::APInt(32, send->args.size(), true)), str);
    }
};

llvm::Value *IREmitterHelpers::callViaRubyVMSimple(CompilerState &cs, llvm::IRBuilderBase &build, llvm::Value *self,
                                                   llvm::Value *argv, llvm::Value *argc, string_view name) {
    auto &builder = builderCast(build);
    auto str = name;
    auto icValidatorFunc = cs.module->getFunction("sorbet_inlineCacheInvalidated");
    auto inlineCacheType =
        (llvm::StructType *)(((llvm::PointerType *)((icValidatorFunc->arg_begin() + 1)->getType()))->getElementType());
    ENFORCE(inlineCacheType != nullptr);
    auto methodEntryType = (llvm::StructType *)(inlineCacheType->elements()[0]);
    auto slowFunctionName = "call_via_vm_" + (string)str;
    auto intt = llvm::Type::getInt64Ty(cs);
    auto nullv = llvm::ConstantPointerNull::get(methodEntryType->getPointerTo());
    auto cache =
        new llvm::GlobalVariable(*cs.module, inlineCacheType, false, llvm::GlobalVariable::InternalLinkage,
                                 llvm::ConstantStruct::get(inlineCacheType, nullv, llvm::ConstantInt::get(intt, 0),
                                                           llvm::ConstantInt::get(intt, 0)),
                                 llvm::Twine("ic_") + slowFunctionName);
    auto rawId = Payload::idIntern(cs, builder, str);
    return builder.CreateCall(cs.module->getFunction("sorbet_callFunc"), {self, rawId, argc, argv, cache},
                              slowFunctionName);
}

} // namespace sorbet::compiler
