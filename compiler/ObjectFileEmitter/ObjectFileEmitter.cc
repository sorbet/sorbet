// These violate our poisons so have to happen first
#include "llvm/Analysis/GlobalsModRef.h"
#include "llvm/Analysis/ScopedNoAliasAA.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/Analysis/TypeBasedAliasAnalysis.h"
#include "llvm/CodeGen/CommandFlags.h"
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
#include "llvm/Transforms/AggressiveInstCombine/AggressiveInstCombine.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/IPO/AlwaysInliner.h"
#include "llvm/Transforms/IPO/Attributor.h"
#include "llvm/Transforms/IPO/ForceFunctionAttrs.h"
#include "llvm/Transforms/IPO/FunctionAttrs.h"
#include "llvm/Transforms/IPO/InferFunctionAttrs.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Instrumentation.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Scalar/InstSimplifyPass.h"
#include "llvm/Transforms/Utils.h"
#include "llvm/Transforms/Utils/Mem2Reg.h"
#include "llvm/Transforms/Utils/PromoteMemToReg.h"
#include "llvm/Transforms/Vectorize.h"

#include "common/FileOps.h"
#include "common/Timer.h"
#include "compiler/Linker/Linker.h"
#include "compiler/ObjectFileEmitter/ObjectFileEmitter.h"
#include "compiler/Passes/Passes.h"
#include <string_view>

