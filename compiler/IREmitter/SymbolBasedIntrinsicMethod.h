#ifndef SORBET_COMPILER_LLVMIREMITTER_SYMINTRINSICS_H
#define SORBET_COMPILER_LLVMIREMITTER_SYMINTRINSICS_H

#include "IREmitterHelpers.h"
#include <string_view>
#include <vector>

namespace sorbet::compiler {

struct IREmitterContext;

class SymbolBasedIntrinsicMethod {
public:
    const Intrinsics::HandleBlock blockHandled;
    virtual llvm::Value *makeCall(CompilerState &cs, cfg::Send *send, llvm::IRBuilderBase &builder,
                                  const IREmitterContext &irctx,
                                  const UnorderedMap<core::LocalVariable, Alias> &aliases, int rubyBlockId,
                                  llvm::Function *blk) const = 0;
    virtual InlinedVector<core::SymbolRef, 2> applicableClasses(CompilerState &cs) const = 0;
    virtual InlinedVector<core::NameRef, 2> applicableMethods(CompilerState &cs) const = 0;

    SymbolBasedIntrinsicMethod(Intrinsics::HandleBlock blockHandled) : blockHandled(blockHandled){};
    virtual ~SymbolBasedIntrinsicMethod() = default;
    static std::vector<const SymbolBasedIntrinsicMethod *> &definedIntrinsics();
};
}; // namespace sorbet::compiler
#endif
