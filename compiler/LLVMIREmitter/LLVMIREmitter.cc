// These violate our poisons so have to happen first
#include "llvm/IR/Attributes.h"
#include "llvm/IR/DerivedTypes.h" // FunctionType, StructType
#include "llvm/IR/IRBuilder.h"
// ^^^ violate our poisons
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "cfg/CFG.h"
#include "common/typecase.h"
#include "compiler/LLVMIREmitter/LLVMIREmitter.h"
#include <string_view>

using namespace std;
namespace sorbet::compiler {

// https://docs.ruby-lang.org/en/2.6.0/extension_rdoc.html
// and https://silverhammermba.github.io/emberb/c/ are your friends
// use the `demo` module for experiments
namespace {

// this is the type that we'll use to represent ruby VALUE types. We box it to get a typed IR
llvm::Type *getValueType(llvm::LLVMContext &lctx) {
    auto intType = llvm::Type::getInt64Ty(lctx);
    return llvm::StructType::create(lctx, intType, "RV");
}

llvm::FunctionType *getRubyFunctionTypeForSymbol(llvm::LLVMContext &lctx, const core::GlobalState &gs,
                                                 core::SymbolRef sym) {
    llvm::Type *args[] = {
        llvm::Type::getInt32Ty(lctx),    // arg count
        llvm::Type::getInt64PtrTy(lctx), // argArray
        llvm::Type::getInt64Ty(lctx)     // self
    };
    return llvm::FunctionType::get(llvm::Type::getInt64Ty(lctx), args, false /*not varargs*/);
}

} // namespace

// boxed raw value from rawData into target. Assumes that types are compatible.
void boxRawValue(llvm::LLVMContext &lctx, llvm::IRBuilder<> &builder, llvm::AllocaInst *target, llvm::Value *rawData) {
    llvm::Value *indices[] = {llvm::ConstantInt::get(lctx, llvm::APInt(32, 0, true)),
                              llvm::ConstantInt::get(lctx, llvm::APInt(32, 0, true))};
    builder.CreateStore(rawData, builder.CreateGEP(target, indices));
}

// boxed raw value from rawData into target. Assumes that types are compatible.
llvm::Value *unboxRawValue(llvm::LLVMContext &lctx, llvm::IRBuilder<> &builder, llvm::AllocaInst *target) {
    llvm::Value *indices[] = {llvm::ConstantInt::get(lctx, llvm::APInt(32, 0, true)),
                              llvm::ConstantInt::get(lctx, llvm::APInt(32, 0, true))};
    return builder.CreateLoad(builder.CreateGEP(target, indices), "rawRubyValue");
}

llvm::Value *getIdFor(llvm::LLVMContext &lctx, llvm::IRBuilder<> &builder, string_view idName,
                      UnorderedMap<string_view, llvm::Value *> &rubyIdRegistry, llvm::BasicBlock *globalInitializers,
                      llvm::BasicBlock *functionEntry, llvm::Module *module) {
    auto &global = rubyIdRegistry[idName];
    auto zero = llvm::ConstantInt::get(llvm::Type::getInt64Ty(lctx), 0);

    auto name = llvm::StringRef(idName.data(), idName.length());
    llvm::Constant *indices[] = {zero};
    if (!global) {
        string rawName = "rubyId_global_" + (string)idName;
        auto tp = llvm::Type::getInt64Ty(lctx);
        auto globalDeclaration = static_cast<llvm::GlobalVariable *>(module->getOrInsertGlobal(rawName, tp, [&] {
            auto ret =
                new llvm::GlobalVariable(*module, tp, false, llvm::GlobalVariable::InternalLinkage, zero, rawName);
            ret->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);
            ret->setAlignment(8);
            return ret;
        }));

        llvm::IRBuilder<> globalInitBuilder(lctx);
        llvm::Constant *indicesString[] = {zero, zero};
        globalInitBuilder.SetInsertPoint(globalInitializers);
        auto gv = builder.CreateGlobalString(name, {"str_global_", name}, 0);
        auto rawCString = llvm::ConstantExpr::getInBoundsGetElementPtr(gv->getValueType(), gv, indicesString);
        auto rawID = globalInitBuilder.CreateCall(module->getFunction("sorbet_IDIntern"), {rawCString}, "rubyID");
        globalInitBuilder.CreateStore(rawID, llvm::ConstantExpr::getInBoundsGetElementPtr(
                                                 globalDeclaration->getValueType(), globalDeclaration, indices));
        globalInitBuilder.SetInsertPoint(functionEntry);
        global = globalInitBuilder.CreateLoad(
            llvm::ConstantExpr::getInBoundsGetElementPtr(globalDeclaration->getValueType(), globalDeclaration, indices),
            llvm::Twine("rubyID_", name));

        // todo(perf): mark these as immutable with https://llvm.org/docs/LangRef.html#llvm-invariant-start-intrinsic
    }
    return global;
}

