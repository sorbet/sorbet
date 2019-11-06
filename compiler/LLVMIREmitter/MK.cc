#include "compiler/IRHelpers/IRHelpers.h"
#include "llvm/IR/DerivedTypes.h" // FunctionType
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/ModuleUtils.h" // appendToGlobalCtors
// needef for LLVMIREmitterHelpers
#include "core/core.h"
// ^^^ violate our poisons
#include "LLVMIREmitterHelpers.h"
#include <string>

using namespace std;

namespace sorbet::compiler {
namespace {
llvm::IRBuilder<> &builderCast(llvm::IRBuilderBase &builder) {
    return static_cast<llvm::IRBuilder<> &>(builder);
};
} // namespace

llvm::Value *MK::setExpectedBool(CompilerState &cs, llvm::IRBuilderBase &builder, llvm::Value *value, bool expected) {
    return builderCast(builder).CreateIntrinsic(llvm::Intrinsic::ID::expect, {llvm::Type::getInt1Ty(cs)},
                                                {value, builder.getInt1(expected)});
}

void MK::boxRawValue(CompilerState &cs, llvm::IRBuilderBase &builder, llvm::AllocaInst *target, llvm::Value *rawData) {
    builderCast(builder).CreateStore(rawData, builderCast(builder).CreateStructGEP(target, 0));
}

llvm::Value *MK::unboxRawValue(CompilerState &cs, llvm::IRBuilderBase &builder, llvm::AllocaInst *target) {
    return builderCast(builder).CreateLoad(builderCast(builder).CreateStructGEP(target, 0), "rawRubyValue");
}

llvm::Value *MK::getRubyNilRaw(CompilerState &cs, llvm::IRBuilderBase &builder) {
    return builderCast(builder).CreateCall(cs.module->getFunction("sorbet_rubyNil"), {}, "nilValueRaw");
}

llvm::Value *MK::getRubyFalseRaw(CompilerState &cs, llvm::IRBuilderBase &builder) {
    return builderCast(builder).CreateCall(cs.module->getFunction("sorbet_rubyFalse"), {}, "falseValueRaw");
}

llvm::Value *MK::getRubyTrueRaw(CompilerState &cs, llvm::IRBuilderBase &builder) {
    return builderCast(builder).CreateCall(cs.module->getFunction("sorbet_rubyTrue"), {}, "trueValueRaw");
}

void MK::emitArgumentMismatch(CompilerState &cs, llvm::IRBuilderBase &builder, llvm::Value *currentArgCount,
                              int minArgs, int maxArgs) {
    builderCast(builder).CreateCall(cs.module->getFunction("sorbet_rb_error_arity"),
                                    {currentArgCount, llvm::ConstantInt::get(cs, llvm::APInt(32, minArgs, true)),
                                     llvm::ConstantInt::get(cs, llvm::APInt(32, maxArgs, true))

                                    });
    builderCast(builder).CreateUnreachable();
}
llvm::Value *MK::getRubyIntRaw(CompilerState &cs, llvm::IRBuilderBase &builder, long num) {
    return builderCast(builder).CreateCall(cs.module->getFunction("sorbet_longToRubyValue"),
                                           {llvm::ConstantInt::get(cs, llvm::APInt(64, num, true))}, "rawRubyInt");
}

llvm::Value *MK::getRubyStringRaw(CompilerState &cs, llvm::IRBuilderBase &builder, std::string_view str) {
    llvm::StringRef userStr(str.data(), str.length());
    auto rawCString = builderCast(builder).CreateGlobalStringPtr(userStr, {"userStr_", userStr});
    return builderCast(builder).CreateCall(
        cs.module->getFunction("sorbet_CPtrToRubyString"),
        {rawCString, llvm::ConstantInt::get(cs, llvm::APInt(64, str.length(), true))}, "rawRubyStr");
}

llvm::Value *MK::getIsTruthyU1(CompilerState &cs, llvm::IRBuilderBase &builder, llvm::Value *val) {
    return builderCast(builder).CreateCall(cs.module->getFunction("sorbet_testIsTruthy"), {val}, "cond");
}

llvm::Value *MK::getRubyIdFor(CompilerState &cs, llvm::IRBuilderBase &builder, std::string_view idName) {
    auto zero = llvm::ConstantInt::get(cs, llvm::APInt(64, 0));
    auto name = llvm::StringRef(idName.data(), idName.length());
    llvm::Constant *indices[] = {zero};
    string rawName = "rubyIdPrecomputed_" + (string)idName;
    auto tp = llvm::Type::getInt64Ty(cs);
    llvm::IRBuilder<> globalInitBuilder(cs);
    auto globalDeclaration = static_cast<llvm::GlobalVariable *>(cs.module->getOrInsertGlobal(rawName, tp, [&] {
        auto ret =
            new llvm::GlobalVariable(*cs.module, tp, false, llvm::GlobalVariable::InternalLinkage, zero, rawName);
        ret->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);
        ret->setAlignment(8);
        // create constructor
        std::vector<llvm::Type *> NoArgs(0, llvm::Type::getVoidTy(cs));
        auto ft = llvm::FunctionType::get(llvm::Type::getVoidTy(cs), NoArgs, false);
        auto constr = llvm::Function::Create(ft, llvm::Function::InternalLinkage, {"Constr_", rawName}, *cs.module);

        auto bb = llvm::BasicBlock::Create(cs, "constr", constr);
        globalInitBuilder.SetInsertPoint(bb);
        llvm::Constant *indicesString[] = {zero, zero};
        auto gv = builder.CreateGlobalString(name, {"str_", name}, 0);
        auto rawCString = llvm::ConstantExpr::getInBoundsGetElementPtr(gv->getValueType(), gv, indicesString);
        auto rawID = globalInitBuilder.CreateCall(
            cs.module->getFunction("sorbet_IDIntern"),
            {rawCString, llvm::ConstantInt::get(cs, llvm::APInt(64, idName.length()))}, "rawId");
        globalInitBuilder.CreateStore(rawID,
                                      llvm::ConstantExpr::getInBoundsGetElementPtr(ret->getValueType(), ret, indices));
        globalInitBuilder.CreateRetVoid();
        llvm::appendToGlobalCtors(*cs.module, constr, 0, ret);

        return ret;
    }));

    globalInitBuilder.SetInsertPoint(cs.functionEntryInitializers);
    auto global = globalInitBuilder.CreateLoad(
        llvm::ConstantExpr::getInBoundsGetElementPtr(globalDeclaration->getValueType(), globalDeclaration, indices),
        {"rubyId_", name});

    // todo(perf): mark these as immutable with https://llvm.org/docs/LangRef.html#llvm-invariant-start-intrinsic
    return global;
}

}; // namespace sorbet::compiler
