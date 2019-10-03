#include "compiler/object_file_emitter/object_file_emitter.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include <string_view>
using namespace std;
namespace sorbet::compiler {

// static unique_ptr<::llvm::Module> TheModule = Payload::readDefaultModule("sorbet", TheContext);

void ObjectFileEmitter::init() {
    // Initialize the target registry etc.
    ::llvm::InitializeAllTargetInfos();
    ::llvm::InitializeAllTargets();
    ::llvm::InitializeAllTargetMCs();
    ::llvm::InitializeAllAsmParsers();
    ::llvm::InitializeAllAsmPrinters();
}

void outputLLVM(string_view dir, string_view fileNameWithoutExtension, const unique_ptr<::llvm::Module> &module) {
    std::error_code ec;
    auto name = ((string)dir) + "/" + (string)fileNameWithoutExtension + ".ll";
    ::llvm::raw_fd_ostream File(name, ec, ::llvm::sys::fs::F_Text);
    module->print(File, nullptr);
}

void outputObjectFile(string_view dir, string_view fileNameWithoutExtension, unique_ptr<::llvm::Module> module) {
    auto targetTriple = ::llvm::sys::getDefaultTargetTriple();
    module->setTargetTriple(targetTriple);

    std::string error;
    auto target = ::llvm::TargetRegistry::lookupTarget(targetTriple, error);

    // Print an error and exit if we couldn't find the requested target.
    // This generally occurs if we've forgotten to initialise the
    // TargetRegistry or we have a bogus target triple.
    if (!target) {
        ::llvm::errs() << error;
        return;
    }

    auto cpu = "generic";
    auto features = "";

    ::llvm::TargetOptions opt;
    auto relocationModel = ::llvm::Optional<::llvm::Reloc::Model>();
    auto targetMachine = target->createTargetMachine(targetTriple, cpu, features, opt, relocationModel);

    module->setDataLayout(targetMachine->createDataLayout());

    std::error_code ec;
    auto fileName = ((string)dir) + "/" + (string)fileNameWithoutExtension + ".o";
    ::llvm::raw_fd_ostream dest(fileName, ec, ::llvm::sys::fs::OF_None);

    if (ec) {
        ::llvm::errs() << "Could not open file: " << ec.message();
        return;
    }

    ::llvm::legacy::PassManager pass;
    auto fileType = ::llvm::TargetMachine::CGFT_ObjectFile;

    if (targetMachine->addPassesToEmitFile(pass, dest, nullptr, fileType)) {
        ::llvm::errs() << "TheTargetMachine can't emit a file of this type";
        return;
    }

    pass.run(*module);
    dest.flush();
}

void ObjectFileEmitter::run(spdlog::logger &logger, ::llvm::LLVMContext &lctx, unique_ptr<::llvm::Module> module, string_view dir,
                 string_view objectName) {
    ::llvm::IRBuilder<> builder(lctx);
    std::vector<::llvm::Type *> NoArgs(0, ::llvm::Type::getVoidTy(lctx));
    auto ft = ::llvm::FunctionType::get(::llvm::Type::getVoidTy(lctx), NoArgs, false);
    auto function = ::llvm::Function::Create(ft, ::llvm::Function::ExternalLinkage,
                                             ((string) "Init_" + (string)objectName), *module);
    auto bb = ::llvm::BasicBlock::Create(lctx, "entry", function);
    builder.SetInsertPoint(bb);
    builder.CreateRet(::llvm::ConstantInt::getTrue(lctx));

    outputLLVM(dir, objectName, module);
    outputObjectFile(dir, objectName, move(module));
}

} // namespace sorbet::compiler
