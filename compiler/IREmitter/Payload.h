#ifndef SORBET_COMPILER_PAYLOAD_H
#define SORBET_COMPILER_PAYLOAD_H

#include "cfg/LocalRef.h"
#include "compiler/Core/ForwardDeclarations.h"
#include "core/core.h"

namespace sorbet::compiler {

struct IREmitterContext;
struct Alias;
struct EscapedUse;
class CompilerState;

struct EscapedVariableInfo {
    const EscapedUse &use;
    // The index of the variable within the frame.
    llvm::Value *index;
    // The number of frames to traverse.
    llvm::Value *level;
};

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
                                 core::ClassOrModuleRef sym);
    static llvm::Value *typeTest(CompilerState &cs, llvm::IRBuilderBase &builder, llvm::Value *val,
                                 const core::TypePtr &type);

    static void assumeType(CompilerState &cs, llvm::IRBuilderBase &builder, llvm::Value *val,
                           core::ClassOrModuleRef sym);
    static void assumeType(CompilerState &cs, llvm::IRBuilderBase &builder, llvm::Value *val,
                           const core::TypePtr &type);

    static llvm::Value *boolToRuby(CompilerState &cs, llvm::IRBuilderBase &builder, llvm::Value *uint8_t);
    static llvm::Value *setRubyStackFrame(CompilerState &cs, llvm::IRBuilderBase &builder,
                                          const IREmitterContext &irctx, const ast::MethodDef &md, int rubyRegionId);

    static llvm::Value *readKWRestArg(CompilerState &cs, llvm::IRBuilderBase &builder, llvm::Value *maybeHash);
    static llvm::Value *assertNoExtraKWArg(CompilerState &cs, llvm::IRBuilderBase &builder, llvm::Value *maybeHash,
                                           llvm::Value *numRequired, llvm::Value *optionalParsed);
    static llvm::Value *assertAllRequiredKWArgs(CompilerState &cs, llvm::IRBuilderBase &builder, llvm::Value *missing);
    static llvm::Value *addMissingKWArg(CompilerState &cs, llvm::IRBuilderBase &builder, llvm::Value *missing,
                                        llvm::Value *sym);

    static llvm::Value *getKWArg(CompilerState &cs, llvm::IRBuilderBase &builder, llvm::Value *maybeHash,
                                 llvm::Value *rubySym);
    static llvm::Value *removeKWArg(CompilerState &cs, llvm::IRBuilderBase &builder, llvm::Value *maybeHash,
                                    llvm::Value *rubySym);
    static llvm::Value *readRestArgs(CompilerState &cs, llvm::IRBuilderBase &builder, int maxPositionalArgCount,
                                     llvm::Value *argCountRaw, llvm::Value *argArrayRaw);
    static core::Loc setLineNumber(CompilerState &cs, llvm::IRBuilderBase &builder, core::Loc loc,
                                   core::Loc methodStart, core::Loc lastLoc, llvm::AllocaInst *lineNumberPtr);
    static llvm::Value *buildInstanceVariableCache(CompilerState &cs, std::string_view name);
    static llvm::Value *getClassVariableStoreClass(CompilerState &cs, llvm::IRBuilderBase &builder,
                                                   const IREmitterContext &irctx);
    static llvm::Value *varGet(CompilerState &cs, cfg::LocalRef local, llvm::IRBuilderBase &builder,
                               const IREmitterContext &irctx, int rubyRegionId);
    static void varSet(CompilerState &cs, cfg::LocalRef local, llvm::Value *var, llvm::IRBuilderBase &builder,
                       const IREmitterContext &irctx, int rubyRegionId);

    static EscapedVariableInfo escapedVariableInfo(CompilerState &cs, cfg::LocalRef local,
                                                   const IREmitterContext &irctx, int rubyRegionId);

    static llvm::Value *retrySingleton(CompilerState &cs, llvm::IRBuilderBase &builder, const IREmitterContext &irctx);
    static llvm::Value *voidSingleton(CompilerState &cs, llvm::IRBuilderBase &builder, const IREmitterContext &irctx);

    static void pushRubyStackVector(CompilerState &cs, llvm::IRBuilderBase &build, llvm::Value *cfp, llvm::Value *recv,
                                    const std::vector<llvm::Value *> &stack);

    static llvm::Value *vmBlockHandlerNone(CompilerState &cs, llvm::IRBuilderBase &builder);
    static llvm::Value *makeBlockHandlerProc(CompilerState &cs, llvm::IRBuilderBase &builder, llvm::Value *block);
    static llvm::Value *getPassedBlockHandler(CompilerState &cs, llvm::IRBuilderBase &builder);

    static llvm::Value *callFuncWithCache(CompilerState &cs, llvm::IRBuilderBase &builder, llvm::Value *cache,
                                          llvm::Value *blockHandler);
    static llvm::Value *callFuncBlockWithCache(CompilerState &cs, llvm::IRBuilderBase &builder, llvm::Value *cache,
                                               bool usesBreak, llvm::Value *ifunc);
    static llvm::Value *callSuperFuncWithCache(CompilerState &cs, llvm::IRBuilderBase &builder, llvm::Value *cache,
                                               llvm::Value *blockHandler);
    static llvm::Value *callSuperFuncBlockWithCache(CompilerState &cs, llvm::IRBuilderBase &builder, llvm::Value *cache,
                                                    bool usesBreak, llvm::Value *ifunc);
    static llvm::Value *callFuncDirect(CompilerState &cs, llvm::IRBuilderBase &builder, llvm::Value *cache,
                                       llvm::Value *fn, llvm::Value *argc, llvm::Value *argv, llvm::Value *recv,
                                       llvm::Value *iseq);
    static void afterIntrinsic(CompilerState &cs, llvm::IRBuilderBase &builder);

    static llvm::Value *rubyStackFrameVar(CompilerState &cs, llvm::IRBuilderBase &builder,
                                          const IREmitterContext &irctx, core::MethodRef methodSym);

    static llvm::Value *getFileLineNumberInfo(CompilerState &gs, llvm::IRBuilderBase &builder, core::FileRef file);
    static llvm::Value *getIseqEncodedPointer(CompilerState &gs, llvm::IRBuilderBase &builder, core::FileRef file);

    static llvm::Value *getCFPForBlock(CompilerState &cs, llvm::IRBuilderBase &builder, const IREmitterContext &irctx,
                                       int rubyRegionId);

    static llvm::Value *buildLocalsOffset(CompilerState &cs);

    static llvm::Value *getOrBuildBlockIfunc(CompilerState &cs, llvm::IRBuilderBase &builder,
                                             const IREmitterContext &irctx, int blkId);
};
} // namespace sorbet::compiler
#endif
