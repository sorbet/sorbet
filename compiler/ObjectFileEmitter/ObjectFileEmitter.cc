#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/IRPrintingPasses.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
// ^^^ violate our poisons
#include "common/FileOps.h"
#include "compiler/ObjectFileEmitter/ObjectFileEmitter.h"

#include <string_view>
using namespace std;
namespace sorbet::compiler {

void ObjectFileEmitter::init() {
    // Initialize the target registry etc.
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();
}

void outputLLVM(llvm::legacy::PassManager &pm, string_view dir, string_view fileNameWithoutExtension,
                const unique_ptr<llvm::Module> &module) {}

void outputObjectFile(llvm::legacy::PassManager &pm, string_view dir, string_view fileNameWithoutExtension,
                      unique_ptr<llvm::Module> module) {
    // We encode this value in our `.exp` files right now so we have to hard
    // code it to something that doesn't chance accross sytems.
    // TODO stop putting it in our .exp files and then unhardcode this
    auto targetTriple = "x86_64-apple-darwin18.2.0";
    // auto targetTriple = llvm::sys::getDefaultTargetTriple();
    module->setTargetTriple(targetTriple);

    std::string error;
    auto target = llvm::TargetRegistry::lookupTarget(targetTriple, error);

    // Print an error and exit if we couldn't find the requested target.
    // This generally occurs if we've forgotten to initialise the
    // TargetRegistry or we have a bogus target triple.
    if (!target) {
        llvm::errs() << error;
        return;
    }

    auto cpu = "skylake"; // this should probably not be hardcoded in future, but for now, this is what llc uses on
                          // mac and thus brings us closer to their assembly
    auto features = "";

    llvm::TargetOptions opt;
    auto relocationModel = llvm::Optional<llvm::Reloc::Model>(llvm::Reloc::PIC_);
    auto targetMachine = target->createTargetMachine(targetTriple, cpu, features, opt, relocationModel);

    module->setDataLayout(targetMachine->createDataLayout());

    std::error_code ec;
    auto fileName = ((string)dir) + "/" + (string)fileNameWithoutExtension + ".o";
    llvm::raw_fd_ostream dest(fileName, ec, llvm::sys::fs::OF_None);

    if (ec) {
        llvm::errs() << "Could not open file: " << ec.message();
        return;
    }

    auto fileType = llvm::TargetMachine::CGFT_ObjectFile;

    if (targetMachine->addPassesToEmitFile(pm, dest, nullptr, fileType, !debug_mode)) {
        llvm::errs() << "TheTargetMachine can't emit a file of this type";
        return;
    }

    pm.run(*module);
    dest.flush();
}

void ObjectFileEmitter::run(const core::GlobalState &gs, llvm::LLVMContext &lctx, unique_ptr<llvm::Module> module,
                            core::SymbolRef sym, string_view dir, string_view objectName,
                            llvm::BasicBlock *globalInitializers) {
    llvm::IRBuilder<> builder(lctx);
    std::vector<llvm::Type *> NoArgs(0, llvm::Type::getVoidTy(lctx));
    auto ft = llvm::FunctionType::get(llvm::Type::getVoidTy(lctx), NoArgs, false);
    bool isSpecialEntrypoint = false;
    if (sym.data(gs)->owner == core::Symbols::rootSingleton() &&
        sym.data(gs)->name.data(gs)->unique.original == core::Names::staticInit()) {
        isSpecialEntrypoint = true;
    }

    auto baseName = sym.data(gs)->toStringFullName(gs);
    if (isSpecialEntrypoint) {
        baseName = FileOps::getFileName(sym.data(gs)->loc().file().data(gs).path());
        baseName = baseName.substr(0, baseName.rfind(".rb"));
    }
    auto entryFunc = llvm::Function::Create(ft, llvm::Function::ExternalLinkage, {"Init_", (string)baseName}, *module);
    globalInitializers->insertInto(entryFunc);

    builder.SetInsertPoint(globalInitializers);
    auto bb = llvm::BasicBlock::Create(lctx, "entry", entryFunc);
    builder.CreateBr(bb);
    builder.SetInsertPoint(bb);

    // Call the LLVM method that was made by LLVMIREmitter from this Init_ method
    if (isSpecialEntrypoint) {
        auto staticInitFunc =
            module->getFunction(gs.lookupStaticInitForFile(sym.data(gs)->loc()).data(gs)->toStringFullName(gs));
        ENFORCE(staticInitFunc);
        builder.CreateCall(staticInitFunc,
                           {llvm::ConstantInt::get(lctx, llvm::APInt(32, 0, true)),
                            llvm::ConstantPointerNull::get(llvm::Type::getInt64PtrTy(lctx)),
                            builder.CreateCall(module->getFunction("sorbet_rb_cObject"))},
                           (string)objectName);
    }

    builder.CreateRetVoid();

    ENFORCE(!llvm::verifyFunction(*entryFunc, &llvm::errs()), "see above");

    /* run optimizations */

    unique_ptr<llvm::legacy::PassManager> pm = make_unique<llvm::legacy::PassManager>();

    // print unoptimized IR
    std::error_code ec;
    auto name = ((string)dir) + "/" + (string)baseName + ".ll";
    llvm::raw_fd_ostream llFile(name, ec, llvm::sys::fs::F_Text);
    pm->add(llvm::createPrintModulePass(llFile, ""));

    // enable optimizations
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
    pmbuilder.populateModulePassManager(*pm);
    pmbuilder.populateLTOPassManager(*pm);
    // print optimized IR
    std::error_code ec1;
    auto nameOpt = ((string)dir) + "/" + (string)baseName + ".llo";
    llvm::raw_fd_ostream lloFile(nameOpt, ec1, llvm::sys::fs::F_Text);
    pm->add(llvm::createPrintModulePass(lloFile, ""));

    outputObjectFile(*pm, dir, baseName, move(module));
}

} // namespace sorbet::compiler
