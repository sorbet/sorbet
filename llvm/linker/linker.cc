#include "llvm/linker/linker.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/FileSystem.h"

namespace sorbet::llvm::linker {

static ::llvm::LLVMContext TheContext;
static ::llvm::IRBuilder<> Builder(TheContext);
static ::llvm::Module TheModule("sorbet", TheContext);

static std::string dir;
void setIROutputDir(std::string d) {
    dir = d;
}

void init() {}

void run(std::shared_ptr<spdlog::logger> logger) {
    std::vector<::llvm::Type *> NoArgs(0, ::llvm::Type::getVoidTy(TheContext));
    auto FT = ::llvm::FunctionType::get(::llvm::Type::getVoidTy(TheContext), NoArgs, false);
    auto TheFunction = ::llvm::Function::Create(FT, ::llvm::Function::ExternalLinkage, "main", TheModule);
    auto BB = ::llvm::BasicBlock::Create(TheContext, "entry", TheFunction);
    Builder.SetInsertPoint(BB);
    Builder.CreateRet(::llvm::ConstantInt::getTrue(TheContext));

    std::error_code EC;
    auto name = dir + "/main.ll";
    ::llvm::errs() << "Writing '" << name << "' ...\n";
    ::llvm::raw_fd_ostream File(name, EC, ::llvm::sys::fs::F_Text);
    TheModule.print(File, nullptr);
}

} // namespace sorbet::llvm::linker
