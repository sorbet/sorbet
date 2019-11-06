// These violate our poisons so have to happen first
#include "llvm/IR/Attributes.h"
#include "llvm/IR/DerivedTypes.h" // FunctionType, StructType
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Verifier.h"
// ^^^ violate our poisons
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
#include "compiler/Names/Names.h"
#include <string_view>

using namespace std;
namespace sorbet::compiler {

// https://docs.ruby-lang.org/en/2.6.0/extension_rdoc.html
// and https://silverhammermba.github.io/emberb/c/ are your friends
// use the `demo` module for experiments
namespace {

bool isStaticInit(CompilerState &cs, core::SymbolRef sym) {
    auto name = sym.data(cs)->name;
    return (name.data(cs)->kind == core::NameKind::UTF8 ? name : name.data(cs)->unique.original) ==
           core::Names::staticInit();
}

llvm::GlobalValue::LinkageTypes getFunctionLinkageType(CompilerState &cs, core::SymbolRef sym) {
    if (isStaticInit(cs, sym)) {
        // this is top level code that shoudln't be callable externally.
        // Even more, sorbet reuses symbols used for these and thus if we mark them non-private we'll get link errors
        return llvm::Function::InternalLinkage;
    }
    return llvm::Function::ExternalLinkage;
}

core::SymbolRef removeRoot(core::SymbolRef sym) {
    if (sym == core::Symbols::root() || sym == core::Symbols::rootSingleton()) {
        // Root methods end up going on object
        sym = core::Symbols::Object();
    }
    return sym;
}

core::SymbolRef typeToSym(const core::GlobalState &gs, core::TypePtr typ) {
    core::SymbolRef sym;
    if (auto classType = core::cast_type<core::ClassType>(typ.get())) {
        sym = classType->symbol;
    } else if (auto appliedType = core::cast_type<core::AppliedType>(typ.get())) {
        sym = appliedType->klass;
    } else {
        ENFORCE(false);
    }
    sym = removeRoot(sym);
    ENFORCE(sym.data(gs)->isClassOrModule());
    return sym;
}

string getFunctionName(CompilerState &cs, core::SymbolRef sym) {
    return "func_" + sym.data(cs)->toStringFullName(cs);
}

llvm::Function *getOrCreateFunction(CompilerState &cs, std::string name, llvm::FunctionType *ft,
                                    llvm::GlobalValue::LinkageTypes linkageType = llvm::Function::InternalLinkage) {
    auto func = cs.module->getFunction(name);
    if (func) {
        return func;
    }
    return llvm::Function::Create(ft, linkageType, name, *cs.module);
}

llvm::Function *getOrCreateFunction(CompilerState &cs, core::SymbolRef sym) {
    return getOrCreateFunction(cs, getFunctionName(cs, sym), cs.getRubyFFIType(), getFunctionLinkageType(cs, sym));
}

llvm::Function *getInitFunction(CompilerState &cs, std::string baseName,
                                llvm::GlobalValue::LinkageTypes linkageType = llvm::Function::InternalLinkage) {
    std::vector<llvm::Type *> NoArgs(0, llvm::Type::getVoidTy(cs));
    auto ft = llvm::FunctionType::get(llvm::Type::getVoidTy(cs), NoArgs, false);
    return getOrCreateFunction(cs, "Init_" + baseName, ft, linkageType);
}

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
        auto argCountRaw = funcId == 0 ? func->arg_begin() : func->arg_begin() + 2;
        auto argArrayRaw = funcId == 0 ? func->arg_begin() + 1 : func->arg_begin() + 3;

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
        if (funcId != 0) {
            minArgCount = 0;
            // blocks Can have 0 args always
        }

