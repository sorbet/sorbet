#ifndef SORBET_COMPILER_IREMITTER_IREMITTERHELPERS_H
#define SORBET_COMPILER_IREMITTER_IREMITTERHELPERS_H
#include "IREmitter.h"
#include "cfg/CFG.h"
#include "compiler/Core/ForwardDeclarations.h"
#include "compiler/IREmitter/CallCacheFlags.h"
#include "compiler/IREmitter/RubyStackArgs.h"
#include "core/core.h"
#include <string_view>
#include <vector>

namespace sorbet::compiler {

struct IREmitterContext;
class MethodCallContext;
struct VMFlag;

// TODO(jez) This shouldn't be at the top-level (sorbet::compiler). It should probably be nested in something.
// (Confusing to see bare `Alias` when there is also `cfg::Alias`)
struct Alias {
    enum class AliasKind { Constant, InstanceField, ClassField, GlobalField };
    AliasKind kind;
    core::SymbolRef constantSym;
    core::NameRef instanceField;
    core::NameRef classField;
    core::NameRef globalField;
    static Alias forConstant(core::SymbolRef sym) {
        Alias ret;
        ret.kind = AliasKind::Constant;
        ret.constantSym = sym;
        return ret;
    }
    static Alias forClassField(core::NameRef name) {
        Alias ret;
        ret.kind = AliasKind::ClassField;
        ret.classField = name;
        return ret;
    }
    static Alias forInstanceField(core::NameRef name) {
        Alias ret;
        ret.kind = AliasKind::InstanceField;
        ret.instanceField = name;
        return ret;
    }
    static Alias forGlobalField(core::NameRef name) {
        Alias ret;
        ret.kind = AliasKind::GlobalField;
        ret.globalField = name;
        return ret;
    }
};

class Intrinsics {
public:
    enum class HandleBlock : uint8_t {
        Handled = 1,
        Unhandled = 2,
    };
};

struct KnownFunction {
    enum class Type {
        // Symbols available for direct comparison
        Symbol,

        // Symbols that are computed by calling another function, and then cached
        CachedSymbol,
    };

    Type type;
    std::string name;

    // Implicit constructor from a string for backwards compatibility
    KnownFunction(std::string name) : KnownFunction(Type::Symbol, std::move(name)) {}

    static KnownFunction cached(std::string name) {
        return KnownFunction{Type::CachedSymbol, std::move(name)};
    }

    // Fetch the function referenced from the payload.
    llvm::Value *getFunction(CompilerState &cs, llvm::IRBuilderBase &builder) const;

private:
    KnownFunction(Type type, std::string name) : type{type}, name{std::move(name)} {}
};

class IREmitterHelpers {
public:
    static bool isClassStaticInit(const core::GlobalState &gs, core::MethodRef sym);
    static bool isFileStaticInit(const core::GlobalState &gs, core::MethodRef sym);
    static bool isFileOrClassStaticInit(const core::GlobalState &gs, core::MethodRef sym);

    static std::string getFunctionName(CompilerState &cs, core::MethodRef sym);
    static llvm::Function *lookupFunction(CompilerState &cs, core::MethodRef sym);
    static llvm::Function *cleanFunctionBody(CompilerState &cs, llvm::Function *func);
    static llvm::Function *getOrCreateStaticInit(CompilerState &cs, core::MethodRef sym, core::LocOffsets loc);
    static llvm::Function *getOrCreateFunction(CompilerState &cs, core::MethodRef sym);
    static llvm::Function *getOrCreateDirectWrapper(CompilerState &cs, core::MethodRef sym);

    static llvm::Function *getInitFunction(CompilerState &cs, core::MethodRef sym);

    static std::size_t sendArgCount(cfg::Send *send);

    struct SendArgInfo {
        SendArgInfo(llvm::Value *argc, llvm::Value *argv, llvm::Value *kw_splat, std::vector<llvm::Value *> argValues);

        llvm::Value *argc;
        llvm::Value *argv;
        llvm::Value *kw_splat;

        std::vector<llvm::Value *> argValues;
    };

    static SendArgInfo fillSendArgArray(MethodCallContext &mcctx, const std::size_t offset);
    static SendArgInfo fillSendArgArray(MethodCallContext &mcctx);

    static llvm::Value *buildU4(CompilerState &cs, uint32_t i);
    static llvm::Value *buildS4(CompilerState &cs, int i);

