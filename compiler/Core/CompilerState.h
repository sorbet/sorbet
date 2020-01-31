#ifndef SORBET_COMPILER_CORE_COMPILER_STATE_H
#define SORBET_COMPILER_CORE_COMPILER_STATE_H

#include "common/ConstExprStr.h"
#include "common/Exception.h"
#include "compiler/Core/ForwardDeclarations.h"
#include <memory>
#include <string_view>

namespace sorbet::compiler {
class CompilerState {
public:
    CompilerState(const core::GlobalState &gs, llvm::LLVMContext &lctx, llvm::Module *);

    const core::GlobalState &gs;
    llvm::LLVMContext &lctx;
    llvm::BasicBlock *functionEntryInitializers;
    llvm::Module *module;

    // useful apis for getting common types

    llvm::StructType *getValueType();
    llvm::FunctionType *getRubyFFIType();
    llvm::FunctionType *getRubyBlockFFIType();
    llvm::FunctionType *getSorbetIntrinsicFFIType();

    /* run optimizations that are super cheap which are expected to be run on each function immediately as it is
     * generated */
    void runCheapOptimizations(llvm::Function *);
    // conversion to Sorbet state
    operator const sorbet::core::GlobalState &() const {
        return gs;
    }

    operator llvm::LLVMContext &() const {
        return lctx;
    }

    // tracing
    void trace(std::string_view) const;
    void failCompilation(const core::Loc &loc, ConstExprStr msg) const;
};
} // namespace sorbet::compiler

#endif
