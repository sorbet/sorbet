#ifndef SORBET_LLVM_LINKER_H
#define SORBET_LLVM_LINKER_H

#include "spdlog/spdlog.h"
#include <memory>
#include <string_view>

namespace llvm {
class LLVMContext;
class Module;
} // namespace llvm

namespace sorbet::compiler {
class Linker {
public:
    static void init();
    static void run(spdlog::logger &logger, ::llvm::LLVMContext &lctx, std::unique_ptr<::llvm::Module> module,
                    std::string_view targetDir, std::string_view fileName);
};
} // namespace sorbet::compiler
#endif
