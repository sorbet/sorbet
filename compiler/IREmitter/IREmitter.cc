// These violate our poisons so have to happen first
#include "llvm/IR/Attributes.h"
#include "llvm/IR/DIBuilder.h"
#include "llvm/IR/DerivedTypes.h" // FunctionType, StructType
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Transforms/Utils/Cloning.h"

#include "ast/Helpers.h"
#include "ast/ast.h"
#include "cfg/CFG.h"
#include "common/FileOps.h"
#include "common/Timer.h"
#include "common/sort.h"
#include "common/typecase.h"
#include "compiler/Core/CompilerState.h"
#include "compiler/Core/FailCompilation.h"
#include "compiler/Errors/Errors.h"
#include "compiler/IREmitter/IREmitter.h"
#include "compiler/IREmitter/IREmitterContext.h"
#include "compiler/IREmitter/IREmitterHelpers.h"
#include "compiler/IREmitter/MethodCallContext.h"
#include "compiler/IREmitter/Payload.h"
#include <string_view>

using namespace std;
namespace sorbet::compiler {

namespace {

vector<core::ArgInfo::ArgFlags> getArgFlagsForBlockId(CompilerState &cs, int blockId, core::MethodRef method,
                                                      const IREmitterContext &irctx) {
    auto ty = irctx.rubyBlockType[blockId];
    ENFORCE(ty == FunctionType::Block || ty == FunctionType::Method || ty == FunctionType::StaticInitFile ||
            ty == FunctionType::StaticInitModule);

    if (ty == FunctionType::Block) {
        auto blockLink = irctx.blockLinks[blockId];
        return blockLink->argFlags;
    }

    vector<core::ArgInfo::ArgFlags> res;
    for (auto &argInfo : method.data(cs)->arguments) {
        res.emplace_back(argInfo.flags);
    }

    return res;
}

void setupStackFrame(CompilerState &cs, const ast::MethodDef &md, const IREmitterContext &irctx,
                     llvm::IRBuilderBase &builder, int rubyRegionId) {
    switch (irctx.rubyBlockType[rubyRegionId]) {
        case FunctionType::Method:
        case FunctionType::StaticInitFile:
        case FunctionType::StaticInitModule:
        case FunctionType::Block:
        case FunctionType::Rescue:
        case FunctionType::Ensure: {
            // Switch the current control frame from a C frame to a Ruby-esque one
            auto pc = Payload::setRubyStackFrame(cs, builder, irctx, md, rubyRegionId);
            builder.CreateStore(pc, irctx.lineNumberPtrsByFunction[rubyRegionId]);
            break;
        }

        case FunctionType::ExceptionBegin: {
            // Exception functions get their pc and iseq_encoded values as arguments
            auto func = irctx.rubyBlocks2Functions[rubyRegionId];
            auto *pc = func->arg_begin();
            builder.CreateStore(pc, irctx.lineNumberPtrsByFunction[rubyRegionId]);
            break;
        }

        case FunctionType::Unused:
            break;
    }
}

} // namespace

void setupStackFrames(CompilerState &base, const ast::MethodDef &md, const IREmitterContext &irctx) {
    llvm::IRBuilder<> builder(base);
    for (auto rubyRegionId = 0; rubyRegionId < irctx.rubyBlocks2Functions.size(); rubyRegionId++) {
        auto cs = base.withFunctionEntry(irctx.functionInitializersByFunction[rubyRegionId]);

        builder.SetInsertPoint(irctx.functionInitializersByFunction[rubyRegionId]);

        switch (irctx.rubyBlockType[rubyRegionId]) {
            case FunctionType::Method:
            case FunctionType::StaticInitFile:
            case FunctionType::StaticInitModule: {
                // We could get this from the frame, as below, but it is slightly more
                // efficient to get it from the function arguments.
                auto selfArgRaw = irctx.rubyBlocks2Functions[rubyRegionId]->arg_begin() + 2;
                Payload::varSet(cs, cfg::LocalRef::selfVariable(), selfArgRaw, builder, irctx, rubyRegionId);
                break;
            }
            case FunctionType::Block:
            case FunctionType::Rescue:
            case FunctionType::Ensure:
            case FunctionType::ExceptionBegin: {
                auto selfArgRaw = builder.CreateCall(cs.getFunction("sorbet_getSelfFromFrame"), {}, "self");
                Payload::varSet(cs, cfg::LocalRef::selfVariable(), selfArgRaw, builder, irctx, rubyRegionId);
                break;
            }
            default:
                break;
        }

        if (irctx.rubyBlockType[rubyRegionId] == FunctionType::Block) {
            auto *cfp = builder.CreateCall(cs.getFunction("sorbet_getCFP"), {}, "cfp");
            builder.CreateStore(cfp, irctx.blockControlFramePtrs.at(rubyRegionId));
        }

        setupStackFrame(cs, md, irctx, builder, rubyRegionId);
        auto lastLoc = core::Loc::none();
        auto startLoc = md.symbol.data(base)->loc();
        Payload::setLineNumber(cs, builder, core::Loc(cs.file, md.loc), startLoc, lastLoc,
                               irctx.lineNumberPtrsByFunction[rubyRegionId]);
    }
}

void parseKeywordArgsFromCallData(CompilerState &cs, llvm::IRBuilderBase &builder, const IREmitterContext &irctx,
                                  cfg::LocalRef kwRestArgName, llvm::Value *argCountRaw, llvm::Value *argArrayRaw,
                                  llvm::Value *hashArgs, int maxPositionalArgCount,
                                  const vector<core::ArgInfo::ArgFlags> &argsFlags, int rubyRegionId) {
    ENFORCE(rubyRegionId == 0);
    auto *func = irctx.rubyBlocks2Functions[rubyRegionId];
    auto &argPresentVariables = irctx.argPresentVariables[rubyRegionId];
    // required arguments remaining to be parsed
    auto numRequiredKwArgs = absl::c_count_if(
        argsFlags, [](auto &argFlag) { return argFlag.isKeyword && !argFlag.isDefault && !argFlag.isRepeated; });
    auto *missingKwargs = Payload::rubyUndef(cs, builder);

    // optional arguments that are present
    auto *optionalKwargs = IREmitterHelpers::buildS4(cs, 0);

    // Sorbet method functions are passed (int, VALUE *, VALUE, rb_control_frame_t *, void *, void *).
    // The second void *, argument 6, is rb_call_data/rb_kwarg_call_data.
    auto *callData = func->arg_begin() + 5;
    // The keywords, for a non-kwsplat situation, are passed after the positional
    // arguments.  The passed argc (argument 1) reflects *only* the number of
    // positional arguments passed; the number of keyword arguments is stored in the
    // call data.  But the keyword arguments are still passed in argv (argument 2).
    llvm::Value *kwargvIndices[] = {argCountRaw};
    auto *kwargv = builder.CreateGEP(argArrayRaw, kwargvIndices, "kwargv");

    for (int argId = maxPositionalArgCount; argId < argsFlags.size(); argId++) {
        if (!argsFlags[argId].isKeyword || argsFlags[argId].isRepeated) {
            continue;
        }
        auto name = irctx.rubyBlockArgs[rubyRegionId][argId];
        auto strviewName = name.data(irctx.cfg)._name.shortName(cs);
        auto rawId = Payload::idIntern(cs, builder, strviewName);

        auto argPresent = argPresentVariables[argId];

        auto *passedValue = builder.CreateCall(cs.getFunction("sorbet_kwarg_passed_value"), {callData, rawId, kwargv},
                                               fmt::format("kwargValueFor_{}", strviewName));
        auto *isItUndef = Payload::testIsUndef(cs, builder, passedValue);

        // Fill in the default value, if any.
        auto kwArgSet = llvm::BasicBlock::Create(cs, fmt::format("kwArgSet_{}", strviewName), func);
        auto kwArgDefault = llvm::BasicBlock::Create(cs, fmt::format("kwArgDefault_{}", strviewName), func);
        auto kwArgContinue = llvm::BasicBlock::Create(cs, fmt::format("kwArgContinue_{}", strviewName), func);

        auto *missingPhi = llvm::PHINode::Create(missingKwargs->getType(), 2,
                                                 fmt::format("missingArgsPhi_{}", strviewName), kwArgContinue);
        auto *optionalPhi = llvm::PHINode::Create(optionalKwargs->getType(), 2,
                                                  fmt::format("optionalArgsPhi_{}", strviewName), kwArgContinue);

        builder.CreateCondBr(isItUndef, kwArgDefault, kwArgSet);

        // Write a default value out, and mark the variable as missing
        builder.SetInsertPoint(kwArgDefault);
        if (argPresent.exists()) {
            Payload::varSet(cs, argPresent, Payload::rubyFalse(cs, builder), builder, irctx, rubyRegionId);
        }

        auto *updatedMissingKwargs = missingKwargs;
        if (!argsFlags[argId].isDefault) {
            auto *rawRubySym = builder.CreateCall(cs.getFunction("rb_id2sym"), {rawId}, "rawSym");
            updatedMissingKwargs = Payload::addMissingKWArg(cs, builder, missingKwargs, rawRubySym);
        }

        optionalPhi->addIncoming(optionalKwargs, builder.GetInsertBlock());
        missingPhi->addIncoming(updatedMissingKwargs, builder.GetInsertBlock());
        builder.CreateBr(kwArgContinue);

        builder.SetInsertPoint(kwArgSet);
        auto *updatedOptionalKwargs = optionalKwargs;
        if (argPresent.exists()) {
            if (argsFlags[argId].isDefault) {
                updatedOptionalKwargs =
                    builder.CreateBinOp(llvm::Instruction::Add, optionalKwargs, IREmitterHelpers::buildS4(cs, 1),
                                        fmt::format("updatedOptional_{}", strviewName));
            }

            Payload::varSet(cs, argPresent, Payload::rubyTrue(cs, builder), builder, irctx, rubyRegionId);
        }
        Payload::varSet(cs, name, passedValue, builder, irctx, rubyRegionId);

        optionalPhi->addIncoming(updatedOptionalKwargs, builder.GetInsertBlock());
        missingPhi->addIncoming(missingKwargs, builder.GetInsertBlock());
        builder.CreateBr(kwArgContinue);

        builder.SetInsertPoint(kwArgContinue);
        optionalKwargs = optionalPhi;
        missingKwargs = missingPhi;
    }
    Payload::assertAllRequiredKWArgs(cs, builder, missingKwargs);
    // We never want to use this path when we have a kwsplat arg, because the caller
    // will roll things up into the kwsplat, so there wouldn't be anything for us.
    ENFORCE(!kwRestArgName.exists());
    builder.CreateCall(cs.getFunction("sorbet_assertCallDataNoExtraKWArg"),
                       {callData, IREmitterHelpers::buildS4(cs, numRequiredKwArgs), optionalKwargs});
}

void parseKeywordArgsFromKwSplat(CompilerState &cs, llvm::IRBuilderBase &builder, const IREmitterContext &irctx,
                                 cfg::LocalRef kwRestArgName, llvm::Value *hashArgs, int maxPositionalArgCount,
                                 const vector<core::ArgInfo::ArgFlags> &argsFlags, int rubyRegionId) {
    auto *func = irctx.rubyBlocks2Functions[rubyRegionId];
    auto &argPresentVariables = irctx.argPresentVariables[rubyRegionId];
    // required arguments remaining to be parsed
    auto numRequiredKwArgs = absl::c_count_if(
        argsFlags, [](auto &argFlag) { return argFlag.isKeyword && !argFlag.isDefault && !argFlag.isRepeated; });
    auto *missingKwargs = Payload::rubyUndef(cs, builder);

    // optional arguments that are present
    auto *optionalKwargs = IREmitterHelpers::buildS4(cs, 0);

    for (int argId = maxPositionalArgCount; argId < argsFlags.size(); argId++) {
        if (!argsFlags[argId].isKeyword || argsFlags[argId].isRepeated) {
            continue;
        }
        auto name = irctx.rubyBlockArgs[rubyRegionId][argId];
        auto rawId = Payload::idIntern(cs, builder, name.data(irctx.cfg)._name.shortName(cs));
        auto rawRubySym = builder.CreateCall(cs.getFunction("rb_id2sym"), {rawId}, "rawSym");

        auto argPresent = argPresentVariables[argId];

        llvm::Value *passedValue;
        if (kwRestArgName.exists()) {
            passedValue = Payload::removeKWArg(cs, builder, hashArgs, rawRubySym);
        } else {
            passedValue = Payload::getKWArg(cs, builder, hashArgs, rawRubySym);
        }

        auto isItUndef = Payload::testIsUndef(cs, builder, passedValue);

        auto kwArgSet = llvm::BasicBlock::Create(cs, "kwArgSet", func);
        auto kwArgDefault = llvm::BasicBlock::Create(cs, "kwArgDefault", func);
        auto kwArgContinue = llvm::BasicBlock::Create(cs, "kwArgContinue", func);

        auto *missingPhi = llvm::PHINode::Create(missingKwargs->getType(), 2, "missingArgsPhi", kwArgContinue);
        auto *optionalPhi = llvm::PHINode::Create(optionalKwargs->getType(), 2, "optionalArgsPhi", kwArgContinue);

        builder.CreateCondBr(isItUndef, kwArgDefault, kwArgSet);

        // Write a default value out, and mark the variable as missing
        builder.SetInsertPoint(kwArgDefault);
        if (argPresent.exists()) {
            Payload::varSet(cs, argPresent, Payload::rubyFalse(cs, builder), builder, irctx, rubyRegionId);
        }

        auto *updatedMissingKwargs = missingKwargs;
        if (!argsFlags[argId].isDefault) {
            updatedMissingKwargs = Payload::addMissingKWArg(cs, builder, missingKwargs, rawRubySym);
        }

        optionalPhi->addIncoming(optionalKwargs, builder.GetInsertBlock());
        missingPhi->addIncoming(updatedMissingKwargs, builder.GetInsertBlock());
        builder.CreateBr(kwArgContinue);

        builder.SetInsertPoint(kwArgSet);
        auto *updatedOptionalKwargs = optionalKwargs;
        if (argPresent.exists()) {
            if (argsFlags[argId].isDefault) {
                updatedOptionalKwargs =
                    builder.CreateBinOp(llvm::Instruction::Add, optionalKwargs, IREmitterHelpers::buildS4(cs, 1));
            }

            Payload::varSet(cs, argPresent, Payload::rubyTrue(cs, builder), builder, irctx, rubyRegionId);
        }
        Payload::varSet(cs, name, passedValue, builder, irctx, rubyRegionId);
        optionalPhi->addIncoming(updatedOptionalKwargs, builder.GetInsertBlock());
        missingPhi->addIncoming(missingKwargs, builder.GetInsertBlock());
        builder.CreateBr(kwArgContinue);

        builder.SetInsertPoint(kwArgContinue);
        optionalKwargs = optionalPhi;
        missingKwargs = missingPhi;
    }
    Payload::assertAllRequiredKWArgs(cs, builder, missingKwargs);
    if (kwRestArgName.exists()) {
        Payload::varSet(cs, kwRestArgName, Payload::readKWRestArg(cs, builder, hashArgs), builder, irctx, rubyRegionId);
    } else {
        Payload::assertNoExtraKWArg(cs, builder, hashArgs, IREmitterHelpers::buildS4(cs, numRequiredKwArgs),
                                    optionalKwargs);
    }
}

void setupArguments(CompilerState &base, cfg::CFG &cfg, const ast::MethodDef &md, const IREmitterContext &irctx) {
    // this function effectively generate an optimized build of
    // https://github.com/ruby/ruby/blob/59c3b1c9c843fcd2d30393791fe224e5789d1677/include/ruby/ruby.h#L2522-L2675
    llvm::IRBuilder<> builder(base);
    for (auto rubyRegionId = 0; rubyRegionId < irctx.rubyBlocks2Functions.size(); rubyRegionId++) {
        auto cs = base.withFunctionEntry(irctx.functionInitializersByFunction[rubyRegionId]);

        builder.SetInsertPoint(irctx.argumentSetupBlocksByFunction[rubyRegionId]);

        // emit a location that corresponds to the function entry
        auto loc = md.symbol.data(cs)->loc();
        IREmitterHelpers::emitDebugLoc(cs, builder, irctx, rubyRegionId, loc);

        auto blockType = irctx.rubyBlockType[rubyRegionId];
        if (blockType == FunctionType::Method || blockType == FunctionType::StaticInitFile ||
            blockType == FunctionType::StaticInitModule || blockType == FunctionType::Block) {
            auto func = irctx.rubyBlocks2Functions[rubyRegionId];
            auto &argPresentVariables = irctx.argPresentVariables[rubyRegionId];
            auto maxPositionalArgCount = 0;
            auto minPositionalArgCount = 0;
            auto isBlock = blockType == FunctionType::Block;
            auto hasRestArgs = false;
            auto hasKWArgs = false;
            auto hasKWRestArgs = false;
            llvm::Value *argCountRaw = !isBlock ? func->arg_begin() : func->arg_begin() + 2;
            llvm::Value *argArrayRaw = !isBlock ? func->arg_begin() + 1 : func->arg_begin() + 3;
            llvm::Value *hashArgs;

            cfg::LocalRef blkArgName;
            cfg::LocalRef restArgName;
            cfg::LocalRef kwRestArgName;

            auto argsFlags = getArgFlagsForBlockId(cs, rubyRegionId, cfg.symbol, irctx);
            {
                auto argId = -1;
                ENFORCE(argsFlags.size() == irctx.rubyBlockArgs[rubyRegionId].size());
                for (auto &argFlags : argsFlags) {
                    argId += 1;
                    if (argFlags.isKeyword) {
                        hasKWArgs = true;
                        if (argFlags.isRepeated) {
                            kwRestArgName = irctx.rubyBlockArgs[rubyRegionId][argId];
                            hasKWRestArgs = true;
                        }
                        continue;
                    }
                    if (argFlags.isRepeated) {
                        restArgName = irctx.rubyBlockArgs[rubyRegionId][argId];
                        hasRestArgs = true;
                        continue;
                    }
                    if (argFlags.isDefault) {
                        maxPositionalArgCount += 1;
                        continue;
                    }
                    if (argFlags.isBlock) {
                        blkArgName = irctx.rubyBlockArgs[rubyRegionId][argId];
                        continue;
                    }
                    maxPositionalArgCount += 1;
                    minPositionalArgCount += 1;
                }
            }

            hashArgs = Payload::rubyUndef(cs, builder);

            if (hasKWArgs) {
                // if last argument is a hash, it's not part of positional arguments - it's going to
                // fullfill all kw arguments instead
                auto *minPositionalArgValue = IREmitterHelpers::buildS4(cs, minPositionalArgCount);
                auto *hashArgsCtx =
                    builder.CreateCall(cs.getFunction("sorbet_determineKwSplatArg"),
                                       {argCountRaw, argArrayRaw, minPositionalArgValue}, "hashArgsCtx");
                argCountRaw = builder.CreateExtractValue(hashArgsCtx, {0}, "argCountRaw");
                hashArgs = builder.CreateExtractValue(hashArgsCtx, {1}, "hashArgs");
            }

            if (isBlock) {
                if (maxPositionalArgCount > 1) {
                    // blocks can expand their first argument in arg array
                    auto arrayTestBlock = llvm::BasicBlock::Create(cs, "argArrayExpandArrayTest", func);
                    auto argExpandBlock = llvm::BasicBlock::Create(cs, "argArrayExpand", func);
                    auto afterArgArrayExpandBlock = llvm::BasicBlock::Create(cs, "afterArgArrayExpand", func);
                    auto argSizeForExpansionCheck = builder.CreateICmpEQ(
                        argCountRaw, llvm::ConstantInt::get(cs, llvm::APInt(32, 1)), "arrayExpansionSizeGuard");
                    builder.CreateCondBr(argSizeForExpansionCheck, arrayTestBlock, afterArgArrayExpandBlock);
                    auto sizeTestEnd = builder.GetInsertBlock();
                    builder.SetInsertPoint(arrayTestBlock);
                    llvm::Value *indices[] = {llvm::ConstantInt::get(cs, llvm::APInt(32, 0, true))};
                    auto rawArg1Value =
                        builder.CreateLoad(builder.CreateGEP(argArrayRaw, indices), "arg1_maybeExpandToFullArgs");
                    auto isArray = Payload::typeTest(cs, builder, rawArg1Value, core::Symbols::Array());
                    auto typeTestEnd = builder.GetInsertBlock();

                    builder.CreateCondBr(isArray, argExpandBlock, afterArgArrayExpandBlock);
                    builder.SetInsertPoint(argExpandBlock);
                    auto newArgArray = builder.CreateCall(cs.getFunction("sorbet_rubyArrayInnerPtr"), {rawArg1Value},
                                                          "expandedArgArray");
                    auto newArgc =
                        builder.CreateCall(cs.getFunction("sorbet_rubyArrayLen"), {rawArg1Value}, "expandedArgc");
                    auto expansionEnd = builder.GetInsertBlock();
                    builder.CreateBr(afterArgArrayExpandBlock);
                    builder.SetInsertPoint(afterArgArrayExpandBlock);
                    auto argcPhi = builder.CreatePHI(builder.getInt32Ty(), 3, "argcPhi");
                    argcPhi->addIncoming(argCountRaw, sizeTestEnd);
                    argcPhi->addIncoming(argCountRaw, typeTestEnd);
                    argcPhi->addIncoming(newArgc, expansionEnd);
                    argCountRaw = argcPhi;
                    auto argArrayPhi = builder.CreatePHI(llvm::Type::getInt64PtrTy(cs), 3, "argArrayPhi");
                    argArrayPhi->addIncoming(argArrayRaw, sizeTestEnd);
                    argArrayPhi->addIncoming(argArrayRaw, typeTestEnd);
                    argArrayPhi->addIncoming(newArgArray, expansionEnd);

                    argArrayRaw = argArrayPhi;
                }
                minPositionalArgCount = 0;
                // blocks Can have 0 args always
            }

            auto numOptionalArgs = maxPositionalArgCount - minPositionalArgCount;
            if (!isBlock) {
                // validate arg count
                auto argCountFailBlock = llvm::BasicBlock::Create(cs, "argCountFailBlock", func);
                auto argCountSecondCheckBlock = llvm::BasicBlock::Create(cs, "argCountSecondCheckBlock", func);
                auto argCountSuccessBlock = llvm::BasicBlock::Create(cs, "argCountSuccess", func);

                if (!hasRestArgs) {
                    auto tooManyArgs = builder.CreateICmpUGT(
                        argCountRaw, llvm::ConstantInt::get(cs, llvm::APInt(32, maxPositionalArgCount)), "tooManyArgs");
                    auto expected1 = Payload::setExpectedBool(cs, builder, tooManyArgs, false);
                    builder.CreateCondBr(expected1, argCountFailBlock, argCountSecondCheckBlock);
                } else {
                    builder.CreateBr(argCountSecondCheckBlock);
                }

                builder.SetInsertPoint(argCountSecondCheckBlock);
                auto tooFewArgs = builder.CreateICmpULT(
                    argCountRaw, llvm::ConstantInt::get(cs, llvm::APInt(32, minPositionalArgCount)), "tooFewArgs");
                auto expected2 = Payload::setExpectedBool(cs, builder, tooFewArgs, false);
                builder.CreateCondBr(expected2, argCountFailBlock, argCountSuccessBlock);

                builder.SetInsertPoint(argCountFailBlock);
                Payload::raiseArity(cs, builder, argCountRaw, minPositionalArgCount,
                                    hasRestArgs ? -1 : maxPositionalArgCount);

                builder.SetInsertPoint(argCountSuccessBlock);
            }

            vector<llvm::BasicBlock *> checkBlocks;
            vector<llvm::BasicBlock *> fillFromArgBlocks;
            vector<llvm::BasicBlock *> fillFromDefaultBlocks;
            {
                // create blocks for arg filling
                for (auto i = 0; i < numOptionalArgs; i++) {
                    auto suffix = to_string(i);
                    checkBlocks.emplace_back(llvm::BasicBlock::Create(cs, {"checkBlock", suffix}, func));
                    fillFromDefaultBlocks.emplace_back(
                        llvm::BasicBlock::Create(cs, {"fillFromDefaultBlock", suffix}, func));
                    fillFromArgBlocks.emplace_back(llvm::BasicBlock::Create(cs, {"fillFromArgBlock", suffix}, func));
                }

                // create "Done" blocks (not needed for fillFromArgBlocks)
                auto suffix = "Done" + to_string(numOptionalArgs);
                checkBlocks.emplace_back(llvm::BasicBlock::Create(cs, {"checkBlock", suffix}, func));
                fillFromDefaultBlocks.emplace_back(
                    llvm::BasicBlock::Create(cs, {"fillFromDefaultBlock", suffix}, func));
            }
            {
                // fill local variables from args
                auto fillRequiredArgs = llvm::BasicBlock::Create(cs, "fillRequiredArgs", func);
                builder.CreateBr(fillRequiredArgs);
                builder.SetInsertPoint(fillRequiredArgs);

                for (auto i = 0; i < maxPositionalArgCount; i++) {
                    if (i >= minPositionalArgCount) {
                        // if these are optional, put them in their own BasicBlock
                        // because we might not run it
                        auto &block = fillFromArgBlocks[i - minPositionalArgCount];
                        builder.SetInsertPoint(block);
                    }
                    const auto a = irctx.rubyBlockArgs[rubyRegionId][i];
                    if (!a.data(cfg)._name.exists()) {
                        failCompilation(cs, core::Loc(cfg.file, md.declLoc),
                                        "this method has a block argument construct that's not supported");
                    }

                    // mark the arg as present
                    auto &argPresent = argPresentVariables[i];
                    if (argPresent.exists()) {
                        Payload::varSet(cs, argPresent, Payload::rubyTrue(cs, builder), builder, irctx, rubyRegionId);
                    }

                    llvm::Value *indices[] = {llvm::ConstantInt::get(cs, llvm::APInt(32, i, true))};
                    auto name = a.data(cfg)._name.shortName(cs);
                    llvm::StringRef nameRef(name.data(), name.length());
                    auto rawValue = builder.CreateLoad(builder.CreateGEP(argArrayRaw, indices), {"rawArg_", nameRef});
                    Payload::varSet(cs, a, rawValue, builder, irctx, rubyRegionId);
                    if (i >= minPositionalArgCount) {
                        // check if we need to fill in the next variable from the arg
                        builder.CreateBr(checkBlocks[i - minPositionalArgCount + 1]);
                    }
                }

                // make the last instruction in all the required args point at the first check block
                builder.SetInsertPoint(fillRequiredArgs);
                //
                if (blkArgName.exists() && irctx.blockArgUsage == BlockArgUsage::Captured) {
                    // TODO: I don't think this correctly handles blocks with block args
                    Payload::varSet(cs, blkArgName, builder.CreateCall(cs.getFunction("sorbet_getMethodBlockAsProc")),
                                    builder, irctx, 0);
                }
                builder.CreateBr(checkBlocks[0]);
            }
            {
                // build check blocks
                for (auto i = 0; i < numOptionalArgs; i++) {
                    auto &block = checkBlocks[i];
                    builder.SetInsertPoint(block);
                    auto argCount = builder.CreateICmpEQ(
                        argCountRaw, llvm::ConstantInt::get(cs, llvm::APInt(32, i + minPositionalArgCount)),
                        llvm::Twine("default") + llvm::Twine(i));
                    auto expected = Payload::setExpectedBool(cs, builder, argCount, false);
                    builder.CreateCondBr(expected, fillFromDefaultBlocks[i], fillFromArgBlocks[i]);
                }
            }

            {
                // build fillFromDefaultBlocks
                for (auto i = 0; i < numOptionalArgs; i++) {
                    builder.SetInsertPoint(fillFromDefaultBlocks[i]);

                    auto argIndex = i + minPositionalArgCount;
                    auto argPresent = argPresentVariables[argIndex];
                    if (argPresent.exists()) {
                        Payload::varSet(cs, argPresent, Payload::rubyFalse(cs, builder), builder, irctx, rubyRegionId);
                    }

                    if (isBlock) {
                        auto a = irctx.rubyBlockArgs[rubyRegionId][argIndex];
                        Payload::varSet(cs, a, Payload::rubyNil(cs, builder), builder, irctx, rubyRegionId);
                    }

                    // fall through to the next default arg init block
                    builder.CreateBr(fillFromDefaultBlocks[i + 1]);
                }
            }

            {
                // Tie up all the "Done" blocks at the end
                builder.SetInsertPoint(checkBlocks[numOptionalArgs]);
                builder.CreateBr(fillFromDefaultBlocks[numOptionalArgs]);
                builder.SetInsertPoint(fillFromDefaultBlocks[numOptionalArgs]);
                if (hasRestArgs) {
                    Payload::varSet(cs, restArgName,
                                    Payload::readRestArgs(cs, builder, maxPositionalArgCount, argCountRaw, argArrayRaw),
                                    builder, irctx, rubyRegionId);
                }
                if (hasKWArgs) {
                    // If we have a kwsplat arg, the caller/VM will always make sure that
                    // our keyword args are rolled up into the splat, so there's no point
                    // in attempting to take the efficient parsing route.
                    //
                    // Blocks also take the splat route always.
                    if (hasKWRestArgs || rubyRegionId != 0) {
                        parseKeywordArgsFromKwSplat(cs, builder, irctx, kwRestArgName, hashArgs, maxPositionalArgCount,
                                                    argsFlags, rubyRegionId);
                    } else {
                        // Due to the wonders of Ruby, we can't always be assured that
                        // our kwargs are passed directly on the stack.  They might
                        // be in the kwsplat arg that we determined earlier.  So we need
                        // a runtime check to determine how to parse the args.
                        auto *check =
                            builder.CreateCall(cs.getFunction("sorbet_can_efficiently_parse_kwargs"),
                                               {hashArgs, func->arg_begin() + 4, func->arg_begin() + 5}, "efficient?");
                        auto *efficientBlock = llvm::BasicBlock::Create(cs, "efficientParsing", func);
                        auto *hashBlock = llvm::BasicBlock::Create(cs, "kwsplatParsing", func);
                        auto *continuationBlock = llvm::BasicBlock::Create(cs, "continuationBlock", func);

                        builder.CreateCondBr(check, efficientBlock, hashBlock);

                        builder.SetInsertPoint(efficientBlock);
                        parseKeywordArgsFromCallData(cs, builder, irctx, kwRestArgName, argCountRaw, argArrayRaw,
                                                     hashArgs, maxPositionalArgCount, argsFlags, rubyRegionId);
                        builder.CreateBr(continuationBlock);

                        builder.SetInsertPoint(hashBlock);
                        parseKeywordArgsFromKwSplat(cs, builder, irctx, kwRestArgName, hashArgs, maxPositionalArgCount,
                                                    argsFlags, rubyRegionId);
                        builder.CreateBr(continuationBlock);

                        builder.SetInsertPoint(continuationBlock);
                    }
                }
            }
        }

        switch (blockType) {
            case FunctionType::Method:
            case FunctionType::StaticInitFile:
            case FunctionType::StaticInitModule:
            case FunctionType::Block:
            case FunctionType::ExceptionBegin:
            case FunctionType::Rescue:
            case FunctionType::Ensure:
                // jump dirrectly to user body
                builder.CreateBr(irctx.userEntryBlockByFunction[rubyRegionId]);
                break;

            case FunctionType::Unused:
                // this function will never be called
                builder.CreateUnreachable();
                break;
        }
    }
}

cfg::LocalRef returnValue(cfg::CFG &cfg, CompilerState &cs) {
    return cfg.enterLocal({core::Names::returnValue(), 1});
}

llvm::BasicBlock *resolveJumpTarget(cfg::CFG &cfg, const IREmitterContext &irctx, const cfg::BasicBlock *from,
                                    const cfg::BasicBlock *to) {
    if (to == cfg.deadBlock()) {
        return irctx.deadBlockMapping[from->rubyRegionId];
    }

    auto remapped = irctx.basicBlockJumpOverrides[to->id];
    if (from->rubyRegionId != irctx.basicBlockRubyBlockId[remapped]) {
        return irctx.blockExitMapping[from->rubyRegionId];
    } else {
        return irctx.llvmBlocksBySorbetBlocks[remapped];
    }
}

void emitUserBody(CompilerState &base, cfg::CFG &cfg, const IREmitterContext &irctx) {
    llvm::IRBuilder<> builder(base);
    auto startLoc = cfg.symbol.data(base)->loc();
    auto &arguments = cfg.symbol.data(base)->arguments;
    for (auto it = cfg.forwardsTopoSort.rbegin(); it != cfg.forwardsTopoSort.rend(); ++it) {
        cfg::BasicBlock *bb = *it;
        auto cs = base.withFunctionEntry(irctx.functionInitializersByFunction[bb->rubyRegionId]);

        auto block = irctx.llvmBlocksBySorbetBlocks[bb->id];
        bool isTerminated = false;

        builder.SetInsertPoint(block);

        // NOTE: explicitly clearing debug location information here so that we don't accidentally inherit the location
        // information from blocks in different target functions.
        builder.SetCurrentDebugLocation(llvm::DebugLoc());

        core::Loc lastLoc;
        if (bb == cfg.deadBlock()) {
            continue;
        }

        for (cfg::Binding &bind : bb->exprs) {
            auto loc = core::Loc(cs.file, bind.loc);

            lastLoc = Payload::setLineNumber(cs, builder, loc, startLoc, lastLoc,
                                             irctx.lineNumberPtrsByFunction[bb->rubyRegionId]);

            IREmitterHelpers::emitDebugLoc(cs, builder, irctx, bb->rubyRegionId, loc);

            typecase(
                bind.value,
                [&](cfg::Ident &i) {
                    auto var = Payload::varGet(cs, i.what, builder, irctx, bb->rubyRegionId);
                    Payload::varSet(cs, bind.bind.variable, var, builder, irctx, bb->rubyRegionId);
                },
                [&](cfg::Alias &i) {
                    // We compute the alias map when IREmitterContext is first created, so if an entry is missing,
                    // there's a problem.
                    ENFORCE(irctx.aliases.find(bind.bind.variable) != irctx.aliases.end(),
                            "Alias is missing from the alias map");
                },
                [&](cfg::SolveConstraint &i) {
                    auto var = Payload::varGet(cs, i.send, builder, irctx, bb->rubyRegionId);
                    Payload::varSet(cs, bind.bind.variable, var, builder, irctx, bb->rubyRegionId);
                },
                [&](cfg::Send &i) {
                    std::optional<int> blk;
                    if (i.link != nullptr) {
                        blk.emplace(i.link->rubyRegionId);
                    }
                    auto mcctx = MethodCallContext::create(cs, builder, irctx, bb->rubyRegionId, &i, blk);
                    auto rawCall = IREmitterHelpers::emitMethodCall(mcctx);
                    mcctx.finalize();
                    Payload::varSet(cs, bind.bind.variable, rawCall, builder, irctx, bb->rubyRegionId);
                },
                [&](cfg::Return &i) {
                    isTerminated = true;

                    auto *var = Payload::varGet(cs, i.what.variable, builder, irctx, bb->rubyRegionId);
                    bool hasBlockAncestor = false;
                    int rubyRegionId = bb->rubyRegionId;

                    while (rubyRegionId != 0) {
                        // We iterate over the entire ancestor chain instead of breaking out early
                        // when we hit a Ruby block.  We do this so we can check this ENFORCE and
                        // ensure that we're not throwing over something that would require postprocessing.
                        ENFORCE(!functionTypeNeedsPostprocessing(irctx.rubyBlockType[rubyRegionId]));
                        hasBlockAncestor = hasBlockAncestor || irctx.rubyBlockType[rubyRegionId] == FunctionType::Block;
                        rubyRegionId = irctx.rubyBlockParent[rubyRegionId];
                    }

                    if (hasBlockAncestor) {
                        ENFORCE(irctx.hasReturnAcrossBlock);
                        IREmitterHelpers::emitReturnAcrossBlock(cs, cfg, builder, irctx, bb->rubyRegionId, var);
                    } else {
                        IREmitterHelpers::emitReturn(cs, builder, irctx, bb->rubyRegionId, var);
                    }
                },
                [&](cfg::BlockReturn &i) {
                    ENFORCE(bb->rubyRegionId != 0, "should never happen");
                    isTerminated = true;
                    auto var = Payload::varGet(cs, i.what.variable, builder, irctx, bb->rubyRegionId);
                    IREmitterHelpers::emitReturn(cs, builder, irctx, bb->rubyRegionId, var);
                },
                [&](cfg::LoadSelf &i) {
                    // it's done in function setup, no need to do anything here
                },
                [&](cfg::Literal &i) {
                    auto rawValue = IREmitterHelpers::emitLiteralish(cs, builder, i.value);
                    Payload::varSet(cs, bind.bind.variable, rawValue, builder, irctx, bb->rubyRegionId);
                },
                [&](cfg::GetCurrentException &i) {
                    // if this block isn't an exception block header, there's nothing to do here.
                    auto bodyRubyRegionId = irctx.exceptionBlockHeader[bb->id];
                    if (bodyRubyRegionId == 0) {
                        return;
                    }

                    IREmitterHelpers::emitExceptionHandlers(cs, builder, irctx, bb->rubyRegionId, bodyRubyRegionId,
                                                            bind.bind.variable);
                },
                [&](cfg::ArgPresent &i) {
                    ENFORCE(bb->rubyRegionId == 0, "ArgPresent found outside of entry-method");
                    // Intentionally omitted: the result of the ArgPresent call is filled out in `setupArguments`
                },
                [&](cfg::LoadArg &i) {
                    ENFORCE(bb->rubyRegionId == 0, "LoadArg found outside of entry-method");

                    // Argument values are loaded by `setupArguments`, we just need to check their type here
                    auto &argInfo = arguments[i.argId];
                    // The runtime doesn't check types for blocks, so we don't either.
                    if (argInfo.flags.isBlock) {
                        return;
                    }

                    auto local = irctx.rubyBlockArgs[0][i.argId];
                    auto var = Payload::varGet(cs, local, builder, irctx, 0);
                    if (auto &expectedType = argInfo.type) {
                        auto description = fmt::format("Parameter '{}'", bind.bind.variable.toString(cs, cfg));
                        if (argInfo.flags.isRepeated && !argInfo.flags.isKeyword) {
                            // Signature types for rest arguments apply to each individual element of
                            // the rest arg, not the actual argument itself.
                            IREmitterHelpers::emitTypeTestForRestArg(cs, builder, var, expectedType, description);
                        } else {
                            IREmitterHelpers::emitTypeTest(cs, builder, var, expectedType, description);
                        }
                    }
                },
                [&](cfg::LoadYieldParams &i) {
                    ENFORCE(bb->rubyRegionId != 0, "LoadYieldParams found outside of ruby block");
                    /* intentionally omitted, it's part of method preambula */
                },
                [&](cfg::YieldParamPresent &i) {
                    ENFORCE(bb->rubyRegionId != 0, "YieldParamPresent found outside of ruby block");
                    // Intentionally omitted: the result of the YieldParamPresent call is filled out in `setupArguments`
                },
                [&](cfg::YieldLoadArg &i) {
                    ENFORCE(bb->rubyRegionId != 0, "YieldLoadArg found outside of ruby block");
                    // Filled out as part of the method preamble.
                },
                [&](cfg::Cast &i) {
                    auto val = Payload::varGet(cs, i.value.variable, builder, irctx, bb->rubyRegionId);

                    // We skip the type test for Cast instructions that assign into <self>.
                    // These instructions only exist in the CFG for the purpose of type checking.
                    // The Ruby VM already checks that self is a valid type when calling `.bind()`
                    // on an UnboundMethod object.
                    auto skipTypeTest = bind.bind.variable.data(cfg) == core::LocalVariable::selfVariable();

                    if (!skipTypeTest) {
                        IREmitterHelpers::emitTypeTest(cs, builder, val, bind.bind.type,
                                                       fmt::format("T.{}", i.cast.shortName(cs)));
                    }

                    if (i.cast == core::Names::let() || i.cast == core::Names::cast()) {
                        Payload::varSet(cs, bind.bind.variable, val, builder, irctx, bb->rubyRegionId);
                    } else if (i.cast == core::Names::assertType()) {
                        Payload::varSet(cs, bind.bind.variable, Payload::rubyFalse(cs, builder), builder, irctx,
                                        bb->rubyRegionId);
                    }
                },
                [&](cfg::TAbsurd &i) {
                    auto val = Payload::varGet(cs, i.what.variable, builder, irctx, bb->rubyRegionId);
                    builder.CreateCall(cs.getFunction("sorbet_t_absurd"), {val});
                });
            if (isTerminated) {
                break;
            }
        }
        if (!isTerminated) {
            auto *thenb = resolveJumpTarget(cfg, irctx, bb, bb->bexit.thenb);
            auto *elseb = resolveJumpTarget(cfg, irctx, bb, bb->bexit.elseb);

            if (thenb != elseb && bb->bexit.cond.variable != cfg::LocalRef::blockCall()) {
                // If the condition is a class variable reference, we can't just access it
                // directly, because it might not have been defined yet, and accessing
                // class variables prior to their definition is an error.
                //
                // The VM will insert an explicit `defined?(@@var)` check for `@@var ||= ...`.
                // We desugar that expression to explicit conditionals (without the `defined?`
                // check, so we have to insert the definedness check for safety.
                //
                // If you write `if @@var; @@var; else; @@var = ...; end`, the VM will
                // test for truthiness directly without doing through `defined?`.  Even
                // so, the translation we're doing here is just as efficient: the VM would
                // do a class variable lookup and truthiness test, which is exactly the
                // same thing as sorbet_classVariableDefinedAndTruthy does.
                auto testref = bb->bexit.cond.variable;
                auto aliasEntry = irctx.aliases.find(testref);
                llvm::Value *condValue;
                if (aliasEntry != irctx.aliases.end() && aliasEntry->second.kind == Alias::AliasKind::ClassField) {
                    auto klass = Payload::getClassVariableStoreClass(cs, builder, irctx);
                    auto id = Payload::idIntern(cs, builder, aliasEntry->second.classField.shortName(cs));
                    condValue = builder.CreateCall(cs.getFunction("sorbet_classVariableDefinedAndTruthy"), {klass, id});
                } else {
                    auto var = Payload::varGet(cs, testref, builder, irctx, bb->rubyRegionId);
                    condValue = Payload::testIsTruthy(cs, builder, var);
                }

                builder.CreateCondBr(condValue, thenb, elseb);
            } else {
                builder.CreateBr(thenb);
            }
        }
    }
}

void emitDeadBlocks(CompilerState &cs, cfg::CFG &cfg, const IREmitterContext &irctx) {
    llvm::IRBuilder<> builder(cs);

    // Emit the dead block body for each ruby block. It should be an error to transition to the dead block, so
    // we mark its body as unreachable.
    for (auto rubyRegionId = 0; rubyRegionId <= cfg.maxRubyRegionId; ++rubyRegionId) {
        auto *dead = irctx.deadBlockMapping[rubyRegionId];
        builder.SetInsertPoint(dead);
        builder.CreateUnreachable();
    }
}

void emitBlockExits(CompilerState &base, cfg::CFG &cfg, const IREmitterContext &irctx) {
    llvm::IRBuilder<> builder(base);

    for (auto rubyRegionId = 0; rubyRegionId <= cfg.maxRubyRegionId; ++rubyRegionId) {
        auto cs = base.withFunctionEntry(irctx.functionInitializersByFunction[rubyRegionId]);

        builder.SetInsertPoint(irctx.blockExitMapping[rubyRegionId]);

        switch (irctx.rubyBlockType[rubyRegionId]) {
            case FunctionType::Method:
            case FunctionType::StaticInitFile:
            case FunctionType::StaticInitModule:
                builder.CreateUnreachable();
                break;

            case FunctionType::Block:
            case FunctionType::ExceptionBegin:
            case FunctionType::Rescue:
            case FunctionType::Ensure:
            case FunctionType::Unused:
                // for non-top-level functions, we return `Qundef` to indicate that this value isn't used for anything.
                IREmitterHelpers::emitReturn(cs, builder, irctx, rubyRegionId, Payload::rubyUndef(cs, builder));
                break;
        }
    }
}

void emitPostProcess(CompilerState &cs, cfg::CFG &cfg, const IREmitterContext &irctx) {
    llvm::IRBuilder<> builder(cs);
    builder.SetInsertPoint(irctx.postProcessBlock);

    // we're only using the top-level ruby block at this point
    auto rubyRegionId = 0;

    auto var = Payload::varGet(cs, returnValue(cfg, cs), builder, irctx, rubyRegionId);
    auto *maybeChecked = IREmitterHelpers::maybeCheckReturnValue(cs, cfg, builder, irctx, var);

    IREmitterHelpers::emitUncheckedReturn(cs, builder, irctx, rubyRegionId, maybeChecked);
}

// Direct wrappers call the wrapped function without checking the receiver's type; the caller is responsible for
// guarding the call to the wrapper with appropriate validation of the receiver.
void emitDirectWrapper(CompilerState &cs, const ast::MethodDef &md, const IREmitterContext &irctx) {
    llvm::IRBuilder<> builder(cs);

    auto *wrapper = IREmitterHelpers::getOrCreateDirectWrapper(cs, md.symbol);
    auto *cache = wrapper->arg_begin();
    auto *argc = wrapper->arg_begin() + 1;
    auto *argv = wrapper->arg_begin() + 2;
    auto *self = wrapper->arg_begin() + 3;

    cache->setName("cache");
    argc->setName("argc");
    argv->setName("argv");
    self->setName("self");

    auto *entry = llvm::BasicBlock::Create(cs, "entry", wrapper);
    builder.SetInsertPoint(entry);

    auto *target = irctx.rubyBlocks2Functions[0];
    auto *stackFrame = builder.CreateLoad(Payload::rubyStackFrameVar(cs, builder, irctx, md.symbol));
    auto *res = Payload::callFuncDirect(cs, builder, cache, target, argc, argv, self, stackFrame);
    builder.CreateRet(res);
}

void IREmitter::run(CompilerState &cs, cfg::CFG &cfg, const ast::MethodDef &md) {
    Timer timer("IREmitter::run");
    cfg::CFG::UnfreezeCFGLocalVariables unfreezeVars(cfg);

    llvm::Function *func;

    if (IREmitterHelpers::isClassStaticInit(cs, md.symbol)) {
        func = IREmitterHelpers::getOrCreateStaticInit(cs, md.symbol, md.declLoc);
    } else {
        func = IREmitterHelpers::getOrCreateFunction(cs, md.symbol);
    }
    func = IREmitterHelpers::cleanFunctionBody(cs, func);
    {
        // setup function argument names
        func->arg_begin()->setName("argc");
        (func->arg_begin() + 1)->setName("argArray");
        (func->arg_begin() + 2)->setName("selfRaw");
        (func->arg_begin() + 3)->setName("cfp");
        (func->arg_begin() + 4)->setName("calling");
        (func->arg_begin() + 5)->setName("callData");
    }
    func->addFnAttr(llvm::Attribute::AttrKind::StackProtectReq);
    func->addFnAttr(llvm::Attribute::AttrKind::NoUnwind);
    func->addFnAttr(llvm::Attribute::AttrKind::UWTable);
    llvm::IRBuilder<> builder(cs);

    const IREmitterContext irctx = IREmitterContext::getSorbetBlocks2LLVMBlockMapping(cs, cfg, md, func);

    ENFORCE(cs.functionEntryInitializers == nullptr, "modules shouldn't be reused");

    setupStackFrames(cs, md, irctx);
    setupArguments(cs, cfg, md, irctx);

    emitUserBody(cs, cfg, irctx);
    emitDeadBlocks(cs, cfg, irctx);
    emitBlockExits(cs, cfg, irctx);
    emitPostProcess(cs, cfg, irctx);

    if (md.symbol.data(cs)->flags.isFinal) {
        emitDirectWrapper(cs, md, irctx);
    }

    // Link the function initializer blocks.
    for (int funId = 0; funId < irctx.functionInitializersByFunction.size(); funId++) {
        llvm::BasicBlock *nextBlock = irctx.argumentSetupBlocksByFunction[funId];
        builder.SetInsertPoint(irctx.functionInitializersByFunction[funId]);
        builder.CreateBr(nextBlock);
    }

    cs.debug->finalize();

    /* run verifier */
    if (debug_mode && llvm::verifyFunction(*func, &llvm::errs())) {
        fmt::print("failed to verify:\n");
        func->dump();
        ENFORCE(false);
    }
    cs.runCheapOptimizations(func);

    // If we are ever returning across blocks, we need to wrap the entire execution
    // of the function in an unwind-protect region that knows about the return.
    if (!irctx.hasReturnAcrossBlock) {
        return;
    }

    llvm::ValueToValueMapTy vMap;
    auto *implementationFunction = llvm::CloneFunction(func, vMap);

    // Completely delete the function body.
    func->dropAllReferences();

    // Turn the original function into a trampoline for this new function.
    auto *entryBlock = llvm::BasicBlock::Create(cs, "entry", func);
    builder.SetInsertPoint(entryBlock);

    auto *wrapper = cs.getFunction("sorbet_vm_return_from_block_wrapper");
    // Adding the function argument at the end means that we don't have to perform
    // any register shuffling.
    auto *status =
        builder.CreateCall(wrapper,
                           {func->arg_begin(), func->arg_begin() + 1, func->arg_begin() + 2, func->arg_begin() + 3,
                            func->arg_begin() + 4, func->arg_begin() + 5, implementationFunction},
                           "returnedFromBlock");

    // TODO(froydnj): LLVM IR is somewhat machine-specific when it comes to calling
    // functions returning a structure, like sorbet_vm_return_from_block_wrapper.
    // The conventions we have here are correct for x86-64 (Linux and Mac), but the
    // LLVM IR generated by clang for returning the same structure on arm64 Linux
    // returns a 2 x i64 vector.  If the structure was slightly bigger, the function
    // would actually take a pointer to the structure as a separate argument.
    // A different architecture might take a pointer always, regardless of how the
    // structure was laid out.
    //
    // It's entirely possible that having the Sorbet compiler generate code for a
    // non-x86-64 architecture would require reworking more stuff than just this
    // bit of code.  But this particular bit seems like an easy place to overlook
    // and puzzle about why things are going wrong.  So try and provide a little
    // advance notice to the would-be porter.
    if (debug_mode) {
        std::string error;
        const auto &targetTriple = cs.module->getTargetTriple();
        auto triple = llvm::Triple(targetTriple);
        ENFORCE(triple.getArch() == llvm::Triple::x86_64);
    }
    // Also make sure that the return value is small enough to be returned in
    // registers, i.e. that the function is actually returning a value directly.
    ENFORCE(!status->getType()->isVoidTy());

    // If we received this return value via throwing (i.e. return-from-block), we
    // didn't typecheck the value when it was thrown, so we need to do it here.
    auto *returnValue = builder.CreateExtractValue(status, {0}, "returnedValue");
    auto *wasThrown = builder.CreateExtractValue(status, {1}, "wasThrown");
    auto *typecheckBlock = llvm::BasicBlock::Create(cs, "typecheck", func);
    auto *exitBlock = llvm::BasicBlock::Create(cs, "exit", func);
    builder.CreateCondBr(builder.CreateICmpEQ(wasThrown, builder.getInt8(1)), typecheckBlock, exitBlock);

    builder.SetInsertPoint(typecheckBlock);
    auto *checkedValue = IREmitterHelpers::maybeCheckReturnValue(cs, cfg, builder, irctx, returnValue);
    typecheckBlock = builder.GetInsertBlock();
    builder.CreateBr(exitBlock);

    builder.SetInsertPoint(exitBlock);
    auto *returnValuePhi = builder.CreatePHI(returnValue->getType(), 2, "value");
    returnValuePhi->addIncoming(returnValue, entryBlock);
    returnValuePhi->addIncoming(checkedValue, typecheckBlock);
    builder.CreateRet(returnValuePhi);

    // Redo verifier on our new function.
    if (debug_mode && llvm::verifyFunction(*func, &llvm::errs())) {
        fmt::print("failed to verify:\n");
        func->dump();
        ENFORCE(false);
    }
    cs.runCheapOptimizations(func);
}

void IREmitter::buildInitFor(CompilerState &cs, const core::MethodRef &sym, string_view objectName) {
    llvm::IRBuilder<> builder(cs);

    auto owner = sym.data(cs)->owner;
    auto isRoot = IREmitterHelpers::isRootishSymbol(cs, owner);
    llvm::Function *entryFunc;

    if (IREmitterHelpers::isFileOrClassStaticInit(cs, sym)) {
        if (!isRoot) {
            return;
        }

        // for a path like `foo/bar/baz.rb`, we want the baseName to be just `baz`, as ruby will be looking for an init
        // function called `Init_baz`
        auto baseName = objectName.substr(0, objectName.rfind(".rb"));
        auto slash = baseName.rfind('/');
        if (slash != string_view::npos) {
            baseName.remove_prefix(slash + 1);
        }

        auto linkageType = llvm::Function::ExternalLinkage;
        std::vector<llvm::Type *> NoArgs(0, llvm::Type::getVoidTy(cs));
        auto ft = llvm::FunctionType::get(llvm::Type::getVoidTy(cs), NoArgs, false);
        entryFunc = llvm::Function::Create(ft, linkageType, "Init_" + string(baseName), *cs.module);
    } else {
        entryFunc = IREmitterHelpers::getInitFunction(cs, sym);
    }

    auto bb = llvm::BasicBlock::Create(cs, "entry", entryFunc);
    builder.SetInsertPoint(bb);

    if (IREmitterHelpers::isFileOrClassStaticInit(cs, sym)) {
        // We include sorbet_version.c when compiling the Sorbet Compiler itself to get the expected version.
        // The actual version will be linked into libruby.so and compared against at runtime.
        auto compileTimeBuildSCMRevision = sorbet_getBuildSCMRevision();
        auto compileTimeIsReleaseBuild = sorbet_getIsReleaseBuild();
        builder.CreateCall(cs.getFunction("sorbet_ensureSorbetRuby"),
                           {
                               llvm::ConstantInt::get(cs, llvm::APInt(32, compileTimeIsReleaseBuild, true)),
                               Payload::toCString(cs, compileTimeBuildSCMRevision, builder),
                           });

        auto realpath = builder.CreateCall(cs.getFunction("sorbet_readRealpath"), {});
        realpath->setName("realpath");

        builder.CreateCall(cs.getFunction("sorbet_globalConstructors"), {realpath});

        core::MethodRef staticInit = cs.gs.lookupStaticInitForFile(sym.data(cs)->loc());

        // Call the LLVM method that was made by run() from this Init_ method
        auto staticInitName = IREmitterHelpers::getFunctionName(cs, staticInit);
        auto staticInitFunc = cs.getFunction(staticInitName);
        ENFORCE(staticInitFunc, "{} does not exist", staticInitName);
        builder.CreateCall(staticInitFunc,
                           {
                               llvm::ConstantInt::get(cs, llvm::APInt(32, 0, true)),
                               llvm::ConstantPointerNull::get(llvm::Type::getInt64PtrTy(cs)),
                               Payload::rubyTopSelf(cs, builder),
                               builder.CreateCall(cs.getFunction("sorbet_getCFP"), {}, "cfpTop"),
                               llvm::ConstantPointerNull::get(llvm::Type::getInt8PtrTy(cs)),
                               llvm::ConstantPointerNull::get(llvm::Type::getInt8PtrTy(cs)),
                           },
                           staticInitName);
    }

    builder.CreateRetVoid();

    if (debug_mode && llvm::verifyFunction(*entryFunc, &llvm::errs())) {
        fmt::print("failed to verify:\n");
        entryFunc->dump();
        ENFORCE(false);
    }
    cs.runCheapOptimizations(entryFunc);
}

} // namespace sorbet::compiler
