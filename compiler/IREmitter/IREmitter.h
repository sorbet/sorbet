#ifndef SORBET_COMPILER_LLVM_IR_EMITTER_H
#define SORBET_COMPILER_LLVM_IR_EMITTER_H
#include "compiler/Payload/ForwardDeclarations.h"
#include <string_view>

namespace sorbet::compiler {
class CompilerState;
class IREmitter {
public:
    static void run(CompilerState &, cfg::CFG &cfg, std::unique_ptr<ast::MethodDef> &md,
                    const std::string &functionName);
    static void buildInitFor(CompilerState &gs, const core::SymbolRef &sym, std::string_view objectName);
};
} // namespace sorbet::compiler
#endif