    static llvm::Value *emitMethodCall(MethodCallContext &mcctx);

    static RubyStackArgs buildSendArgs(MethodCallContext &mcctx, cfg::LocalRef recv, const std::size_t offset);

    static llvm::Value *makeInlineCache(CompilerState &cs, llvm::IRBuilderBase &build, std::string methodName,
                                        CallCacheFlags flags, int argc, const std::vector<std::string_view> &keywords);

    static llvm::Value *emitMethodCallViaRubyVM(MethodCallContext &mcctx);

    static void emitExceptionHandlers(CompilerState &gs, llvm::IRBuilderBase &builder, const IREmitterContext &irctx,
                                      int rubyRegionId, int bodyRubyRegionId, cfg::LocalRef exceptionValue);

    static void emitDebugLoc(CompilerState &gs, llvm::IRBuilderBase &builder, const IREmitterContext &irctx,
                             int rubyRegionId, core::Loc loc);

    static void emitUncheckedReturn(CompilerState &gs, llvm::IRBuilderBase &builder, const IREmitterContext &irctx,
                                    int rubyRegionId, llvm::Value *retVal);
    static void emitReturn(CompilerState &gs, llvm::IRBuilderBase &builder, const IREmitterContext &irctx,
                           int rubyRegionId, llvm::Value *retVal);
    static void emitReturnAcrossBlock(CompilerState &gs, cfg::CFG &cfg, llvm::IRBuilderBase &builder,
                                      const IREmitterContext &irctx, int rubyRegionId, llvm::Value *retVal);
    // Typecheck returnValue as the return value of cfg, if necessary.  Returns the actual
    // value to be returned, which may be different than returnValue e.g. in the case of a
    // void-returning method.
    static llvm::Value *maybeCheckReturnValue(CompilerState &cs, cfg::CFG &cfg, llvm::IRBuilderBase &build,
                                              const IREmitterContext &irctx, llvm::Value *returnValue);

    // Emit a type test.  The insertion point of the builder is set to the start of
    // the block following a successful test.
    static void emitTypeTest(CompilerState &gs, llvm::IRBuilderBase &builder, llvm::Value *value,
                             const core::TypePtr &expectedType, std::string_view description);
    static void emitTypeTestForRestArg(CompilerState &gs, llvm::IRBuilderBase &builder, llvm::Value *value,
                                       const core::TypePtr &expectedType, std::string_view description);

    // Return a value representing the literalish thing, which is either a NamedLiteralType
    // or a type representing nil, false, or true.
    static llvm::Value *emitLiteralish(CompilerState &gs, llvm::IRBuilderBase &builder,
                                       const core::TypePtr &literalish);

    // Return true if the given blockId has a block argument.
    static bool hasBlockArgument(CompilerState &gs, int blockId, core::MethodRef method, const IREmitterContext &irctx);

    // Given an owner as the Sorbet-visible symbol, return the parent symbol
    // as seen by the Ruby VM.
    static core::SymbolRef fixupOwningSymbol(const core::GlobalState &gs, core::SymbolRef sym);

    static std::string showClassNameWithoutOwner(const core::GlobalState &gs, core::SymbolRef sym);

    // Return true if the given symbol is a "root" symbol.
    static bool isRootishSymbol(const core::GlobalState &gs, core::SymbolRef sym);

    struct FinalMethodInfo {
        core::ClassOrModuleRef recv;
        core::MethodRef method;
        core::FileRef file;
    };

    // Return true when the symbol is a final method
    static std::optional<FinalMethodInfo> isFinalMethod(const core::GlobalState &gs, core::TypePtr recvType,
                                                        core::NameRef fun);

    static llvm::Value *receiverFastPathTestWithCache(MethodCallContext &mcctx,
                                                      const std::vector<KnownFunction> &expectedRubyCFuncs,
                                                      const std::string &methodNameForDebug);

    static bool isAliasToSingleton(const core::GlobalState &gs, const IREmitterContext &irctx, cfg::LocalRef var,
                                   core::ClassOrModuleRef klass);

    // Given a method call context representing a send, determine whether a variable
    // representing a block can be fetched out of the Ruby VM state.
    static bool canPassThroughBlockViaRubyVM(MethodCallContext &mcctx, cfg::LocalRef var);
};
} // namespace sorbet::compiler
#endif
