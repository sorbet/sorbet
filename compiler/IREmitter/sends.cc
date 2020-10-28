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
#include "compiler/IREmitter/MethodCallContext.h"
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

llvm::Value *tryNameBasedIntrinsic(MethodCallContext &mcctx) {
    for (auto nameBasedIntrinsic : NameBasedIntrinsicMethod::definedIntrinsics()) {
        if (!absl::c_linear_search(nameBasedIntrinsic->applicableMethods(mcctx.cs), mcctx.send->fun)) {
            continue;
        }

        if (mcctx.blk != nullptr && nameBasedIntrinsic->blockHandled == Intrinsics::HandleBlock::Unhandled) {
            continue;
        }

        return nameBasedIntrinsic->makeCall(mcctx);
    }
    return IREmitterHelpers::emitMethodCallViaRubyVM(mcctx);
}

llvm::Value *trySymbolBasedIntrinsic(MethodCallContext &mcctx) {
    auto &cs = mcctx.cs;
    auto *send = mcctx.send;
    auto &builder = builderCast(mcctx.build);
    auto remainingType = mcctx.send->recv.type;
    auto afterSend = llvm::BasicBlock::Create(cs, "afterSend", builder.GetInsertBlock()->getParent());
    auto rememberStart = builder.GetInsertBlock();
    builder.SetInsertPoint(afterSend);
    auto methodName = send->fun.data(cs)->shortName(cs);
    llvm::StringRef methodNameRef(methodName.data(), methodName.size());
    auto phi = builder.CreatePHI(builder.getInt64Ty(), 2, llvm::Twine("symIntrinsicRawPhi_") + methodNameRef);
    builder.SetInsertPoint(rememberStart);
    if (!remainingType->isUntyped()) {
        for (auto symbolBasedIntrinsic : SymbolBasedIntrinsicMethod::definedIntrinsics()) {
            if (!absl::c_linear_search(symbolBasedIntrinsic->applicableMethods(cs), send->fun)) {
                continue;
            }

            if (mcctx.blk != nullptr && symbolBasedIntrinsic->blockHandled == Intrinsics::HandleBlock::Unhandled) {
                continue;
            }

            auto potentialClasses = symbolBasedIntrinsic->applicableClasses(cs);
            for (auto &c : potentialClasses) {
                auto leftType = core::Types::dropSubtypesOf(cs, remainingType, c);

                if (leftType == remainingType) {
                    continue;
                }

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
                    auto recv = Payload::varGet(cs, send->recv.variable, builder, mcctx.irctx, mcctx.rubyBlockId);
                    typeTest = Payload::typeTest(cs, builder, recv, core::make_type<core::ClassType>(c));
                }

                builder.CreateCondBr(Payload::setExpectedBool(cs, builder, typeTest, true), fastPath, alternative);
                builder.SetInsertPoint(fastPath);
                auto fastPathRes = symbolBasedIntrinsic->makeCall(mcctx);
                auto fastPathEnd = builder.GetInsertBlock();
                builder.CreateBr(afterSend);
                phi->addIncoming(fastPathRes, fastPathEnd);
                builder.SetInsertPoint(alternative);
            }
        }
    }
    auto slowPathRes = tryNameBasedIntrinsic(mcctx);
    auto slowPathEnd = builder.GetInsertBlock();
    builder.CreateBr(afterSend);
    builder.SetInsertPoint(afterSend);
    phi->addIncoming(slowPathRes, slowPathEnd);
    return phi;
}

} // namespace
llvm::Value *IREmitterHelpers::emitMethodCall(MethodCallContext &mcctx) {
    return trySymbolBasedIntrinsic(mcctx);
}

std::size_t IREmitterHelpers::sendArgCount(cfg::Send *send) {
    std::size_t numPosArgs = send->numPosArgs;
    if (numPosArgs < send->args.size()) {
        numPosArgs++;
    }
    return numPosArgs;
}

