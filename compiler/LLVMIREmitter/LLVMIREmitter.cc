// These violate our poisons so have to happen first
#include "llvm/IR/Attributes.h"
#include "llvm/IR/DerivedTypes.h" // FunctionType, StructType
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Verifier.h"
// ^^^ violate our poisons
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "cfg/CFG.h"
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

llvm::CallInst *resolveSymbol(const core::GlobalState &gs, core::SymbolRef sym, llvm::IRBuilder<> &builder,
                              llvm::Module *module) {
    sym = removeRoot(sym);
    auto str = sym.data(gs)->name.show(gs);
    auto rawCString = builder.CreateGlobalStringPtr(str, "rubyID_" + str);
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

} // namespace

llvm::Value *getIdFor(CompilerState &gs, llvm::IRBuilder<> &builder, string_view idName,
                      UnorderedMap<string_view, llvm::Value *> &rubyIdRegistry) {
    auto &global = rubyIdRegistry[idName];
    auto zero = llvm::ConstantInt::get(llvm::Type::getInt64Ty(gs), 0);

    auto name = llvm::StringRef(idName.data(), idName.length());
    llvm::Constant *indices[] = {zero};
    if (!global) {
        string rawName = "rubyId_global_" + (string)idName;
        auto tp = llvm::Type::getInt64Ty(gs);
        auto globalDeclaration = static_cast<llvm::GlobalVariable *>(gs.module->getOrInsertGlobal(rawName, tp, [&] {
            auto ret =
                new llvm::GlobalVariable(*gs.module, tp, false, llvm::GlobalVariable::InternalLinkage, zero, rawName);
            ret->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);
            ret->setAlignment(8);
            return ret;
        }));

        llvm::IRBuilder<> globalInitBuilder(gs);
        llvm::Constant *indicesString[] = {zero, zero};
        globalInitBuilder.SetInsertPoint(gs.globalInitializers);
        auto gv = builder.CreateGlobalString(name, {"str_global_", name}, 0);
        auto rawCString = llvm::ConstantExpr::getInBoundsGetElementPtr(gv->getValueType(), gv, indicesString);
        auto rawID = globalInitBuilder.CreateCall(gs.module->getFunction("sorbet_IDIntern"), {rawCString}, "rubyID");
        globalInitBuilder.CreateStore(rawID, llvm::ConstantExpr::getInBoundsGetElementPtr(
                                                 globalDeclaration->getValueType(), globalDeclaration, indices));
        globalInitBuilder.SetInsertPoint(gs.functionEntryInitializers);
        global = globalInitBuilder.CreateLoad(
            llvm::ConstantExpr::getInBoundsGetElementPtr(globalDeclaration->getValueType(), globalDeclaration, indices),
            {"rubyID_", name});

        // todo(perf): mark these as immutable with https://llvm.org/docs/LangRef.html#llvm-invariant-start-intrinsic
    }
    return global;
}

