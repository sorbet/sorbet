#ifndef SORBET_COMPILER_OBJECT_FILE_EMITTER_H
#define SORBET_COMPILER_OBJECT_FILE_EMITTER_H

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
    static void run(llvm::LLVMContext &lctx, std::unique_ptr<llvm::Module> module, std::string_view targetDir,
                    std::string_view fileName);
};
} // namespace sorbet::compiler
#endif
