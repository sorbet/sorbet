#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
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
#include "common/Timer.h"
#include "compiler/Linker/Linker.h"
#include "compiler/ObjectFileEmitter/ObjectFileEmitter.h"
#include "compiler/Passes/Passes.h"

#include <string_view>
using namespace std;
namespace sorbet::compiler {

void ObjectFileEmitter::init() {
    // Initialize the target registry etc.
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();
}

bool outputObjectFile(spdlog::logger &logger, llvm::legacy::PassManager &pm, const string &fileName,
                      unique_ptr<llvm::Module> module, llvm::TargetMachine *targetMachine) {
    Timer timer(logger, "objectFileEmission");
    std::error_code ec;
    llvm::raw_fd_ostream dest(fileName, ec, llvm::sys::fs::OF_None);

    if (ec) {
        llvm::errs() << "Could not open file: " << ec.message();
        return false;
    }

    auto fileType = llvm::CGFT_ObjectFile;

    if (targetMachine->addPassesToEmitFile(pm, dest, nullptr, fileType, !debug_mode)) {
        llvm::errs() << "TheTargetMachine can't emit a file of this type";
        return false;
    }

    pm.run(*module);
    dest.flush();
    return true;
}

bool ObjectFileEmitter::run(spdlog::logger &logger, llvm::LLVMContext &lctx, unique_ptr<llvm::Module> module,
                            string_view dir, string_view objectName) {
    /* setup target */
    std::string error;
    auto targetTriple = llvm::sys::getDefaultTargetTriple();
    auto target = llvm::TargetRegistry::lookupTarget(targetTriple, error);

    // Print an error and exit if we couldn't find the requested target.
    // This generally occurs if we've forgotten to initialise the
    // TargetRegistry or we have a bogus target triple.
    if (!target) {
        llvm::errs() << error;
        return false;
    }

    auto cpu = "skylake"; // this should probably not be hardcoded in future, but for now, this is what llc uses on
                          // mac and thus brings us closer to their assembly
    auto features = "";

    llvm::TargetOptions opt;
    auto relocationModel = llvm::Optional<llvm::Reloc::Model>(llvm::Reloc::PIC_);
    auto targetMachine = target->createTargetMachine(targetTriple, cpu, features, opt, relocationModel);
    ENFORCE(targetMachine);

    // We encode this value in our `.exp` files right now so we have to hard
    // code it to something that doesn't chance accross sytems.
    // TODO stop putting it in our .exp files and then unhardcode this
    // auto targetTriple = llvm::sys::getDefaultTargetTriple();

    module->setTargetTriple(targetMachine->getTargetTriple().str());
    module->setDataLayout(targetMachine->createDataLayout());

    /* run optimizations */

    llvm::legacy::PassManager pm;
    // print unoptimized IR
    {
        llvm::legacy::PassManager ppm;
        std::error_code ec;
        auto name = ((string)dir) + "/" + (string)objectName + ".ll";
        llvm::raw_fd_ostream llFile(name, ec, llvm::sys::fs::F_Text);
        ppm.add(llvm::createPrintModulePass(llFile, ""));
        ppm.run(*module);
    }

    // add platform specific information
    pm.add(new llvm::TargetLibraryInfoWrapperPass(targetMachine->getTargetTriple()));
    pm.add(llvm::createTargetTransformInfoWrapperPass(targetMachine->getTargetIRAnalysis()));

    llvm::legacy::FunctionPassManager fnPasses(module.get());
    fnPasses.add(llvm::createTargetTransformInfoWrapperPass(targetMachine->getTargetIRAnalysis()));

    // enable optimizations
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
    targetMachine->adjustPassManager(pmbuilder);
    pmbuilder.populateFunctionPassManager(fnPasses);
    for (auto pass : Passes::standardLowerings()) {
        pm.add(pass);
    }
    std::error_code ec1;
    // print lowered IR
    auto nameOptl = ((string)dir) + "/" + (string)objectName + ".lll";
    llvm::raw_fd_ostream lllFile(nameOptl, ec1, llvm::sys::fs::F_Text);
    pm.add(llvm::createPrintModulePass(lllFile, ""));

    pmbuilder.populateModulePassManager(pm);
    pmbuilder.populateLTOPassManager(pm);
    // print optimized IR
    auto nameOpt = ((string)dir) + "/" + (string)objectName + ".llo";
    llvm::raw_fd_ostream lloFile(nameOpt, ec1, llvm::sys::fs::F_Text);
    pm.add(llvm::createPrintModulePass(lloFile, ""));
    {
        Timer timer(logger, "functionPasses");

        fnPasses.doInitialization();
        for (llvm::Function &func : *module) {
            fnPasses.run(func);
        }
        fnPasses.doFinalization();
    }
    if (debug_mode) {
        pm.add(llvm::createVerifierPass());
    }
    auto objectFileName = (string)fmt::format("{}/{}.o", dir, objectName);
    auto soNamePrefix = (string)fmt::format("{}/{}", dir, objectName);
    if (!outputObjectFile(logger, pm, objectFileName, move(module), targetMachine)) {
        return false;
    }
    if (!Linker::run(logger, {objectFileName}, soNamePrefix)) {
        return false;
    }
    FileOps::removeFile(objectFileName);
    return true;
}

} // namespace sorbet::compiler