        auto numOptionalArgs = maxArgCount - minArgCount;
        if (funcId == 0) {
            // validate arg count
            auto argCountFailBlock = llvm::BasicBlock::Create(cs, "argCountFailBlock", func);
            auto argCountSecondCheckBlock = llvm::BasicBlock::Create(cs, "argCountSecondCheckBlock", func);
            auto argCountSuccessBlock = llvm::BasicBlock::Create(cs, "argCountSuccess", func);

            auto tooManyArgs = builder.CreateICmpUGT(
                argCountRaw, llvm::ConstantInt::get(cs, llvm::APInt(32, maxArgCount)), "tooManyArgs");
            auto expected1 = MK::setExpectedBool(cs, builder, tooManyArgs, false);
            builder.CreateCondBr(expected1, argCountFailBlock, argCountSecondCheckBlock);

            builder.SetInsertPoint(argCountSecondCheckBlock);
            auto tooFewArgs = builder.CreateICmpULT(
                argCountRaw, llvm::ConstantInt::get(cs, llvm::APInt(32, minArgCount)), "tooFewArgs");
            auto expected2 = MK::setExpectedBool(cs, builder, tooFewArgs, false);
            builder.CreateCondBr(expected2, argCountFailBlock, argCountSuccessBlock);

            builder.SetInsertPoint(argCountFailBlock);
            MK::emitArgumentMismatch(cs, builder, argCountRaw, minArgCount, maxArgCount);

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
            if (funcId == 0) {
                auto selfArgRaw = func->arg_begin() + 2;
                MK::varSet(cs, core::LocalVariable::selfVariable(), selfArgRaw, builder, aliases, blockMap, funcId);
            }

            for (auto i = 0; i < maxArgCount; i++) {
                if (i >= minArgCount) {
                    // if these are optional, put them in their own BasicBlock
                    // because we might not run it
                    auto &block = fillFromArgBlocks[i - minArgCount];
                    builder.SetInsertPoint(block);
                }
                const auto a = blockMap.rubyBlockArgs[funcId][i];

                llvm::Value *indices[] = {llvm::ConstantInt::get(cs, llvm::APInt(32, i, true))};
                auto name = a._name.data(cs)->shortName(cs);
                llvm::StringRef nameRef(name.data(), name.length());
                auto rawValue = builder.CreateLoad(builder.CreateGEP(argArrayRaw, indices), {"rawArg_", nameRef});
                MK::varSet(cs, a, rawValue, builder, aliases, blockMap, funcId);
                if (i >= minArgCount) {
                    // check if we need to fill in the next variable from the arg
                    builder.CreateBr(checkBlocks[i - minArgCount + 1]);
                }
            }

            // make the last instruction in all the required args point at the first check block
            builder.SetInsertPoint(fillRequiredArgs);
            // TODO(perf): if block isn't captured, we can optimize this to skip proc conversion
            // TODO(perf): if block isn't used, don't load it at all
            //
            if (blkArgName.exists()) {
                // TODO: I don't think this correctly handles blocks with block args
                MK::varSet(cs, blkArgName, builder.CreateCall(cs.module->getFunction("sorbet_getMethodBlockAsProc")),
                           builder, aliases, blockMap, 0);
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
                auto expected = MK::setExpectedBool(cs, builder, argCount, false);
                builder.CreateCondBr(expected, fillFromDefaultBlocks[i], fillFromArgBlocks[i]);
            }
        }
        {
            // build fillFromDefaultBlocks
            auto optionalMethodIndex = 0;
            for (auto i = 0; i < numOptionalArgs; i++) {
                auto &block = fillFromDefaultBlocks[i];
                builder.SetInsertPoint(block);
                if (funcId == 0 && md->name.data(cs)->kind == core::NameKind::UNIQUE &&
                    md->name.data(cs)->unique.uniqueNameKind == core::UniqueNameKind::DefaultArg) {
                    // This method is already a default method so don't fill in
                    // another other defaults for it or else it is turtles all the
                    // way down
                } else {
                    llvm::Value *rawValue;
                    if (funcId == 0) {
                        optionalMethodIndex++;
                        auto argMethodName =
                            cs.gs.lookupNameUnique(core::UniqueNameKind::DefaultArg, md->name, optionalMethodIndex);
                        ENFORCE(argMethodName.exists(), "Default argument method for " + md->name.toString(cs) +
                                                            to_string(optionalMethodIndex) + " does not exist");
                        auto argMethod = md->symbol.data(cs)->owner.data(cs)->findMember(cs, argMethodName);
                        ENFORCE(argMethod.exists());
                        auto fillDefaultFunc = getOrCreateFunction(cs, argMethod);
                        rawValue =
                            builder.CreateCall(fillDefaultFunc, {argCountRaw, argArrayRaw,
                                                                 func->arg_begin() + 2 /* this is wrong for block*/});
                    } else {
                        rawValue = MK::getRubyNilRaw(cs, builder);
                    }
                    auto argIndex = i + minArgCount;
                    auto a = blockMap.rubyBlockArgs[funcId][argIndex];

                    MK::varSet(cs, a, rawValue, builder, aliases, blockMap, 0);
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

        if (funcId == 0) {
            // jump to user body
            builder.CreateBr(blockMap.sigVerificationBlock);
        } else {
            builder.CreateBr(blockMap.userEntryBlockByFunction[funcId]);
        }
    }
}

void defineMethod(CompilerState &cs, cfg::Send *i, bool isSelf, llvm::IRBuilder<> &builder) {
    ENFORCE(i->args.size() == 2);
    auto ownerSym = typeToSym(cs, i->args[0].type);

    auto lit = core::cast_type<core::LiteralType>(i->args[1].type.get());
    ENFORCE(lit->literalKind == core::LiteralType::LiteralTypeKind::Symbol);
    core::NameRef funcNameRef(cs, lit->value);

    auto lookupSym = isSelf ? ownerSym : ownerSym.data(cs)->attachedClass(cs);
    if (ownerSym == core::Symbols::Object() && !isSelf) {
        // TODO Figure out if this speicial case is right
        lookupSym = core::Symbols::Object();
    }
    auto funcSym = lookupSym.data(cs)->findMember(cs, funcNameRef);
    ENFORCE(funcSym.exists());
    ENFORCE(funcSym.data(cs)->isMethod());
    auto llvmFuncName = getFunctionName(cs, funcSym);
    auto funcHandle = getOrCreateFunction(cs, funcSym);
    auto universalSignature = llvm::PointerType::getUnqual(llvm::FunctionType::get(llvm::Type::getInt64Ty(cs), true));
    auto ptr = builder.CreateBitCast(funcHandle, universalSignature);

    auto rubyFunc = cs.module->getFunction(isSelf ? "sorbet_defineMethodSingleton" : "sorbet_defineMethod");
    ENFORCE(rubyFunc);
    builder.CreateCall(rubyFunc, {MK::getRubyConstantValueRaw(cs, ownerSym, builder),
                                  MK::toCString(cs, funcNameRef.show(cs), builder), ptr,
                                  llvm::ConstantInt::get(cs, llvm::APInt(32, -1, true))});

    builder.CreateCall(getInitFunction(cs, llvmFuncName), {});
}

std::string showClassNameWithoutOwner(const core::GlobalState &gs, core::SymbolRef sym) {
    auto name = sym.data(gs)->name;
    if (name.data(gs)->kind == core::NameKind::UNIQUE) {
        return name.data(gs)->unique.original.data(gs)->show(gs);
    }
    return name.data(gs)->show(gs);
}

void defineClass(CompilerState &cs, cfg::Send *i, llvm::IRBuilder<> &builder) {
    auto sym = typeToSym(cs, i->args[0].type);
    // this is wrong and will not work for `class <<self`
    auto classNameCStr = MK::toCString(cs, showClassNameWithoutOwner(cs, sym), builder);
    auto isModule = sym.data(cs)->superClass() == core::Symbols::Module();

    if (sym.data(cs)->owner != core::Symbols::root()) {
        auto getOwner = MK::getRubyConstantValueRaw(cs, sym.data(cs)->owner, builder);
        if (isModule) {
            builder.CreateCall(cs.module->getFunction("sorbet_defineNestedModule"), {getOwner, classNameCStr});
        } else {
            auto rawCall = MK::getRubyConstantValueRaw(cs, sym.data(cs)->superClass(), builder);
            builder.CreateCall(cs.module->getFunction("sorbet_defineNestedClass"), {getOwner, classNameCStr, rawCall});
        }
    } else {
        if (isModule) {
            builder.CreateCall(cs.module->getFunction("sorbet_defineTopLevelModule"), {classNameCStr});
        } else {
            auto rawCall = MK::getRubyConstantValueRaw(cs, sym.data(cs)->superClass(), builder);
            builder.CreateCall(cs.module->getFunction("sorbet_defineTopClassOrModule"), {classNameCStr, rawCall});
        }
    }

    auto funcSym = cs.gs.lookupStaticInitForClass(sym.data(cs)->attachedClass(cs));
    auto llvmFuncName = getFunctionName(cs, funcSym);
    builder.CreateCall(getInitFunction(cs, llvmFuncName), {});
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
                        auto var = MK::varGet(cs, i->what, builder, aliases, blockMap, bb->rubyBlockId);
                        MK::varSet(cs, bind.bind.variable, var, builder, aliases, blockMap, bb->rubyBlockId);
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
                                aliases[bind.bind.variable] = Alias::forConstant(i->what);
                            }
                        }
                    },
                    [&](cfg::SolveConstraint *i) {
                        auto var = MK::varGet(cs, i->send, builder, aliases, blockMap, bb->rubyBlockId);
                        MK::varSet(cs, bind.bind.variable, var, builder, aliases, blockMap, bb->rubyBlockId);
                    },
                    [&](cfg::Send *i) {
                        if (i->recv.variable._name == core::Names::blkArg() &&
                            loadYieldParamsResults.contains(i->recv.variable)) {
                            // this loads an argument of a block.
                            // They are already loaded in preambula of the method
                            return;
                        }
                        auto str = i->fun.data(cs)->shortName(cs);
                        if (i->fun == core::Names::keepForIde() || i->fun == core::Names::keepForTypechecking()) {
                            return;
                        }
                        if (i->fun == core::Names::buildHash()) {
                            auto ret =
                                builder.CreateCall(cs.module->getFunction("sorbet_newRubyHash"), {}, "rawHashLiteral");
                            // TODO(perf): in 2.7 use rb_hash_bulk_insert will give 2x speedup
                            int argc = 0;
                            while (argc < i->args.size()) {
                                auto key = i->args[argc].variable;
                                auto value = i->args[argc + 1].variable;
                                builder.CreateCall(
                                    cs.module->getFunction("sorbet_hashStore"),
                                    {ret, MK::varGet(cs, key, builder, aliases, blockMap, bb->rubyBlockId),
                                     MK::varGet(cs, value, builder, aliases, blockMap, bb->rubyBlockId)});
                                argc += 2;
                            }
                            MK::varSet(cs, bind.bind.variable, ret, builder, aliases, blockMap, bb->rubyBlockId);
                            return;
                        }
                        if (i->fun == core::Names::buildArray()) {
                            auto ret = builder.CreateCall(
                                cs.module->getFunction("sorbet_newRubyArray"),
                                {llvm::ConstantInt::get(cs, llvm::APInt(64, i->args.size(), true))}, "rawArrayLiteral");
                            for (int argc = 0; argc < i->args.size(); argc++) {
                                auto value = i->args[argc].variable;
                                builder.CreateCall(
                                    cs.module->getFunction("sorbet_arrayPush"),
                                    {ret, MK::varGet(cs, value, builder, aliases, blockMap, bb->rubyBlockId)});
                            }
                            MK::varSet(cs, bind.bind.variable, ret, builder, aliases, blockMap, bb->rubyBlockId);
                            return;
                        }
                        if (i->fun == Names::sorbet_defineTopClassOrModule(cs)) {
                            defineClass(cs, i, builder);
                            return;
                        }
                        if (i->fun == Names::sorbet_defineMethod(cs)) {
                            defineMethod(cs, i, false, builder);
                            return;
                        }
                        if (i->fun == Names::sorbet_defineMethodSingleton(cs)) {
                            defineMethod(cs, i, true, builder);
                            return;
                        }
                        auto rawId = MK::getRubyIdFor(cs, builder, str);

                        // fill in args
                        {
                            int argId = -1;
                            for (auto &arg : i->args) {
                                argId += 1;
                                llvm::Value *indices[] = {llvm::ConstantInt::get(cs, llvm::APInt(32, 0, true)),
                                                          llvm::ConstantInt::get(cs, llvm::APInt(64, argId, true))};
                                auto var = MK::varGet(cs, arg.variable, builder, aliases, blockMap, bb->rubyBlockId);
                                builder.CreateStore(var,
                                                    builder.CreateGEP(blockMap.sendArgArrayByBlock[bb->rubyBlockId],
                                                                      indices, "callArgsAddr"));
                            }
                        }
                        llvm::Value *indices[] = {llvm::ConstantInt::get(cs, llvm::APInt(64, 0, true)),
                                                  llvm::ConstantInt::get(cs, llvm::APInt(64, 0, true))};

                        // TODO(perf): call
                        // https://github.com/ruby/ruby/blob/3e3cc0885a9100e9d1bfdb77e136416ec803f4ca/internal.h#L2372
                        // to get inline caching.
                        // before this, perf will not be good
                        auto var = MK::varGet(cs, i->recv.variable, builder, aliases, blockMap, bb->rubyBlockId);
                        llvm::Value *rawCall;
                        if (i->link != nullptr) {
                            // this send has a block!
                            rawCall = builder.CreateCall(
                                cs.module->getFunction("sorbet_callFuncBlock"),
                                {var, rawId, llvm::ConstantInt::get(cs, llvm::APInt(32, i->args.size(), true)),
                                 builder.CreateGEP(blockMap.sendArgArrayByBlock[bb->rubyBlockId], indices),
                                 blockMap.rubyBlocks2Functions[i->link->rubyBlockId],
                                 blockMap.escapedClosure[bb->rubyBlockId]},
                                "rawSendResult");

                        } else {
                            rawCall = builder.CreateCall(
                                cs.module->getFunction("sorbet_callFunc"),
                                {var, rawId, llvm::ConstantInt::get(cs, llvm::APInt(32, i->args.size(), true)),
                                 builder.CreateGEP(blockMap.sendArgArrayByBlock[bb->rubyBlockId], indices)},
                                "rawSendResult");
                        }
                        MK::varSet(cs, bind.bind.variable, rawCall, builder, aliases, blockMap, bb->rubyBlockId);
                    },
                    [&](cfg::Return *i) {
                        isTerminated = true;
                        ENFORCE(bb->rubyBlockId == 0, "returns through multiple stacks not implemented");
                        auto var = MK::varGet(cs, i->what.variable, builder, aliases, blockMap, bb->rubyBlockId);
                        builder.CreateRet(var);
                    },
                    [&](cfg::BlockReturn *i) {
                        ENFORCE(bb->rubyBlockId != 0, "should never happen");

                        isTerminated = true;
                        auto var = MK::varGet(cs, i->what.variable, builder, aliases, blockMap, bb->rubyBlockId);
                        builder.CreateRet(var);
                    },
                    [&](cfg::LoadSelf *i) {
                        auto var = MK::varGet(cs, i->fallback, builder, aliases, blockMap, bb->rubyBlockId);
                        MK::varSet(cs, bind.bind.variable, var, builder, aliases, blockMap, bb->rubyBlockId);
                    },
                    [&](cfg::Literal *i) {
                        if (i->value->derivesFrom(cs, core::Symbols::FalseClass())) {
                            MK::varSet(cs, bind.bind.variable, MK::getRubyFalseRaw(cs, builder), builder, aliases,
                                       blockMap, bb->rubyBlockId);
                            return;
                        }
                        if (i->value->derivesFrom(cs, core::Symbols::TrueClass())) {
                            MK::varSet(cs, bind.bind.variable, MK::getRubyTrueRaw(cs, builder), builder, aliases,
                                       blockMap, bb->rubyBlockId);
                            return;
                        }
                        if (i->value->derivesFrom(cs, core::Symbols::NilClass())) {
                            MK::varSet(cs, bind.bind.variable, MK::getRubyNilRaw(cs, builder), builder, aliases,
                                       blockMap, bb->rubyBlockId);
                            return;
                        }

                        auto litType = core::cast_type<core::LiteralType>(i->value.get());
                        ENFORCE(litType);
                        switch (litType->literalKind) {
                            case core::LiteralType::LiteralTypeKind::Integer: {
                                auto rawInt = MK::getRubyIntRaw(cs, builder, litType->value);
                                MK::varSet(cs, bind.bind.variable, rawInt, builder, aliases, blockMap, bb->rubyBlockId);
                                break;
                            }
                            case core::LiteralType::LiteralTypeKind::Symbol: {
                                auto str = core::NameRef(cs, litType->value).data(cs)->shortName(cs);
                                auto rawId = MK::getRubyIdFor(cs, builder, str);
                                auto rawRubySym =
                                    builder.CreateCall(cs.module->getFunction("rb_id2sym"), {rawId}, "rawSym");
                                MK::varSet(cs, bind.bind.variable, rawRubySym, builder, aliases, blockMap,
                                           bb->rubyBlockId);
                                break;
                            }
                            case core::LiteralType::LiteralTypeKind::String: {
                                auto str = core::NameRef(cs, litType->value).data(cs)->shortName(cs);
                                auto rawRubyString = MK::getRubyStringRaw(cs, builder, str);
                                MK::varSet(cs, bind.bind.variable, rawRubyString, builder, aliases, blockMap,
                                           bb->rubyBlockId);
                                break;
                            }
                            default:
                                cs.trace("UnsupportedLiteral");
                        }
                    },
                    [&](cfg::Unanalyzable *i) {
                        if (auto e = cs.gs.beginError(bind.loc, core::errors::Compiler::Unanalyzable)) {
                            e.setHeader("Unsupported node: `{}`", i->toString(core::Context(cs.gs, cfg.symbol)));
                        }
                    },
                    [&](cfg::LoadArg *i) {
                        /* intentionally omitted, it's part of method preambula */
                    },
                    [&](cfg::LoadYieldParams *i) {
                        loadYieldParamsResults.insert(bind.bind.variable);
                        /* intentionally omitted, it's part of method preambula */
                    },
                    [&](cfg::Cast *i) {
                        auto val = MK::varGet(cs, i->value.variable, builder, aliases, blockMap, bb->rubyBlockId);
                        auto passedTypeTest = MK::createTypeTestU1(cs, builder, val, bind.bind.type);
                        auto successBlock =
                            llvm::BasicBlock::Create(cs, "typeTestSuccess", builder.GetInsertBlock()->getParent());

                        auto failBlock =
                            llvm::BasicBlock::Create(cs, "typeTestFail", builder.GetInsertBlock()->getParent());

                        auto expected = MK::setExpectedBool(cs, builder, passedTypeTest, true);
                        builder.CreateCondBr(expected, successBlock, failBlock);
                        builder.SetInsertPoint(failBlock);
                        // this will throw exception
                        builder.CreateCall(cs.module->getFunction("sorbet_cast_failure"),
                                           {val, MK::toCString(cs, i->cast.data(cs)->shortName(cs), builder),
                                            MK::toCString(cs, bind.bind.type->show(cs), builder)});
                        builder.CreateUnreachable();
                        builder.SetInsertPoint(successBlock);

                        if (i->cast == core::Names::let() || i->cast == core::Names::cast()) {
                            MK::varSet(cs, bind.bind.variable, val, builder, aliases, blockMap, bb->rubyBlockId);
                        } else if (i->cast == core::Names::assertType()) {
                            MK::varSet(cs, bind.bind.variable, MK::getRubyFalseRaw(cs, builder), builder, aliases,
                                       blockMap, bb->rubyBlockId);
                        }
                    },
                    [&](cfg::TAbsurd *i) { cs.trace("TAbsurd\n"); });
                if (isTerminated) {
                    break;
                }
            }
            if (!isTerminated) {
                if (bb->bexit.thenb != bb->bexit.elseb && bb->bexit.cond.variable != core::LocalVariable::blockCall()) {
                    auto var = MK::varGet(cs, bb->bexit.cond.variable, builder, aliases, blockMap, bb->rubyBlockId);
                    auto condValue = MK::getIsTruthyU1(cs, builder, var);

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
            builder.CreateRet(MK::getRubyNilRaw(cs, builder));
        }
    }
}

