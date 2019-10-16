#ifndef SORBET_COMPILER_LLVM_IR_EMITTER_H
#define SORBET_COMPILER_LLVM_IR_EMITTER_H
#include <string_view>

namespace llvm {
class LLVMContext;
class Module;
class BasicBlock;
} // namespace llvm

namespace sorbet::core {
class GlobalState;
class SymbolRef;
} // namespace sorbet::core
namespace sorbet::cfg {
class CFG;
}
namespace sorbet::ast {
class MethodDef;
}

namespace sorbet::compiler {
class CompilerState;
class LLVMIREmitter {
public:
    static void run(CompilerState &, cfg::CFG &cfg, std::unique_ptr<ast::MethodDef> &md,
                    const std::string &functionName);
    static void buildInitFor(const core::GlobalState &gs, llvm::LLVMContext &lctx, llvm::Module *module,
                             const core::SymbolRef &sym, llvm::BasicBlock *globalInitializers,
                             std::string_view objectName);
};
} // namespace sorbet::compiler
#endif