using namespace std;
namespace sorbet::compiler {
namespace {

constexpr int sizeLevel = 0;
constexpr int optLevel = 2;

bool outputObjectFile(spdlog::logger &logger, llvm::legacy::PassManager &pm, const string &rubyFileName,
                      const string &fileName, unique_ptr<llvm::Module> module, llvm::TargetMachine *targetMachine) {
    Timer timer("objectFileEmission", {{"file", rubyFileName}});
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

constexpr bool runExpensiveInstructionConbining = false; // true in O3
constexpr bool unnecessaryForUs = false; // phases that don't do anything due to us not being a c++ compiler

void addModulePasses(llvm::legacy::PassManager &pm) {
    // this is intended to mimic llvm::PassManagerBuilder::populateModulePassManager
    // while disabling optimizations that don't help us much to speedup and adding ones that do. Please explicitly
    // leave comments with added/removed passes
    // pmbuilder.populateModulePassManager(pm);

    pm.add(llvm::createSROAPass()); // this is super useful for us so we want to run it early
    if (unnecessaryForUs) {
        pm.add(llvm::createForceFunctionAttrsLegacyPass());
    }
    pm.add(llvm::createTypeBasedAAWrapperPass());
    pm.add(llvm::createScopedNoAliasAAWrapperPass());
    pm.add(llvm::createInferFunctionAttrsLegacyPass());
    // pm.add(llvm::createCallSiteSplittingPass()); // this is only enabled under O3 for C++
    pm.add(llvm::createIPSCCPPass());
    pm.add(llvm::createCalledValuePropagationPass());
    pm.add(llvm::createAttributorLegacyPass());
    pm.add(llvm::createGlobalOptimizerPass());
    pm.add(llvm::createPromoteMemoryToRegisterPass());
    pm.add(llvm::createDeadArgEliminationPass());
    pm.add(llvm::createInstructionCombiningPass(runExpensiveInstructionConbining));
    pm.add(llvm::createCFGSimplificationPass());
    // We add a module alias analysis pass here. In part due to bugs in the
    // analysis infrastructure this "works" in that the analysis stays alive
    // for the entire SCC pass run below.
    pm.add(llvm::createGlobalsAAWrapperPass());
    pm.add(llvm::createPruneEHPass());

    pm.add(llvm::createFunctionInliningPass(optLevel, sizeLevel, false));
    pm.add(llvm::createPostOrderFunctionAttrsLegacyPass());
    // pm.add(llvm::createArgumentPromotionPass()); // O3
    pm.add(llvm::createEarlyCSEPass(true /* Enable mem-ssa. */));
    if (unnecessaryForUs) {
        pm.add(llvm::createSpeculativeExecutionIfHasBranchDivergencePass());
    }
    pm.add(llvm::createJumpThreadingPass());
    pm.add(llvm::createCorrelatedValuePropagationPass());
    pm.add(llvm::createCFGSimplificationPass());
    // pm.add(llvm::createAggressiveInstCombinerPass()); // O3
    pm.add(llvm::createInstructionCombiningPass(runExpensiveInstructionConbining));
    if (unnecessaryForUs) {
        pm.add(llvm::createLibCallsShrinkWrapPass());
    }
    pm.add(llvm::createPGOMemOPSizeOptLegacyPass());
    pm.add(llvm::createTailCallEliminationPass());
    pm.add(llvm::createCFGSimplificationPass());
    pm.add(llvm::createReassociatePass());
    pm.add(llvm::createLoopRotatePass(-1));
    pm.add(llvm::createLICMPass(100, 250));
    pm.add(llvm::createLoopUnswitchPass(false, false)); // first bool is true for O2 and lower
    pm.add(llvm::createCFGSimplificationPass());
    pm.add(llvm::createInstructionCombiningPass(runExpensiveInstructionConbining));
    pm.add(llvm::createIndVarSimplifyPass());
    pm.add(llvm::createLoopIdiomPass());
    pm.add(llvm::createLoopDeletionPass());
    pm.add(llvm::createSimpleLoopUnrollPass(2, false, false));
    pm.add(llvm::createMergedLoadStoreMotionPass());
    // pm.add(llvm::createGVNPass(false));
    pm.add(llvm::createNewGVNPass()); // by default clang uses _old_ GVN
    pm.add(llvm::createMemCpyOptPass());
    pm.add(llvm::createSCCPPass());
    pm.add(llvm::createBitTrackingDCEPass());
    pm.add(llvm::createInstructionCombiningPass(runExpensiveInstructionConbining));
    pm.add(llvm::createJumpThreadingPass()); // Thread jumps
    pm.add(llvm::createCorrelatedValuePropagationPass());
    pm.add(llvm::createDeadStoreEliminationPass()); // Delete dead stores
    pm.add(llvm::createLICMPass(100, 250));
    pm.add(llvm::createAggressiveDCEPass());     // Delete dead instructions
    pm.add(llvm::createCFGSimplificationPass()); // Merge & remove BBs
    pm.add(llvm::createInstructionCombiningPass(runExpensiveInstructionConbining));
    pm.add(llvm::createBarrierNoopPass());
    // pm.add(llvm::createPartialInliningPass());
    pm.add(llvm::createEliminateAvailableExternallyPass());
    pm.add(llvm::createReversePostOrderFunctionAttrsPass());
    pm.add(llvm::createGlobalOptimizerPass());
    pm.add(llvm::createGlobalDCEPass());
    pm.add(llvm::createGlobalsAAWrapperPass());
    pm.add(llvm::createFloat2IntPass());
    // pm.add(llvm::createLowerConstantIntrinsicsPass()); // llvm 10-prerelease
    pm.add(llvm::createLoopRotatePass(-1));
    pm.add(llvm::createLoopDistributePass());

    pm.add(llvm::createLoopVectorizePass(false, false));
    pm.add(llvm::createLoopLoadEliminationPass());
    pm.add(llvm::createInstructionCombiningPass(runExpensiveInstructionConbining));
    // pm.add(llvm::createCFGSimplificationPass(1, true, true, false, true));
    pm.add(llvm::createCFGSimplificationPass(llvm::SimplifyCFGOptions()
                                                 .forwardSwitchCondToPhi(true)
                                                 .convertSwitchToLookupTable(true)
                                                 .needCanonicalLoops(false)
                                                 .sinkCommonInsts(true)));
    pm.add(llvm::createSLPVectorizerPass());
    pm.add(llvm::createInstructionCombiningPass(runExpensiveInstructionConbining));
    pm.add(llvm::createLoopUnrollPass(2, false, false));
    pm.add(llvm::createInstructionCombiningPass(runExpensiveInstructionConbining));
    if (unnecessaryForUs) {
        pm.add(llvm::createLICMPass(100, 250));
        pm.add(llvm::createAlignmentFromAssumptionsPass());
        pm.add(llvm::createStripDeadPrototypesPass());
    }
    pm.add(llvm::createGlobalDCEPass());     // Remove dead fns and globals.
    pm.add(llvm::createConstantMergePass()); // Merge dup global constants

    pm.add(llvm::createHotColdSplittingPass());
    pm.add(llvm::createMergeFunctionsPass());
    // START SORBET ADDITION
    // after splitting hot/cold paths, inline hot paths before GVN
    pm.add(llvm::createFunctionInliningPass(optLevel, sizeLevel, false));
    // END SORBET ADDITION
    if (unnecessaryForUs) {
        pm.add(llvm::createLoopSinkPass());
        pm.add(llvm::createInstSimplifyLegacyPass());
        pm.add(llvm::createDivRemPairsPass());
        pm.add(llvm::createCFGSimplificationPass());
    }
}
void addLTOPasses(llvm::legacy::PassManager &pm, llvm::ModulePass *printLowered) {
    // this is intended to mimic llvm::PassManagerBuilder::populateLTOPassManager
    // while disabling optimizations that don't help us much to speedup and adding ones that do. Please explicitly
    // leave comments with added/removed passes
    // pmbuilder.populateLTOPassManager(pm);

    // these are optimizations
    if (unnecessaryForUs) {
        pm.add(llvm::createGlobalDCEPass());
    }
    pm.add(llvm::createTypeBasedAAWrapperPass());
    pm.add(llvm::createScopedNoAliasAAWrapperPass());
    if (unnecessaryForUs) {
        pm.add(llvm::createForceFunctionAttrsLegacyPass());
        pm.add(llvm::createInferFunctionAttrsLegacyPass());
        pm.add(llvm::createCallSiteSplittingPass());
        pm.add(llvm::createPGOIndirectCallPromotionLegacyPass(true, false));
    }
    pm.add(llvm::createIPSCCPPass());
    pm.add(llvm::createCalledValuePropagationPass()); // should follow IPSCCP
    if (unnecessaryForUs) {
        pm.add(llvm::createAttributorLegacyPass());
        pm.add(llvm::createPostOrderFunctionAttrsLegacyPass());
        pm.add(llvm::createReversePostOrderFunctionAttrsPass());
        pm.add(llvm::createGlobalSplitPass());
    }

    if (unnecessaryForUs) {
        pm.add(llvm::createWholeProgramDevirtPass(nullptr, nullptr));
    }
    pm.add(llvm::createGlobalOptimizerPass());
    pm.add(llvm::createPromoteMemoryToRegisterPass());
    if (unnecessaryForUs) {
        pm.add(llvm::createConstantMergePass());
    }
    pm.add(llvm::createDeadArgEliminationPass());
    if (unnecessaryForUs) {
        pm.add(llvm::createAggressiveInstCombinerPass()); // O3
        pm.add(llvm::createInstructionCombiningPass(runExpensiveInstructionConbining));

        pm.add(llvm::createFunctionInliningPass(optLevel, sizeLevel, false));
        pm.add(llvm::createPruneEHPass());
        pm.add(llvm::createGlobalOptimizerPass());
        pm.add(llvm::createGlobalDCEPass());
    }
    pm.add(llvm::createArgumentPromotionPass());
    if (unnecessaryForUs) {
        pm.add(llvm::createInstructionCombiningPass(runExpensiveInstructionConbining));
        pm.add(llvm::createJumpThreadingPass());
        pm.add(llvm::createSROAPass());
        pm.add(llvm::createTailCallEliminationPass());
        pm.add(llvm::createPostOrderFunctionAttrsLegacyPass());
        pm.add(llvm::createGlobalsAAWrapperPass());
    }
    pm.add(llvm::createLICMPass(100, 250));
    if (unnecessaryForUs) {
        pm.add(llvm::createMergedLoadStoreMotionPass());
        pm.add(llvm::createGVNPass(false));
    }
    pm.add(llvm::createNewGVNPass()); // by default clang uses _old_ GVN
    if (unnecessaryForUs) {
        pm.add(llvm::createMemCpyOptPass());
    }
    pm.add(llvm::createDeadStoreEliminationPass());
    pm.add(llvm::createIndVarSimplifyPass());
    if (unnecessaryForUs) {
        pm.add(llvm::createLoopDeletionPass());
        pm.add(llvm::createSimpleLoopUnrollPass(2, false, false));
        pm.add(llvm::createLoopVectorizePass(false, false));
    }
    pm.add(llvm::createLoopUnrollPass(2, false, false));
    pm.add(llvm::createInstructionCombiningPass(runExpensiveInstructionConbining));
    {
        // print out the IR right before the sorbet lowerings
        if (printLowered != nullptr) {
            pm.add(printLowered);
        }

        // Sorbet modifications, run our lowering super late in pipeline
        // this allows other optimizations to move intrinsics around as black boxes and keep optimizing them under
        // assumptions(that are marked with function attributes)
        for (auto pass : Passes::standardLowerings()) {
            pm.add(pass);
        }

        {
            pm.add(llvm::createFunctionInliningPass(optLevel, sizeLevel, false));
            pm.add(llvm::createPruneEHPass());
            pm.add(llvm::createGlobalOptimizerPass());
            pm.add(llvm::createGlobalDCEPass());
            pm.add(llvm::createInstructionCombiningPass(runExpensiveInstructionConbining));
            pm.add(llvm::createTailCallEliminationPass());
            // pm.add(llvm::createNewGVNPass()); // a single benchmark(fib) benefits from this. Wonder if pay-server
            // does?
        }
    }
    pm.add(llvm::createCFGSimplificationPass());
    pm.add(llvm::createSCCPPass());
    pm.add(llvm::createInstructionCombiningPass(runExpensiveInstructionConbining));
    if (unnecessaryForUs) {
        pm.add(llvm::createBitTrackingDCEPass());
    }
    pm.add(llvm::createSLPVectorizerPass());
    if (unnecessaryForUs) {
        pm.add(llvm::createAlignmentFromAssumptionsPass());
    }
    pm.add(llvm::createInstructionCombiningPass(runExpensiveInstructionConbining));
    pm.add(llvm::createJumpThreadingPass());

    if (unnecessaryForUs) {
        // non-optimization passes
        pm.add(llvm::createCrossDSOCFIPass());
    }
    if (unnecessaryForUs) {
        pm.add(llvm::createLowerTypeTestsPass(nullptr, nullptr));
    }

    if (unnecessaryForUs) {
        // late optimization passes
        pm.add(llvm::createHotColdSplittingPass());
        pm.add(llvm::createCFGSimplificationPass());
        pm.add(llvm::createEliminateAvailableExternallyPass());
    }
    pm.add(llvm::createGlobalDCEPass());
    // pm.add(llvm::createMergeFunctionsPass());
}
}; // namespace

void ObjectFileEmitter::init() {
    // Initialize the target registry etc.
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();
}

[[nodiscard]] bool ObjectFileEmitter::run(spdlog::logger &logger, llvm::LLVMContext &lctx,
                                          unique_ptr<llvm::Module> module, string_view soDir,
                                          optional<string_view> llvmIrDir, string_view objectName) {
    // We need to ensure that the codegen flags have been initialized, so that InitTargetOptionsFromCodeGenFlags has
    // sane defaults to use.
    static llvm::codegen::RegisterCodeGenFlags codeGenFlags;

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

    auto opt = llvm::codegen::InitTargetOptionsFromCodeGenFlags(llvm::Triple(targetTriple));
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
    if (llvmIrDir.has_value()) {
        llvm::legacy::PassManager ppm;
        std::error_code ec;
        auto name = fmt::format("{}/{}.ll", llvmIrDir.value(), objectName);
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
    pmbuilder.OptLevel = optLevel;
    pmbuilder.SizeLevel = sizeLevel;
    pmbuilder.Inliner = llvm::createFunctionInliningPass(optLevel, sizeLevel, false);
    pmbuilder.DisableUnrollLoops = false;
    pmbuilder.LoopVectorize = true;
    pmbuilder.SLPVectorize = true;
    pmbuilder.VerifyInput = debug_mode;
    targetMachine->adjustPassManager(pmbuilder);
    pmbuilder.populateFunctionPassManager(fnPasses);
    std::error_code ec1;
    // We need to run this early, prior to inlining, so the intrinsics to remove
    // still exist in some fashion.
    pm.add(Passes::createDeleteUnusedSorbetIntrinsticsPass());
    pm.add(Passes::createDeleteUnusedInlineCachesPass());
    pm.add(llvm::createAlwaysInlinerLegacyPass(false)); // Force inline functions early
    pm.add(llvm::createGlobalDCEPass());                // Remove dead fns and globals. We benefit from this a lot
    // Module passes
    addModulePasses(pm);
    pm.add(Passes::createRemoveUnnecessaryHashDupsPass());
    // print lowered IR
    llvm::ModulePass *printLowered = nullptr;
    // We put the ostream in an std::optional declared outside the "if", because unfortunately `createPrintModulePass`
    // doesn't take ownership of it.
    std::optional<llvm::raw_fd_ostream> loweredllFile;
    if (llvmIrDir.has_value()) {
        auto nameOptl = fmt::format("{}/{}.lowered.ll", llvmIrDir.value(), objectName);
        loweredllFile.emplace(nameOptl, ec1, llvm::sys::fs::F_Text);
        printLowered = llvm::createPrintModulePass(loweredllFile.value(), "");
    }
    // LTO passes
    addLTOPasses(pm, printLowered);
    // print optimized IR
    // We put the ostream in an std::optional declared outside the "if", because unfortunately `createPrintModulePass`
    // doesn't take ownership of it.
    std::optional<llvm::raw_fd_ostream> optllFile;
    if (llvmIrDir.has_value()) {
        auto nameOpt = fmt::format("{}/{}.opt.ll", llvmIrDir.value(), objectName);
        optllFile.emplace(nameOpt, ec1, llvm::sys::fs::F_Text);
        pm.add(llvm::createPrintModulePass(optllFile.value(), ""));
    }
    {
        Timer timer("functionPasses");

        fnPasses.doInitialization();
        for (llvm::Function &func : *module) {
            if (debug_mode && llvm::verifyFunction(func, &llvm::errs())) {
                fmt::print("failed to verify:\n");
                func.dump();
                ENFORCE(false);
            }

            fnPasses.run(func);
        }
        fnPasses.doFinalization();
    }
    if (debug_mode) {
        pm.add(llvm::createVerifierPass());
    }
    auto objectFileName = fmt::format("{}/{}.o", soDir, objectName);
    auto soNamePrefix = fmt::format("{}/{}", soDir, objectName);
    if (!outputObjectFile(logger, pm, string(objectName), objectFileName, move(module), targetMachine)) {
        return false;
    }
    if (!Linker::run(logger, {objectFileName}, soNamePrefix)) {
        return false;
    }
    FileOps::removeFile(objectFileName);
    return true;
}

} // namespace sorbet::compiler
