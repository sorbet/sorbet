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
llvm::Function *getFinalForwarder(MethodCallContext &mcctx, IREmitterHelpers::FinalMethodInfo &finalInfo) {
    if (auto *func = IREmitterHelpers::lookupFunction(mcctx.cs, finalInfo.method)) {
        return func;
    }

    auto *send = mcctx.send;

    auto *func = IREmitterHelpers::getOrCreateFunctionWeak(mcctx.cs, finalInfo.method);
    llvm::IRBuilder builder{mcctx.cs};

    auto *argc = func->arg_begin();
    auto *argv = func->arg_begin() + 1;
    auto *self = func->arg_begin() + 2;

    auto *entry = llvm::BasicBlock::Create(mcctx.cs, "entry", func);
    auto *forward = llvm::BasicBlock::Create(mcctx.cs, "forward", func);
    auto funcCs = mcctx.cs.withFunctionEntry(entry);

    // setup the forwarding call
    {
        builder.SetInsertPoint(forward);

        // TODO(trevor) we could probably handle methods wih block args as well, following callWithSplatAndBlock

        // build a splat array
        auto *splat =
            builder.CreateCall(funcCs.module->getFunction("sorbet_arrayNewFromValues"), {argc, argv}, "splat");

        // setup the ruby stack
        auto *cfp = builder.CreateCall(funcCs.getFunction("sorbet_getCFP"), {}, "cfp");
        Payload::pushRubyStackVector(funcCs, builder, cfp, self, {splat});
        auto methodName = string(send->fun.shortName(funcCs));
        CallCacheFlags flags;
        flags.args_splat = true;
        auto *cache = IREmitterHelpers::makeInlineCache(funcCs, builder, methodName, flags, 1, {});
        auto *bh = Payload::vmBlockHandlerNone(funcCs, builder);
        auto *res = Payload::callFuncWithCache(funcCs, builder, cache, bh);
        builder.CreateRet(res);
    }

    // finalize the entry block
    builder.SetInsertPoint(entry);
    builder.CreateBr(forward);

    ENFORCE(!llvm::verifyFunction(*func, &llvm::dbgs()));

    return func;
}

llvm::Value *tryFinalMethodCall(MethodCallContext &mcctx) {
    // TODO(trevor) we could probably handle methods wih block args as well, by passing the block handler through the
    // current ruby execution context.
    if (mcctx.blk.has_value()) {
        return IREmitterHelpers::emitMethodCallViaRubyVM(mcctx);
    }

    // NOTE: if we don't see a final call to another compiled method, we skip this optimization as it would just degrade
    // the performance of a normal send.
    auto &cs = mcctx.cs;
    auto recvType = mcctx.send->recv.type;
    auto finalInfo = IREmitterHelpers::isFinalMethod(cs, recvType, mcctx.send->fun);
    if (!finalInfo.has_value()) {
        return IREmitterHelpers::emitMethodCallViaRubyVM(mcctx);
    }

    // We eventually want to eliminate this check.
    if (finalInfo->file != mcctx.irctx.cfg.file) {
        return IREmitterHelpers::emitMethodCallViaRubyVM(mcctx);
    }

    auto *forwarder = getFinalForwarder(mcctx, *finalInfo);
    if (forwarder == nullptr) {
        return IREmitterHelpers::emitMethodCallViaRubyVM(mcctx);
    }

    auto &builder = mcctx.builder;
    auto *send = mcctx.send;
    auto *recv = mcctx.varGetRecv();

    auto methodName = string(send->fun.shortName(cs));
    auto *fastFinalCall =
        llvm::BasicBlock::Create(cs, llvm::Twine("fastFinalCall_") + methodName, builder.GetInsertBlock()->getParent());
    auto *slowFinalCall =
        llvm::BasicBlock::Create(cs, llvm::Twine("slowFinalCall_") + methodName, builder.GetInsertBlock()->getParent());
    auto *afterFinalCall = llvm::BasicBlock::Create(cs, llvm::Twine("afterFinalCall_") + methodName,
                                                    builder.GetInsertBlock()->getParent());

    auto *typeTest = Payload::typeTest(cs, builder, recv, core::make_type<core::ClassType>(finalInfo->recv));
    builder.CreateCondBr(Payload::setExpectedBool(cs, builder, typeTest, true), fastFinalCall, slowFinalCall);

    // fast path: emit a direct call
    builder.SetInsertPoint(fastFinalCall);

    // we need a method entry to be able to perform a direct call, so we ensure that an empty inline cache is available,
    // and populate it on the first call to the fast path
    CallCacheFlags flags;
    if (send->isPrivateOk) {
        flags.fcall = true;
    } else {
        flags.args_simple = true;
    }
    auto *cache = IREmitterHelpers::makeInlineCache(cs, builder, methodName, flags, 0, {});

    // this is unfortunate: fillSendArgsArray will allocate a hash when keyword arguments are present.
    auto args = IREmitterHelpers::fillSendArgArray(mcctx);
    auto *stackFrameVar = Payload::rubyStackFrameVar(cs, builder, mcctx.irctx, finalInfo->method);
    auto *stackFrame = builder.CreateLoad(stackFrameVar);
    auto *fastPathResult =
        Payload::callFuncDirect(cs, builder, cache, forwarder, args.argc, args.argv, recv, stackFrame);
    auto *fastPathEnd = builder.GetInsertBlock();
    builder.CreateBr(afterFinalCall);

    // slow path: emit a call via the ruby vm
    builder.SetInsertPoint(slowFinalCall);
    auto *slowPathResult = IREmitterHelpers::emitMethodCallViaRubyVM(mcctx);
    auto *slowPathEnd = builder.GetInsertBlock();
    builder.CreateBr(afterFinalCall);

    // merge the two paths
    builder.SetInsertPoint(afterFinalCall);
    auto *phi = builder.CreatePHI(builder.getInt64Ty(), 2, llvm::Twine("finalCallResult_") + methodName);
    phi->addIncoming(fastPathResult, fastPathEnd);
    phi->addIncoming(slowPathResult, slowPathEnd);

    return phi;
}

