#ifndef SORBET_COMPILER_CORE_COMPILER_STATE_H
#define SORBET_COMPILER_CORE_COMPILER_STATE_H

#include "common/ConstExprStr.h"
#include "common/Exception.h"
#include "compiler/Core/ForwardDeclarations.h"
#include "core/Files.h"
#include <memory>
#include <string_view>

namespace sorbet::compiler {

// Like GlobalState, but for sorbet_llvm.
class CompilerState {
public:
    // Things created and managed ouside of us (by either Sorbet or plugin_injector)
    CompilerState(const core::GlobalState &gs, llvm::LLVMContext &lctx, llvm::Module *, llvm::DIBuilder *,
                  llvm::DICompileUnit *, core::FileRef, llvm::BasicBlock *globalConstructorsEntry);

    const core::GlobalState &gs;
    llvm::LLVMContext &lctx;
    llvm::Module *module;
    llvm::BasicBlock *globalConstructorsEntry;

    // Debug info
    llvm::DIBuilder *debug;
    llvm::DICompileUnit *compileUnit;

    // Our own state

    // TODO(jez) Only used by IREmitter, thus this probably shouldn't live in CompilerState longer term.
    // (A better option might be to create an IREmitterContext of some sort.)
    llvm::BasicBlock *functionEntryInitializers;

    core::FileRef file;
    // useful apis for getting common types

    llvm::StructType *getValueType();
    llvm::FunctionType *getRubyFFIType();
    llvm::FunctionType *getRubyBlockFFIType();
    llvm::FunctionType *getRubyExceptionFFIType();
    llvm::FunctionType *getSorbetIntrinsicFFIType();

    // Run some cheap, per-function optimizations immediately after IR emission.
    void runCheapOptimizations(llvm::Function *);

    // Implicit conversion to Sorbet state
    operator const sorbet::core::GlobalState &() const {
        return gs;
    }

    // Implicit conversion to LLVMContext
    operator llvm::LLVMContext &() const {
        return lctx;
    }

    // Forwards to `GlobalState::trace`
    void trace(std::string_view) const;

    // Add an error to GlobalState, and then throw to abort compilation.
    // Use only when compilation CANNOT continue.
    // (Emitting any old GlobalState error will still cause Sorbet to exit non-zero.)
    void failCompilation(const core::Loc &loc, ConstExprStr msg) const;

    CompilerState withFunctionEntry(llvm::BasicBlock *functionEntry);
};

} // namespace sorbet::compiler

#endif
