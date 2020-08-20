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
    virtual llvm::Value *makeCall(MethodCallContext &mcctx) const = 0;
    virtual InlinedVector<core::SymbolRef, 2> applicableClasses(CompilerState &cs) const = 0;
    virtual InlinedVector<core::NameRef, 2> applicableMethods(CompilerState &cs) const = 0;

    // Before emitting a call to a symbol-based intrinsic, the compiled code will emit a runtime
    // type test to ensure that the receiver actually has the expected type at runtime (i.e., the
    // type in the CFG could be wrong).
    //
    // Sometimes we need to explicitly opt out of that type test, because it doesn't make sense, and
    // we completely trust the type of the receiver (e.g., because it's controlled exclusively by
    // Sorbet, not the user).
    //
    // When using this, you must be very careful to handle cases where the runtime type information
    // doesn't match up with the static type information, or ensure that it is impossible for that
    // information to mismatch. This most commonly means falling back to dispatch the method via the
    // Ruby VM.
    virtual bool skipReceiverTypeTest() const {
        return false;
    };

    SymbolBasedIntrinsicMethod(Intrinsics::HandleBlock blockHandled) : blockHandled(blockHandled){};
    virtual ~SymbolBasedIntrinsicMethod() = default;
    static std::vector<const SymbolBasedIntrinsicMethod *> &definedIntrinsics();
};
}; // namespace sorbet::compiler
#endif
