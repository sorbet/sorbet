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
    virtual InlinedVector<core::ClassOrModuleRef, 2> applicableClasses(const core::GlobalState &gs) const = 0;
    virtual InlinedVector<core::NameRef, 2> applicableMethods(const core::GlobalState &gs) const = 0;

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
    virtual llvm::Value *receiverFastPathTest(MethodCallContext &mcctx, core::ClassOrModuleRef potentialClass) const;

    // The above is a runtime test, since it returns an llvm::Value (which might be `true`).
    // This method is a compile test for instances where we statically know the runtime
    // test would succeed.  This might be done for intrinsics which we know don't have
    // a slow path through the VM (e.g. calls on Sorbet::Private::Static) or where having
    // a slow path through the VM would hinder optimizations like having an assumption
    // about the return type of the method.
    //
    // You should not typically be returning true from this.
    virtual bool skipFastPathTest(MethodCallContext &mcctx, core::ClassOrModuleRef potentialClass) const;

    // Returns whether we should be calling Payload::afterIntrinsic to handle any
    // post-intrinsic processing (e.g. checking whether VM-level interrupts have arrived).
    // The default is to call.
    //
    // You should not typically be returning false from this.
    virtual bool needsAfterIntrinsicProcessing() const;

    SymbolBasedIntrinsicMethod(Intrinsics::HandleBlock blockHandled) : blockHandled(blockHandled){};
    virtual ~SymbolBasedIntrinsicMethod() = default;
    static std::vector<const SymbolBasedIntrinsicMethod *> &definedIntrinsics(const core::GlobalState &gs);

    virtual void sanityCheck(const core::GlobalState &gs) const;
};
}; // namespace sorbet::compiler
#endif
