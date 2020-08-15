#ifndef SORBET_COMPILER_PAYLOAD_H
#define SORBET_COMPILER_PAYLOAD_H

#include "cfg/LocalRef.h"
#include "compiler/Core/ForwardDeclarations.h"
#include "core/core.h"

namespace sorbet::compiler {

struct IREmitterContext;
struct Alias;
class CompilerState;

// This class serves as forwarder to payload.c, which are the c wrappers for
// Ruby functions. These functions can (and do) use information known during
// compile time to dispatch to different c functions, but other than that, they
// should mostly be forwarders.
class Payload {
public:
    // api for payload debugging utilities
    static void rubyStopInDebugger(CompilerState &cs, llvm::IRBuilderBase &builder);
    static void dbg_p(CompilerState &cs, llvm::IRBuilderBase &builder, llvm::Value *val);

    // api for actual code emission
    static llvm::Value *idIntern(CompilerState &cs, llvm::IRBuilderBase &builder, std::string_view idName);
    static llvm::Value *setExpectedBool(CompilerState &cs, llvm::IRBuilderBase &builder, llvm::Value *boolean,
                                        bool expected);
    // boxed raw value from rawData into target. Assumes that types are compatible.
    static void boxRawValue(CompilerState &cs, llvm::IRBuilderBase &builder, llvm::AllocaInst *storeTarget,
                            llvm::Value *rawData);
    static llvm::Value *unboxRawValue(CompilerState &cs, llvm::IRBuilderBase &builder, llvm::AllocaInst *storeTarget);

    static llvm::Value *rubyUndef(CompilerState &cs, llvm::IRBuilderBase &builder);
    static llvm::Value *rubyNil(CompilerState &cs, llvm::IRBuilderBase &builder);
    static llvm::Value *rubyFalse(CompilerState &cs, llvm::IRBuilderBase &builder);
    static llvm::Value *rubyTrue(CompilerState &cs, llvm::IRBuilderBase &builder);
    static llvm::Value *rubyTopSelf(CompilerState &cs, llvm::IRBuilderBase &builder);
    static void raiseArity(CompilerState &cs, llvm::IRBuilderBase &builder, llvm::Value *currentArgCount, int minArgs,
                           int maxArgs);
    static llvm::Value *longToRubyValue(CompilerState &cs, llvm::IRBuilderBase &builder, long num);
    static llvm::Value *doubleToRubyValue(CompilerState &cs, llvm::IRBuilderBase &builder, double num);
    static llvm::Value *cPtrToRubyString(CompilerState &cs, llvm::IRBuilderBase &builder, std::string_view str,
                                         bool frozen);
    static llvm::Value *cPtrToRubyRegexp(CompilerState &cs, llvm::IRBuilderBase &builder, std::string_view str,
                                         int options);
    static llvm::Value *testIsUndef(CompilerState &cs, llvm::IRBuilderBase &builder, llvm::Value *val);
    static llvm::Value *testIsTruthy(CompilerState &cs, llvm::IRBuilderBase &builder, llvm::Value *val);
    static llvm::Value *getRubyConstant(CompilerState &cs, core::SymbolRef sym, llvm::IRBuilderBase &builder);
    static llvm::Value *toCString(CompilerState &cs, std::string_view str, llvm::IRBuilderBase &builder);
    static llvm::Value *typeTest(CompilerState &cs, llvm::IRBuilderBase &builder, llvm::Value *val,
                                 const core::TypePtr &type);
    static llvm::Value *boolToRuby(CompilerState &cs, llvm::IRBuilderBase &builder, llvm::Value *u1);
    static std::pair<llvm::Value *, llvm::Value *> setRubyStackFrame(CompilerState &cs, llvm::IRBuilderBase &builder,
                                                                     const IREmitterContext &irctx,
                                                                     const ast::MethodDef &md, int rubyBlockId);

    static llvm::Value *readKWRestArg(CompilerState &cs, llvm::IRBuilderBase &builder, llvm::Value *maybeHash);
    static llvm::Value *assertNoExtraKWArg(CompilerState &cs, llvm::IRBuilderBase &builder, llvm::Value *maybeHash);
    static llvm::Value *getKWArg(CompilerState &cs, llvm::IRBuilderBase &builder, llvm::Value *maybeHash,
                                 llvm::Value *rubySym);
    static llvm::Value *readRestArgs(CompilerState &cs, llvm::IRBuilderBase &builder, int maxPositionalArgCount,
                                     llvm::Value *argCountRaw, llvm::Value *argArrayRaw);
    static core::Loc setLineNumber(CompilerState &cs, llvm::IRBuilderBase &builder, core::Loc loc,
                                   core::Loc methodStart, core::Loc lastLoc, llvm::AllocaInst *iseqEncodedPtr,
                                   llvm::AllocaInst *lineNumberPtr);
    static llvm::Value *varGet(CompilerState &cs, cfg::LocalRef local, llvm::IRBuilderBase &builder,
                               const IREmitterContext &irctx, int rubyBlockId);
    static void varSet(CompilerState &cs, cfg::LocalRef local, llvm::Value *var, llvm::IRBuilderBase &builder,
                       const IREmitterContext &irctx, int rubyBlockId);

    static llvm::Value *retrySingleton(CompilerState &cs, llvm::IRBuilderBase &builder, const IREmitterContext &irctx);
};
} // namespace sorbet::compiler
#endif
