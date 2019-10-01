#include "llvm/linker/linker.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

namespace sorbet::llvm::linker {

static ::llvm::LLVMContext TheContext;
static ::llvm::IRBuilder<> Builder(TheContext);
static ::llvm::Module TheModule("sorbet", TheContext);

void init() {}

void run(std::shared_ptr<spdlog::logger> logger) {
    std::vector<::llvm::Type *> NoArgs(0, ::llvm::Type::getVoidTy(TheContext));
    auto FT = ::llvm::FunctionType::get(::llvm::Type::getVoidTy(TheContext), NoArgs, false);
    auto TheFunction = ::llvm::Function::Create(FT, ::llvm::Function::ExternalLinkage, "main", TheModule);
    auto BB = ::llvm::BasicBlock::Create(TheContext, "entry", TheFunction);
    Builder.SetInsertPoint(BB);
    Builder.CreateRet(::llvm::ConstantInt::getTrue(TheContext));
    TheModule.print(::llvm::errs(), nullptr);
}

} // namespace sorbet::llvm::linker
