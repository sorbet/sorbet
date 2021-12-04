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
                             llvm::DIBuilder *debug, llvm::DICompileUnit *compileUnit, core::FileRef file,
                             llvm::BasicBlock *allocRubyIdsEntry, llvm::BasicBlock *globalConstructorsEntry)
    : gs(gs), lctx(lctx), module(module), allocRubyIdsEntry(allocRubyIdsEntry),
      globalConstructorsEntry(globalConstructorsEntry), debug(debug), compileUnit(compileUnit),
      functionEntryInitializers(nullptr), file(file) {}

llvm::StructType *CompilerState::getValueType() {
    auto intType = llvm::Type::getInt64Ty(lctx);
    return llvm::StructType::create(lctx, intType, "RV");
};

void CompilerState::trace(string_view msg) const {
    gs.trace(msg);
}

llvm::FunctionType *CompilerState::getRubyFFIType() {
    llvm::Type *args[] = {
        llvm::Type::getInt32Ty(lctx),    // arg count
        llvm::Type::getInt64PtrTy(lctx), // argArray
        llvm::Type::getInt64Ty(lctx),    // self
        llvm::StructType::getTypeByName(lctx, "struct.rb_control_frame_struct")->getPointerTo(),
        llvm::Type::getInt8PtrTy(lctx), // void* (struct rb_calling_info)
        llvm::Type::getInt8PtrTy(lctx), // void* (struct rb_call_data / struct rb_kwarg_call_data)
    };
    return llvm::FunctionType::get(llvm::Type::getInt64Ty(lctx), args, false /*not varargs*/);
}

llvm::FunctionType *CompilerState::getDirectWrapperFunctionType() {
    llvm::Type *args[] = {
        llvm::StructType::getTypeByName(lctx, "struct.FunctionInlineCache")->getPointerTo(), // cache
        llvm::Type::getInt32Ty(lctx),                                                        // arg count
        llvm::Type::getInt64PtrTy(lctx),                                                     // argArray
        llvm::Type::getInt64Ty(lctx),                                                        // self
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
        llvm::Type::getInt64PtrTy(lctx)->getPointerTo(),                                         // VALUE **pc
        llvm::Type::getInt64Ty(lctx),                                                            // VALUE captures
        llvm::StructType::getTypeByName(lctx, "struct.rb_control_frame_struct")->getPointerTo(), // cfp
    };
    return llvm::FunctionType::get(llvm::Type::getInt64Ty(lctx), args, false /*not varargs*/);
}

llvm::FunctionType *CompilerState::getInlineForwarderType() {
    llvm::Type *args[] = {
        llvm::Type::getInt64Ty(lctx), // VALUE val
    };
    return llvm::FunctionType::get(llvm::Type::getInt64Ty(lctx), args, false /*not varargs*/);
}

llvm::FunctionType *CompilerState::getAnyRubyCApiFunctionType() {
    auto args = vector<llvm::Type *>{};
    return llvm::FunctionType::get(llvm::Type::getInt64Ty(lctx), args, true /*IS varargs*/);
}

llvm::Function *CompilerState::getFunction(llvm::StringRef name) const {
    llvm::Function *f = module->getFunction(name);
    ENFORCE(f, "could not find {} in the payload", name.str());
    return f;
}

CompilerState CompilerState::withFunctionEntry(llvm::BasicBlock *entry) {
    auto res = CompilerState(*this);
    res.functionEntryInitializers = entry;
    return res;
}

void CompilerState::runCheapOptimizations(llvm::Function *func) {
    llvm::legacy::FunctionPassManager pm(module);
    llvm::PassManagerBuilder pmbuilder;
    int optLevel = 2;
    int sizeLevel = 0;
    pmbuilder.OptLevel = optLevel;
    pmbuilder.SizeLevel = sizeLevel;
    pmbuilder.Inliner = nullptr;
    pmbuilder.DisableUnrollLoops = false;
    pmbuilder.LoopVectorize = true;
    pmbuilder.SLPVectorize = true;
    pmbuilder.VerifyInput = debug_mode;
    pmbuilder.populateFunctionPassManager(pm);
    pm.run(*func);
}

} // namespace sorbet::compiler