void LLVMIREmitter::run(const core::GlobalState &gs, llvm::LLVMContext &lctx, cfg::CFG &cfg,
                        std::unique_ptr<ast::MethodDef> &md, const string &functionName, llvm::Module *module,
                        llvm::BasicBlock *globalInitializers) {
    UnorderedMap<string_view, llvm::Value *> rubyIDRegistry;

    auto functionType = getRubyFunctionTypeForSymbol(lctx, gs, cfg.symbol);
    auto func = llvm::Function::Create(functionType, llvm::Function::ExternalLinkage, functionName, module);
    func->addFnAttr(llvm::Attribute::AttrKind::StackProtectReq);
    func->addFnAttr(llvm::Attribute::AttrKind::NoUnwind);
    func->addFnAttr(llvm::Attribute::AttrKind::UWTable);
    llvm::IRBuilder<> builder(lctx);

    auto readGlobals = llvm::BasicBlock::Create(lctx, "readGlobals", func);
    auto rawEntryBlock = llvm::BasicBlock::Create(lctx, "entry", func);
    auto userBodyEntry = llvm::BasicBlock::Create(lctx, "userBody", func);
    builder.SetInsertPoint(rawEntryBlock);
    UnorderedMap<core::LocalVariable, llvm::AllocaInst *> llvmVariables;
    auto nilValueRaw = builder.CreateCall(module->getFunction("sorbet_rubyNil"), {}, "nilValueRaw");
    auto falseValueRaw = builder.CreateCall(module->getFunction("sorbet_rubyFalse"), {}, "falseValueRaw");
    auto trueValueRaw = builder.CreateCall(module->getFunction("sorbet_rubyTrue"), {}, "trueValueRaw");
    auto valueType = getValueType(lctx);

    // nill out variables.
    for (const auto &entry : cfg.minLoops) {
        auto var = entry.first;
        auto alloca = llvmVariables[var] = builder.CreateAlloca(
            valueType, nullptr,
            var.toString(gs)); // TODO: toString here is slow, we should probably only use it in debug builds
        boxRawValue(lctx, builder, alloca, nilValueRaw);
    }

    auto argCountRaw = func->arg_begin();
    auto argArrayRaw = func->arg_begin() + 1;
    auto requiredArgumentCount =
        md->args.size() - 1; // todo: make optional arguments actually optional. -1 because of block args
    {
        // validate arg count
        auto argCountSuccessBlock = llvm::BasicBlock::Create(lctx, "argCountSuccess", func);
        auto argCountFailBlock = llvm::BasicBlock::Create(lctx, "argCountFailBlock", func);
        auto isWrongArgCount = builder.CreateICmpNE(
            argCountRaw,
            llvm::ConstantInt::get(llvm::Type::getInt32Ty(lctx), llvm::APInt(32, requiredArgumentCount, true)),
            "isWrongArgCount");

        builder.CreateCall(
            llvm::Intrinsic::getDeclaration(module, llvm::Intrinsic::ID::expect, {llvm::Type::getInt1Ty(lctx)}),
            {isWrongArgCount, llvm::ConstantInt::get(llvm::Type::getInt1Ty(lctx), llvm::APInt(1, 0, true))});
        builder.CreateCondBr(isWrongArgCount, argCountFailBlock, argCountSuccessBlock);

        builder.SetInsertPoint(argCountFailBlock);
        builder.CreateCall(
            module->getFunction("rb_error_arity"),
            {argCountRaw,
             llvm::ConstantInt::get(llvm::Type::getInt32Ty(lctx), llvm::APInt(32, requiredArgumentCount, true)),
             llvm::ConstantInt::get(llvm::Type::getInt32Ty(lctx), llvm::APInt(32, requiredArgumentCount, true))

            });
        builder.CreateUnreachable();
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
            llvm::Value *indices[] = {llvm::ConstantInt::get(lctx, llvm::APInt(32, argId, true))};
            auto rawValue = builder.CreateLoad(builder.CreateGEP(argArrayRaw, indices), "rawArgValue");
            boxRawValue(lctx, builder, llvmVariables[a->localVariable], rawValue);
        }
    }
    {
        // box `self`
        auto selfArgRaw = (func->arg_begin() + 2);
        boxRawValue(lctx, builder, llvmVariables[core::LocalVariable::selfVariable()], selfArgRaw);
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
            llvmBlocks[b->id] = llvm::BasicBlock::Create(lctx, "BB" + to_string(b->id),
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
    builder.SetInsertPoint(readGlobals);
    auto sendArgArray =
        builder.CreateAlloca(llvm::ArrayType::get(llvm::Type::getInt64Ty(lctx), maxSendArgCount), nullptr, "callArgs");

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
                        builder.CreateStore(builder.CreateLoad(llvmVariables[i->what]), targetAlloca);
                    },
                    [&](cfg::Alias *i) { gs.trace("Alias\n"); },
                    [&](cfg::SolveConstraint *i) { gs.trace("SolveConstraint\n"); },
                    [&](cfg::Send *i) {
                        auto str = i->fun.data(gs)->shortName(gs);
                        /*
                        if (isMagic(str)) {
                            builder.CreateCall(module->getFunction("<static-init>"), {rawCString});
                            return
                        }
                        */
                        auto rawId =
                            getIdFor(lctx, builder, str, rubyIDRegistry, globalInitializers, readGlobals, module);

                        // fill in args
                        {
                            int argId = -1;
                            for (auto &arg : i->args) {
                                argId += 1;
                                llvm::Value *indices[] = {llvm::ConstantInt::get(lctx, llvm::APInt(32, 0, true)),
                                                          llvm::ConstantInt::get(lctx, llvm::APInt(64, argId, true))};
                                builder.CreateStore(unboxRawValue(lctx, builder, llvmVariables[arg.variable]),
                                                    builder.CreateGEP(sendArgArray, indices, "callArgsAddr"));
                            }
                        }
                        llvm::Value *indices[] = {llvm::ConstantInt::get(lctx, llvm::APInt(64, 0, true)),
                                                  llvm::ConstantInt::get(lctx, llvm::APInt(64, 0, true))};

                        // TODO(perf): call
                        // https://github.com/ruby/ruby/blob/3e3cc0885a9100e9d1bfdb77e136416ec803f4ca/internal.h#L2372
                        // to get inline caching.
                        // before this, perf will not be good
                        //
                        //
                        // todo(perf): mark the arguments with
                        // https://llvm.org/docs/LangRef.html#llvm-invariant-start-intrinsic for duration of the call
                        auto rawCall =
                            builder.CreateCall(module->getFunction("sorbet_callFunc"),
                                               {unboxRawValue(lctx, builder, llvmVariables[i->recv.variable]), rawId,
                                                llvm::ConstantInt::get(llvm::Type::getInt32Ty(lctx),
                                                                       llvm::APInt(32, i->args.size(), true)),
                                                builder.CreateGEP(sendArgArray, indices)},
                                               "rawSendResult");
                        boxRawValue(lctx, builder, targetAlloca, rawCall);
                    },
                    [&](cfg::Return *i) {
                        isTerminated = true;
                        builder.CreateRet(unboxRawValue(lctx, builder, llvmVariables[i->what.variable]));
                    },
                    [&](cfg::BlockReturn *i) { gs.trace("BlockReturn\n"); },
                    [&](cfg::LoadSelf *i) { gs.trace("LoadSelf\n"); },
                    [&](cfg::Literal *i) {
                        gs.trace("Literal\n");
                        if (i->value->derivesFrom(gs, core::Symbols::NilClass())) {
                            boxRawValue(lctx, builder, targetAlloca, nilValueRaw);
                            return;
                        }
                        auto litType = core::cast_type<core::LiteralType>(i->value.get());
                        ENFORCE(litType);
                        switch (litType->literalKind) {
                            case core::LiteralType::LiteralTypeKind::Integer: {
                                auto rawInt = builder.CreateCall(
                                    module->getFunction("sorbet_longToRubyValue"),
                                    {llvm::ConstantInt::get(lctx, llvm::APInt(64, litType->value, true))},
                                    "rawRubyInt");
                                boxRawValue(lctx, builder, targetAlloca, rawInt);
                                break;
                            }
                            case core::LiteralType::LiteralTypeKind::True:
                                boxRawValue(lctx, builder, targetAlloca, trueValueRaw);
                                break;
                            case core::LiteralType::LiteralTypeKind::False:
                                boxRawValue(lctx, builder, targetAlloca, falseValueRaw);
                                break;
                            case core::LiteralType::LiteralTypeKind::String: {
                                auto str = core::NameRef(gs, litType->value).data(gs)->shortName(gs);
                                auto rawCString =
                                    builder.CreateGlobalStringPtr(llvm::StringRef(str.data(), str.length()));
                                auto rawRubyString = builder.CreateCall(
                                    module->getFunction("sorbet_CPtrToRubyString"),
                                    {rawCString, llvm::ConstantInt::get(lctx, llvm::APInt(64, str.length(), true))},
                                    "rawRubyStr");
                                boxRawValue(lctx, builder, targetAlloca, rawRubyString);
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
                    auto condValue = builder.CreateCall(
                        module->getFunction("sorbet_testIsTruthy"),
                        {unboxRawValue(lctx, builder, llvmVariables[bb->bexit.cond.variable])}, "cond");
                    builder.CreateCondBr(condValue, llvmBlocks[bb->bexit.thenb->id], llvmBlocks[bb->bexit.elseb->id]);
                } else {
                    builder.CreateBr(llvmBlocks[bb->bexit.thenb->id]);
                }
            }
        } else {
            // handle dead block. TODO: this should throw
            builder.CreateRet(nilValueRaw);
        }
    }

    builder.SetInsertPoint(readGlobals);
    builder.CreateBr(rawEntryBlock);
}

} // namespace sorbet::compiler
