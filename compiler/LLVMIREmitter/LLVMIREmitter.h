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
    static void buildInitFor(CompilerState &gs, const core::SymbolRef &sym, std::string_view objectName);
};
} // namespace sorbet::compiler
#endif