namespace {

void setSendArgsEntry(CompilerState &cs, llvm::IRBuilderBase &build, llvm::Value *sendArgs, int index,
                      llvm::Value *val) {
    auto &builder = builderCast(build);
    // the sendArgArray is always a pointer to an array of VALUEs. That is, since the
    // outermost type is a pointer, not an array, the first 0 in the instruction:
    //   getelementptr inbounds <type>, <type>* %s, i64 0, i64 <argId>
    // just means to offset 0 from the pointer's contents. Then the second index is into the
    // array (so for this type of pointer-to-array, the first index will always be 0,
    // unless you're trying to do something more powerful with GEP, like compute sizeof)
    llvm::Value *indices[] = {llvm::ConstantInt::get(cs, llvm::APInt(32, 0, true)),
                              llvm::ConstantInt::get(cs, llvm::APInt(64, index, true))};
    builder.CreateStore(val, builder.CreateGEP(sendArgs, indices, fmt::format("callArgs{}Addr", index)));
}

llvm::Value *getSendArgsPointer(CompilerState &cs, llvm::IRBuilderBase &build, llvm::Value *sendArgs) {
    auto &builder = builderCast(build);
    llvm::Value *indices[] = {llvm::ConstantInt::get(cs, llvm::APInt(64, 0, true)),
                              llvm::ConstantInt::get(cs, llvm::APInt(64, 0, true))};
    return builder.CreateGEP(sendArgs, indices);
}

} // namespace

pair<llvm::Value *, llvm::Value *> IREmitterHelpers::fillSendArgArray(MethodCallContext &mcctx,
                                                                      const std::size_t offset) {
    auto &cs = mcctx.cs;
    auto &builder = builderCast(mcctx.build);
    auto &args = mcctx.send->args;
    auto irctx = mcctx.irctx;
    auto rubyBlockId = mcctx.rubyBlockId;

    auto posEnd = mcctx.send->numPosArgs;
    auto kwEnd = mcctx.send->args.size();
    bool hasKwSplat = (kwEnd - posEnd) & 0x1;
    if (hasKwSplat) {
        kwEnd--;
    }

    ENFORCE(offset <= posEnd, "Invalid positional offset given to fillSendArgArray");

    // compute the number of actual args that will be filled in
    auto numPosArgs = posEnd - offset;
    auto numKwArgs = kwEnd - posEnd;
    auto length = numPosArgs;
    bool hasKwArgs = numKwArgs > 0 || hasKwSplat;
    if (hasKwArgs) {
        length++;
    }

    auto *argc = llvm::ConstantInt::get(cs, llvm::APInt(32, length, true));
    if (length == 0) {
        return {argc, llvm::Constant::getNullValue(llvm::Type::getInt64PtrTy(cs))};
    }

    auto *sendArgs = irctx.sendArgArrayByBlock[rubyBlockId];

    // fill in keyword args first, so that we can re-use the args vector to build the initial hash
    if (hasKwArgs) {
        llvm::Value *kwHash = nullptr;
        if (numKwArgs == 0) {
            // no inlined keyword args, lookup the hash to be splatted
            auto *splat = Payload::varGet(cs, args.back().variable, builder, irctx, rubyBlockId);
            kwHash = builder.CreateCall(cs.module->getFunction("sorbet_hashDup"), {splat});
        } else {
            // fill in inlined args (posEnd .. kwEnd)
            auto it = args.begin() + posEnd;
            for (auto argId = 0; argId < numKwArgs; ++argId, ++it) {
                auto var = Payload::varGet(cs, it->variable, builder, irctx, rubyBlockId);
                setSendArgsEntry(cs, builder, sendArgs, argId, var);
            }

            kwHash = builder.CreateCall(cs.module->getFunction("sorbet_hashBuild"),
                                        {llvm::ConstantInt::get(cs, llvm::APInt(32, numKwArgs, true)),
                                         getSendArgsPointer(cs, builder, sendArgs)},
                                        "kwargsHash");

            // merge in the splat if it's present (mcctx.send->args.back())
            if (hasKwSplat) {
                auto *splat = Payload::varGet(cs, args.back().variable, builder, irctx, rubyBlockId);
                builder.CreateCall(cs.module->getFunction("sorbet_hashUpdate"), {kwHash, splat});
            }
        }

        setSendArgsEntry(cs, builder, sendArgs, numPosArgs, kwHash);
    }

    // fill in positional args
    {
        int argId = 0;
        auto it = args.begin() + offset;
        for (; argId < numPosArgs; argId += 1, ++it) {
            auto var = Payload::varGet(cs, it->variable, builder, irctx, rubyBlockId);
            setSendArgsEntry(cs, builder, sendArgs, argId, var);
        }
    }

    return {argc, getSendArgsPointer(cs, builder, sendArgs)};
}

pair<llvm::Value *, llvm::Value *> IREmitterHelpers::fillSendArgArray(MethodCallContext &mcctx) {
    return fillSendArgArray(mcctx, 0);
}

