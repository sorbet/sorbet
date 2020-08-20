#ifndef SORBET_COMPILER_IREMITTER_METHODCALLCONTEXT_H
#define SORBET_COMPILER_IREMITTER_METHODCALLCONTEXT_H

#include "compiler/Core/ForwardDeclarations.h"

namespace sorbet::compiler {

class CompilerState;
struct IREmitterContext;

struct MethodCallContext {
    CompilerState &cs;

    llvm::IRBuilderBase &build;

    const IREmitterContext &irctx;

    // See IREmitterContext for a description of rubyBlockId. Primarily used to index into the
    // various vectors in `irctx`.
    int rubyBlockId;

    // The method call being emitted right now.
    cfg::Send *send;

    // The compiled function (like `func_A.main$block_1`) associated with this send via the
    // core::SendAndBlockLink. `nullptr` if no `do ... end` block provided at the call site.
    llvm::Function *blk;
};

} // namespace sorbet::compiler

#endif