void emitSigVerification(CompilerState &cs, cfg::CFG &cfg, unique_ptr<ast::MethodDef> &md,
                         const UnorderedMap<core::LocalVariable, Alias> &aliases, const BasicBlockMap &blockMap) {
    llvm::IRBuilder<> builder(cs);
    builder.SetInsertPoint(blockMap.sigVerificationBlock);
    builder.CreateBr(blockMap.userEntryBlockByFunction[0]);
}

} // namespace

void LLVMIREmitter::run(CompilerState &cs, cfg::CFG &cfg, unique_ptr<ast::MethodDef> &md, const string &functionName) {
    UnorderedMap<core::LocalVariable, Alias> aliases;
    auto func = getOrCreateFunction(cs, md->symbol);
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

    const BasicBlockMap blockMap = LLVMIREmitterHelpers::getSorbetBlocks2LLVMBlockMapping(cs, cfg, md, aliases, func);

    ENFORCE(cs.functionEntryInitializers == nullptr, "modules shouldn't be reused");

    setupArguments(cs, cfg, md, blockMap, aliases);
    emitSigVerification(cs, cfg, md, aliases, blockMap);

    emitUserBody(cs, cfg, blockMap, aliases);
    for (int funId = 0; funId < blockMap.functionInitializersByFunction.size(); funId++) {
        builder.SetInsertPoint(blockMap.functionInitializersByFunction[funId]);
        builder.CreateBr(blockMap.argumentSetupBlocksByFunction[funId]);
    }
    /* run verifier */
    ENFORCE(!llvm::verifyFunction(*func, &llvm::errs()), "see above");
    // cs.runCheapOptimizations(func);
}