llvm::Value *IREmitterHelpers::emitMethodCallDirrect(MethodCallContext &mcctx, core::SymbolRef funSym) {
    auto &cs = mcctx.cs;
    auto &builder = builderCast(mcctx.build);
    auto &irctx = mcctx.irctx;
    auto rubyBlockId = mcctx.rubyBlockId;
    auto *send = mcctx.send;
    auto llvmFunc = IREmitterHelpers::lookupFunction(cs, funSym);
    ENFORCE(llvmFunc != nullptr);
    // TODO: insert type guard

    auto [argc, argv] = IREmitterHelpers::fillSendArgArray(mcctx);
    auto var = Payload::varGet(cs, send->recv.variable, builder, irctx, rubyBlockId);
    builder.CreateCall(cs.module->getFunction("sorbet_checkStack"), {});
    llvm::Value *rawCall = builder.CreateCall(llvmFunc, {argc, argv, var}, "directSendResult");
    return rawCall;
}

llvm::Value *IREmitterHelpers::emitMethodCallViaRubyVM(MethodCallContext &mcctx) {
    auto &cs = mcctx.cs;
    auto &builder = builderCast(mcctx.build);
    auto *send = mcctx.send;
    auto &irctx = mcctx.irctx;
    auto rubyBlockId = mcctx.rubyBlockId;
    auto str = send->fun.data(cs)->shortName(cs);

    // fill in args
    auto [argc, argv] = IREmitterHelpers::fillSendArgArray(mcctx);

    // TODO(perf): call
    // https://github.com/ruby/ruby/blob/3e3cc0885a9100e9d1bfdb77e136416ec803f4ca/internal.h#L2372
    // to get inline caching.
    // before this, perf will not be good
    auto *self = Payload::varGet(cs, send->recv.variable, builder, irctx, rubyBlockId);
    if (send->fun == core::Names::super()) {
        return builder.CreateCall(cs.module->getFunction("sorbet_callSuper"), {argc, argv}, "rawSendResult");
    } else {
        return callViaRubyVMSimple(cs, mcctx.build, irctx, self, argv, argc, str, mcctx.blk,
                                   irctx.localsOffset[rubyBlockId]);
    }
};

llvm::Value *IREmitterHelpers::makeInlineCache(CompilerState &cs, string slowFunName) {
    auto icValidatorFunc = cs.module->getFunction("sorbet_inlineCacheInvalidated");
    auto inlineCacheType =
        (llvm::StructType *)(((llvm::PointerType *)((icValidatorFunc->arg_begin() + 1)->getType()))->getElementType());
    ENFORCE(inlineCacheType != nullptr);

    auto methodEntryType = (llvm::StructType *)(inlineCacheType->elements()[0]);
    auto intTy = llvm::Type::getInt64Ty(cs);
    auto nullv = llvm::ConstantPointerNull::get(methodEntryType->getPointerTo());

    auto *cache =
        new llvm::GlobalVariable(*cs.module, inlineCacheType, false, llvm::GlobalVariable::InternalLinkage,
                                 llvm::ConstantStruct::get(inlineCacheType, nullv, llvm::ConstantInt::get(intTy, 0),
                                                           llvm::ConstantInt::get(intTy, 0)),
                                 llvm::Twine("ic_") + slowFunName);

    return cache;
}

llvm::Value *IREmitterHelpers::callViaRubyVMSimple(CompilerState &cs, llvm::IRBuilderBase &build,
                                                   const IREmitterContext &irctx, llvm::Value *self, llvm::Value *argv,
                                                   llvm::Value *argc, string_view name, llvm::Function *blkFun,
                                                   llvm::Value *localsOffset) {
    auto &builder = builderCast(build);

    auto rawId = Payload::idIntern(cs, builder, name);
    if (blkFun != nullptr) {
        // blocks require a locals offset parameter
        ENFORCE(localsOffset != nullptr);

        auto slowFunctionName = "callFuncWithBlock_" + (string)name;
        auto *cache = makeInlineCache(cs, slowFunctionName);
        return builder.CreateCall(cs.module->getFunction("sorbet_callFuncBlockWithCache"),
                                  {self, rawId, argc, argv, blkFun, localsOffset, cache}, slowFunctionName);
    } else {
        auto slowFunctionName = "callFunc_" + (string)name;
        auto *cache = makeInlineCache(cs, slowFunctionName);
        return builder.CreateCall(cs.module->getFunction("sorbet_callFuncWithCache"), {self, rawId, argc, argv, cache},
                                  slowFunctionName);
    }
}

} // namespace sorbet::compiler
