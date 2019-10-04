// Sorbet doesn't like a library that llvm pulls in... so we have to pull it in
// beforehand
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
    auto valueType = getValueType(lctx);
    vector<llvm::Type *> args{
        llvm::Type::getInt32Ty(lctx),            // arg count
        llvm::PointerType::getUnqual(valueType), // argArray
        valueType                                // self
    };
    return llvm::FunctionType::get(valueType, args, false /*not varargs*/);
}

} // namespace

void LLVMIREmitter::run(const core::GlobalState &gs, llvm::LLVMContext &lctx, cfg::CFG &cfg,
                        std::unique_ptr<ast::MethodDef> &md, const string &functionName, llvm::Module *module) {
    auto functionType = getRubyFunctionTypeForSymbol(lctx, gs, cfg.symbol);
    auto function = llvm::Function::Create(functionType, llvm::Function::WeakAnyLinkage, functionName, module);

    llvm::IRBuilder<> builder(lctx);

    auto bb = llvm::BasicBlock::Create(lctx, "entry", function);
    builder.SetInsertPoint(bb);
    auto selfArg = (function->arg_end() - 1);
    // TODO: use https://silverhammermba.github.io/emberb/c/#parsing-arguments<Paste> to extract arguments

    // TODO: iterate over cfg.minLoops to create local variables. Initialize all of them to `nil`.
    // create them as `alloc`s and let SSA figure it out.

    for (auto it = cfg.forwardsTopoSort.rbegin(); it != cfg.forwardsTopoSort.rend(); ++it) {
        cfg::BasicBlock *bb = *it;
        if (bb == cfg.deadBlock()) {
            continue;
        }
        for (cfg::Binding &bind : bb->exprs) {
            typecase(
                bind.value.get(), [&](cfg::Ident *i) { gs.trace("Ident\n"); },
                [&](cfg::Alias *i) { gs.trace("Alias\n"); },
                [&](cfg::SolveConstraint *i) { gs.trace("SolveConstraint\n"); },
                [&](cfg::Send *i) { gs.trace("Send\n"); }, [&](cfg::Return *i) { gs.trace("Return\n"); },
                [&](cfg::BlockReturn *i) { gs.trace("BlockReturn\n"); },
                [&](cfg::LoadSelf *i) { gs.trace("LoadSelf\n"); }, [&](cfg::Literal *i) { gs.trace("Literal\n"); },
                [&](cfg::Unanalyzable *i) { gs.trace("Unanalyzable\n"); },
                [&](cfg::LoadArg *i) { gs.trace("LoadArg\n"); },
                [&](cfg::LoadYieldParams *i) { gs.trace("LoadYieldParams\n"); },
                [&](cfg::Cast *i) { gs.trace("Cast\n"); }, [&](cfg::TAbsurd *i) { gs.trace("TAbsurd\n"); });
        }
    }

    builder.CreateRet(selfArg); // we need to return something otherwise LLVM crashes. Should be removed when we
                                // implement `return` instruction(CFG always has `return nil` in the end
}

} // namespace sorbet::compiler
