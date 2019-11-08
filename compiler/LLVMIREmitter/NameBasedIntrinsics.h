#ifndef SORBET_COMPILER_LLVMIREMITTER_INTRINSICS_H
#define SORBET_COMPILER_LLVMIREMITTER_INTRINSICS_H

#include "LLVMIREmitterHelpers.h"
#include <string_view>
#include <vector>

namespace sorbet::compiler {
class NameBasedIntrinsicMethod {
public:
    virtual llvm::Value *makeCall(CompilerState &cs, cfg::Send *i, llvm::IRBuilderBase &builder,
                                  const BasicBlockMap &blockMap,
                                  const UnorderedMap<core::LocalVariable, Alias> &aliases,
                                  int currentRubyBlockId) const = 0;
    virtual InlinedVector<core::NameRef, 2> applicableMethods(CompilerState &cs) const = 0;

    static const std::vector<const NameBasedIntrinsicMethod *> &definedIntrinsics();
};
}; // namespace sorbet::compiler
#endif
