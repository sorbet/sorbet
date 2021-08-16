#ifndef SORBET_COMPILER_LLVMIREMITTER_INTRINSICS_H
#define SORBET_COMPILER_LLVMIREMITTER_INTRINSICS_H

#include "IREmitterHelpers.h"
#include <string_view>
#include <vector>

namespace sorbet::compiler {

struct IREmitterContext;

class NameBasedIntrinsicMethod {
public:
    const Intrinsics::HandleBlock blockHandled;
    virtual llvm::Value *makeCall(MethodCallContext &mcctx) const = 0;
    virtual InlinedVector<core::NameRef, 2> applicableMethods(CompilerState &cs) const = 0;
    NameBasedIntrinsicMethod(Intrinsics::HandleBlock blockHandled) : blockHandled(blockHandled){};
    virtual ~NameBasedIntrinsicMethod() = default;
    static const std::vector<const NameBasedIntrinsicMethod *> &definedIntrinsics();
};
}; // namespace sorbet::compiler
#endif
