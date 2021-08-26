#ifndef SORBET_COMPILER_OBJECT_FILE_EMITTER_H
#define SORBET_COMPILER_OBJECT_FILE_EMITTER_H

#include <memory>
#include <optional>
#include <string_view>
namespace spdlog {
class logger;
}
namespace llvm {
class LLVMContext;
class Module;
class BasicBlock;
} // namespace llvm

namespace sorbet::compiler {
class ObjectFileEmitter {
public:
    static void init();
    [[nodiscard]] static bool run(spdlog::logger &logger, llvm::LLVMContext &lctx, std::unique_ptr<llvm::Module> module,
                                  std::string_view soDir, std::optional<std::string_view> llvmIrDir,
                                  std::string_view fileName);
};
} // namespace sorbet::compiler
#endif