llvm::Value *tryNameBasedIntrinsic(MethodCallContext &mcctx) {
    for (auto nameBasedIntrinsic : NameBasedIntrinsicMethod::definedIntrinsics()) {
        if (!absl::c_linear_search(nameBasedIntrinsic->applicableMethods(mcctx.cs), mcctx.send->fun)) {
            continue;
        }

        if (mcctx.blk.has_value() && nameBasedIntrinsic->blockHandled == Intrinsics::HandleBlock::Unhandled) {
            continue;
        }

        return nameBasedIntrinsic->makeCall(mcctx);
    }
    return tryFinalMethodCall(mcctx);
}

// We want at least inline storage for one intrinsic so we don't allocate during
// the search process.  Two intrinsics are reasonably common (e.g. .length) and
// three could come up quite a bit in numeric code.
const size_t NUM_INTRINSICS = 3;

struct ApplicableIntrinsic {
    ApplicableIntrinsic(const SymbolBasedIntrinsicMethod *method, core::ClassOrModuleRef klass)
        : method(method), klass(klass) {}
    const SymbolBasedIntrinsicMethod *method;
    core::ClassOrModuleRef klass;
};

InlinedVector<ApplicableIntrinsic, NUM_INTRINSICS> applicableIntrinsics(MethodCallContext &mcctx) {
    InlinedVector<ApplicableIntrinsic, NUM_INTRINSICS> intrinsics;
    auto remainingType = mcctx.send->recv.type;
    for (auto symbolBasedIntrinsic : SymbolBasedIntrinsicMethod::definedIntrinsics(mcctx.cs)) {
        if (!absl::c_linear_search(symbolBasedIntrinsic->applicableMethods(mcctx.cs), mcctx.send->fun)) {
            continue;
        }

        if (mcctx.blk.has_value() && symbolBasedIntrinsic->blockHandled == Intrinsics::HandleBlock::Unhandled) {
            continue;
        }

        auto potentialClasses = symbolBasedIntrinsic->applicableClasses(mcctx.cs);
        for (auto &c : potentialClasses) {
            auto leftType = core::Types::dropSubtypesOf(mcctx.cs, remainingType, c);

            if (leftType == remainingType) {
                continue;
            }

            remainingType = leftType;
            intrinsics.emplace_back(symbolBasedIntrinsic, c);
        }
    }

    return intrinsics;
}

