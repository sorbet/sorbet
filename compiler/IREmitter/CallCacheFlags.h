#ifndef SORBET_COMPILER_CALLCACHEFLAGS_H
#define SORBET_COMPILER_CALLCACHEFLAGS_H

#include "compiler/Core/ForwardDeclarations.h"

namespace sorbet::compiler {
class CompilerState;

struct CallCacheFlags {
    // Field names correspond to VM_CALL_* flags.
    bool args_simple = false;
    bool args_splat = false;
    bool kwarg = false;
    bool kw_splat = false;
    bool fcall = false;
    bool blockarg = false;

    llvm::Value *build(CompilerState &cs, llvm::IRBuilderBase &builder);
};

} // namespace sorbet::compiler

#endif
