#include "llvm/linker/linker.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"

namespace sorbet::llvm::linker {

static ::llvm::LLVMContext TheContext;
static ::llvm::IRBuilder<> Builder(TheContext);
static ::llvm::Module TheModule("sorbet", TheContext);

static std::string dir;
void setIROutputDir(std::string d) {
    dir = d;
}

void init() {}

void outputLLVM() {
    std::error_code EC;
    auto name = dir + "/main.ll";
    ::llvm::raw_fd_ostream File(name, EC, ::llvm::sys::fs::F_Text);
    TheModule.print(File, nullptr);
}

void outputObjectFile() {
    // Initialize the target registry etc.
    ::llvm::InitializeAllTargetInfos();
    ::llvm::InitializeAllTargets();
    ::llvm::InitializeAllTargetMCs();
    ::llvm::InitializeAllAsmParsers();
    ::llvm::InitializeAllAsmPrinters();

    auto TargetTriple = ::llvm::sys::getDefaultTargetTriple();
    TheModule.setTargetTriple(TargetTriple);

    std::string Error;
    auto Target = ::llvm::TargetRegistry::lookupTarget(TargetTriple, Error);

    // Print an error and exit if we couldn't find the requested target.
    // This generally occurs if we've forgotten to initialise the
    // TargetRegistry or we have a bogus target triple.
    if (!Target) {
        ::llvm::errs() << Error;
        return;
    }

    auto CPU = "generic";
    auto Features = "";

    ::llvm::TargetOptions opt;
    auto RM = ::llvm::Optional<::llvm::Reloc::Model>();
    auto TheTargetMachine =
        Target->createTargetMachine(TargetTriple, CPU, Features, opt, RM);

    TheModule.setDataLayout(TheTargetMachine->createDataLayout());

    std::error_code EC;
    ::llvm::raw_fd_ostream dest(dir + "/main.o", EC, ::llvm::sys::fs::OF_None);

    if (EC) {
        ::llvm::errs() << "Could not open file: " << EC.message();
        return;
    }

    ::llvm::legacy::PassManager pass;
    auto FileType = ::llvm::TargetMachine::CGFT_ObjectFile;

    if (TheTargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType)) {
        ::llvm::errs() << "TheTargetMachine can't emit a file of this type";
        return;
    }

    pass.run(TheModule);
    dest.flush();
}

void run(std::shared_ptr<spdlog::logger> logger) {
    std::vector<::llvm::Type *> NoArgs(0, ::llvm::Type::getVoidTy(TheContext));
    auto FT = ::llvm::FunctionType::get(::llvm::Type::getVoidTy(TheContext), NoArgs, false);
    auto TheFunction = ::llvm::Function::Create(FT, ::llvm::Function::ExternalLinkage, "main", TheModule);
    auto BB = ::llvm::BasicBlock::Create(TheContext, "entry", TheFunction);
    Builder.SetInsertPoint(BB);
    Builder.CreateRet(::llvm::ConstantInt::getTrue(TheContext));

    outputLLVM();
    outputObjectFile();
}

} // namespace sorbet::llvm::linker