void LLVMIREmitter::run(CompilerState &gs, cfg::CFG &cfg, std::unique_ptr<ast::MethodDef> &md,
                        const string &functionName) {
    UnorderedMap<string_view, llvm::Value *> rubyIDRegistry;

    auto functionType = gs.getRubyFFIType();
    auto func = llvm::Function::Create(functionType, llvm::Function::ExternalLinkage, functionName, gs.module);
    func->addFnAttr(llvm::Attribute::AttrKind::StackProtectReq);
    func->addFnAttr(llvm::Attribute::AttrKind::NoUnwind);
    func->addFnAttr(llvm::Attribute::AttrKind::UWTable);
    llvm::IRBuilder<> builder(gs);

    ENFORCE(gs.functionEntryInitializers == nullptr, "modules shouldn't be reused");

    gs.functionEntryInitializers = llvm::BasicBlock::Create(gs, "functionEntryInitializers", func);
    auto rawEntryBlock = llvm::BasicBlock::Create(gs, "entry", func);
    auto userBodyEntry = llvm::BasicBlock::Create(gs, "userBody", func);
    builder.SetInsertPoint(rawEntryBlock);
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

    auto argCountRaw = func->arg_begin();
    auto argArrayRaw = func->arg_begin() + 1;
    auto requiredArgumentCount =
        md->args.size() - 1; // todo: make optional arguments actually optional. -1 because of block args
    {
        // validate arg count
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
    // TODO: use https://silverhammermba.github.io/emberb/c/#parsing-arguments to extract arguments
    // and box them to "RV" type
    //
    builder.CreateBr(userBodyEntry);

    vector<llvm::BasicBlock *> llvmBlocks(cfg.maxBasicBlockId);
    for (auto &b : cfg.basicBlocks) {
        if (b.get() == cfg.entry()) {
            llvmBlocks[b->id] = userBodyEntry;
        } else {
            llvmBlocks[b->id] = llvm::BasicBlock::Create(gs, {"BB", to_string(b->id)},
                                                         func); // to_s is slow. We should only use it in debug builds
        }
    }

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

    for (auto it = cfg.forwardsTopoSort.rbegin(); it != cfg.forwardsTopoSort.rend(); ++it) {
        cfg::BasicBlock *bb = *it;
        auto block = llvmBlocks[bb->id];
        bool isTerminated = false;
        builder.SetInsertPoint(block);
        if (bb != cfg.deadBlock()) {
            for (cfg::Binding &bind : bb->exprs) {
                auto targetAlloca = llvmVariables[bind.bind.variable];
                typecase(
                    bind.value.get(),
                    [&](cfg::Ident *i) {
                        // Magical call. Others use boxRawValue.
                        builder.CreateStore(builder.CreateLoad(llvmVariables[i->what]), targetAlloca);
                    },
                    [&](cfg::Alias *i) {
                        auto rawCall = resolveSymbol(gs, i->what, builder, gs.module);
                        gs.boxRawValue(builder, targetAlloca, rawCall);
                    },
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
                            auto name = sym.data(gs)->name;
                            ENFORCE(name.data(gs)->kind == core::NameKind::UNIQUE);
                            auto className = name.data(gs)->unique.original.data(gs)->show(gs);
                            auto classNameCStr = builder.CreateGlobalStringPtr(className, {"className_", className});
                            auto rawCall = resolveSymbol(gs, sym.data(gs)->superClass(), builder, gs.module);
                            builder.CreateCall(gs.module->getFunction("sorbet_defineTopLevelClass"), {classNameCStr, rawCall});
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
                            ENFORCE(funcHandle);
                            auto universalSignature =
                                llvm::PointerType::getUnqual(llvm::FunctionType::get(llvm::Type::getInt64Ty(gs), true));
                            auto ptr = builder.CreateBitCast(funcHandle, universalSignature);

                            builder.CreateCall(gs.module->getFunction("sorbet_defineMethod"),
                                               {resolveSymbol(gs, ownerSym, builder, gs.module), funcNameCStr, ptr,
                                                llvm::ConstantInt::get(gs, llvm::APInt(32, -1, true))});

                            std::vector<llvm::Type *> NoArgs(0, llvm::Type::getVoidTy(gs));
                            auto ft = llvm::FunctionType::get(llvm::Type::getVoidTy(gs), NoArgs, false);
                            auto initFunc = gs.module->getOrInsertFunction("Init_" + llvmFuncName, ft);
                            ENFORCE(initFunc);
                            builder.CreateCall(initFunc, {});
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
                        auto rawId = getIdFor(gs, builder, str, rubyIDRegistry);

                        // fill in args
                        {
                            int argId = -1;
                            for (auto &arg : i->args) {
                                argId += 1;
                                llvm::Value *indices[] = {llvm::ConstantInt::get(gs, llvm::APInt(32, 0, true)),
                                                          llvm::ConstantInt::get(gs, llvm::APInt(64, argId, true))};
                                builder.CreateStore(gs.unboxRawValue(builder, llvmVariables[arg.variable]),
                                                    builder.CreateGEP(sendArgArray, indices, "callArgsAddr"));
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
                        auto rawCall = builder.CreateCall(
                            gs.module->getFunction("sorbet_callFunc"),
                            {gs.unboxRawValue(builder, llvmVariables[i->recv.variable]), rawId,
                             llvm::ConstantInt::get(llvm::Type::getInt32Ty(gs), llvm::APInt(32, i->args.size(), true)),
                             builder.CreateGEP(sendArgArray, indices)},
                            "rawSendResult");
                        gs.boxRawValue(builder, targetAlloca, rawCall);
                    },
                    [&](cfg::Return *i) {
                        isTerminated = true;
                        builder.CreateRet(gs.unboxRawValue(builder, llvmVariables[i->what.variable]));
                    },
                    [&](cfg::BlockReturn *i) { gs.trace("BlockReturn\n"); },
                    [&](cfg::LoadSelf *i) { gs.trace("LoadSelf\n"); },
                    [&](cfg::Literal *i) {
                        gs.trace("Literal\n");
                        if (i->value->derivesFrom(gs, core::Symbols::NilClass())) {
                            gs.boxRawValue(builder, targetAlloca, gs.getRubyNilRaw(builder));
                            return;
                        }
                        auto litType = core::cast_type<core::LiteralType>(i->value.get());
                        ENFORCE(litType);
                        switch (litType->literalKind) {
                            case core::LiteralType::LiteralTypeKind::Integer: {
                                auto rawInt = gs.getRubyIntRaw(builder, litType->value);
                                gs.boxRawValue(builder, targetAlloca, rawInt);

                                break;
                            }
                            case core::LiteralType::LiteralTypeKind::True:
                                gs.boxRawValue(builder, targetAlloca, gs.getRubyTrueRaw(builder));
                                break;
                            case core::LiteralType::LiteralTypeKind::False:
                                gs.boxRawValue(builder, targetAlloca, gs.getRubyFalseRaw(builder));
                                break;
                            case core::LiteralType::LiteralTypeKind::String: {
                                auto str = core::NameRef(gs, litType->value).data(gs)->shortName(gs);
                                auto rawRubyString = gs.getRubyStringRaw(builder, str);
                                gs.boxRawValue(builder, targetAlloca, rawRubyString);
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
                    auto condValue =
                        gs.getIsTruthyU1(builder, gs.unboxRawValue(builder, llvmVariables[bb->bexit.cond.variable]));

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

    builder.SetInsertPoint(gs.functionEntryInitializers);
    builder.CreateBr(rawEntryBlock);
    /* run verifier */
    ENFORCE(!llvm::verifyFunction(*func, &llvm::errs()), "see above");
}

} // namespace sorbet::compiler
