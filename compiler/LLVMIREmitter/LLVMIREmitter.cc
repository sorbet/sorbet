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
#include "common/typecase.h"
#include "compiler/IRHelpers/IRHelpers.h"
#include "compiler/LLVMIREmitter/LLVMIREmitter.h"
#include "compiler/Names/Names.h"
#include <string_view>

using namespace std;
namespace sorbet::compiler {

// https://docs.ruby-lang.org/en/2.6.0/extension_rdoc.html
// and https://silverhammermba.github.io/emberb/c/ are your friends
// use the `demo` module for experiments
namespace {

core::SymbolRef removeRoot(core::SymbolRef sym) {
    if (sym == core::Symbols::root() || sym == core::Symbols::rootSingleton()) {
        // Root methods end up going on object
        sym = core::Symbols::Object();
    }
    return sym;
}

std::string showClassName(const core::GlobalState &gs, core::SymbolRef sym) {
    auto name = sym.data(gs)->name;
    if (name.data(gs)->kind == core::NameKind::UNIQUE) {
        return name.data(gs)->unique.original.data(gs)->show(gs);
    }
    return name.data(gs)->show(gs);
}

llvm::CallInst *resolveSymbol(const core::GlobalState &gs, core::SymbolRef sym, llvm::IRBuilder<> &builder,
                              llvm::Module *module) {
    sym = removeRoot(sym);
    auto str = showClassName(gs, sym);
    auto rawCString = builder.CreateGlobalStringPtr(str, "str_" + str);
    return builder.CreateCall(module->getFunction("sorbet_getConstant"), {rawCString});
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

llvm::Value *varGet(CompilerState &gs, core::LocalVariable var, llvm::IRBuilder<> &builder,
                    const UnorderedMap<core::LocalVariable, llvm::AllocaInst *> &llvmVariables,
                    const UnorderedMap<llvm::AllocaInst *, core::SymbolRef> &aliases) {
    auto ret = llvmVariables.at(var);
    if (!aliases.contains(ret)) {
        return gs.unboxRawValue(builder, ret);
    }

    return resolveSymbol(gs.gs, aliases.at(ret), builder, gs.module);
}

void varSet(CompilerState &gs, llvm::AllocaInst *alloca, llvm::Value *var, llvm::IRBuilder<> &builder,
            const UnorderedMap<core::LocalVariable, llvm::AllocaInst *> &llvmVariables,
            UnorderedMap<llvm::AllocaInst *, core::SymbolRef> &aliases) {
    gs.boxRawValue(builder, alloca, var);
    if (!aliases.contains(alloca)) {
        return;
    }
    // TODO do const_set for the alias
}

// Create local allocas for local variables, initialize them all to `nil`.
// load arguments, check their count
// load self
UnorderedMap<core::LocalVariable, llvm::AllocaInst *>
setupArgumentsAndLocalVariables(CompilerState &gs, llvm::IRBuilder<> &builder, cfg::CFG &cfg,
                                unique_ptr<ast::MethodDef> &md, llvm::Function *func) {
    UnorderedMap<core::LocalVariable, llvm::AllocaInst *> llvmVariables;
    {
        // nill out variables.
        auto valueType = gs.getValueType();
        auto nilValueRaw = gs.getRubyNilRaw(builder);
        for (const auto &entry : cfg.minLoops) {
            auto var = entry.first;
            auto svName = var._name.data(gs)->shortName(gs);
            auto alloca = llvmVariables[var] =
                builder.CreateAlloca(valueType, nullptr, llvm::StringRef(svName.data(), svName.length()));
            gs.boxRawValue(builder, alloca, nilValueRaw);
        }
    }
    auto argArrayRaw = func->arg_begin() + 1;
    auto requiredArgumentCount =
        md->args.size() - 1; // todo: make optional arguments actually optional. -1 because of block args
    {
        // validate arg count
        auto argCountRaw = func->arg_begin();
        auto argCountSuccessBlock = llvm::BasicBlock::Create(gs, "argCountSuccess", func);
        auto argCountFailBlock = llvm::BasicBlock::Create(gs, "argCountFailBlock", func);
        auto isWrongArgCount = builder.CreateICmpNE(
            argCountRaw,
            llvm::ConstantInt::get(llvm::Type::getInt32Ty(gs), llvm::APInt(32, requiredArgumentCount, true)),
            "isWrongArgCount");
        gs.setExpectedBool(builder, isWrongArgCount, false);
        builder.CreateCondBr(isWrongArgCount, argCountFailBlock, argCountSuccessBlock);

        builder.SetInsertPoint(argCountFailBlock);
        gs.emitArgumentMismatch(builder, argCountRaw, requiredArgumentCount, requiredArgumentCount);
        builder.SetInsertPoint(argCountSuccessBlock);
    }
    {
        // box required args
        int argId = -1;
        for (const auto &arg : md->args) {
            argId += 1;
            if (argId == requiredArgumentCount) {
                // block arg isn't passed in args
                break;
            }

            auto *a = ast::MK::arg2Local(arg.get());
            llvm::Value *indices[] = {llvm::ConstantInt::get(gs, llvm::APInt(32, argId, true))};
            auto rawValue = builder.CreateLoad(builder.CreateGEP(argArrayRaw, indices), "rawArgValue");
            gs.boxRawValue(builder, llvmVariables[a->localVariable], rawValue);
        }
    }
    {
        // box `self`
        auto selfArgRaw = (func->arg_begin() + 2);
        gs.boxRawValue(builder, llvmVariables[core::LocalVariable::selfVariable()], selfArgRaw);
    }

    return llvmVariables;
}

vector<llvm::BasicBlock *> getSorbetBlocks2LLVMBlockMapping(CompilerState &gs, cfg::CFG &cfg, llvm::Function *func,
                                                            llvm::BasicBlock *llvmFuncEntry) {
    vector<llvm::BasicBlock *> llvmBlocks(cfg.maxBasicBlockId);
    for (auto &b : cfg.basicBlocks) {
        if (b.get() == cfg.entry()) {
            llvmBlocks[b->id] = llvmFuncEntry;
        } else {
            llvmBlocks[b->id] = llvm::BasicBlock::Create(gs, {"BB", to_string(b->id)},
                                                         func); // to_s is slow. We should only use it in debug builds
        }
    }
    return llvmBlocks;
}

void emitUserBody(CompilerState &gs, cfg::CFG &cfg, const vector<llvm::BasicBlock *> llvmBlocks,
                  const UnorderedMap<core::LocalVariable, llvm::AllocaInst *> llvmVariables,
                  llvm::AllocaInst *sendArgArray) {
    UnorderedMap<llvm::AllocaInst *, core::SymbolRef> aliases;
    llvm::IRBuilder<> builder(gs);
    for (auto it = cfg.forwardsTopoSort.rbegin(); it != cfg.forwardsTopoSort.rend(); ++it) {
        cfg::BasicBlock *bb = *it;
        auto block = llvmBlocks[bb->id];
        bool isTerminated = false;
        builder.SetInsertPoint(block);
        if (bb != cfg.deadBlock()) {
            for (cfg::Binding &bind : bb->exprs) {
                auto targetAlloca = llvmVariables.at(bind.bind.variable);
                typecase(
                    bind.value.get(),
                    [&](cfg::Ident *i) {
                        auto var = varGet(gs, i->what, builder, llvmVariables, aliases);
                        varSet(gs, targetAlloca, var, builder, llvmVariables, aliases);
                    },
                    [&](cfg::Alias *i) { aliases[targetAlloca] = i->what; },
                    [&](cfg::SolveConstraint *i) { gs.trace("SolveConstraint\n"); },
                    [&](cfg::Send *i) {
                        auto str = i->fun.data(gs)->shortName(gs);
                        if (i->fun == Names::sorbet_defineTopLevelModule) {
                            return;
                        }
                        if (i->fun == Names::sorbet_defineNestedModule) {
                            return;
                        }
                        if (i->fun == Names::sorbet_defineTopLevelClass) {
                            auto sym = typeToSym(gs, i->args[0].type);
                            auto className = showClassName(gs, sym);
                            auto classNameCStr = builder.CreateGlobalStringPtr(className, {"className_", className});
                            auto rawCall = resolveSymbol(gs, sym.data(gs)->superClass(), builder, gs.module);
                            builder.CreateCall(gs.module->getFunction("sorbet_defineTopLevelClass"),
                                               {classNameCStr, rawCall});
                            return;
                        }
                        if (i->fun == Names::sorbet_defineNestedClass) {
                            return;
                        }
                        if (i->fun == Names::sorbet_defineMethod) {
                            ENFORCE(i->args.size() == 2);
                            auto ownerSym = typeToSym(gs, i->args[0].type);

                            auto lit = core::cast_type<core::LiteralType>(i->args[1].type.get());
                            ENFORCE(lit->literalKind == core::LiteralType::LiteralTypeKind::Symbol);
                            core::NameRef funcNameRef(gs, lit->value);
                            auto funcSym = ownerSym.data(gs)->findMember(gs, funcNameRef);
                            ENFORCE(funcSym.data(gs)->isMethod());

                            auto funcName = funcNameRef.show(gs);
                            auto funcNameCStr = builder.CreateGlobalStringPtr(funcName, {"funcName_", funcName});

                            auto llvmFuncName = funcSym.data(gs)->toStringFullName(gs);
                            auto funcHandle = gs.module->getOrInsertFunction(llvmFuncName, gs.getRubyFFIType());
                            auto universalSignature =
                                llvm::PointerType::getUnqual(llvm::FunctionType::get(llvm::Type::getInt64Ty(gs), true));
                            auto ptr = builder.CreateBitCast(funcHandle, universalSignature);

                            builder.CreateCall(gs.module->getFunction("sorbet_defineMethod"),
                                               {resolveSymbol(gs, ownerSym, builder, gs.module), funcNameCStr, ptr,
                                                llvm::ConstantInt::get(gs, llvm::APInt(32, -1, true))});

                            std::vector<llvm::Type *> NoArgs(0, llvm::Type::getVoidTy(gs));
                            auto ft = llvm::FunctionType::get(llvm::Type::getVoidTy(gs), NoArgs, false);
                            builder.CreateCall(gs.module->getOrInsertFunction("Init_" + llvmFuncName, ft), {});
                            return;
                        }
                        if (i->fun == Names::sorbet_defineMethodSingleton) {
                            return;
                        }
                        // TODO: Should only be ::Sorbet::Private::Static.keep_for_ide
                        if (i->fun == core::Names::keepForIde()) {
                            return;
                        }
                        // TODO: Should only be T.unsafe
                        if (i->fun == core::Names::unsafe()) {
                            return;
                        }
                        auto rawId = gs.getRubyIdFor(builder, str);

                        // fill in args
                        {
                            int argId = -1;
                            for (auto &arg : i->args) {
                                argId += 1;
                                llvm::Value *indices[] = {llvm::ConstantInt::get(gs, llvm::APInt(32, 0, true)),
                                                          llvm::ConstantInt::get(gs, llvm::APInt(64, argId, true))};
                                auto var = varGet(gs, arg.variable, builder, llvmVariables, aliases);
                                builder.CreateStore(var, builder.CreateGEP(sendArgArray, indices, "callArgsAddr"));
                            }
                        }
                        llvm::Value *indices[] = {llvm::ConstantInt::get(gs, llvm::APInt(64, 0, true)),
                                                  llvm::ConstantInt::get(gs, llvm::APInt(64, 0, true))};

                        // TODO(perf): call
                        // https://github.com/ruby/ruby/blob/3e3cc0885a9100e9d1bfdb77e136416ec803f4ca/internal.h#L2372
                        // to get inline caching.
                        // before this, perf will not be good
                        //
                        //
                        // todo(perf): mark the arguments with
                        // https://llvm.org/docs/LangRef.html#llvm-invariant-start-intrinsic for duration of the call
                        auto var = varGet(gs, i->recv.variable, builder, llvmVariables, aliases);
                        auto rawCall = builder.CreateCall(
                            gs.module->getFunction("sorbet_callFunc"),
                            {var, rawId,
                             llvm::ConstantInt::get(llvm::Type::getInt32Ty(gs), llvm::APInt(32, i->args.size(), true)),
                             builder.CreateGEP(sendArgArray, indices)},
                            "rawSendResult");
                        varSet(gs, targetAlloca, rawCall, builder, llvmVariables, aliases);
                    },
                    [&](cfg::Return *i) {
                        isTerminated = true;
                        auto var = varGet(gs, i->what.variable, builder, llvmVariables, aliases);
                        builder.CreateRet(var);
                    },
                    [&](cfg::BlockReturn *i) { gs.trace("BlockReturn\n"); },
                    [&](cfg::LoadSelf *i) { gs.trace("LoadSelf\n"); },
                    [&](cfg::Literal *i) {
                        gs.trace("Literal\n");
                        if (i->value->derivesFrom(gs, core::Symbols::NilClass())) {
                            varSet(gs, targetAlloca, gs.getRubyNilRaw(builder), builder, llvmVariables, aliases);
                            return;
                        }
                        auto litType = core::cast_type<core::LiteralType>(i->value.get());
                        ENFORCE(litType);
                        switch (litType->literalKind) {
                            case core::LiteralType::LiteralTypeKind::Integer: {
                                auto rawInt = gs.getRubyIntRaw(builder, litType->value);
                                varSet(gs, targetAlloca, rawInt, builder, llvmVariables, aliases);

                                break;
                            }
                            case core::LiteralType::LiteralTypeKind::True:
                                varSet(gs, targetAlloca, gs.getRubyTrueRaw(builder), builder, llvmVariables, aliases);
                                break;
                            case core::LiteralType::LiteralTypeKind::False:
                                varSet(gs, targetAlloca, gs.getRubyFalseRaw(builder), builder, llvmVariables, aliases);
                                break;
                            case core::LiteralType::LiteralTypeKind::String: {
                                auto str = core::NameRef(gs, litType->value).data(gs)->shortName(gs);
                                auto rawRubyString = gs.getRubyStringRaw(builder, str);
                                varSet(gs, targetAlloca, rawRubyString, builder, llvmVariables, aliases);
                                break;
                            }
                            default:
                                gs.trace("UnsupportedLiteral");
                        }
                    },
                    [&](cfg::Unanalyzable *i) { gs.trace("Unanalyzable\n"); },
                    [&](cfg::LoadArg *i) { gs.trace("LoadArg\n"); },
                    [&](cfg::LoadYieldParams *i) { gs.trace("LoadYieldParams\n"); },
                    [&](cfg::Cast *i) { gs.trace("Cast\n"); }, [&](cfg::TAbsurd *i) { gs.trace("TAbsurd\n"); });
                if (isTerminated) {
                    break;
                }
            }
            if (!isTerminated) {
                if (bb->bexit.thenb != bb->bexit.elseb) {
                    auto var = varGet(gs, bb->bexit.cond.variable, builder, llvmVariables, aliases);
                    auto condValue = gs.getIsTruthyU1(builder, var);

                    builder.CreateCondBr(condValue, llvmBlocks[bb->bexit.thenb->id], llvmBlocks[bb->bexit.elseb->id]);
                } else {
                    builder.CreateBr(llvmBlocks[bb->bexit.thenb->id]);
                }
            }
        } else {
            // handle dead block. TODO: this should throw
            builder.CreateRet(gs.getRubyNilRaw(builder));
        }
    }
}
} // namespace

void LLVMIREmitter::run(CompilerState &gs, cfg::CFG &cfg, unique_ptr<ast::MethodDef> &md, const string &functionName) {
    auto functionType = gs.getRubyFFIType();
    auto func = llvm::Function::Create(functionType, llvm::Function::ExternalLinkage, functionName, gs.module);
    func->addFnAttr(llvm::Attribute::AttrKind::StackProtectReq);
    func->addFnAttr(llvm::Attribute::AttrKind::NoUnwind);
    func->addFnAttr(llvm::Attribute::AttrKind::UWTable);
    llvm::IRBuilder<> builder(gs);

    ENFORCE(gs.functionEntryInitializers == nullptr, "modules shouldn't be reused");

    gs.functionEntryInitializers = llvm::BasicBlock::Create(
        gs, "functionEntryInitializers",
        func); // we will build a link for this block later, after we finish building expressions into it

    auto rawEntryBlock = llvm::BasicBlock::Create(gs, "setupLocalsAndArguments", func);
    builder.SetInsertPoint(rawEntryBlock);
    const UnorderedMap<core::LocalVariable, llvm::AllocaInst *> llvmVariables =
        setupArgumentsAndLocalVariables(gs, builder, cfg, md, func);

    auto userBodyEntry = llvm::BasicBlock::Create(gs, "userBody", func);
    builder.CreateBr(userBodyEntry);

    const vector<llvm::BasicBlock *> llvmBlocks = getSorbetBlocks2LLVMBlockMapping(gs, cfg, func, userBodyEntry);

    int maxSendArgCount = 0;
    for (auto it = cfg.forwardsTopoSort.rbegin(); it != cfg.forwardsTopoSort.rend(); ++it) {
        cfg::BasicBlock *bb = *it;
        for (cfg::Binding &bind : bb->exprs) {
            if (auto snd = cfg::cast_instruction<cfg::Send>(bind.value.get())) {
                if (maxSendArgCount < snd->args.size()) {
                    maxSendArgCount = snd->args.size();
                }
            }
        }
    }
    builder.SetInsertPoint(gs.functionEntryInitializers);
    auto sendArgArray =
        builder.CreateAlloca(llvm::ArrayType::get(llvm::Type::getInt64Ty(gs), maxSendArgCount), nullptr, "callArgs");
    emitUserBody(gs, cfg, llvmBlocks, llvmVariables, sendArgArray);
    builder.SetInsertPoint(gs.functionEntryInitializers);
    builder.CreateBr(rawEntryBlock);
    /* run verifier */
    ENFORCE(!llvm::verifyFunction(*func, &llvm::errs()), "see above");
    gs.runCheapOptimizations(func);
}

void LLVMIREmitter::buildInitFor(CompilerState &gs, const core::SymbolRef &sym, llvm::BasicBlock *globalInitializers,
                                 string_view objectName) {
    llvm::IRBuilder<> builder(gs);
    std::vector<llvm::Type *> NoArgs(0, llvm::Type::getVoidTy(gs));
    auto ft = llvm::FunctionType::get(llvm::Type::getVoidTy(gs), NoArgs, false);
    bool isSpecialEntrypoint = false;
    if (sym.data(gs)->owner == core::Symbols::rootSingleton() &&
        sym.data(gs)->name.data(gs)->unique.original == core::Names::staticInit()) {
        isSpecialEntrypoint = true;
    }

    auto baseName = sym.data(gs)->toStringFullName(gs);
    auto linkageType = llvm::Function::InternalLinkage;
    if (isSpecialEntrypoint) {
        baseName = FileOps::getFileName(sym.data(gs)->loc().file().data(gs).path());
        baseName = baseName.substr(0, baseName.rfind(".rb"));
        linkageType = llvm::Function::ExternalLinkage;
    }
    auto entryFunc = llvm::Function::Create(ft, linkageType, {"Init_", (string)baseName}, *gs.module);
    globalInitializers->insertInto(entryFunc);

    builder.SetInsertPoint(globalInitializers);
    auto bb = llvm::BasicBlock::Create(gs, "entry", entryFunc);
    builder.CreateBr(bb);
    builder.SetInsertPoint(bb);

    // Call the LLVM method that was made by LLVMIREmitter from this Init_ method
    if (isSpecialEntrypoint) {
        auto staticInitFunc =
            gs.module->getFunction(gs.gs.lookupStaticInitForFile(sym.data(gs)->loc()).data(gs)->toStringFullName(gs));
        ENFORCE(staticInitFunc);
        builder.CreateCall(staticInitFunc,
                           {llvm::ConstantInt::get(gs, llvm::APInt(32, 0, true)),
                            llvm::ConstantPointerNull::get(llvm::Type::getInt64PtrTy(gs)),
                            builder.CreateCall(gs.module->getFunction("sorbet_rb_cObject"))},
                           (string)objectName);
    }

    builder.CreateRetVoid();

    ENFORCE(!llvm::verifyFunction(*entryFunc, &llvm::errs()), "see above");
    gs.runCheapOptimizations(entryFunc);
}

} // namespace sorbet::compiler
