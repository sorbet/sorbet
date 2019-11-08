#ifndef SORBET_COMPILER_LLVMIREMITTER_SYMINTRINSICS_H
#define SORBET_COMPILER_LLVMIREMITTER_SYMINTRINSICS_H

#include "LLVMIREmitterHelpers.h"
#include <string_view>
#include <vector>

namespace sorbet::compiler {
class SymbolBasedIntrinsicMethod {
public:
    virtual llvm::Value *makeCall(CompilerState &cs, cfg::Send *i, llvm::IRBuilderBase &builder,
                                  const BasicBlockMap &blockMap,
                                  const UnorderedMap<core::LocalVariable, Alias> &aliases, int rubyBlockId) const = 0;
    virtual InlinedVector<core::SymbolRef, 2> applicableClasses(CompilerState &cs) const = 0;
    virtual InlinedVector<core::NameRef, 2> applicableMethods(CompilerState &cs) const = 0;

    virtual ~SymbolBasedIntrinsicMethod() = default;
    static std::vector<const SymbolBasedIntrinsicMethod *> &definedIntrinsics();
};
}; // namespace sorbet::compiler
#endif
