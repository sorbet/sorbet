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
#include "llvm/Transforms/Utils/ModuleUtils.h" // appendToGlobalCtors
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

CompilerState::CompilerState(const core::GlobalState &gs, llvm::LLVMContext &lctx, llvm::Module *module)
    : gs(gs), lctx(lctx), functionEntryInitializers(nullptr), module(module) {}
namespace {
llvm::IRBuilder<> &builderCast(llvm::IRBuilderBase &builder) {
    return static_cast<llvm::IRBuilder<> &>(builder);
};
} // namespace

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
    builderCast(builder).CreateCall(
        llvm::Intrinsic::getDeclaration(module, llvm::Intrinsic::ID::expect, {llvm::Type::getInt1Ty(lctx)}),
        {value, llvm::ConstantInt::get(llvm::Type::getInt1Ty(lctx), llvm::APInt(1, expected ? 1 : 0, true))});
}

void CompilerState::boxRawValue(llvm::IRBuilderBase &builder, llvm::AllocaInst *target, llvm::Value *rawData) {
    llvm::Value *indices[] = {llvm::ConstantInt::get(lctx, llvm::APInt(32, 0, true)),
                              llvm::ConstantInt::get(lctx, llvm::APInt(32, 0, true))};
    builderCast(builder).CreateStore(rawData, builderCast(builder).CreateGEP(target, indices));
}

llvm::Value *CompilerState::unboxRawValue(llvm::IRBuilderBase &builder, llvm::AllocaInst *target) {
    llvm::Value *indices[] = {llvm::ConstantInt::get(lctx, llvm::APInt(32, 0, true)),
                              llvm::ConstantInt::get(lctx, llvm::APInt(32, 0, true))};
    return builderCast(builder).CreateLoad(builderCast(builder).CreateGEP(target, indices), "rawRubyValue");
}

llvm::Value *CompilerState::getRubyNilRaw(llvm::IRBuilderBase &builder) {
    return builderCast(builder).CreateCall(module->getFunction("sorbet_rubyNil"), {}, "nilValueRaw");
}

llvm::Value *CompilerState::getRubyFalseRaw(llvm::IRBuilderBase &builder) {
    return builderCast(builder).CreateCall(module->getFunction("sorbet_rubyFalse"), {}, "falseValueRaw");
}

llvm::Value *CompilerState::getRubyTrueRaw(llvm::IRBuilderBase &builder) {
    return builderCast(builder).CreateCall(module->getFunction("sorbet_rubyTrue"), {}, "trueValueRaw");
}

void CompilerState::emitArgumentMismatch(llvm::IRBuilderBase &builder, llvm::Value *currentArgCount, int minArgs,
                                         int maxArgs) {
    builderCast(builder).CreateCall(
        module->getFunction("sorbet_rb_error_arity"),
        {currentArgCount, llvm::ConstantInt::get(llvm::Type::getInt32Ty(lctx), llvm::APInt(32, minArgs, true)),
         llvm::ConstantInt::get(llvm::Type::getInt32Ty(lctx), llvm::APInt(32, maxArgs, true))

        });
    builderCast(builder).CreateUnreachable();
}
llvm::Value *CompilerState::getRubyIntRaw(llvm::IRBuilderBase &builder, long num) {
    return builderCast(builder).CreateCall(module->getFunction("sorbet_longToRubyValue"),
                                           {llvm::ConstantInt::get(lctx, llvm::APInt(64, num, true))}, "rawRubyInt");
}

llvm::Value *CompilerState::getRubyStringRaw(llvm::IRBuilderBase &builder, std::string_view str) {
    llvm::StringRef userStr(str.data(), str.length());
    auto rawCString = builderCast(builder).CreateGlobalStringPtr(userStr, {"userStr_", userStr});
    return builderCast(builder).CreateCall(
        module->getFunction("sorbet_CPtrToRubyString"),
        {rawCString, llvm::ConstantInt::get(lctx, llvm::APInt(64, str.length(), true))}, "rawRubyStr");
}

llvm::Value *CompilerState::getIsTruthyU1(llvm::IRBuilderBase &builder, llvm::Value *val) {
    return builderCast(builder).CreateCall(module->getFunction("sorbet_testIsTruthy"), {val}, "cond");
}

llvm::Value *CompilerState::getRubyIdFor(llvm::IRBuilderBase &builder, std::string_view idName) {
    auto zero = llvm::ConstantInt::get(llvm::Type::getInt64Ty(lctx), 0);
    auto name = llvm::StringRef(idName.data(), idName.length());
    llvm::Constant *indices[] = {zero};
    string rawName = "rubyIdPrecomputed_" + (string)idName;
    auto tp = llvm::Type::getInt64Ty(lctx);
    llvm::IRBuilder<> globalInitBuilder(lctx);
    auto globalDeclaration = static_cast<llvm::GlobalVariable *>(module->getOrInsertGlobal(rawName, tp, [&] {
        auto ret = new llvm::GlobalVariable(*module, tp, false, llvm::GlobalVariable::InternalLinkage, zero, rawName);
        ret->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);
        ret->setAlignment(8);
        // create constructor
        std::vector<llvm::Type *> NoArgs(0, llvm::Type::getVoidTy(lctx));
        auto ft = llvm::FunctionType::get(llvm::Type::getVoidTy(lctx), NoArgs, false);
        auto constr = llvm::Function::Create(ft, llvm::Function::InternalLinkage, {"Constr_", rawName}, *module);

        auto bb = llvm::BasicBlock::Create(lctx, "constr", constr);
        globalInitBuilder.SetInsertPoint(bb);
        llvm::Constant *indicesString[] = {zero, zero};
        auto gv = builder.CreateGlobalString(name, {"str_", name}, 0);
        auto rawCString = llvm::ConstantExpr::getInBoundsGetElementPtr(gv->getValueType(), gv, indicesString);
        auto rawID = globalInitBuilder.CreateCall(module->getFunction("sorbet_IDIntern"), {rawCString}, "rawId");
        globalInitBuilder.CreateStore(rawID,
                                      llvm::ConstantExpr::getInBoundsGetElementPtr(ret->getValueType(), ret, indices));
        globalInitBuilder.CreateRetVoid();
        llvm::appendToGlobalCtors(*module, constr, 0, ret);

        return ret;
    }));

    globalInitBuilder.SetInsertPoint(functionEntryInitializers);
    auto global = globalInitBuilder.CreateLoad(
        llvm::ConstantExpr::getInBoundsGetElementPtr(globalDeclaration->getValueType(), globalDeclaration, indices),
        {"rubyId_", name});

    // todo(perf): mark these as immutable with https://llvm.org/docs/LangRef.html#llvm-invariant-start-intrinsic
    return global;
}

void CompilerState::runCheapOptimizations(llvm::Function *func) {
    llvm::legacy::FunctionPassManager pm(module);
    llvm::PassManagerBuilder pmbuilder;
    int optLevel = 2;
    int sizeLevel = 0;
    pmbuilder.OptLevel = optLevel;
    pmbuilder.SizeLevel = sizeLevel;
    pmbuilder.Inliner = llvm::createFunctionInliningPass(optLevel, sizeLevel, false);
    pmbuilder.DisableUnitAtATime = false;
    pmbuilder.DisableUnrollLoops = false;
    pmbuilder.LoopVectorize = true;
    pmbuilder.SLPVectorize = true;
    pmbuilder.VerifyInput = debug_mode;
    pmbuilder.populateFunctionPassManager(pm);
    pm.run(*func);
}

} // namespace sorbet::compiler
