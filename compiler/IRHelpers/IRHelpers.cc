#include "IRHelpers.h"
#include "llvm/Bitcode/BitcodeReader.h"
#include "llvm/IR/DerivedTypes.h" // FunctionType, StructType
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/MemoryBuffer.h"
#include <string_view>
// ^^^ violate poisons
#include "core/core.h"
using namespace std;
namespace sorbet::compiler {
string_view getDefaultModuleBitcode();
std::unique_ptr<llvm::Module> IRHelpers::readDefaultModule(const char *name, llvm::LLVMContext &lctx) {
    auto bitCode = getDefaultModuleBitcode();
    llvm::StringRef bitcodeAsStringRef(bitCode.data(), bitCode.size());
    auto memoryBuffer = llvm::MemoryBuffer::getMemBuffer(bitcodeAsStringRef);
    auto ret = llvm::parseBitcodeFile(*memoryBuffer, lctx);
    return move(ret.get());
}

CompilerState::CompilerState(const core::GlobalState &gs, llvm::LLVMContext &lctx, llvm::Module *module,
                             llvm::BasicBlock *globalInits)
    : gs(gs), lctx(lctx), globalInitializers(globalInits), functionEntryInitializers(nullptr), module(module) {}

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
        llvm::Type::getInt64Ty(lctx)     // self
    };
    return llvm::FunctionType::get(llvm::Type::getInt64Ty(lctx), args, false /*not varargs*/);
}

void CompilerState::setExpectedBool(llvm::IRBuilderBase &builder, llvm::Value *value, bool expected) {
    static_cast<llvm::IRBuilder<> &>(builder).CreateCall(
        llvm::Intrinsic::getDeclaration(module, llvm::Intrinsic::ID::expect, {llvm::Type::getInt1Ty(lctx)}),
        {value, llvm::ConstantInt::get(llvm::Type::getInt1Ty(lctx), llvm::APInt(1, expected ? 1 : 0, true))});
}

void CompilerState::boxRawValue(llvm::IRBuilderBase &builder, llvm::AllocaInst *target, llvm::Value *rawData) {
    llvm::Value *indices[] = {llvm::ConstantInt::get(lctx, llvm::APInt(32, 0, true)),
                              llvm::ConstantInt::get(lctx, llvm::APInt(32, 0, true))};
    static_cast<llvm::IRBuilder<> &>(builder).CreateStore(
        rawData, static_cast<llvm::IRBuilder<> &>(builder).CreateGEP(target, indices));
}

llvm::Value *CompilerState::unboxRawValue(llvm::IRBuilderBase &builder, llvm::AllocaInst *target) {
    llvm::Value *indices[] = {llvm::ConstantInt::get(lctx, llvm::APInt(32, 0, true)),
                              llvm::ConstantInt::get(lctx, llvm::APInt(32, 0, true))};
    return static_cast<llvm::IRBuilder<> &>(builder).CreateLoad(
        static_cast<llvm::IRBuilder<> &>(builder).CreateGEP(target, indices), "rawRubyValue");
}

} // namespace sorbet::compiler
