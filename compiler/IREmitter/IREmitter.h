#ifndef SORBET_COMPILER_LLVM_IR_EMITTER_H
#define SORBET_COMPILER_LLVM_IR_EMITTER_H
#include "compiler/Core/ForwardDeclarations.h"
#include <string_view>

namespace sorbet::compiler {
class CompilerState;
class IREmitter {
public:
    static void run(CompilerState &, cfg::CFG &cfg, const ast::MethodDef &md);
    static void buildInitFor(CompilerState &gs, const core::MethodRef &sym, std::string_view objectName);
};
} // namespace sorbet::compiler
#endif
