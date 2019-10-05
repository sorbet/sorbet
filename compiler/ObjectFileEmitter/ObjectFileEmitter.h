#ifndef SORBET_COMPILER_OBJECT_FILE_EMITTER_H
#define SORBET_COMPILER_OBJECT_FILE_EMITTER_H

#include "core/core.h"
#include <memory>
#include <string_view>

namespace llvm {
class LLVMContext;
class Module;
class BasicBlock;
} // namespace llvm

namespace sorbet::compiler {
class ObjectFileEmitter {
public:
    static void init();
    static void run(const core::GlobalState &gs, llvm::LLVMContext &lctx, std::unique_ptr<llvm::Module> module,
                    core::SymbolRef sym, std::string_view targetDir, std::string_view fileName,
                    llvm::BasicBlock *globalInitializers);
};
} // namespace sorbet::compiler
#endif
