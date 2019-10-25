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

std::string showClassNameWithoutOwner(const core::GlobalState &gs, core::SymbolRef sym) {
    auto name = sym.data(gs)->name;
    if (name.data(gs)->kind == core::NameKind::UNIQUE) {
        return name.data(gs)->unique.original.data(gs)->show(gs);
    }
    return name.data(gs)->show(gs);
}

std::string showClassName(const core::GlobalState &gs, core::SymbolRef sym) {
    bool includeOwner = sym.data(gs)->owner.exists() && sym.data(gs)->owner != core::Symbols::root();
    string owner = includeOwner ? showClassName(gs, sym.data(gs)->owner) + "::" : "";
    return owner + showClassNameWithoutOwner(gs, sym);
}

llvm::Constant *toCString(std::string str, llvm::IRBuilder<> &builder) {
    return builder.CreateGlobalStringPtr(str, "str_" + str);
}

llvm::CallInst *resolveSymbol(CompilerState &cs, core::SymbolRef sym, llvm::IRBuilder<> &builder) {
    // TODO(perf): use something similar to
    // https://git.corp.stripe.com/stripe-internal/ruby/blob/48bf9833/vm_insnhelper.c#L3258-L3275
    sym = removeRoot(sym);
    auto str = showClassName(cs, sym);
    ENFORCE(str.length() < 2 || (str[0] != ':'), "implementation assumes that strings dont start with ::");
    return builder.CreateCall(cs.module->getFunction("sorbet_getConstant"),
                              {toCString(str, builder), llvm::ConstantInt::get(cs, llvm::APInt(64, str.length()))});
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

vector<llvm::Function *> getRubyBlocks2FunctionsMapping(CompilerState &cs, cfg::CFG &cfg, llvm::Function *func) {
    vector<llvm::Function *> res;
    res.emplace_back(func);
    return res;
}

llvm::Value *varGet(CompilerState &cs, core::LocalVariable var, llvm::IRBuilder<> &builder,
                    const UnorderedMap<core::LocalVariable, llvm::AllocaInst *> &llvmVariables,
                    const UnorderedMap<llvm::AllocaInst *, core::SymbolRef> &aliases) {
    auto ret = llvmVariables.at(var);
    if (!aliases.contains(ret)) {
        return cs.unboxRawValue(builder, ret);
    }

    return resolveSymbol(cs, aliases.at(ret), builder);
}

void varSet(CompilerState &cs, llvm::AllocaInst *alloca, llvm::Value *var, llvm::IRBuilder<> &builder,
            const UnorderedMap<core::LocalVariable, llvm::AllocaInst *> &llvmVariables,
            UnorderedMap<llvm::AllocaInst *, core::SymbolRef> &aliases) {
    cs.boxRawValue(builder, alloca, var);
    if (!aliases.contains(alloca)) {
        return;
    }

    auto sym = aliases.at(alloca);
    auto name = sym.data(cs.gs)->name.show(cs.gs);
    auto owner = sym.data(cs.gs)->owner;
    builder.CreateCall(cs.module->getFunction("sorbet_setConstant"),
                       {resolveSymbol(cs, owner, builder), toCString(name, builder),
                        llvm::ConstantInt::get(cs, llvm::APInt(64, name.length())), var});
}

llvm::Function *getInitFunction(CompilerState &cs, std::string baseName,
                                llvm::GlobalValue::LinkageTypes linkageType = llvm::Function::InternalLinkage) {
    auto name = "Init_" + baseName;

    auto func = cs.module->getFunction(name);
    if (func) {
        return func;
    }

    std::vector<llvm::Type *> NoArgs(0, llvm::Type::getVoidTy(cs));
    auto ft = llvm::FunctionType::get(llvm::Type::getVoidTy(cs), NoArgs, false);
    return llvm::Function::Create(ft, linkageType, name, *cs.module);
}

// Create local allocas for local variables, initialize them all to `nil`.
// load arguments, check their count
// load self
UnorderedMap<core::LocalVariable, llvm::AllocaInst *>
setupArgumentsAndLocalVariables(CompilerState &cs, llvm::IRBuilder<> &builder, cfg::CFG &cfg,
                                unique_ptr<ast::MethodDef> &md, llvm::Function *func,
                                vector<llvm::Function *> rubyBlocks2Functions) {
    UnorderedMap<core::LocalVariable, llvm::AllocaInst *> llvmVariables;
    {
        // nill out variables.
        auto valueType = cs.getValueType();
        auto nilValueRaw = cs.getRubyNilRaw(builder);
        for (const auto &entry : cfg.minLoops) {
            auto var = entry.first;
            auto svName = var._name.data(cs)->shortName(cs);
            auto alloca = llvmVariables[var] =
                builder.CreateAlloca(valueType, nullptr, llvm::StringRef(svName.data(), svName.length()));
            cs.boxRawValue(builder, alloca, nilValueRaw);
        }
    }
    auto maxArgCount = 0;
    auto minArgCount = 0;
    {
        for (auto &arg : md->args) {
            if (ast::isa_tree<ast::OptionalArg>(arg.get())) {
                maxArgCount += 1;
                continue;
            }
            auto local = ast::cast_tree<ast::Local>(arg.get());
            ENFORCE(local);
            if (local->localVariable._name == core::Names::blkArg()) {
                continue;
            }
            maxArgCount += 1;
            minArgCount += 1;
        }
    }
    {
        // validate arg count
        auto argCountRaw = func->arg_begin();
        auto argCountFailBlock = llvm::BasicBlock::Create(cs, "argCountFailBlock", func);
        auto argCountSecondCheckBlock = llvm::BasicBlock::Create(cs, "argCountSecondCheckBlock", func);
        auto argCountSuccessBlock = llvm::BasicBlock::Create(cs, "argCountSuccess", func);

        auto tooManyArgs =
            builder.CreateICmpUGT(argCountRaw, llvm::ConstantInt::get(cs, llvm::APInt(32, maxArgCount)), "tooManyArgs");
        cs.setExpectedBool(builder, tooManyArgs, false);
        builder.CreateCondBr(tooManyArgs, argCountFailBlock, argCountSecondCheckBlock);

        builder.SetInsertPoint(argCountSecondCheckBlock);
        auto tooFewArgs =
            builder.CreateICmpULT(argCountRaw, llvm::ConstantInt::get(cs, llvm::APInt(32, minArgCount)), "tooFewArgs");
        cs.setExpectedBool(builder, tooFewArgs, false);
        builder.CreateCondBr(tooFewArgs, argCountFailBlock, argCountSuccessBlock);

        builder.SetInsertPoint(argCountFailBlock);
        cs.emitArgumentMismatch(builder, argCountRaw, minArgCount, maxArgCount);

        builder.SetInsertPoint(argCountSuccessBlock);
    }
    {
        // box required args
        int argId = -1;
        for (const auto &arg : md->args) {
            argId += 1;
            auto *a = ast::MK::arg2Local(arg.get());
            llvm::Value *indices[] = {llvm::ConstantInt::get(cs, llvm::APInt(32, argId, true))};
            auto name = a->localVariable._name.data(cs)->shortName(cs);
            llvm::StringRef nameRef(name.data(), name.length());
            auto argArrayRaw = func->arg_begin() + 1;
            auto rawValue = builder.CreateLoad(builder.CreateGEP(argArrayRaw, indices), {"rawArg_", nameRef});
            cs.boxRawValue(builder, llvmVariables[a->localVariable], rawValue);
        }
    }
    {
        // box `self`
        auto selfArgRaw = (func->arg_begin() + 2);
        cs.boxRawValue(builder, llvmVariables[core::LocalVariable::selfVariable()], selfArgRaw);
    }

    return llvmVariables;
}

vector<llvm::BasicBlock *> getSorbetBlocks2LLVMBlockMapping(CompilerState &cs, cfg::CFG &cfg, llvm::Function *func,
                                                            llvm::BasicBlock *llvmFuncEntry,
                                                            vector<llvm::Function *> rubyBlocks2Functions) {
    vector<llvm::BasicBlock *> llvmBlocks(cfg.maxBasicBlockId);
    for (auto &b : cfg.basicBlocks) {
        if (b.get() == cfg.entry()) {
            llvmBlocks[b->id] = llvmFuncEntry;
        } else {
            llvmBlocks[b->id] = llvm::BasicBlock::Create(cs, {"BB", to_string(b->id)},
                                                         func); // to_s is slow. We should only use it in debug builds
        }
    }
    return llvmBlocks;
}

void defineMethod(CompilerState &cs, cfg::Send *i, bool isSelf, llvm::IRBuilder<> builder) {
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

    auto llvmFuncName = funcSym.data(cs)->toStringFullName(cs);
    auto funcHandle = cs.module->getFunction(llvmFuncName);
    if (funcHandle == nullptr) {
        funcHandle =
            llvm::Function::Create(cs.getRubyFFIType(), getFunctionLinkageType(cs, funcSym), llvmFuncName, *cs.module);
    }
    auto universalSignature = llvm::PointerType::getUnqual(llvm::FunctionType::get(llvm::Type::getInt64Ty(cs), true));
    auto ptr = builder.CreateBitCast(funcHandle, universalSignature);

    auto rubyFunc = cs.module->getFunction(isSelf ? "sorbet_defineMethodSingleton" : "sorbet_defineMethod");
    ENFORCE(rubyFunc);
    builder.CreateCall(rubyFunc, {resolveSymbol(cs, ownerSym, builder), toCString(funcNameRef.show(cs), builder), ptr,
                                  llvm::ConstantInt::get(cs, llvm::APInt(32, -1, true))});

    builder.CreateCall(getInitFunction(cs, llvmFuncName), {});
}

void defineClass(CompilerState &cs, cfg::Send *i, llvm::IRBuilder<> builder) {
    auto sym = typeToSym(cs, i->args[0].type);
    auto classNameCStr = toCString(showClassNameWithoutOwner(cs, sym), builder);

    if (sym.data(cs)->owner != core::Symbols::root()) {
        auto getOwner = resolveSymbol(cs, sym.data(cs)->owner, builder);
        if (sym.data(cs)->superClass() == core::Symbols::Module()) {
            builder.CreateCall(cs.module->getFunction("sorbet_defineNestedModule"), {getOwner, classNameCStr});
        } else {
            auto rawCall = resolveSymbol(cs, sym.data(cs)->superClass(), builder);
            builder.CreateCall(cs.module->getFunction("sorbet_defineNestedClass"), {getOwner, classNameCStr, rawCall});
        }
    } else {
        if (sym.data(cs)->superClass() == core::Symbols::Module()) {
            builder.CreateCall(cs.module->getFunction("sorbet_defineTopLevelModule"), {classNameCStr});
        } else {
            auto rawCall = resolveSymbol(cs, sym.data(cs)->superClass(), builder);
            builder.CreateCall(cs.module->getFunction("sorbet_defineTopClassOrModule"), {classNameCStr, rawCall});
        }
    }

    auto funcSym = cs.gs.lookupStaticInitForClass(sym.data(cs)->attachedClass(cs));
    auto llvmFuncName = funcSym.data(cs)->toStringFullName(cs);
    builder.CreateCall(getInitFunction(cs, llvmFuncName), {});
}

void trackBlockUsage(CompilerState &cs, cfg::CFG &cfg, core::LocalVariable lv, cfg::BasicBlock *bb) {
    // TODO
}

UnorderedMap<core::LocalVariable, int> findCaptures(CompilerState &cs, cfg::CFG &cfg) {
    UnorderedMap<core::LocalVariable, int> ret;

    for (auto &bb : cfg.basicBlocks) {
        for (cfg::Binding &bind : bb->exprs) {
            trackBlockUsage(cs, cfg, bind.bind.variable, bb.get());
            typecase(
                bind.value.get(), [&](cfg::Ident *i) { trackBlockUsage(cs, cfg, i->what, bb.get()); },
                [&](cfg::Alias *i) { /* nothing */ }, [&](cfg::SolveConstraint *i) { /* nothing*/ },
                [&](cfg::Send *i) {
                    for (auto &arg : i->args) {
                        trackBlockUsage(cs, cfg, arg.variable, bb.get());
                    }
                    trackBlockUsage(cs, cfg, i->recv.variable, bb.get());
                },
                [&](cfg::Return *i) { trackBlockUsage(cs, cfg, i->what.variable, bb.get()); },
                [&](cfg::BlockReturn *i) { trackBlockUsage(cs, cfg, i->what.variable, bb.get()); },
                [&](cfg::LoadSelf *i) { /*nothing*/ /*todo: how does instance exec pass self?*/ },
                [&](cfg::Literal *i) { /* nothing*/ }, [&](cfg::Unanalyzable *i) { cs.trace("Unanalyzable\n"); },
                [&](cfg::LoadArg *i) { /*nothing*/ }, [&](cfg::LoadYieldParams *i) { cs.trace("LoadYieldParams\n"); },
                [&](cfg::Cast *i) { trackBlockUsage(cs, cfg, i->value.variable, bb.get()); },
                [&](cfg::TAbsurd *i) { /*nothing*/ });
        }
    }
    return ret;
}

void emitUserBody(CompilerState &cs, cfg::CFG &cfg, const vector<llvm::BasicBlock *> llvmBlocks,
                  const UnorderedMap<core::LocalVariable, llvm::AllocaInst *> llvmVariables,
                  llvm::AllocaInst *sendArgArray) {
    UnorderedMap<llvm::AllocaInst *, core::SymbolRef> aliases;
    llvm::IRBuilder<> builder(cs);
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
                        auto var = varGet(cs, i->what, builder, llvmVariables, aliases);
                        varSet(cs, targetAlloca, var, builder, llvmVariables, aliases);
                    },
                    [&](cfg::Alias *i) { aliases[targetAlloca] = i->what; },
                    [&](cfg::SolveConstraint *i) { cs.trace("SolveConstraint\n"); },
                    [&](cfg::Send *i) {
                        auto str = i->fun.data(cs)->shortName(cs);
                        if (i->fun == core::Names::buildHash()) {
                            auto ret = builder.CreateCall(cs.module->getFunction("rb_hash_new"), {}, "rawHashLiteral");
                            // TODO(perf): in 2.7 use rb_hash_bulk_insert will give 2x speedup
                            int argc = 0;
                            while (argc < i->args.size()) {
                                auto key = i->args[argc].variable;
                                auto value = i->args[argc + 1].variable;
                                builder.CreateCall(cs.module->getFunction("rb_hash_aset"),
                                                   {ret, varGet(cs, key, builder, llvmVariables, aliases),
                                                    varGet(cs, value, builder, llvmVariables, aliases)});
                                argc += 2;
                            }
                            varSet(cs, targetAlloca, ret, builder, llvmVariables, aliases);
                            return;
                        }
                        if (i->fun == Names::sorbet_defineTopClassOrModule) {
                            defineClass(cs, i, builder);
                            return;
                        }
                        if (i->fun == Names::sorbet_defineMethod) {
                            defineMethod(cs, i, false, builder);
                            return;
                        }
                        if (i->fun == Names::sorbet_defineMethodSingleton) {
                            defineMethod(cs, i, true, builder);
                            return;
                        }
                        auto rawId = cs.getRubyIdFor(builder, str);

                        // fill in args
                        {
                            int argId = -1;
                            for (auto &arg : i->args) {
                                argId += 1;
                                llvm::Value *indices[] = {llvm::ConstantInt::get(cs, llvm::APInt(32, 0, true)),
                                                          llvm::ConstantInt::get(cs, llvm::APInt(64, argId, true))};
                                auto var = varGet(cs, arg.variable, builder, llvmVariables, aliases);
                                builder.CreateStore(var, builder.CreateGEP(sendArgArray, indices, "callArgsAddr"));
                            }
                        }
                        llvm::Value *indices[] = {llvm::ConstantInt::get(cs, llvm::APInt(64, 0, true)),
                                                  llvm::ConstantInt::get(cs, llvm::APInt(64, 0, true))};

                        // TODO(perf): call
                        // https://github.com/ruby/ruby/blob/3e3cc0885a9100e9d1bfdb77e136416ec803f4ca/internal.h#L2372
                        // to get inline caching.
                        // before this, perf will not be good
                        auto var = varGet(cs, i->recv.variable, builder, llvmVariables, aliases);
                        auto rawCall = builder.CreateCall(
                            cs.module->getFunction("sorbet_callFunc"),
                            {var, rawId, llvm::ConstantInt::get(cs, llvm::APInt(32, i->args.size(), true)),
                             builder.CreateGEP(sendArgArray, indices)},
                            "rawSendResult");
                        varSet(cs, targetAlloca, rawCall, builder, llvmVariables, aliases);
                    },
                    [&](cfg::Return *i) {
                        isTerminated = true;
                        auto var = varGet(cs, i->what.variable, builder, llvmVariables, aliases);
                        builder.CreateRet(var);
                    },
                    [&](cfg::BlockReturn *i) { cs.trace("BlockReturn\n"); },
                    [&](cfg::LoadSelf *i) { cs.trace("LoadSelf\n"); },
                    [&](cfg::Literal *i) {
                        cs.trace("Literal\n");
                        if (i->value->derivesFrom(cs, core::Symbols::FalseClass())) {
                            varSet(cs, targetAlloca, cs.getRubyFalseRaw(builder), builder, llvmVariables, aliases);
                            return;
                        }
                        if (i->value->derivesFrom(cs, core::Symbols::TrueClass())) {
                            varSet(cs, targetAlloca, cs.getRubyTrueRaw(builder), builder, llvmVariables, aliases);
                            return;
                        }
                        if (i->value->derivesFrom(cs, core::Symbols::NilClass())) {
                            varSet(cs, targetAlloca, cs.getRubyNilRaw(builder), builder, llvmVariables, aliases);
                            return;
                        }

                        auto litType = core::cast_type<core::LiteralType>(i->value.get());
                        ENFORCE(litType);
                        switch (litType->literalKind) {
                            case core::LiteralType::LiteralTypeKind::Integer: {
                                auto rawInt = cs.getRubyIntRaw(builder, litType->value);
                                varSet(cs, targetAlloca, rawInt, builder, llvmVariables, aliases);
                                break;
                            }
                            case core::LiteralType::LiteralTypeKind::Symbol: {
                                auto str = core::NameRef(cs, litType->value).data(cs)->shortName(cs);
                                auto rawId = cs.getRubyIdFor(builder, str);
                                auto rawRubySym =
                                    builder.CreateCall(cs.module->getFunction("rb_id2sym"), {rawId}, "rawSym");
                                varSet(cs, targetAlloca, rawRubySym, builder, llvmVariables, aliases);
                                break;
                            }
                            case core::LiteralType::LiteralTypeKind::String: {
                                auto str = core::NameRef(cs, litType->value).data(cs)->shortName(cs);
                                auto rawRubyString = cs.getRubyStringRaw(builder, str);
                                varSet(cs, targetAlloca, rawRubyString, builder, llvmVariables, aliases);
                                break;
                            }
                            default:
                                cs.trace("UnsupportedLiteral");
                        }
                    },
                    [&](cfg::Unanalyzable *i) { cs.trace("Unanalyzable\n"); },
                    [&](cfg::LoadArg *i) { cs.trace("LoadArg\n"); },
                    [&](cfg::LoadYieldParams *i) { cs.trace("LoadYieldParams\n"); },
                    [&](cfg::Cast *i) { cs.trace("Cast\n"); }, [&](cfg::TAbsurd *i) { cs.trace("TAbsurd\n"); });
                if (isTerminated) {
                    break;
                }
            }
            if (!isTerminated) {
                if (bb->bexit.thenb != bb->bexit.elseb) {
                    auto var = varGet(cs, bb->bexit.cond.variable, builder, llvmVariables, aliases);
                    auto condValue = cs.getIsTruthyU1(builder, var);

                    builder.CreateCondBr(condValue, llvmBlocks[bb->bexit.thenb->id], llvmBlocks[bb->bexit.elseb->id]);
                } else {
                    builder.CreateBr(llvmBlocks[bb->bexit.thenb->id]);
                }
            }
        } else {
            // handle dead block. TODO: this should throw
            builder.CreateRet(cs.getRubyNilRaw(builder));
        }
    }
}

} // namespace

