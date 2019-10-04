#ifndef SORBET_COMPILER_LLVM_IR_EMITTER_H
#define SORBET_COMPILER_LLVM_IR_EMITTER_H

namespace llvm {
class LLVMContext;
class Module;
} // namespace llvm

namespace sorbet::core {
class GlobalState;
}
namespace sorbet::cfg {
class CFG;
}
namespace sorbet::ast {
class MethodDef;
}

namespace sorbet::compiler {
class LLVMIREmitter {
public:
    static void run(const core::GlobalState &, llvm::LLVMContext &lctx, cfg::CFG &cfg,
                    std::unique_ptr<ast::MethodDef> &md, const std::string &functionName, llvm::Module *);
};
} // namespace sorbet::compiler
#endif
