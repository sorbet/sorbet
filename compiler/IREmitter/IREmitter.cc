// These violate our poisons so have to happen first
#include "llvm/IR/Attributes.h"
#include "llvm/IR/DIBuilder.h"
#include "llvm/IR/DerivedTypes.h" // FunctionType, StructType
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Verifier.h"

#include "absl/base/casts.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "cfg/CFG.h"
#include "common/FileOps.h"
#include "common/Timer.h"
#include "common/sort.h"
#include "common/typecase.h"
#include "compiler/Core/CompilerState.h"
#include "compiler/Errors/Errors.h"
#include "compiler/IREmitter/IREmitter.h"
#include "compiler/IREmitter/IREmitterContext.h"
#include "compiler/IREmitter/IREmitterHelpers.h"
#include "compiler/IREmitter/Payload.h"
#include "compiler/Names/Names.h"
#include <string_view>

using namespace std;
namespace sorbet::compiler {

namespace {

vector<core::ArgInfo::ArgFlags> getArgFlagsForBlockId(CompilerState &cs, int blockId, core::SymbolRef method,
                                                      const IREmitterContext &irctx) {
    auto ty = irctx.rubyBlockType[blockId];
    ENFORCE(ty == FunctionType::Block || ty == FunctionType::TopLevel);

    if (ty == FunctionType::Block) {
        auto blockLink = irctx.blockLinks[blockId];
        return blockLink->argFlags;
    }

    vector<core::ArgInfo::ArgFlags> res;
    for (auto &argInfo : method.data(cs)->arguments()) {
        res.emplace_back(argInfo.flags);
    }

    return res;
}

void setupStackFrame(CompilerState &cs, const ast::MethodDef &md, const IREmitterContext &irctx,
                     llvm::IRBuilderBase &build, int rubyBlockId) {
    llvm::IRBuilder<> builder = static_cast<llvm::IRBuilder<> &>(build);

    switch (irctx.rubyBlockType[rubyBlockId]) {
        case FunctionType::TopLevel:
        case FunctionType::Block:
        case FunctionType::Rescue:
        case FunctionType::Ensure: {
            // Switch the current control frame from a C frame to a Ruby-esque one
            auto [pc, iseq_encoded] = Payload::setRubyStackFrame(cs, builder, irctx, md, rubyBlockId);
            builder.CreateStore(pc, irctx.lineNumberPtrsByFunction[rubyBlockId]);
            builder.CreateStore(iseq_encoded, irctx.iseqEncodedPtrsByFunction[rubyBlockId]);
            break;
        }

        case FunctionType::ExceptionBegin: {
            // Exception functions get their pc and iseq_encoded values as arguments
            auto func = irctx.rubyBlocks2Functions[rubyBlockId];
            auto *pc = func->arg_begin();
            auto *iseq_encoded = func->arg_begin() + 1;
            builder.CreateStore(pc, irctx.lineNumberPtrsByFunction[rubyBlockId]);
            builder.CreateStore(iseq_encoded, irctx.iseqEncodedPtrsByFunction[rubyBlockId]);
            break;
        }

        case FunctionType::Unused:
            break;
    }
}

} // namespace

void setupStackFrames(CompilerState &base, const ast::MethodDef &md, const IREmitterContext &irctx) {
    llvm::IRBuilder<> builder(base);
    for (auto rubyBlockId = 0; rubyBlockId < irctx.rubyBlocks2Functions.size(); rubyBlockId++) {
        auto cs = base.withFunctionEntry(irctx.functionInitializersByFunction[rubyBlockId]);

        builder.SetInsertPoint(irctx.functionInitializersByFunction[rubyBlockId]);
        setupStackFrame(cs, md, irctx, builder, rubyBlockId);
        auto lastLoc = core::Loc::none();
        Payload::setLineNumber(cs, builder, core::Loc(cs.file, md.loc), md.symbol, lastLoc,
                               irctx.iseqEncodedPtrsByFunction[rubyBlockId],
                               irctx.lineNumberPtrsByFunction[rubyBlockId]);
    }
}

void setupArguments(CompilerState &base, cfg::CFG &cfg, const ast::MethodDef &md, const IREmitterContext &irctx) {
    // this function effectively generate an optimized build of
    // https://github.com/ruby/ruby/blob/59c3b1c9c843fcd2d30393791fe224e5789d1677/include/ruby/ruby.h#L2522-L2675
    llvm::IRBuilder<> builder(base);
    for (auto rubyBlockId = 0; rubyBlockId < irctx.rubyBlocks2Functions.size(); rubyBlockId++) {
        auto cs = base.withFunctionEntry(irctx.functionInitializersByFunction[rubyBlockId]);

        builder.SetInsertPoint(irctx.argumentSetupBlocksByFunction[rubyBlockId]);

        // emit a location that corresponds to the function entry
        auto loc = md.symbol.data(cs)->loc();
        IREmitterHelpers::emitDebugLoc(cs, builder, irctx, rubyBlockId, loc);

        auto blockType = irctx.rubyBlockType[rubyBlockId];
        if (blockType == FunctionType::TopLevel || blockType == FunctionType::Block) {
            auto func = irctx.rubyBlocks2Functions[rubyBlockId];
            auto maxPositionalArgCount = 0;
            auto minPositionalArgCount = 0;
            auto isBlock = blockType == FunctionType::Block;
            auto hasRestArgs = false;
            auto hasKWArgs = false;
            auto hasKWRestArgs = false;
            llvm::Value *argCountRaw = !isBlock ? func->arg_begin() : func->arg_begin() + 2;
            llvm::Value *argArrayRaw = !isBlock ? func->arg_begin() + 1 : func->arg_begin() + 3;
            llvm::Value *hashArgs;

            core::LocalVariable blkArgName;
            core::LocalVariable restArgName;
            core::LocalVariable kwRestArgName;

            auto argsFlags = getArgFlagsForBlockId(cs, rubyBlockId, cfg.symbol, irctx);
            {
                auto argId = -1;
                ENFORCE(argsFlags.size() == irctx.rubyBlockArgs[rubyBlockId].size());
                for (auto &argFlags : argsFlags) {
                    argId += 1;
                    if (argFlags.isKeyword) {
                        hasKWArgs = true;
                        if (argFlags.isRepeated) {
                            kwRestArgName = irctx.rubyBlockArgs[rubyBlockId][argId];
                            hasKWRestArgs = true;
                        }
                        continue;
                    }
                    if (argFlags.isRepeated) {
                        restArgName = irctx.rubyBlockArgs[rubyBlockId][argId];
                        hasRestArgs = true;
                        continue;
                    }
                    if (argFlags.isDefault) {
                        maxPositionalArgCount += 1;
                        continue;
                    }
                    if (argFlags.isBlock) {
                        blkArgName = irctx.rubyBlockArgs[rubyBlockId][argId];
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
                auto hasEnoughArgs = llvm::BasicBlock::Create(cs, "readKWHashArgCountSuccess", func);
                auto hasPassedHash = llvm::BasicBlock::Create(cs, "readKWHash", func);
                auto afterHash = llvm::BasicBlock::Create(cs, "afterKWHash", func);

                // Check that there are enough arguments to fill out minPositionalArgCount
                // https://github.com/ruby/ruby/blob/59c3b1c9c843fcd2d30393791fe224e5789d1677/include/ruby/ruby.h#L2547
                auto argSizeForHashCheck =
                    builder.CreateICmpULT(llvm::ConstantInt::get(cs, llvm::APInt(32, minPositionalArgCount)),
                                          argCountRaw, "hashAttemptReadGuard");
                builder.CreateCondBr(argSizeForHashCheck, hasEnoughArgs, afterHash);

                auto sizeTestFailedEnd = builder.GetInsertBlock();
                builder.SetInsertPoint(hasEnoughArgs);
                llvm::Value *argsWithoutHashCount = builder.CreateSub(
                    argCountRaw, llvm::ConstantInt::get(cs, llvm::APInt(32, 1)), "argsWithoutHashCount");

                llvm::Value *indices[] = {argsWithoutHashCount};

                auto maybeHashValue = builder.CreateLoad(builder.CreateGEP(argArrayRaw, indices), "KWArgHash");

                // checkIfLastArgIsHash
                auto isHashValue = Payload::typeTest(cs, builder, maybeHashValue,
                                                     core::make_type<core::ClassType>(core::Symbols::Hash()));

                builder.CreateCondBr(isHashValue, hasPassedHash, afterHash);

                auto hashTypeFailedTestEnd = builder.GetInsertBlock();
                builder.SetInsertPoint(hasPassedHash);
                // yes, this is an empty block. It's used only for Phi node
                auto hasPassedHashEnd = builder.GetInsertBlock();
                builder.CreateBr(afterHash);
                builder.SetInsertPoint(afterHash);
                auto hashArgsPhi = builder.CreatePHI(builder.getInt64Ty(), 3, "hashArgsPhi");
                auto argcPhi = builder.CreatePHI(builder.getInt32Ty(), 3, "argcPhi");
                argcPhi->addIncoming(argCountRaw, sizeTestFailedEnd);
                argcPhi->addIncoming(argCountRaw, hashTypeFailedTestEnd);
                hashArgsPhi->addIncoming(hashArgs, sizeTestFailedEnd);
                hashArgsPhi->addIncoming(hashArgs, hashTypeFailedTestEnd);
                argcPhi->addIncoming(argsWithoutHashCount, hasPassedHashEnd);
                hashArgsPhi->addIncoming(maybeHashValue, hasPassedHashEnd);

                argCountRaw = argcPhi;
                hashArgs = hashArgsPhi;
            }

            if (isBlock) {
                if (minPositionalArgCount != 1) {
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
                    auto isArray = Payload::typeTest(cs, builder, rawArg1Value,
                                                     core::make_type<core::ClassType>(core::Symbols::Array()));
                    auto typeTestEnd = builder.GetInsertBlock();

                    builder.CreateCondBr(isArray, argExpandBlock, afterArgArrayExpandBlock);
                    builder.SetInsertPoint(argExpandBlock);
                    auto newArgArray = builder.CreateCall(cs.module->getFunction("sorbet_rubyArrayInnerPtr"),
                                                          {rawArg1Value}, "expandedArgArray");
                    auto newArgc = builder.CreateCall(cs.module->getFunction("sorbet_rubyArrayLen"), {rawArg1Value},
                                                      "expandedArgc");
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
                for (auto i = 0; i < numOptionalArgs + 1; i++) {
                    auto suffix = i == numOptionalArgs ? "Done" : to_string(i);
                    checkBlocks.emplace_back(llvm::BasicBlock::Create(cs, {"checkBlock", suffix}, func));
                    fillFromDefaultBlocks.emplace_back(
                        llvm::BasicBlock::Create(cs, {"fillFromDefaultBlock", suffix}, func));
                    // Don't bother making the "Done" block for fillFromArgBlocks
                    if (i < numOptionalArgs) {
                        fillFromArgBlocks.emplace_back(
                            llvm::BasicBlock::Create(cs, {"fillFromArgBlock", suffix}, func));
                    }
                }
            }
            {
                // fill local variables from args
                auto fillRequiredArgs = llvm::BasicBlock::Create(cs, "fillRequiredArgs", func);
                builder.CreateBr(fillRequiredArgs);
                builder.SetInsertPoint(fillRequiredArgs);

                // box `self`
                if (!isBlock) {
                    auto selfArgRaw = func->arg_begin() + 2;
                    Payload::varSet(cs, core::LocalVariable::selfVariable(), selfArgRaw, builder, irctx, rubyBlockId);
                }

                for (auto i = 0; i < maxPositionalArgCount; i++) {
                    if (i >= minPositionalArgCount) {
                        // if these are optional, put them in their own BasicBlock
                        // because we might not run it
                        auto &block = fillFromArgBlocks[i - minPositionalArgCount];
                        builder.SetInsertPoint(block);
                    }
                    const auto a = irctx.rubyBlockArgs[rubyBlockId][i];
                    if (!a._name.exists()) {
                        cs.failCompilation(md.declLoc,
                                           "this method has a block argument construct that's not supported");
                    }

                    llvm::Value *indices[] = {llvm::ConstantInt::get(cs, llvm::APInt(32, i, true))};
                    auto name = a._name.data(cs)->shortName(cs);
                    llvm::StringRef nameRef(name.data(), name.length());
                    auto rawValue = builder.CreateLoad(builder.CreateGEP(argArrayRaw, indices), {"rawArg_", nameRef});
                    Payload::varSet(cs, a, rawValue, builder, irctx, rubyBlockId);
                    if (i >= minPositionalArgCount) {
                        // check if we need to fill in the next variable from the arg
                        builder.CreateBr(checkBlocks[i - minPositionalArgCount + 1]);
                    }
                }

                // make the last instruction in all the required args point at the first check block
                builder.SetInsertPoint(fillRequiredArgs);
                //
                if (blkArgName.exists() && irctx.usesBlockArgs) {
                    // TODO: I don't think this correctly handles blocks with block args
                    Payload::varSet(cs, blkArgName,
                                    builder.CreateCall(cs.module->getFunction("sorbet_getMethodBlockAsProc")), builder,
                                    irctx, 0);
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
            auto optionalMethodIndex = 0;
            {
                // build fillFromDefaultBlocks
                for (auto i = 0; i < numOptionalArgs; i++) {
                    auto &block = fillFromDefaultBlocks[i];
                    builder.SetInsertPoint(block);
                    if (!isBlock && md.name.data(cs)->kind == core::NameKind::UNIQUE &&
                        md.name.data(cs)->unique.uniqueNameKind == core::UniqueNameKind::DefaultArg) {
                        // This method is already a default method so don't fill in
                        // another other defaults for it or else it is turtles all the
                        // way down
                    } else {
                        llvm::Value *rawValue;
                        if (!isBlock) {
                            optionalMethodIndex++;
                            auto argMethodName =
                                cs.gs.lookupNameUnique(core::UniqueNameKind::DefaultArg, md.name, optionalMethodIndex);
                            ENFORCE(argMethodName.exists(), "Default argument method for " + md.name.toString(cs) +
                                                                to_string(optionalMethodIndex) + " does not exist");
                            auto argMethod = md.symbol.data(cs)->owner.data(cs)->findMember(cs, argMethodName);
                            ENFORCE(argMethod.exists());
                            auto fillDefaultFunc = IREmitterHelpers::getOrCreateFunction(cs, argMethod);
                            rawValue = builder.CreateCall(
                                fillDefaultFunc,
                                {argCountRaw, argArrayRaw, func->arg_begin() + 2 /* this is wrong for block*/});
                        } else {
                            rawValue = Payload::rubyNil(cs, builder);
                        }
                        auto argIndex = i + minPositionalArgCount;
                        auto a = irctx.rubyBlockArgs[rubyBlockId][argIndex];

                        Payload::varSet(cs, a, rawValue, builder, irctx, rubyBlockId);
                    }
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
                                    builder, irctx, rubyBlockId);
                }
                if (hasKWArgs) {
                    for (int argId = maxPositionalArgCount; argId < argsFlags.size(); argId++) {
                        if (argsFlags[argId].isKeyword && !argsFlags[argId].isRepeated) {
                            auto name = irctx.rubyBlockArgs[rubyBlockId][argId];
                            auto rawId = Payload::idIntern(cs, builder, name._name.data(cs)->shortName(cs));
                            auto rawRubySym =
                                builder.CreateCall(cs.module->getFunction("rb_id2sym"), {rawId}, "rawSym");

                            auto passedValue = Payload::getKWArg(cs, builder, hashArgs, rawRubySym);
                            auto isItUndef = Payload::testIsUndef(cs, builder, passedValue);

                            auto kwArgDefault = llvm::BasicBlock::Create(cs, "kwArgDefault", func);
                            auto kwArgContinue = llvm::BasicBlock::Create(cs, "kwArgContinue", func);
                            auto isUndefEnd = builder.GetInsertBlock();
                            builder.CreateCondBr(isItUndef, kwArgDefault, kwArgContinue);
                            builder.SetInsertPoint(kwArgDefault);
                            llvm::Value *defaultValue;
                            if ((md.name.data(cs)->kind == core::NameKind::UNIQUE &&
                                 md.name.data(cs)->unique.uniqueNameKind == core::UniqueNameKind::DefaultArg) ||
                                !argsFlags[argId].isDefault) {
                                // This method is already a default method so don't fill in
                                // another other defaults for it or else it is turtles all the
                                // way down

                                defaultValue = Payload::rubyNil(cs, builder);
                            } else {
                                optionalMethodIndex++;
                                auto argMethodName = cs.gs.lookupNameUnique(core::UniqueNameKind::DefaultArg, md.name,
                                                                            optionalMethodIndex);
                                ENFORCE(argMethodName.exists(), "Default argument method for " + md.name.toString(cs) +
                                                                    to_string(optionalMethodIndex) + " does not exist");
                                auto argMethod = md.symbol.data(cs)->owner.data(cs)->findMember(cs, argMethodName);
                                ENFORCE(argMethod.exists());
                                auto fillDefaultFunc = IREmitterHelpers::getOrCreateFunction(cs, argMethod);
                                defaultValue = builder.CreateCall(
                                    fillDefaultFunc,
                                    {argCountRaw, argArrayRaw, func->arg_begin() + 2 /* this is wrong for block*/});
                                // insert default computation
                            }
                            auto kwArgDefaultEnd = builder.GetInsertBlock();
                            builder.CreateBr(kwArgContinue);
                            builder.SetInsertPoint(kwArgContinue);

                            auto kwArgValue = builder.CreatePHI(builder.getInt64Ty(), 2, "kwArgValue");
                            kwArgValue->addIncoming(passedValue, isUndefEnd);
                            kwArgValue->addIncoming(defaultValue, kwArgDefaultEnd);

                            Payload::varSet(cs, name, kwArgValue, builder, irctx, rubyBlockId);
                        }
                    }
                    if (hasKWRestArgs) {
                        Payload::varSet(cs, kwRestArgName, Payload::readKWRestArg(cs, builder, hashArgs), builder,
                                        irctx, rubyBlockId);
                    } else {
                        Payload::assertNoExtraKWArg(cs, builder, hashArgs);
                    }
                }
            }
        }

        switch (blockType) {
            case FunctionType::TopLevel:
                // jump to sig verification that will come before user body
                builder.CreateBr(irctx.sigVerificationBlock);
                break;

            case FunctionType::Block:
            case FunctionType::ExceptionBegin:
            case FunctionType::Rescue:
            case FunctionType::Ensure:
                // jump dirrectly to user body
                builder.CreateBr(irctx.userEntryBlockByFunction[rubyBlockId]);
                break;

            case FunctionType::Unused:
                // this function will never be called
                builder.CreateUnreachable();
                break;
        }
    }
}

core::LocalVariable returnValue(CompilerState &cs) {
    return {Names::returnValue(cs), 1};
}

llvm::BasicBlock *resolveJumpTarget(cfg::CFG &cfg, const IREmitterContext &irctx, const cfg::BasicBlock *from,
                                    const cfg::BasicBlock *to) {
    if (to == cfg.deadBlock()) {
        return irctx.deadBlockMapping[from->rubyBlockId];
    }

    auto remapped = irctx.basicBlockJumpOverrides[to->id];
    if (from->rubyBlockId != irctx.basicBlockRubyBlockId[remapped]) {
        return irctx.blockExitMapping[from->rubyBlockId];
    } else {
        return irctx.llvmBlocksBySorbetBlocks[irctx.basicBlockJumpOverrides[to->id]];
    }
}

void emitUserBody(CompilerState &base, cfg::CFG &cfg, const IREmitterContext &irctx) {
    llvm::IRBuilder<> builder(base);
    UnorderedSet<core::LocalVariable> loadYieldParamsResults; // methods calls on these are ignored
    for (auto it = cfg.forwardsTopoSort.rbegin(); it != cfg.forwardsTopoSort.rend(); ++it) {
        cfg::BasicBlock *bb = *it;
        auto cs = base.withFunctionEntry(irctx.functionInitializersByFunction[bb->rubyBlockId]);

        auto block = irctx.llvmBlocksBySorbetBlocks[bb->id];
        bool isTerminated = false;

        builder.SetInsertPoint(block);

        // NOTE: explicitly clearing debug location information here so that we don't accidentally inherit the location
        // information from blocks in different target functions.
        builder.SetCurrentDebugLocation(llvm::DebugLoc());

        core::Loc lastLoc;
        if (bb != cfg.deadBlock()) {
            for (cfg::Binding &bind : bb->exprs) {
                auto loc = core::Loc(cs.file, bind.loc);

                lastLoc = Payload::setLineNumber(cs, builder, loc, cfg.symbol, lastLoc,
                                                 irctx.iseqEncodedPtrsByFunction[bb->rubyBlockId],
                                                 irctx.lineNumberPtrsByFunction[bb->rubyBlockId]);

                IREmitterHelpers::emitDebugLoc(cs, builder, irctx, bb->rubyBlockId, loc);

                typecase(
                    bind.value.get(),
                    [&](cfg::Ident *i) {
                        auto var = Payload::varGet(cs, i->what, builder, irctx, bb->rubyBlockId);
                        Payload::varSet(cs, bind.bind.variable, var, builder, irctx, bb->rubyBlockId);
                    },
                    [&](cfg::Alias *i) {
                        // We compute the alias map when IREmitterContext is first created, so if an entry is missing,
                        // there's a problem.
                        ENFORCE(irctx.aliases.find(bind.bind.variable) != irctx.aliases.end(),
                                "Alias is missing from the alias map");
                    },
                    [&](cfg::SolveConstraint *i) {
                        auto var = Payload::varGet(cs, i->send, builder, irctx, bb->rubyBlockId);
                        Payload::varSet(cs, bind.bind.variable, var, builder, irctx, bb->rubyBlockId);
                    },
                    [&](cfg::Send *i) {
                        if (i->recv.variable._name == core::Names::blkArg() &&
                            loadYieldParamsResults.contains(i->recv.variable)) {
                            // this loads an argument of a block.
                            // They are already loaded in preambula of the method
                            return;
                        }

                        auto rawCall = IREmitterHelpers::emitMethodCall(cs, builder, i, irctx, bb->rubyBlockId);
                        Payload::varSet(cs, bind.bind.variable, rawCall, builder, irctx, bb->rubyBlockId);
                    },
                    [&](cfg::Return *i) {
                        isTerminated = true;
                        auto *var = Payload::varGet(cs, i->what.variable, builder, irctx, bb->rubyBlockId);
                        switch (irctx.rubyBlockType[bb->rubyBlockId]) {
                            case FunctionType::TopLevel: {
                                Payload::varSet(cs, returnValue(cs), var, builder, irctx, bb->rubyBlockId);
                                builder.CreateBr(irctx.postProcessBlock);
                                break;
                            }

                            case FunctionType::Block:
                                // NOTE: this doesn't catch all block-return cases:
                                // https://github.com/stripe/sorbet_llvm/issues/94
                                cs.failCompilation(core::Loc(cs.file, bind.loc),
                                                   "returns through multiple stacks not implemented");
                                break;

                            case FunctionType::ExceptionBegin:
                            case FunctionType::Rescue:
                            case FunctionType::Ensure:
                            case FunctionType::Unused:
                                IREmitterHelpers::emitReturn(cs, builder, irctx, bb->rubyBlockId, var);
                                break;
                        }
                    },
                    [&](cfg::BlockReturn *i) {
                        ENFORCE(bb->rubyBlockId != 0, "should never happen");
                        isTerminated = true;
                        auto var = Payload::varGet(cs, i->what.variable, builder, irctx, bb->rubyBlockId);
                        IREmitterHelpers::emitReturn(cs, builder, irctx, bb->rubyBlockId, var);
                    },
                    [&](cfg::LoadSelf *i) {
                        // it's done in function setup, no need to do anything here
                    },
                    [&](cfg::Literal *i) {
                        if (i->value->derivesFrom(cs, core::Symbols::FalseClass())) {
                            Payload::varSet(cs, bind.bind.variable, Payload::rubyFalse(cs, builder), builder, irctx,
                                            bb->rubyBlockId);
                            return;
                        }
                        if (i->value->derivesFrom(cs, core::Symbols::TrueClass())) {
                            Payload::varSet(cs, bind.bind.variable, Payload::rubyTrue(cs, builder), builder, irctx,
                                            bb->rubyBlockId);
                            return;
                        }
                        if (i->value->derivesFrom(cs, core::Symbols::NilClass())) {
                            Payload::varSet(cs, bind.bind.variable, Payload::rubyNil(cs, builder), builder, irctx,
                                            bb->rubyBlockId);
                            return;
                        }

                        auto litType = core::cast_type<core::LiteralType>(i->value.get());
                        ENFORCE(litType);
                        switch (litType->literalKind) {
                            case core::LiteralType::LiteralTypeKind::Integer: {
                                auto rawInt = Payload::longToRubyValue(cs, builder, litType->value);
                                Payload::varSet(cs, bind.bind.variable, rawInt, builder, irctx, bb->rubyBlockId);
                                break;
                            }
                            case core::LiteralType::LiteralTypeKind::Float: {
                                auto rawInt =
                                    Payload::doubleToRubyValue(cs, builder, absl::bit_cast<double>(litType->value));
                                Payload::varSet(cs, bind.bind.variable, rawInt, builder, irctx, bb->rubyBlockId);
                                break;
                            }
                            case core::LiteralType::LiteralTypeKind::Symbol: {
                                auto str = core::NameRef(cs, litType->value).data(cs)->shortName(cs);
                                auto rawId = Payload::idIntern(cs, builder, str);
                                auto rawRubySym =
                                    builder.CreateCall(cs.module->getFunction("rb_id2sym"), {rawId}, "rawSym");
                                Payload::varSet(cs, bind.bind.variable, rawRubySym, builder, irctx, bb->rubyBlockId);
                                break;
                            }
                            case core::LiteralType::LiteralTypeKind::String: {
                                auto str = core::NameRef(cs, litType->value).data(cs)->shortName(cs);
                                auto rawRubyString = Payload::cPtrToRubyString(cs, builder, str, true);
                                Payload::varSet(cs, bind.bind.variable, rawRubyString, builder, irctx, bb->rubyBlockId);
                                break;
                            }
                            default:
                                cs.failCompilation(core::Loc(cs.file, bind.loc), "UnsupportedLiteral");
                        }
                    },
                    [&](cfg::GetCurrentException *i) {
                        // if this block isn't an exception block header, there's nothing to do here.
                        auto bodyRubyBlockId = irctx.exceptionBlockHeader[bb->id];
                        if (bodyRubyBlockId == 0) {
                            return;
                        }

                        IREmitterHelpers::emitExceptionHandlers(cs, builder, irctx, bb->rubyBlockId, bodyRubyBlockId,
                                                                bind.bind.variable);
                    },
                    [&](cfg::LoadArg *i) {
                        /* intentionally omitted, it's part of method preambula */
                    },
                    [&](cfg::LoadYieldParams *i) {
                        loadYieldParamsResults.insert(bind.bind.variable);
                        /* intentionally omitted, it's part of method preambula */
                    },
                    [&](cfg::Cast *i) {
                        auto val = Payload::varGet(cs, i->value.variable, builder, irctx, bb->rubyBlockId);
                        auto passedTypeTest = Payload::typeTest(cs, builder, val, bind.bind.type);
                        auto successBlock =
                            llvm::BasicBlock::Create(cs, "typeTestSuccess", builder.GetInsertBlock()->getParent());

                        auto failBlock =
                            llvm::BasicBlock::Create(cs, "typeTestFail", builder.GetInsertBlock()->getParent());

                        auto expected = Payload::setExpectedBool(cs, builder, passedTypeTest, true);
                        builder.CreateCondBr(expected, successBlock, failBlock);
                        builder.SetInsertPoint(failBlock);
                        // this will throw exception
                        builder.CreateCall(cs.module->getFunction("sorbet_cast_failure"),
                                           {val, Payload::toCString(cs, i->cast.data(cs)->shortName(cs), builder),
                                            Payload::toCString(cs, bind.bind.type->show(cs), builder)});
                        builder.CreateUnreachable();
                        builder.SetInsertPoint(successBlock);

                        if (i->cast == core::Names::let() || i->cast == core::Names::cast()) {
                            Payload::varSet(cs, bind.bind.variable, val, builder, irctx, bb->rubyBlockId);
                        } else if (i->cast == core::Names::assertType()) {
                            Payload::varSet(cs, bind.bind.variable, Payload::rubyFalse(cs, builder), builder, irctx,
                                            bb->rubyBlockId);
                        }
                    },
                    [&](cfg::TAbsurd *i) {
                        auto val = Payload::varGet(cs, i->what.variable, builder, irctx, bb->rubyBlockId);
                        builder.CreateCall(cs.module->getFunction("sorbet_t_absurd"), {val});
                    });
                if (isTerminated) {
                    break;
                }
            }
            if (!isTerminated) {
                auto *thenb = resolveJumpTarget(cfg, irctx, bb, bb->bexit.thenb);
                auto *elseb = resolveJumpTarget(cfg, irctx, bb, bb->bexit.elseb);

                if (thenb != elseb && bb->bexit.cond.variable != core::LocalVariable::blockCall()) {
                    auto var = Payload::varGet(cs, bb->bexit.cond.variable, builder, irctx, bb->rubyBlockId);
                    auto condValue = Payload::testIsTruthy(cs, builder, var);

                    builder.CreateCondBr(condValue, thenb, elseb);
                } else {
                    builder.CreateBr(thenb);
                }
            }
        }
    }
}

void emitDeadBlocks(CompilerState &cs, cfg::CFG &cfg, const IREmitterContext &irctx) {
    llvm::IRBuilder<> builder(cs);

    // Emit the dead block body for each ruby block. It should be an error to transition to the dead block, so
    // we mark its body as unreachable.
    for (auto rubyBlockId = 0; rubyBlockId <= cfg.maxRubyBlockId; ++rubyBlockId) {
        auto *dead = irctx.deadBlockMapping[rubyBlockId];
        builder.SetInsertPoint(dead);
        builder.CreateUnreachable();
    }
}

void emitBlockExits(CompilerState &base, cfg::CFG &cfg, const IREmitterContext &irctx) {
    llvm::IRBuilder<> builder(base);

    for (auto rubyBlockId = 0; rubyBlockId <= cfg.maxRubyBlockId; ++rubyBlockId) {
        auto cs = base.withFunctionEntry(irctx.functionInitializersByFunction[rubyBlockId]);

        builder.SetInsertPoint(irctx.blockExitMapping[rubyBlockId]);

        switch (irctx.rubyBlockType[rubyBlockId]) {
            case FunctionType::TopLevel:
                builder.CreateUnreachable();
                break;

            case FunctionType::Block:
            case FunctionType::ExceptionBegin:
            case FunctionType::Rescue:
            case FunctionType::Ensure:
            case FunctionType::Unused:
                // for non-top-level functions, we return `Qundef` to indicate that this value isn't used for anything.
                IREmitterHelpers::emitReturn(cs, builder, irctx, rubyBlockId, Payload::rubyUndef(cs, builder));
                break;
        }
    }
}

void emitPostProcess(CompilerState &cs, cfg::CFG &cfg, const IREmitterContext &irctx) {
    llvm::IRBuilder<> builder(cs);
    builder.SetInsertPoint(irctx.postProcessBlock);

    // we're only using the top-level ruby block at this point
    auto rubyBlockId = 0;

    auto var = Payload::varGet(cs, returnValue(cs), builder, irctx, rubyBlockId);
    auto expectedType = cfg.symbol.data(cs)->resultType;
    if (expectedType == nullptr) {
        IREmitterHelpers::emitReturn(cs, builder, irctx, rubyBlockId, var);
        return;
    }
    auto ct = core::cast_type<core::ClassType>(expectedType.get());
    if (ct != nullptr && ct->symbol == core::Symbols::void_()) {
        auto void_ = Payload::getRubyConstant(cs, core::Symbols::T_Private_Types_Void_VOIDSingleton(), builder);
        IREmitterHelpers::emitReturn(cs, builder, irctx, rubyBlockId, void_);
        return;
    }
    auto passedTypeTest = Payload::typeTest(cs, builder, var, expectedType);
    auto successBlock = llvm::BasicBlock::Create(cs, "typeTestSuccess", builder.GetInsertBlock()->getParent());

    auto failBlock = llvm::BasicBlock::Create(cs, "typeTestFail", builder.GetInsertBlock()->getParent());

    auto expected = Payload::setExpectedBool(cs, builder, passedTypeTest, true);
    builder.CreateCondBr(expected, successBlock, failBlock);
    builder.SetInsertPoint(failBlock);
    // this will throw exception
    builder.CreateCall(cs.module->getFunction("sorbet_cast_failure"),
                       {var, Payload::toCString(cs, "Return value", builder),
                        Payload::toCString(cs, expectedType->show(cs), builder)});
    builder.CreateUnreachable();
    builder.SetInsertPoint(successBlock);

    IREmitterHelpers::emitReturn(cs, builder, irctx, rubyBlockId, var);
}

void emitSigVerification(CompilerState &base, cfg::CFG &cfg, const ast::MethodDef &md, const IREmitterContext &irctx) {
    auto cs = base.withFunctionEntry(irctx.functionInitializersByFunction[0]);
    llvm::IRBuilder<> builder(cs);
    builder.SetInsertPoint(irctx.sigVerificationBlock);

    if (!(md.name.data(cs)->kind == core::NameKind::UNIQUE &&
          md.name.data(cs)->unique.uniqueNameKind == core::UniqueNameKind::DefaultArg)) {
        int argId = -1;
        for (auto &argInfo : cfg.symbol.data(cs)->arguments()) {
            argId += 1;
            auto local = irctx.rubyBlockArgs[0][argId];
            auto var = Payload::varGet(cs, local, builder, irctx, 0);
            auto &expectedType = argInfo.type;
            if (!expectedType) {
                continue;
            }
            auto passedTypeTest = Payload::typeTest(cs, builder, var, expectedType);
            auto successBlock = llvm::BasicBlock::Create(cs, "typeTestSuccess", builder.GetInsertBlock()->getParent());

            auto failBlock = llvm::BasicBlock::Create(cs, "typeTestFail", builder.GetInsertBlock()->getParent());

            auto expected = Payload::setExpectedBool(cs, builder, passedTypeTest, true);
            builder.CreateCondBr(expected, successBlock, failBlock);
            builder.SetInsertPoint(failBlock);
            // this will throw exception
            builder.CreateCall(
                cs.module->getFunction("sorbet_cast_failure"),
                {var, Payload::toCString(cs, "sig", builder), Payload::toCString(cs, expectedType->show(cs), builder)});
            builder.CreateUnreachable();
            builder.SetInsertPoint(successBlock);
        }
    }

    builder.CreateBr(irctx.userEntryBlockByFunction[0]);
}

void IREmitter::run(CompilerState &cs, cfg::CFG &cfg, const ast::MethodDef &md) {
    Timer timer(cs.gs.tracer(), "IREmitter::run");

    llvm::Function *func;

    if (md.symbol.data(cs)->name != core::Names::staticInit()) {
        func = IREmitterHelpers::getOrCreateFunction(cs, md.symbol);
    } else {
        func = IREmitterHelpers::getOrCreateStaticInit(cs, md.symbol, md.declLoc);
    }
    func = IREmitterHelpers::cleanFunctionBody(cs, func);
    {
        // setup function argument names
        func->arg_begin()->setName("argc");
        (func->arg_begin() + 1)->setName("argArray");
        (func->arg_begin() + 2)->setName("selfRaw");
    }
    func->addFnAttr(llvm::Attribute::AttrKind::StackProtectReq);
    func->addFnAttr(llvm::Attribute::AttrKind::NoUnwind);
    func->addFnAttr(llvm::Attribute::AttrKind::UWTable);
    llvm::IRBuilder<> builder(cs);

    const IREmitterContext irctx = IREmitterHelpers::getSorbetBlocks2LLVMBlockMapping(cs, cfg, md, func);

    ENFORCE(cs.functionEntryInitializers == nullptr, "modules shouldn't be reused");

    setupStackFrames(cs, md, irctx);
    setupArguments(cs, cfg, md, irctx);
    emitSigVerification(cs, cfg, md, irctx);

    emitUserBody(cs, cfg, irctx);
    emitDeadBlocks(cs, cfg, irctx);
    emitBlockExits(cs, cfg, irctx);
    emitPostProcess(cs, cfg, irctx);
    for (int funId = 0; funId < irctx.functionInitializersByFunction.size(); funId++) {
        builder.SetInsertPoint(irctx.functionInitializersByFunction[funId]);
        builder.CreateBr(irctx.argumentSetupBlocksByFunction[funId]);
    }

    cs.debug->finalize();

    /* run verifier */
    if (debug_mode && llvm::verifyFunction(*func, &llvm::errs())) {
        fmt::print("failed to verify:\n");
        func->dump();
        ENFORCE(false);
    }
    cs.runCheapOptimizations(func);
}

void IREmitter::buildInitFor(CompilerState &cs, const core::SymbolRef &sym, string_view objectName) {
    llvm::IRBuilder<> builder(cs);

    auto owner = sym.data(cs)->owner;
    auto isRoot = owner == core::Symbols::rootSingleton();
    llvm::Function *entryFunc;

    if (IREmitterHelpers::isStaticInit(cs, sym)) {
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
        entryFunc = llvm::Function::Create(ft, linkageType, "Init_" + (string)baseName, *cs.module);
    } else {
        entryFunc = IREmitterHelpers::getInitFunction(cs, sym);
    }

    auto bb = llvm::BasicBlock::Create(cs, "entry", entryFunc);
    builder.SetInsertPoint(bb);

    if (IREmitterHelpers::isStaticInit(cs, sym)) {
        // We include sorbet_version.c when compiling sorbet_llvm itself to get the expected version.
        // The actual version will be linked into libruby.so and compared against at runtime.
        auto compileTimeBuildSCMRevision = sorbet_getBuildSCMRevision();
        auto compileTimeIsReleaseBuild = sorbet_getIsReleaseBuild();
        builder.CreateCall(cs.module->getFunction("sorbet_ensureSorbetRuby"),
                           {
                               llvm::ConstantInt::get(cs, llvm::APInt(32, compileTimeIsReleaseBuild, true)),
                               Payload::toCString(cs, compileTimeBuildSCMRevision, builder),
                           });

        auto realpath = builder.CreateCall(cs.module->getFunction("sorbet_readRealpath"), {});
        realpath->setName("realpath");

        builder.CreateCall(cs.module->getFunction("sorbet_globalConstructors"), {realpath});

        core::SymbolRef staticInit = cs.gs.lookupStaticInitForFile(sym.data(cs)->loc());

        // Call the LLVM method that was made by run() from this Init_ method
        auto staticInitName = IREmitterHelpers::getFunctionName(cs, staticInit);
        auto staticInitFunc = cs.module->getFunction(staticInitName);
        ENFORCE(staticInitFunc, staticInitName + " does not exist");
        builder.CreateCall(staticInitFunc,
                           {
                               llvm::ConstantInt::get(cs, llvm::APInt(32, 0, true)),
                               llvm::ConstantPointerNull::get(llvm::Type::getInt64PtrTy(cs)),
                               Payload::rubyTopSelf(cs, builder),
                           },
                           staticInitName);
    }

    builder.CreateRetVoid();

    ENFORCE(!llvm::verifyFunction(*entryFunc, &llvm::errs()), "see above");
    cs.runCheapOptimizations(entryFunc);
}

} // namespace sorbet::compiler
