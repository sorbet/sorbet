#include "llvm/linker/linker.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

namespace sorbet::llvm::linker {

static ::llvm::LLVMContext TheContext;
static ::llvm::IRBuilder<> Builder(TheContext);
static ::llvm::Module TheModule("sorbet", TheContext);

void init() {
}

void run(std::shared_ptr<spdlog::logger> logger) {
    TheModule.print(::llvm::errs(), nullptr);
}

} // namespace sorbet::llvm::linker