void LLVMIREmitter::run(CompilerState &cs, cfg::CFG &cfg, unique_ptr<ast::MethodDef> &md, const string &functionName) {
    auto functionType = cs.getRubyFFIType();
    auto func = llvm::Function::Create(functionType, getFunctionLinkageType(cs, md->symbol), functionName, cs.module);
    func->addFnAttr(llvm::Attribute::AttrKind::StackProtectReq);
    func->addFnAttr(llvm::Attribute::AttrKind::NoUnwind);
    func->addFnAttr(llvm::Attribute::AttrKind::UWTable);
    llvm::IRBuilder<> builder(cs);

    vector<llvm::Function *> rubyBlocks2Functions = getRubyBlocks2FunctionsMapping(cs, cfg, func);

    ENFORCE(cs.functionEntryInitializers == nullptr, "modules shouldn't be reused");

    cs.functionEntryInitializers = llvm::BasicBlock::Create(
        cs, "functionEntryInitializers",
        func); // we will build a link for this block later, after we finish building expressions into it

    auto rawEntryBlock = llvm::BasicBlock::Create(cs, "setupLocalsAndArguments", func);
    builder.SetInsertPoint(rawEntryBlock);

    UnorderedMap<core::LocalVariable, int> variablesCapturedAtLeastOnce = findCaptures(cs, cfg);

    auto userBodyEntry = llvm::BasicBlock::Create(cs, "userBody", func);

    const vector<llvm::BasicBlock *> llvmBlocks =
        getSorbetBlocks2LLVMBlockMapping(cs, cfg, func, userBodyEntry, rubyBlocks2Functions);

    const UnorderedMap<core::LocalVariable, llvm::AllocaInst *> llvmVariables =
        setupArgumentsAndLocalVariables(cs, builder, cfg, md, func, rubyBlocks2Functions);
    builder.CreateBr(userBodyEntry);

    int maxSendArgCount = 0;
    for (auto &bb : cfg.basicBlocks) {
        for (cfg::Binding &bind : bb->exprs) {
            if (auto snd = cfg::cast_instruction<cfg::Send>(bind.value.get())) {
                if (maxSendArgCount < snd->args.size()) {
                    maxSendArgCount = snd->args.size();
                }
            }
        }
    }

    builder.SetInsertPoint(cs.functionEntryInitializers);
    auto sendArgArray =
        builder.CreateAlloca(llvm::ArrayType::get(llvm::Type::getInt64Ty(cs), maxSendArgCount), nullptr, "callArgs");
    emitUserBody(cs, cfg, llvmBlocks, llvmVariables, sendArgArray);
    builder.SetInsertPoint(cs.functionEntryInitializers);
    builder.CreateBr(rawEntryBlock);
    /* run verifier */
    ENFORCE(!llvm::verifyFunction(*func, &llvm::errs()), "see above");
    cs.runCheapOptimizations(func);
}

