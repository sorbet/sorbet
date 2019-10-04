// These violate our poisons so have to happen first
#include "llvm/IR/DerivedTypes.h" // FunctionType, StructType
#include "llvm/IR/IRBuilder.h"
// ^^^ violate our poisons
#include "ast/ast.h"
#include "cfg/CFG.h"
#include "common/typecase.h"
#include "compiler/llvm_ir_emitter/llvm_ir_emitter.h"
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
    vector<llvm::Type *> args{
        llvm::Type::getInt32Ty(lctx),                               // arg count
        llvm::PointerType::getUnqual(llvm::Type::getInt64Ty(lctx)), // argArray
        llvm::Type::getInt64Ty(lctx)                                // self
    };
    return llvm::FunctionType::get(llvm::Type::getInt64Ty(lctx), args, false /*not varargs*/);
}

} // namespace

// boxed raw value from rawData into target. Assumes that types are compatible.
void boxRawValue(llvm::LLVMContext &lctx, llvm::IRBuilder<> &builder, llvm::AllocaInst *target, llvm::Value *rawData) {
    std::vector<llvm::Value *> indices(2);
    indices[0] = llvm::ConstantInt::get(lctx, llvm::APInt(32, 0, true));
    indices[1] = indices[0];
    builder.CreateStore(rawData, builder.CreateGEP(target, indices));
}

// boxed raw value from rawData into target. Assumes that types are compatible.
llvm::Value *unboxRawValue(llvm::LLVMContext &lctx, llvm::IRBuilder<> &builder, llvm::AllocaInst *target) {
    std::vector<llvm::Value *> indices(2);
    indices[0] = llvm::ConstantInt::get(lctx, llvm::APInt(32, 0, true));
    indices[1] = indices[0];
    return builder.CreateLoad(builder.CreateGEP(target, indices));
}

void LLVMIREmitter::run(const core::GlobalState &gs, llvm::LLVMContext &lctx, cfg::CFG &cfg,
                        std::unique_ptr<ast::MethodDef> &md, const string &functionName, llvm::Module *module) {
    auto functionType = getRubyFunctionTypeForSymbol(lctx, gs, cfg.symbol);
    auto func = llvm::Function::Create(functionType, llvm::Function::WeakAnyLinkage, functionName, module);

    llvm::IRBuilder<> builder(lctx);

    auto entryBlock = llvm::BasicBlock::Create(lctx, "entry", func);
    builder.SetInsertPoint(entryBlock);
    UnorderedMap<core::LocalVariable, llvm::AllocaInst *> llvmVariables;
    auto nilValueRaw = builder.CreateCall(module->getFunction("sorbet_rubyNil"), {}, "nilValueRaw");
    auto falseValueRaw = builder.CreateCall(module->getFunction("sorbet_rubyFalse"), {}, "falseValueRaw");
    auto trueValueRaw = builder.CreateCall(module->getFunction("sorbet_rubyTrue"), {}, "trueValueRaw");
    for (const auto &entry : cfg.minLoops) {
        auto var = entry.first;
        auto alloca = llvmVariables[var] = builder.CreateAlloca(
            getValueType(lctx), nullptr,
            var.toString(gs)); // TODO: toString here is slow, we should probably only use it in debug builds
        boxRawValue(lctx, builder, alloca, nilValueRaw);
    }
    // auto argCountRaw = func->arg_begin();
    {
        // box `self`
        auto selfArgRaw = (func->arg_end() - 1);
        boxRawValue(lctx, builder, llvmVariables[core::LocalVariable::selfVariable()], selfArgRaw);
    }
    // Should be removed when we implement `return`
    // instruction(CFG always has `return nil` in the end
    //
    // TODO: use https://silverhammermba.github.io/emberb/c/#parsing-arguments to extract arguments
    // and box them to "RV" type

    vector<llvm::BasicBlock *> llvmBlocks;
    for (auto &b : cfg.basicBlocks) {
        if (b.get() == cfg.entry()) {
            llvmBlocks.emplace_back(entryBlock);
        } else {
            llvmBlocks.emplace_back(llvm::BasicBlock::Create(
                lctx, "BB" + to_string(b->id), func)); // to_s is slow. We should only use it in debug builds
        }
    }
    for (auto it = cfg.forwardsTopoSort.rbegin(); it != cfg.forwardsTopoSort.rend(); ++it) {
        cfg::BasicBlock *bb = *it;
        auto block = llvmBlocks[bb->id];
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
                        auto rawCString = builder.CreateGlobalStringPtr(llvm::StringRef(str.data(), str.length()));
                        auto rawID = builder.CreateCall(module->getFunction("sorbet_IDIntern"), {rawCString});
                        // we should compute these ^^^ on load are reuse them
                        auto argArray =
                            builder.CreateAlloca(llvm::Type::getInt64Ty(lctx),
                                                 llvm::ConstantInt::get(lctx, llvm::APInt(64, i->args.size(), true)));

                        // fill in args
                        {
                            int argId = -1;
                            for (auto &arg : i->args) {
                                argId += 1;
                                std::vector<llvm::Value *> indices(1);
                                indices[0] =

                                    llvm::ConstantInt::get(lctx, llvm::APInt(64, argId, true));
                                builder.CreateStore(unboxRawValue(lctx, builder, llvmVariables[arg.variable]),
                                                    builder.CreateGEP(argArray, indices));
                            }
                        }
                        std::vector<llvm::Value *> indices(1);
                        indices[0] = llvm::ConstantInt::get(lctx, llvm::APInt(64, 0, true));

                        auto rawCall =
                            builder.CreateCall(module->getFunction("sorbet_callFunc"),
                                               {unboxRawValue(lctx, builder, llvmVariables[i->recv.variable]), rawID,
                                                llvm::ConstantInt::get(llvm::Type::getInt32Ty(lctx),
                                                                       llvm::APInt(32, i->args.size(), true)),
                                                builder.CreateGEP(argArray, indices)});
                        boxRawValue(lctx, builder, targetAlloca, rawCall);
                    },
                    [&](cfg::Return *i) {
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
                                    {llvm::ConstantInt::get(lctx, llvm::APInt(64, litType->value, true))});
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
                                    {rawCString, llvm::ConstantInt::get(lctx, llvm::APInt(64, str.length(), true))});
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
            }
            if (bb->bexit.thenb != bb->bexit.elseb) {
                auto condValue =
                    builder.CreateCall(module->getFunction("sorbet_testIsTruthy"),
                                       {unboxRawValue(lctx, builder, llvmVariables[bb->bexit.cond.variable])});
                builder.CreateCondBr(condValue, llvmBlocks[bb->bexit.thenb->id], llvmBlocks[bb->bexit.elseb->id]);
            } else {
                builder.CreateBr(llvmBlocks[bb->bexit.thenb->id]);
            }
        } else {
            // handle dead block. TODO: this should throw
            builder.CreateRet(nilValueRaw);
        }
    }
} // namespace sorbet::compiler

} // namespace sorbet::compiler
