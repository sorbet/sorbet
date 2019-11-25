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
#include "common/Timer.h"
#include "common/sort.h"
#include "common/typecase.h"
#include "compiler/Errors/Errors.h"
#include "compiler/IREmitter/IREmitter.h"
#include "compiler/IREmitter/IREmitterHelpers.h"
#include "compiler/IREmitter/Payload.h"
#include "compiler/Payload/Payload.h"
#include "compiler/Names/Names.h"
#include <string_view>

using namespace std;
namespace sorbet::compiler {

// https://docs.ruby-lang.org/en/2.6.0/extension_rdoc.html
// and https://silverhammermba.github.io/emberb/c/ are your friends
// use the `demo` module for experiments
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
    for (auto funcId = 0; funcId < blockMap.rubyBlocks2Functions.size(); funcId++) {
        auto func = blockMap.rubyBlocks2Functions[funcId];
        builder.SetInsertPoint(blockMap.argumentSetupBlocksByFunction[funcId]);
        auto maxArgCount = 0;
        auto minArgCount = 0;
        auto isBlock = funcId != 0;
        llvm::Value *argCountRaw = !isBlock ? func->arg_begin() : func->arg_begin() + 2;
        llvm::Value *argArrayRaw = !isBlock ? func->arg_begin() + 1 : func->arg_begin() + 3;

        core::LocalVariable blkArgName;
        {
            auto argId = -1;
            auto args = getArgFlagsForBlockId(cs, funcId, cfg.symbol, blockMap);
            ENFORCE(args.size() == blockMap.rubyBlockArgs[funcId].size());
            for (auto &argFlags : args) {
                argId += 1;
                if (argFlags.isDefault) {
                    maxArgCount += 1;
                    continue;
                }
                if (argFlags.isBlock) {
                    blkArgName = blockMap.rubyBlockArgs[funcId][argId];
                    continue;
                }
                maxArgCount += 1;
                minArgCount += 1;
            }
        }
        if (isBlock) {
            if (minArgCount != 1) {
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
            minArgCount = 0;
            // blocks Can have 0 args always
        }

        auto numOptionalArgs = maxArgCount - minArgCount;
        if (!isBlock) {
            // validate arg count
            auto argCountFailBlock = llvm::BasicBlock::Create(cs, "argCountFailBlock", func);
            auto argCountSecondCheckBlock = llvm::BasicBlock::Create(cs, "argCountSecondCheckBlock", func);
            auto argCountSuccessBlock = llvm::BasicBlock::Create(cs, "argCountSuccess", func);

            auto tooManyArgs = builder.CreateICmpUGT(
                argCountRaw, llvm::ConstantInt::get(cs, llvm::APInt(32, maxArgCount)), "tooManyArgs");
            auto expected1 = Payload::setExpectedBool(cs, builder, tooManyArgs, false);
            builder.CreateCondBr(expected1, argCountFailBlock, argCountSecondCheckBlock);

            builder.SetInsertPoint(argCountSecondCheckBlock);
            auto tooFewArgs = builder.CreateICmpULT(
                argCountRaw, llvm::ConstantInt::get(cs, llvm::APInt(32, minArgCount)), "tooFewArgs");
            auto expected2 = Payload::setExpectedBool(cs, builder, tooFewArgs, false);
            builder.CreateCondBr(expected2, argCountFailBlock, argCountSuccessBlock);

            builder.SetInsertPoint(argCountFailBlock);
            Payload::raiseArity(cs, builder, argCountRaw, minArgCount, maxArgCount);

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
                                funcId);
            }

            for (auto i = 0; i < maxArgCount; i++) {
                if (i >= minArgCount) {
                    // if these are optional, put them in their own BasicBlock
                    // because we might not run it
                    auto &block = fillFromArgBlocks[i - minArgCount];
                    builder.SetInsertPoint(block);
                }
                const auto a = blockMap.rubyBlockArgs[funcId][i];
                if (!a._name.exists()) {
                    cs.failCompilation(md->loc, "this method has a block argument construct that's not supported");
                }

                llvm::Value *indices[] = {llvm::ConstantInt::get(cs, llvm::APInt(32, i, true))};
                auto name = a._name.data(cs)->shortName(cs);
                llvm::StringRef nameRef(name.data(), name.length());
                auto rawValue = builder.CreateLoad(builder.CreateGEP(argArrayRaw, indices), {"rawArg_", nameRef});
                Payload::varSet(cs, a, rawValue, builder, blockMap, aliases, funcId);
                if (i >= minArgCount) {
                    // check if we need to fill in the next variable from the arg
                    builder.CreateBr(checkBlocks[i - minArgCount + 1]);
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
                auto argCount =
                    builder.CreateICmpEQ(argCountRaw, llvm::ConstantInt::get(cs, llvm::APInt(32, i + minArgCount)),
                                         llvm::Twine("default") + llvm::Twine(i));
                auto expected = Payload::setExpectedBool(cs, builder, argCount, false);
                builder.CreateCondBr(expected, fillFromDefaultBlocks[i], fillFromArgBlocks[i]);
            }
        }
        {
            // build fillFromDefaultBlocks
            auto optionalMethodIndex = 0;
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
                    auto argIndex = i + minArgCount;
                    auto a = blockMap.rubyBlockArgs[funcId][argIndex];

                    Payload::varSet(cs, a, rawValue, builder, blockMap, aliases, 0);
                }
                builder.CreateBr(fillFromDefaultBlocks[i + 1]);
            }
        }
        {
            // Tie up all the "Done" blocks at the end
            builder.SetInsertPoint(checkBlocks[numOptionalArgs]);
            builder.CreateBr(fillFromDefaultBlocks[numOptionalArgs]);
            builder.SetInsertPoint(fillFromDefaultBlocks[numOptionalArgs]);
        }

        if (!isBlock) {
            // jump to user body
            builder.CreateBr(blockMap.sigVerificationBlock);
        } else {
            builder.CreateBr(blockMap.userEntryBlockByFunction[funcId]);
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
        if (bb != cfg.deadBlock()) {
            for (cfg::Binding &bind : bb->exprs) {
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
                            aliases[bind.bind.variable] = Alias::forConstant(i->what);
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
                        ENFORCE(bb->rubyBlockId == 0, "returns through multiple stacks not implemented");
                        isTerminated = true;
                        Payload::popControlFrame(cs, builder);
                        auto var = Payload::varGet(cs, i->what.variable, builder, blockMap, aliases, bb->rubyBlockId);
                        Payload::varSet(cs, returnValue(cs), var, builder, blockMap, aliases, bb->rubyBlockId);
                        builder.CreateBr(blockMap.postProcessBlock);
                    },
                    [&](cfg::BlockReturn *i) {
                        ENFORCE(bb->rubyBlockId != 0, "should never happen");
                        isTerminated = true;
                        Payload::popControlFrame(cs, builder);
                        auto var = Payload::varGet(cs, i->what.variable, builder, blockMap, aliases, bb->rubyBlockId);
                        builder.CreateRet(var);
                    },
                    [&](cfg::LoadSelf *i) {
                        auto var = Payload::varGet(cs, i->fallback, builder, blockMap, aliases, bb->rubyBlockId);
                        Payload::varSet(cs, bind.bind.variable, var, builder, blockMap, aliases, bb->rubyBlockId);
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
                                auto rawRubyString = Payload::cPtrToRubyString(cs, builder, str);
                                Payload::varSet(cs, bind.bind.variable, rawRubyString, builder, blockMap, aliases,
                                                bb->rubyBlockId);
                                break;
                            }
                            default:
                                cs.trace("UnsupportedLiteral");
                        }
                    },
                    [&](cfg::Unanalyzable *i) {
                        cs.failCompilation(bind.loc, "exceptions are not supported in compiled mode");
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
    builder.CreateRet(var);
}