void LLVMIREmitter::buildInitFor(CompilerState &cs, const core::SymbolRef &sym) {
    llvm::IRBuilder<> builder(cs);

    auto baseName = sym.data(cs)->toStringFullName(cs);
    auto linkageType = llvm::Function::InternalLinkage;
    auto owner = sym.data(cs)->owner;
    auto isRoot = owner == core::Symbols::rootSingleton();

    if (isStaticInit(cs, sym) && isRoot) {
        baseName = FileOps::getFileName(sym.data(cs)->loc().file().data(cs).path());
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
            staticInit = cs.gs.lookupStaticInitForFile(attachedClass.data(cs)->loc());
        } else {
            staticInit = cs.gs.lookupStaticInitForClass(attachedClass);
        }

        // Call the LLVM method that was made by run() from this Init_ method
        auto staticInitName = staticInit.data(cs)->toStringFullName(cs);
        auto staticInitFunc = cs.module->getFunction(staticInitName);
        ENFORCE(staticInitFunc);
        builder.CreateCall(staticInitFunc,
                           {llvm::ConstantInt::get(cs, llvm::APInt(32, 0, true)),
                            llvm::ConstantPointerNull::get(llvm::Type::getInt64PtrTy(cs)),
                            builder.CreateCall(cs.module->getFunction("sorbet_rb_cObject"))},
                           staticInitName);
    }

    builder.CreateRetVoid();

    ENFORCE(!llvm::verifyFunction(*entryFunc, &llvm::errs()), "see above");
    cs.runCheapOptimizations(entryFunc);
}

} // namespace sorbet::compiler
