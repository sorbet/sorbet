#include "llvm/IR/DerivedTypes.h" // FunctionType, StructType
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
// ^^^ violate poisons
#include "compiler/Core/AbortCompilation.h"
#include "compiler/Core/CompilerState.h"
#include "compiler/Errors/Errors.h"
#include "core/core.h"
#include <string_view>

using namespace std;
namespace sorbet::compiler {

CompilerState::CompilerState(const core::GlobalState &gs, llvm::LLVMContext &lctx, llvm::Module *module,
                             core::FileRef file, llvm::BasicBlock *globalConstructorsEntry)
    : gs(gs), lctx(lctx), module(module), globalConstructorsEntry(globalConstructorsEntry),
      functionEntryInitializers(nullptr), file(file) {}

llvm::StructType *CompilerState::getValueType() {
    auto intType = llvm::Type::getInt64Ty(lctx);
    return llvm::StructType::create(lctx, intType, "RV");
};

void CompilerState::failCompilation(const core::Loc &loc, ConstExprStr msg) const {
    if (auto e = gs.beginError(loc, core::errors::Compiler::Unanalyzable)) {
        e.setHeader(msg);
    }

    throw AbortCompilation(msg.str);
}

void CompilerState::trace(string_view msg) const {
    gs.trace(msg);
}

llvm::FunctionType *CompilerState::getRubyFFIType() {
    llvm::Type *args[] = {
        llvm::Type::getInt32Ty(lctx),    // arg count
        llvm::Type::getInt64PtrTy(lctx), // argArray
        llvm::Type::getInt64Ty(lctx)     // self
    };
    return llvm::FunctionType::get(llvm::Type::getInt64Ty(lctx), args, false /*not varargs*/);
}

llvm::FunctionType *CompilerState::getRubyBlockFFIType() {
    llvm::Type *args[] = {
        llvm::Type::getInt64Ty(lctx),    // first yielded argument(first argument is both here and in argArray
        llvm::Type::getInt64Ty(lctx),    // data
        llvm::Type::getInt32Ty(lctx),    // arg count
        llvm::Type::getInt64PtrTy(lctx), // argArray
        llvm::Type::getInt64Ty(lctx),    // blockArg
    };
    return llvm::FunctionType::get(llvm::Type::getInt64Ty(lctx), args, false /*not varargs*/);
}

llvm::FunctionType *CompilerState::getRubyExceptionFFIType() {
    llvm::Type *args[] = {
        llvm::Type::getInt64PtrTy(lctx)->getPointerTo(), // VALUE **pc
        llvm::Type::getInt64PtrTy(lctx),                 // VALUE *iseq_encoded
        llvm::Type::getInt64Ty(lctx),                    // VALUE captures
    };
    return llvm::FunctionType::get(llvm::Type::getInt64Ty(lctx), args, false /*not varargs*/);
}

llvm::FunctionType *CompilerState::getSorbetIntrinsicFFIType() {
    llvm::Type *args[] = {
        llvm::Type::getInt64Ty(lctx),          // self
        llvm::Type::getInt32Ty(lctx),          // arg count
        llvm::Type::getInt64PtrTy(lctx),       // argArray
        getRubyBlockFFIType()->getPointerTo(), // block
        llvm::Type::getInt64Ty(lctx)           // closure

    };
    return llvm::FunctionType::get(llvm::Type::getInt64Ty(lctx), args, false /*not varargs*/);
}

void CompilerState::runCheapOptimizations(llvm::Function *func) {
    llvm::legacy::FunctionPassManager pm(module);
    llvm::PassManagerBuilder pmbuilder;
    int optLevel = 2;
    int sizeLevel = 0;
    pmbuilder.OptLevel = optLevel;
    pmbuilder.SizeLevel = sizeLevel;
    pmbuilder.Inliner = llvm::createFunctionInliningPass(optLevel, sizeLevel, false);
    pmbuilder.DisableUnrollLoops = false;
    pmbuilder.LoopVectorize = true;
    pmbuilder.SLPVectorize = true;
    pmbuilder.VerifyInput = debug_mode;
    pmbuilder.populateFunctionPassManager(pm);
    pm.run(*func);
}

} // namespace sorbet::compiler