void LLVMIREmitter::buildInitFor(CompilerState &cs, const core::SymbolRef &sym, string_view objectName) {
    llvm::IRBuilder<> builder(cs);

    auto baseName = getFunctionName(cs, sym);
    auto linkageType = llvm::Function::InternalLinkage;
    auto owner = sym.data(cs)->owner;
    auto isRoot = owner == core::Symbols::rootSingleton();

    if (isStaticInit(cs, sym) && isRoot) {
        baseName = objectName;
        baseName = baseName.substr(0, baseName.rfind(".rb"));
        linkageType = llvm::Function::ExternalLinkage;
    }
    auto entryFunc = getInitFunction(cs, baseName, linkageType);

    auto bb = llvm::BasicBlock::Create(cs, "entry", entryFunc);
    builder.SetInsertPoint(bb);

    if (isStaticInit(cs, sym)) {
        core::SymbolRef staticInit;
        auto attachedClass = owner.data(cs)->attachedClass(cs);
        if (isRoot) {
            staticInit = cs.gs.lookupStaticInitForFile(sym.data(cs)->loc());
        } else {
            staticInit = cs.gs.lookupStaticInitForClass(attachedClass);
        }

        // Call the LLVM method that was made by run() from this Init_ method
        auto staticInitName = getFunctionName(cs, staticInit);
        auto staticInitFunc = cs.module->getFunction(staticInitName);
        ENFORCE(staticInitFunc, staticInitName + " does not exist");
        builder.CreateCall(staticInitFunc,
                           {
                               llvm::ConstantInt::get(cs, llvm::APInt(32, 0, true)),
                               llvm::ConstantPointerNull::get(llvm::Type::getInt64PtrTy(cs)),
                               MK::getRubyConstantValueRaw(cs, owner, builder),
                           },
                           staticInitName);
    }

    builder.CreateRetVoid();

    ENFORCE(!llvm::verifyFunction(*entryFunc, &llvm::errs()), "see above");
    cs.runCheapOptimizations(entryFunc);
}

} // namespace sorbet::compiler