void emitSigVerification(CompilerState &cs, cfg::CFG &cfg, unique_ptr<ast::MethodDef> &md,
                         const BasicBlockMap &blockMap, const UnorderedMap<core::LocalVariable, Alias> &aliases) {
    cs.functionEntryInitializers = blockMap.functionInitializersByFunction[0];
    llvm::IRBuilder<> builder(cs);
    builder.SetInsertPoint(blockMap.sigVerificationBlock);
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

    Payload::pushControlFrame(cs, builder, cfg.symbol);
    builder.CreateBr(blockMap.userEntryBlockByFunction[0]);
}

} // namespace

void IREmitter::run(CompilerState &cs, cfg::CFG &cfg, unique_ptr<ast::MethodDef> &md, const string &functionName) {
    Timer timer(cs.gs.tracer(), "IREmitter::run");
    UnorderedMap<core::LocalVariable, Alias> aliases;
    auto func = IREmitterHelpers::cleanFunctionBody(cs, IREmitterHelpers::getOrCreateFunction(cs, md->symbol));
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

    if (IREmitterHelpers::isStaticInit(cs, sym) && isRoot) {
        auto baseName = objectName.substr(0, objectName.rfind(".rb"));
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
        core::SymbolRef staticInit;
        auto attachedClass = owner.data(cs)->attachedClass(cs);
        if (isRoot) {
            staticInit = cs.gs.lookupStaticInitForFile(sym.data(cs)->loc());
        } else {
            staticInit = cs.gs.lookupStaticInitForClass(attachedClass);
        }

        // Call the LLVM method that was made by run() from this Init_ method
        auto staticInitName = IREmitterHelpers::getFunctionName(cs, staticInit);
        auto staticInitFunc = cs.module->getFunction(staticInitName);
        ENFORCE(staticInitFunc, staticInitName + " does not exist");
        builder.CreateCall(staticInitFunc,
                           {
                               llvm::ConstantInt::get(cs, llvm::APInt(32, 0, true)),
                               llvm::ConstantPointerNull::get(llvm::Type::getInt64PtrTy(cs)),
                               Payload::getRubyConstant(cs, owner, builder),
                           },
                           staticInitName);
    }

    builder.CreateRetVoid();

    ENFORCE(!llvm::verifyFunction(*entryFunc, &llvm::errs()), "see above");
    cs.runCheapOptimizations(entryFunc);
}

} // namespace sorbet::compiler
