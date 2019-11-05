#ifndef SORBET_COMPILER_LLVM_IR_EMITTER_IMPL_H
#define SORBET_COMPILER_LLVM_IR_EMITTER_IMPL_H
#include "LLVMIREmitter.h"
#include <vector>

namespace llvm {
class Function;
};
namespace sorbet::compiler {
class LLVMIREmitterHelpers {
public:
    static std::vector<llvm::Function *> getRubyBlocks2FunctionsMapping(CompilerState &cs, cfg::CFG &cfg,
                                                                        llvm::Function *func);
};
} // namespace sorbet::compiler
#endif