llvm::Value *trySymbolBasedIntrinsic(MethodCallContext &mcctx) {
    auto &cs = mcctx.cs;
    auto *send = mcctx.send;
    auto &builder = mcctx.builder;
    auto remainingType = mcctx.send->recv.type;
    auto afterSend = llvm::BasicBlock::Create(cs, "afterSend", builder.GetInsertBlock()->getParent());
    auto rememberStart = builder.GetInsertBlock();
    builder.SetInsertPoint(afterSend);
    auto methodName = send->fun.shortName(cs);
    llvm::StringRef methodNameRef(methodName.data(), methodName.size());
    auto phi = builder.CreatePHI(builder.getInt64Ty(), 2, llvm::Twine("symIntrinsicRawPhi_") + methodNameRef);
    builder.SetInsertPoint(rememberStart);
    if (!remainingType.isUntyped()) {
        auto intrinsics = applicableIntrinsics(mcctx);

        if (intrinsics.size() == 1 && intrinsics[0].method->skipFastPathTest(mcctx, intrinsics[0].klass)) {
            auto fastPathRes = intrinsics[0].method->makeCall(mcctx);
            Payload::afterIntrinsic(cs, builder);
            auto fastPathEnd = builder.GetInsertBlock();
            builder.CreateBr(afterSend);
            builder.SetInsertPoint(afterSend);
            phi->addIncoming(fastPathRes, fastPathEnd);
            return phi;
        }

        for (auto &intrinsic : intrinsics) {
            auto clazName = intrinsic.klass.data(cs)->name.shortName(cs);
            llvm::StringRef clazNameRef(clazName.data(), clazName.size());

            auto alternative = llvm::BasicBlock::Create(
                cs, llvm::Twine("alternativeCallIntrinsic_") + clazNameRef + "_" + methodNameRef,
                builder.GetInsertBlock()->getParent());
            auto fastPath =
                llvm::BasicBlock::Create(cs, llvm::Twine("fastSymCallIntrinsic_") + clazNameRef + "_" + methodNameRef,
                                         builder.GetInsertBlock()->getParent());

            auto *typeTest = intrinsic.method->receiverFastPathTest(mcctx, intrinsic.klass);
            builder.CreateCondBr(Payload::setExpectedBool(cs, builder, typeTest, true), fastPath, alternative);
            builder.SetInsertPoint(fastPath);
            auto fastPathRes = intrinsic.method->makeCall(mcctx);
            Payload::afterIntrinsic(cs, builder);
            auto fastPathEnd = builder.GetInsertBlock();
            builder.CreateBr(afterSend);
            phi->addIncoming(fastPathRes, fastPathEnd);
            builder.SetInsertPoint(alternative);
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

void setSendArgsEntry(CompilerState &cs, llvm::IRBuilderBase &builder, llvm::Value *sendArgs, int index,
                      llvm::Value *val) {
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

llvm::Value *getSendArgsPointer(CompilerState &cs, llvm::IRBuilderBase &builder, llvm::Value *sendArgs) {
    llvm::Value *indices[] = {llvm::ConstantInt::get(cs, llvm::APInt(64, 0, true)),
                              llvm::ConstantInt::get(cs, llvm::APInt(64, 0, true))};
    return builder.CreateGEP(sendArgs, indices);
}

} // namespace

IREmitterHelpers::SendArgInfo::SendArgInfo(llvm::Value *argc, llvm::Value *argv, llvm::Value *kw_splat,
                                           vector<llvm::Value *> argValues)
    : argc{argc}, argv{argv}, kw_splat{kw_splat}, argValues(std::move(argValues)) {}

IREmitterHelpers::SendArgInfo IREmitterHelpers::fillSendArgArray(MethodCallContext &mcctx, const std::size_t offset) {
    auto &cs = mcctx.cs;
    auto &builder = mcctx.builder;
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

    // keep track of the intermediate argument values that we've seen
    vector<llvm::Value *> argValues;

    // compute the number of actual args that will be filled in
    auto numPosArgs = posEnd - offset;
    auto numKwArgs = kwEnd - posEnd;
    auto length = numPosArgs;
    bool hasKwArgs = numKwArgs > 0 || hasKwSplat;
    llvm::Value *kw_splat;
    if (hasKwArgs) {
        length++;
        kw_splat = llvm::ConstantInt::get(cs, llvm::APInt(32, 1, true));
    } else {
        kw_splat = llvm::ConstantInt::get(cs, llvm::APInt(32, 0, true));
    }

    auto *argc = llvm::ConstantInt::get(cs, llvm::APInt(32, length, true));
    if (length == 0) {
        return SendArgInfo{argc, llvm::Constant::getNullValue(llvm::Type::getInt64PtrTy(cs)), kw_splat,
                           std::move(argValues)};
    }

    auto *sendArgs = irctx.sendArgArrayByBlock[rubyBlockId];

    // fill in keyword args first, so that we can re-use the args vector to build the initial hash
    if (hasKwArgs) {
        llvm::Value *kwHash = nullptr;
        if (numKwArgs == 0) {
            // no inlined keyword args, lookup the hash to be splatted
            kwHash = Payload::varGet(cs, args.back().variable, builder, irctx, rubyBlockId);
        } else {
            // fill in inlined args (posEnd .. kwEnd)
            auto it = args.begin() + posEnd;
            for (auto argId = 0; argId < numKwArgs; ++argId, ++it) {
                auto var = Payload::varGet(cs, it->variable, builder, irctx, rubyBlockId);
                argValues.emplace_back(var);
                setSendArgsEntry(cs, builder, sendArgs, argId, var);
            }

            kwHash = builder.CreateCall(cs.getFunction("sorbet_hashBuild"),
                                        {llvm::ConstantInt::get(cs, llvm::APInt(32, numKwArgs, true)),
                                         getSendArgsPointer(cs, builder, sendArgs)},
                                        "kwargsHash");

            // merge in the splat if it's present (mcctx.send->args.back())
            if (hasKwSplat) {
                auto *splat = Payload::varGet(cs, args.back().variable, builder, irctx, rubyBlockId);
                argValues.emplace_back(splat);
                builder.CreateCall(cs.getFunction("sorbet_hashUpdate"), {kwHash, splat});
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
            argValues.emplace_back(var);
            setSendArgsEntry(cs, builder, sendArgs, argId, var);
        }
    }

    return SendArgInfo{argc, getSendArgsPointer(cs, builder, sendArgs), kw_splat, std::move(argValues)};
}

IREmitterHelpers::SendArgInfo IREmitterHelpers::fillSendArgArray(MethodCallContext &mcctx) {
    return fillSendArgArray(mcctx, 0);
}

IREmitterHelpers::RubyStackArgs::RubyStackArgs(std::vector<llvm::Value *> stack, std::vector<std::string_view> keywords,
                                               CallCacheFlags flags)
    : stack{std::move(stack)}, keywords{std::move(keywords)}, flags{flags} {}

IREmitterHelpers::RubyStackArgs IREmitterHelpers::buildSendArgs(MethodCallContext &mcctx, cfg::LocalRef recv,
                                                                const std::size_t offset) {
    auto &cs = mcctx.cs;
    auto &irctx = mcctx.irctx;
    auto &builder = mcctx.builder;
    auto &send = mcctx.send;
    auto rubyBlockId = mcctx.rubyBlockId;

    CallCacheFlags flags;

    auto name = string(send->fun.shortName(cs));
    vector<string_view> keywords{};
    vector<llvm::Value *> stack{};

    // push positional args onto the ruby stack
    auto posEnd = send->numPosArgs;
    auto argIdx = offset;
    for (; argIdx < posEnd; ++argIdx) {
        stack.emplace_back(Payload::varGet(cs, send->args[argIdx].variable, builder, irctx, rubyBlockId));
    }

    // push keyword argument values, and populate the keywords vector
    auto kwEnd = send->args.size();
    if (posEnd < kwEnd) {
        // if there is a keyword splat, merge everything into a hash that's used for the last argument, otherwise
        // keywords go in the inline cache, and values go on the stack.
        int numKwArgs = kwEnd - posEnd;
        bool hasKwSplat = numKwArgs & 0x1;
        if (hasKwSplat) {
            // The current desugaring behavior of keyword splats is that it will merge all keyword arguments into the
            // splat, even inlined ones. As a result, we'll either see keyword args inlined, or one hash argument that
            // we'll pass in. This enforce is here so that we remember to update the behavior of this function when we
            // eventually fix that desugaring.
            ENFORCE(numKwArgs == 1);
            flags.kw_splat = true;

            auto var = Payload::varGet(cs, send->args[argIdx].variable, builder, irctx, rubyBlockId);
            stack.emplace_back(var);
        } else {
            flags.kwarg = true;

            while (argIdx < kwEnd) {
                auto kwArg = send->args[argIdx++].variable;
                auto it = irctx.symbols.find(kwArg);
                ENFORCE(it != irctx.symbols.end(), "Keyword arg present with non-symbol keyword");
                keywords.emplace_back(it->second);

                stack.emplace_back(Payload::varGet(cs, send->args[argIdx++].variable, builder, irctx, rubyBlockId));
            }
        }
    } else {
        flags.args_simple = true;
    }

    if (send->isPrivateOk) {
        flags.fcall = true;
    }

    return IREmitterHelpers::RubyStackArgs(std::move(stack), std::move(keywords), flags);
}

namespace {
bool canCallBlockViaRubyVM(MethodCallContext &mcctx) {
    auto &cs = mcctx.cs;
    auto *send = mcctx.send;
    auto &irctx = mcctx.irctx;
    auto rubyBlockId = mcctx.rubyBlockId;

    ENFORCE(IREmitterHelpers::hasBlockArgument(cs, rubyBlockId, irctx.cfg.symbol, irctx))

    // If the receiver is not the distinguished block argument then we have to be
    // more general in our send.
    auto recv = send->recv.variable;
    if (recv != irctx.rubyBlockArgs[rubyBlockId].back()) {
        return false;
    }

    // Can't handle invoking blocks that take blocks.
    if (mcctx.blk.has_value()) {
        return false;
    }

    return true;
}

} // namespace

llvm::Value *IREmitterHelpers::emitMethodCallViaRubyVM(MethodCallContext &mcctx) {
    auto &cs = mcctx.cs;
    auto &builder = mcctx.builder;
    auto *send = mcctx.send;
    auto &irctx = mcctx.irctx;
    auto rubyBlockId = mcctx.rubyBlockId;

    // If we get here with <Magic>, then something has gone wrong.
    // TODO(froydnj): We want to do the same thing with Sorbet::Private::Static,
    // but we'd need to do some surgery on either a) making those methods not
    // be emitted via symbol-based intrinsics or b) making it possible for
    // (some) symbol-based intrinsics to bypass typechecks completely.
    if (auto *at = core::cast_type<core::AppliedType>(send->recv.type)) {
        if (at->klass == core::Symbols::MagicSingleton()) {
            failCompilation(cs, core::Loc(irctx.cfg.file, send->receiverLoc),
                            "No runtime implemention for <Magic> method exists");
        }
        if (at->klass == core::Symbols::Sorbet_Private_StaticSingleton()) {
            failCompilation(cs, core::Loc(irctx.cfg.file, send->receiverLoc),
                            "No runtime implementation for Sorbet::Private::Static method exists");
        }
    }

    // TODO(perf): call
    // https://github.com/ruby/ruby/blob/3e3cc0885a9100e9d1bfdb77e136416ec803f4ca/internal.h#L2372
    // to get inline caching.
    // before this, perf will not be good
    if (send->fun == core::Names::super()) {
        // fill in args
        auto args = IREmitterHelpers::fillSendArgArray(mcctx);

        if (auto *blk = mcctx.blkAsFunction()) {
            // blocks require a locals offset parameter
            llvm::Value *localsOffset = Payload::buildLocalsOffset(cs);
            ENFORCE(localsOffset != nullptr);
            return builder.CreateCall(cs.getFunction("sorbet_callSuperBlock"),
                                      {args.argc, args.argv, args.kw_splat, blk, localsOffset}, "rawSendResult");
        }

        return builder.CreateCall(cs.getFunction("sorbet_callSuper"), {args.argc, args.argv, args.kw_splat},
                                  "rawSendResult");
    }

    // Try to call blocks directly without reifying the block into a proc.
    if (send->fun == core::Names::call() &&
        IREmitterHelpers::hasBlockArgument(cs, rubyBlockId, irctx.cfg.symbol, irctx)) {
        if (canCallBlockViaRubyVM(mcctx)) {
            // fill in args
            auto args = IREmitterHelpers::fillSendArgArray(mcctx);
            auto *cfp = Payload::getCFPForBlock(cs, builder, irctx, rubyBlockId);

            return builder.CreateCall(cs.getFunction("sorbet_vm_callBlock"), {cfp, args.argc, args.argv, args.kw_splat},
                                      "rawBlockSendResult");
        }
    }

    return callViaRubyVMSimple(mcctx);
}

// Create a global to hold the FunctionInlineCache value, and setup its initialization in the `Init_` function.
llvm::Value *IREmitterHelpers::makeInlineCache(CompilerState &cs, llvm::IRBuilderBase &builder, string methodName,
                                               CallCacheFlags flags, int argc, const vector<string_view> &keywords) {
    auto *setupFn = cs.getFunction("sorbet_setupFunctionInlineCache");
    auto *cacheTy = static_cast<llvm::PointerType *>(setupFn->arg_begin()->getType())->getElementType();
    auto *zero = llvm::ConstantAggregateZero::get(cacheTy);

    auto *cache = new llvm::GlobalVariable(*cs.module, cacheTy, false, llvm::GlobalVariable::InternalLinkage, zero,
                                           llvm::Twine("ic_") + methodName);

    // initialize the global during GlobalConstructorsInit
    {
        auto restore = builder.saveIP();

        builder.SetInsertPoint(cs.globalConstructorsEntry);

        auto *midVal = Payload::idIntern(cs, builder, methodName);

        auto *argcVal = llvm::ConstantInt::get(cs, llvm::APInt(32, argc, true));

        auto *keywordsLenVal = llvm::ConstantInt::get(cs, llvm::APInt(32, keywords.size(), true));
        llvm::Value *keywordsVal = nullptr;

        if (keywords.empty()) {
            keywordsVal = llvm::ConstantPointerNull::get(llvm::Type::getInt64Ty(cs)->getPointerTo());
        } else {
            // NOTE: we may want to create one array that has the max number of slots available and re-use it for each
            // initialization
            auto *numKeywords = llvm::ConstantInt::get(cs, llvm::APInt(32, keywords.size()));
            keywordsVal = builder.CreateAlloca(llvm::Type::getInt64Ty(cs), numKeywords, "keywords");

            int index = 0;
            for (auto kw : keywords) {
                auto *kwVal = Payload::idIntern(cs, builder, kw);
                auto *symVal = builder.CreateCall(cs.getFunction("sorbet_IDToSym"), {kwVal}, "symbol");

                auto *offset = llvm::ConstantInt::get(cs, llvm::APInt(32, index++, false));
                llvm::Value *indices[] = {offset};
                builder.CreateStore(symVal, builder.CreateGEP(keywordsVal, indices));
            }
        }

        auto *flagVal = flags.build(cs, builder);
        builder.CreateCall(setupFn, {cache, midVal, flagVal, argcVal, keywordsLenVal, keywordsVal});

        builder.restoreIP(restore);
    }

    return cache;
}

llvm::Value *IREmitterHelpers::callViaRubyVMSimple(MethodCallContext &mcctx) {
    auto &cs = mcctx.cs;
    auto &builder = mcctx.builder;
    auto &irctx = mcctx.irctx;
    auto rubyBlockId = mcctx.rubyBlockId;
    auto *cfp = Payload::getCFPForBlock(cs, builder, irctx, rubyBlockId);

    auto &stack = mcctx.getStackArgs();
    auto *cache = mcctx.getInlineCache();
    auto *recv = mcctx.varGetRecv();

    auto *closure = Payload::buildLocalsOffset(mcctx.cs);
    vector<llvm::Value *> args;
    args.emplace_back(cache);

    if (auto *blk = mcctx.blkAsFunction()) {
        auto blkId = mcctx.blk.value();
        args.emplace_back(llvm::ConstantInt::get(cs, llvm::APInt(1, static_cast<bool>(irctx.blockUsesBreak[blkId]))));
        args.emplace_back(blk);

        auto &arity = irctx.rubyBlockArity[mcctx.blk.value()];
        args.emplace_back(IREmitterHelpers::buildS4(cs, arity.min));
        args.emplace_back(IREmitterHelpers::buildS4(cs, arity.max));
    } else {
        args.emplace_back(llvm::ConstantInt::get(cs, llvm::APInt(1, static_cast<bool>(false))));
        args.emplace_back(llvm::ConstantPointerNull::get(llvm::PointerType::getUnqual(cs.getRubyBlockFFIType())));
        args.emplace_back(IREmitterHelpers::buildS4(cs, 0));
        args.emplace_back(IREmitterHelpers::buildS4(cs, 0));
    }
    args.emplace_back(closure);
    args.emplace_back(cfp);
    args.emplace_back(recv);
    for (auto *arg : stack) {
        args.emplace_back(arg);
    }

    // TODO(neil), RUBYLANG-338: add methodName as a phantom arg to sorbet_i_send
    return builder.CreateCall(cs.getFunction("sorbet_i_send"), llvm::ArrayRef(args));
}

} // namespace sorbet::compiler
