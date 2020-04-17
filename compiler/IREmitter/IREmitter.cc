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
#include "common/Timer.h"
#include "common/sort.h"
#include "common/typecase.h"
#include "compiler/Core/CompilerState.h"
#include "compiler/Errors/Errors.h"
#include "compiler/IREmitter/BasicBlockMap.h"
#include "compiler/IREmitter/IREmitter.h"
#include "compiler/IREmitter/IREmitterHelpers.h"
#include "compiler/IREmitter/Payload.h"
#include "compiler/Names/Names.h"
#include <string_view>

using namespace std;
namespace sorbet::compiler {

namespace {

vector<core::ArgInfo::ArgFlags> getArgFlagsForBlockId(CompilerState &cs, int blockId, core::SymbolRef method,
                                                      const BasicBlockMap &blockMap) {
    if (blockId != 0) {
        return blockMap.blockLinks[blockId]->argFlags;
    }
    vector<core::ArgInfo::ArgFlags> res;
    for (auto &argInfo : method.data(cs)->arguments()) {
        res.emplace_back(argInfo.flags);
    }

    return res;
}

void setupArguments(CompilerState &cs, cfg::CFG &cfg, unique_ptr<ast::MethodDef> &md, const BasicBlockMap &blockMap,
                    UnorderedMap<core::LocalVariable, Alias> &aliases) {
    // this function effectively generate an optimized build of
    // https://github.com/ruby/ruby/blob/59c3b1c9c843fcd2d30393791fe224e5789d1677/include/ruby/ruby.h#L2522-L2675
    llvm::IRBuilder<> builder(cs);
    for (auto rubyBlockId = 0; rubyBlockId < blockMap.rubyBlocks2Functions.size(); rubyBlockId++) {
        auto func = blockMap.rubyBlocks2Functions[rubyBlockId];
        cs.functionEntryInitializers = blockMap.functionInitializersByFunction[rubyBlockId];
        builder.SetInsertPoint(blockMap.argumentSetupBlocksByFunction[rubyBlockId]);
        auto maxPositionalArgCount = 0;
        auto minPositionalArgCount = 0;
        auto isBlock = rubyBlockId != 0;
        auto hasRestArgs = false;
        auto hasKWArgs = false;
        auto hasKWRestArgs = false;
        llvm::Value *argCountRaw = !isBlock ? func->arg_begin() : func->arg_begin() + 2;
        llvm::Value *argArrayRaw = !isBlock ? func->arg_begin() + 1 : func->arg_begin() + 3;
        llvm::Value *hashArgs;

        core::LocalVariable blkArgName;
        core::LocalVariable restArgName;
        core::LocalVariable kwRestArgName;

        auto argsFlags = getArgFlagsForBlockId(cs, rubyBlockId, cfg.symbol, blockMap);
        {
            auto argId = -1;
            ENFORCE(argsFlags.size() == blockMap.rubyBlockArgs[rubyBlockId].size());
            for (auto &argFlags : argsFlags) {
                argId += 1;
                if (argFlags.isKeyword) {
                    hasKWArgs = true;
                    if (argFlags.isRepeated) {
                        kwRestArgName = blockMap.rubyBlockArgs[rubyBlockId][argId];
                        hasKWRestArgs = true;
                    }
                    continue;
                }
                if (argFlags.isRepeated) {
                    restArgName = blockMap.rubyBlockArgs[rubyBlockId][argId];
                    hasRestArgs = true;
                    continue;
                }
                if (argFlags.isDefault) {
                    maxPositionalArgCount += 1;
                    continue;
                }
                if (argFlags.isBlock) {
                    blkArgName = blockMap.rubyBlockArgs[rubyBlockId][argId];
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
            // checkForArgSize
            auto argSizeForHashCheck = builder.CreateICmpUGE(
                argCountRaw, llvm::ConstantInt::get(cs, llvm::APInt(32, 1)), "hashAttemptReadGuard");
            builder.CreateCondBr(argSizeForHashCheck, hasEnoughArgs, afterHash);

            auto sizeTestFailedEnd = builder.GetInsertBlock();
            builder.SetInsertPoint(hasEnoughArgs);
            llvm::Value *argsWithoutHashCount =
                builder.CreateSub(argCountRaw, llvm::ConstantInt::get(cs, llvm::APInt(32, 1)), "argsWithoutHashCount");

            llvm::Value *indices[] = {argsWithoutHashCount};

            auto maybeHashValue = builder.CreateLoad(builder.CreateGEP(argArrayRaw, indices), "KWArgHash");

            // checkIfLastArgIsHash
            auto isHashValue =
                Payload::typeTest(cs, builder, maybeHashValue, core::make_type<core::ClassType>(core::Symbols::Hash()));

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
                auto newArgc =
                    builder.CreateCall(cs.module->getFunction("sorbet_rubyArrayLen"), {rawArg1Value}, "expandedArgc");
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
                    fillFromArgBlocks.emplace_back(llvm::BasicBlock::Create(cs, {"fillFromArgBlock", suffix}, func));
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
                Payload::varSet(cs, core::LocalVariable::selfVariable(), selfArgRaw, builder, blockMap, aliases,
                                rubyBlockId);
            }

            for (auto i = 0; i < maxPositionalArgCount; i++) {
                if (i >= minPositionalArgCount) {
                    // if these are optional, put them in their own BasicBlock
                    // because we might not run it
                    auto &block = fillFromArgBlocks[i - minPositionalArgCount];
                    builder.SetInsertPoint(block);
                }
                const auto a = blockMap.rubyBlockArgs[rubyBlockId][i];
                if (!a._name.exists()) {
                    cs.failCompilation(md->declLoc, "this method has a block argument construct that's not supported");
                }

                llvm::Value *indices[] = {llvm::ConstantInt::get(cs, llvm::APInt(32, i, true))};
                auto name = a._name.data(cs)->shortName(cs);
                llvm::StringRef nameRef(name.data(), name.length());
                auto rawValue = builder.CreateLoad(builder.CreateGEP(argArrayRaw, indices), {"rawArg_", nameRef});
                Payload::varSet(cs, a, rawValue, builder, blockMap, aliases, rubyBlockId);
                if (i >= minPositionalArgCount) {
                    // check if we need to fill in the next variable from the arg
                    builder.CreateBr(checkBlocks[i - minPositionalArgCount + 1]);
                }
            }

            // make the last instruction in all the required args point at the first check block
            builder.SetInsertPoint(fillRequiredArgs);
            //
            if (blkArgName.exists() && blockMap.usesBlockArgs) {
                // TODO: I don't think this correctly handles blocks with block args
                Payload::varSet(cs, blkArgName,
                                builder.CreateCall(cs.module->getFunction("sorbet_getMethodBlockAsProc")), builder,
                                blockMap, aliases, 0);
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
                if (!isBlock && md->name.data(cs)->kind == core::NameKind::UNIQUE &&
                    md->name.data(cs)->unique.uniqueNameKind == core::UniqueNameKind::DefaultArg) {
                    // This method is already a default method so don't fill in
                    // another other defaults for it or else it is turtles all the
                    // way down
                } else {
                    llvm::Value *rawValue;
                    if (!isBlock) {
                        optionalMethodIndex++;
                        auto argMethodName =
                            cs.gs.lookupNameUnique(core::UniqueNameKind::DefaultArg, md->name, optionalMethodIndex);
                        ENFORCE(argMethodName.exists(), "Default argument method for " + md->name.toString(cs) +
                                                            to_string(optionalMethodIndex) + " does not exist");
                        auto argMethod = md->symbol.data(cs)->owner.data(cs)->findMember(cs, argMethodName);
                        ENFORCE(argMethod.exists());
                        auto fillDefaultFunc = IREmitterHelpers::getOrCreateFunction(cs, argMethod);
                        rawValue =
                            builder.CreateCall(fillDefaultFunc, {argCountRaw, argArrayRaw,
                                                                 func->arg_begin() + 2 /* this is wrong for block*/});
                    } else {
                        rawValue = Payload::rubyNil(cs, builder);
                    }
                    auto argIndex = i + minPositionalArgCount;
                    auto a = blockMap.rubyBlockArgs[rubyBlockId][argIndex];

                    Payload::varSet(cs, a, rawValue, builder, blockMap, aliases, rubyBlockId);
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
                                builder, blockMap, aliases, rubyBlockId);
            }
            if (hasKWArgs) {
                for (int argId = maxPositionalArgCount; argId < argsFlags.size(); argId++) {
                    if (argsFlags[argId].isKeyword && !argsFlags[argId].isRepeated) {
                        auto name = blockMap.rubyBlockArgs[rubyBlockId][argId];
                        auto rawId = Payload::idIntern(cs, builder, name._name.data(cs)->shortName(cs));
                        auto rawRubySym = builder.CreateCall(cs.module->getFunction("rb_id2sym"), {rawId}, "rawSym");

                        auto passedValue = Payload::getKWArg(cs, builder, hashArgs, rawRubySym);
                        auto isItUndef = Payload::testIsUndef(cs, builder, passedValue);

                        auto kwArgDefault = llvm::BasicBlock::Create(cs, "kwArgDefault", func);
                        auto kwArgContinue = llvm::BasicBlock::Create(cs, "kwArgContinue", func);
                        auto isUndefEnd = builder.GetInsertBlock();
                        builder.CreateCondBr(isItUndef, kwArgDefault, kwArgContinue);
                        builder.SetInsertPoint(kwArgDefault);
                        llvm::Value *defaultValue;
                        if ((md->name.data(cs)->kind == core::NameKind::UNIQUE &&
                             md->name.data(cs)->unique.uniqueNameKind == core::UniqueNameKind::DefaultArg) ||
                            !argsFlags[argId].isDefault) {
                            // This method is already a default method so don't fill in
                            // another other defaults for it or else it is turtles all the
                            // way down

                            defaultValue = Payload::rubyNil(cs, builder);
                        } else {
                            optionalMethodIndex++;
                            auto argMethodName =
                                cs.gs.lookupNameUnique(core::UniqueNameKind::DefaultArg, md->name, optionalMethodIndex);
                            ENFORCE(argMethodName.exists(), "Default argument method for " + md->name.toString(cs) +
                                                                to_string(optionalMethodIndex) + " does not exist");
                            auto argMethod = md->symbol.data(cs)->owner.data(cs)->findMember(cs, argMethodName);
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

                        Payload::varSet(cs, name, kwArgValue, builder, blockMap, aliases, rubyBlockId);
                    }
                }
                if (hasKWRestArgs) {
                    Payload::varSet(cs, kwRestArgName, Payload::readKWRestArg(cs, builder, hashArgs), builder, blockMap,
                                    aliases, rubyBlockId);
                } else {
                    Payload::assertNoExtraKWArg(cs, builder, hashArgs);
                }
            }
        }

        {
            // Switch the current control frame from a C frame to a Ruby-esque one
            auto [pc, iseq_encoded] = Payload::setRubyStackFrame(cs, builder, md);
            builder.CreateStore(pc, blockMap.lineNumberPtrsByFunction[rubyBlockId]);
            builder.CreateStore(iseq_encoded, blockMap.iseqEncodedPtrsByFunction[rubyBlockId]);
        }

        if (!isBlock) {
            // jump to sig verification that will come before user body
            builder.CreateBr(blockMap.sigVerificationBlock);
        } else {
            // jump dirrectly to user body
            builder.CreateBr(blockMap.userEntryBlockByFunction[rubyBlockId]);
        }
    }
}

core::LocalVariable returnValue(CompilerState &cs) {
    return {Names::returnValue(cs), 1};
}

void emitUserBody(CompilerState &cs, cfg::CFG &cfg, const BasicBlockMap &blockMap,
                  UnorderedMap<core::LocalVariable, Alias> &aliases) {
    llvm::IRBuilder<> builder(cs);
    UnorderedSet<core::LocalVariable> loadYieldParamsResults; // methods calls on these are ignored
    for (auto it = cfg.forwardsTopoSort.rbegin(); it != cfg.forwardsTopoSort.rend(); ++it) {
        cfg::BasicBlock *bb = *it;
        auto block = blockMap.llvmBlocksBySorbetBlocks[bb->id];
        cs.functionEntryInitializers = blockMap.functionInitializersByFunction[bb->rubyBlockId];
        bool isTerminated = false;
        builder.SetInsertPoint(block);
        core::Loc lastLoc;
        if (bb != cfg.deadBlock()) {
            for (cfg::Binding &bind : bb->exprs) {
                lastLoc = Payload::setLineNumber(cs, builder, core::Loc(cs.file, bind.loc), cfg.symbol, lastLoc,
                                                 blockMap.iseqEncodedPtrsByFunction[bb->rubyBlockId],
                                                 blockMap.lineNumberPtrsByFunction[bb->rubyBlockId]);
                typecase(
                    bind.value.get(),
                    [&](cfg::Ident *i) {
                        auto var = Payload::varGet(cs, i->what, builder, blockMap, aliases, bb->rubyBlockId);
                        Payload::varSet(cs, bind.bind.variable, var, builder, blockMap, aliases, bb->rubyBlockId);
                    },
                    [&](cfg::Alias *i) {
                        if (i->what == core::Symbols::Magic_undeclaredFieldStub()) {
                            auto name = bind.bind.variable._name.data(cs)->shortName(cs);
                            if (name.size() > 2 && name[0] == '@' && name[1] == '@') {
                                aliases[bind.bind.variable] = Alias::forClassField(bind.bind.variable._name);
                            } else if (name.size() > 1 && name[0] == '@') {
                                aliases[bind.bind.variable] = Alias::forInstanceField(bind.bind.variable._name);
                            } else if (name.size() > 1 && name[0] == '$') {
                                aliases[bind.bind.variable] = Alias::forGlobalField(i->what);
                            } else {
                                ENFORCE(stoi((string)name) > 0, "'" + ((string)name) + "' is not a valid global name");
                                aliases[bind.bind.variable] = Alias::forGlobalField(i->what);
                            }
                        } else {
                            // It's currently impossible in Sorbet to declare a global field with a T.let
                            // (they will all be Magic_undeclaredFieldStub)
                            auto name = i->what.data(cs)->name;
                            auto shortName = name.data(cs)->shortName(cs);
                            ENFORCE(!(shortName.size() > 0 && shortName[0] == '$'));

                            if (i->what.data(cs)->isField()) {
                                aliases[bind.bind.variable] = Alias::forInstanceField(name);
                            } else if (i->what.data(cs)->isStaticField()) {
                                if (shortName.size() > 2 && shortName[0] == '@' && shortName[1] == '@') {
                                    aliases[bind.bind.variable] = Alias::forClassField(name);
                                } else {
                                    aliases[bind.bind.variable] = Alias::forConstant(i->what);
                                }
                            } else {
                                aliases[bind.bind.variable] = Alias::forConstant(i->what);
                            }
                        }
                    },
                    [&](cfg::SolveConstraint *i) {
                        auto var = Payload::varGet(cs, i->send, builder, blockMap, aliases, bb->rubyBlockId);
                        Payload::varSet(cs, bind.bind.variable, var, builder, blockMap, aliases, bb->rubyBlockId);
                    },
                    [&](cfg::Send *i) {
                        if (i->recv.variable._name == core::Names::blkArg() &&
                            loadYieldParamsResults.contains(i->recv.variable)) {
                            // this loads an argument of a block.
                            // They are already loaded in preambula of the method
                            return;
                        }

                        auto rawCall =
                            IREmitterHelpers::emitMethodCall(cs, builder, i, blockMap, aliases, bb->rubyBlockId);
                        Payload::varSet(cs, bind.bind.variable, rawCall, builder, blockMap, aliases, bb->rubyBlockId);
                    },
                    [&](cfg::Return *i) {
                        if (bb->rubyBlockId != 0) {
                            cs.failCompilation(core::Loc(cs.file, bind.loc),
                                               "returns through multiple stacks not implemented");
                        }
                        isTerminated = true;
                        auto var = Payload::varGet(cs, i->what.variable, builder, blockMap, aliases, bb->rubyBlockId);
                        Payload::varSet(cs, returnValue(cs), var, builder, blockMap, aliases, bb->rubyBlockId);
                        builder.CreateBr(blockMap.postProcessBlock);
                    },
                    [&](cfg::BlockReturn *i) {
                        ENFORCE(bb->rubyBlockId != 0, "should never happen");
                        isTerminated = true;
                        auto var = Payload::varGet(cs, i->what.variable, builder, blockMap, aliases, bb->rubyBlockId);
                        builder.CreateRet(var);
                    },
                    [&](cfg::LoadSelf *i) {
                        // it's done in function setup, no need to do anything here
                    },
                    [&](cfg::Literal *i) {
                        if (i->value->derivesFrom(cs, core::Symbols::FalseClass())) {
                            Payload::varSet(cs, bind.bind.variable, Payload::rubyFalse(cs, builder), builder, blockMap,
                                            aliases, bb->rubyBlockId);
                            return;
                        }
                        if (i->value->derivesFrom(cs, core::Symbols::TrueClass())) {
                            Payload::varSet(cs, bind.bind.variable, Payload::rubyTrue(cs, builder), builder, blockMap,
                                            aliases, bb->rubyBlockId);
                            return;
                        }
                        if (i->value->derivesFrom(cs, core::Symbols::NilClass())) {
                            Payload::varSet(cs, bind.bind.variable, Payload::rubyNil(cs, builder), builder, blockMap,
                                            aliases, bb->rubyBlockId);
                            return;
                        }

                        auto litType = core::cast_type<core::LiteralType>(i->value.get());
                        ENFORCE(litType);
                        switch (litType->literalKind) {
                            case core::LiteralType::LiteralTypeKind::Integer: {
                                auto rawInt = Payload::longToRubyValue(cs, builder, litType->value);
                                Payload::varSet(cs, bind.bind.variable, rawInt, builder, blockMap, aliases,
                                                bb->rubyBlockId);
                                break;
                            }
                            case core::LiteralType::LiteralTypeKind::Float: {
                                auto rawInt =
                                    Payload::doubleToRubyValue(cs, builder, absl::bit_cast<double>(litType->value));
                                Payload::varSet(cs, bind.bind.variable, rawInt, builder, blockMap, aliases,
                                                bb->rubyBlockId);
                                break;
                            }
                            case core::LiteralType::LiteralTypeKind::Symbol: {
                                auto str = core::NameRef(cs, litType->value).data(cs)->shortName(cs);
                                auto rawId = Payload::idIntern(cs, builder, str);
                                auto rawRubySym =
                                    builder.CreateCall(cs.module->getFunction("rb_id2sym"), {rawId}, "rawSym");
                                Payload::varSet(cs, bind.bind.variable, rawRubySym, builder, blockMap, aliases,
                                                bb->rubyBlockId);
                                break;
                            }
                            case core::LiteralType::LiteralTypeKind::String: {
                                auto str = core::NameRef(cs, litType->value).data(cs)->shortName(cs);
                                auto rawRubyString = Payload::cPtrToRubyString(cs, builder, str, true);
                                Payload::varSet(cs, bind.bind.variable, rawRubyString, builder, blockMap, aliases,
                                                bb->rubyBlockId);
                                break;
                            }
                            default:
                                cs.trace("UnsupportedLiteral");
                        }
                    },
                    [&](cfg::Unanalyzable *i) {
                        cs.failCompilation(core::Loc(cs.file, bind.loc),
                                           "exceptions are not supported in compiled mode");
                    },
                    [&](cfg::LoadArg *i) {
                        /* intentionally omitted, it's part of method preambula */
                    },
                    [&](cfg::LoadYieldParams *i) {
                        loadYieldParamsResults.insert(bind.bind.variable);
                        /* intentionally omitted, it's part of method preambula */
                    },
                    [&](cfg::Cast *i) {
                        auto val = Payload::varGet(cs, i->value.variable, builder, blockMap, aliases, bb->rubyBlockId);
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
                            Payload::varSet(cs, bind.bind.variable, val, builder, blockMap, aliases, bb->rubyBlockId);
                        } else if (i->cast == core::Names::assertType()) {
                            Payload::varSet(cs, bind.bind.variable, Payload::rubyFalse(cs, builder), builder, blockMap,
                                            aliases, bb->rubyBlockId);
                        }
                    },
                    [&](cfg::TAbsurd *i) { cs.trace("TAbsurd\n"); });
                if (isTerminated) {
                    break;
                }
            }
            if (!isTerminated) {
                if (bb->bexit.thenb != bb->bexit.elseb && bb->bexit.cond.variable != core::LocalVariable::blockCall()) {
                    auto var =
                        Payload::varGet(cs, bb->bexit.cond.variable, builder, blockMap, aliases, bb->rubyBlockId);
                    auto condValue = Payload::testIsTruthy(cs, builder, var);

                    builder.CreateCondBr(
                        condValue,
                        blockMap.llvmBlocksBySorbetBlocks[blockMap.basicBlockJumpOverrides[bb->bexit.thenb->id]],
                        blockMap.llvmBlocksBySorbetBlocks[blockMap.basicBlockJumpOverrides[bb->bexit.elseb->id]]);
                } else {
                    builder.CreateBr(
                        blockMap.llvmBlocksBySorbetBlocks[blockMap.basicBlockJumpOverrides[bb->bexit.thenb->id]]);
                }
            }
        } else {
            // handle dead block. TODO: this should throw
            auto var = Payload::rubyNil(cs, builder);
            Payload::varSet(cs, returnValue(cs), var, builder, blockMap, aliases, bb->rubyBlockId);
            builder.CreateBr(blockMap.postProcessBlock);
        }
    }
}

void emitPostProcess(CompilerState &cs, cfg::CFG &cfg, const BasicBlockMap &blockMap,
                     UnorderedMap<core::LocalVariable, Alias> &aliases) {
    llvm::IRBuilder<> builder(cs);
    builder.SetInsertPoint(blockMap.postProcessBlock);
    auto var = Payload::varGet(cs, returnValue(cs), builder, blockMap, aliases, 0);
    auto expectedType = cfg.symbol.data(cs)->resultType;
    if (expectedType == nullptr) {
        builder.CreateRet(var);
        return;
    }
    auto ct = core::cast_type<core::ClassType>(expectedType.get());
    if (ct != nullptr && ct->symbol == core::Symbols::void_()) {
        auto void_ = Payload::getRubyConstant(cs, core::Symbols::T_Private_Types_Void_VOIDSingleton(), builder);
        builder.CreateRet(void_);
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

    builder.CreateRet(var);
}

void emitSigVerification(CompilerState &cs, cfg::CFG &cfg, unique_ptr<ast::MethodDef> &md,
                         const BasicBlockMap &blockMap, const UnorderedMap<core::LocalVariable, Alias> &aliases) {
    cs.functionEntryInitializers = blockMap.functionInitializersByFunction[0];
    llvm::IRBuilder<> builder(cs);
    builder.SetInsertPoint(blockMap.sigVerificationBlock);

    if (!(md->name.data(cs)->kind == core::NameKind::UNIQUE &&
          md->name.data(cs)->unique.uniqueNameKind == core::UniqueNameKind::DefaultArg)) {
        int argId = -1;
        for (auto &argInfo : cfg.symbol.data(cs)->arguments()) {
            argId += 1;
            auto local = blockMap.rubyBlockArgs[0][argId];
            auto var = Payload::varGet(cs, local, builder, blockMap, aliases, 0);
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

    builder.CreateBr(blockMap.userEntryBlockByFunction[0]);
}

} // namespace

void IREmitter::run(CompilerState &cs, cfg::CFG &cfg, unique_ptr<ast::MethodDef> &md) {
    Timer timer(cs.gs.tracer(), "IREmitter::run");

    llvm::Function *func;

    if (md->symbol.data(cs)->name != core::Names::staticInit()) {
        func = IREmitterHelpers::getOrCreateFunction(cs, md->symbol);
    } else {
        func = IREmitterHelpers::getOrCreateStaticInit(cs, md->symbol, md->declLoc);
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

    // Sorbet uses cfg::Alias to link a local variable to a global construct, like an instance variable or a constant.
    //
    // This mapping is essentially, "if you were about to access a local variable corresponding to an alias,
    // this is the thing you should access instead"
    //
    // TODO(jez) Move this into BasicBlockMap
    UnorderedMap<core::LocalVariable, Alias> aliases;

    const BasicBlockMap blockMap = IREmitterHelpers::getSorbetBlocks2LLVMBlockMapping(cs, cfg, md, aliases, func);

    ENFORCE(cs.functionEntryInitializers == nullptr, "modules shouldn't be reused");

    setupArguments(cs, cfg, md, blockMap, aliases);
    emitSigVerification(cs, cfg, md, blockMap, aliases);

    emitUserBody(cs, cfg, blockMap, aliases);
    emitPostProcess(cs, cfg, blockMap, aliases);
    for (int funId = 0; funId < blockMap.functionInitializersByFunction.size(); funId++) {
        builder.SetInsertPoint(blockMap.functionInitializersByFunction[funId]);
        builder.CreateBr(blockMap.argumentSetupBlocksByFunction[funId]);
    }

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
