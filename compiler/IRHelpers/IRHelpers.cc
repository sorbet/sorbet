#include "IRHelpers.h"
#include "llvm/Bitcode/BitcodeReader.h"
#include "llvm/IR/DerivedTypes.h" // FunctionType, StructType
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include <string_view>
// ^^^ violate poisons
#include "compiler/Errors/Errors.h"
#include "core/core.h"
using namespace std;
namespace sorbet::compiler {
string_view getDefaultModuleBitcode();

std::unique_ptr<llvm::Module> IRHelpers::readDefaultModule(const char *name, llvm::LLVMContext &lctx) {
    auto bitCode = getDefaultModuleBitcode();
    llvm::StringRef bitcodeAsStringRef(bitCode.data(), bitCode.size());
    auto memoryBuffer = llvm::MemoryBuffer::getMemBuffer(bitcodeAsStringRef, "payload", false);
    auto ret = llvm::parseBitcodeFile(*memoryBuffer, lctx);
    ret.get()->getFunction("sorbet_only_exists_to_keep_functions_alive")->eraseFromParent();
    return move(ret.get());
}

CompilerState::CompilerState(const core::GlobalState &gs, llvm::LLVMContext &lctx, llvm::Module *module)
    : gs(gs), lctx(lctx), functionEntryInitializers(nullptr), module(module) {}

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
