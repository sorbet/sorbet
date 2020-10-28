#ifndef SORBET_COMPILER_IREMITTER_IREMITTERHELPERS_H
#define SORBET_COMPILER_IREMITTER_IREMITTERHELPERS_H
#include "IREmitter.h"
#include "cfg/CFG.h"
#include "compiler/Core/ForwardDeclarations.h"
#include "core/core.h"
#include <string_view>
#include <vector>

namespace sorbet::compiler {

struct IREmitterContext;
struct MethodCallContext;

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
    enum class HandleBlock : u1 {
        Handled = 1,
        Unhandled = 2,
    };
};

class IREmitterHelpers {
public:
    static bool isFileOrClassStaticInit(const core::GlobalState &gs, core::SymbolRef sym);

    // Returns a core::Loc whose start and end positions containt the bounds of the method sym.
    static core::Loc getMethodLineBounds(const core::GlobalState &gs, core::SymbolRef sym, core::Loc declLoc,
                                         core::LocOffsets offsets);

    // Returns a core::Loc whose begin pos contains the start line of the method.
    static core::Loc getMethodStart(const core::GlobalState &gs, core::SymbolRef sym) {
        auto loc = sym.data(gs)->loc();
        return getMethodLineBounds(gs, sym, loc, loc.offsets());
    }

    static llvm::GlobalVariable *getStaticInitLocalsOffset(CompilerState &cs, core::SymbolRef sym);

    static std::string getFunctionName(CompilerState &cs, core::SymbolRef sym);
    static llvm::Function *lookupFunction(CompilerState &cs, core::SymbolRef sym);
    static llvm::Function *getOrCreateFunctionWeak(CompilerState &cs, core::SymbolRef sym);
    static llvm::Function *cleanFunctionBody(CompilerState &cs, llvm::Function *func);
    static llvm::Function *getOrCreateStaticInit(CompilerState &cs, core::SymbolRef sym, core::Loc loc);
    static llvm::Function *getOrCreateFunction(CompilerState &cs, core::SymbolRef sym);

    static llvm::Function *getInitFunction(CompilerState &cs, core::SymbolRef sym);

    static std::size_t sendArgCount(cfg::Send *send);

    static std::pair<llvm::Value *, llvm::Value *> fillSendArgArray(MethodCallContext &mcctx, const std::size_t offset);
    static std::pair<llvm::Value *, llvm::Value *> fillSendArgArray(MethodCallContext &mcctx);

    static llvm::Value *emitMethodCall(MethodCallContext &mcctx);

    static llvm::Value *makeInlineCache(CompilerState &cs, std::string slowFunName);

    static llvm::Value *callViaRubyVMSimple(CompilerState &cs, llvm::IRBuilderBase &build,
                                            const IREmitterContext &irctx, llvm::Value *self, llvm::Value *argv,
                                            llvm::Value *argc, std::string_view name, llvm::Function *blkFun = nullptr,
                                            llvm::Value *localsOffset = nullptr);

    static llvm::Value *emitMethodCallDirrect(MethodCallContext &mcctx, core::SymbolRef funSym);

    static llvm::Value *emitMethodCallViaRubyVM(MethodCallContext &mcctx);

    static IREmitterContext getSorbetBlocks2LLVMBlockMapping(CompilerState &cs, cfg::CFG &cfg, const ast::MethodDef &md,
                                                             llvm::Function *mainFunc);

    static void emitExceptionHandlers(CompilerState &gs, llvm::IRBuilderBase &builder, const IREmitterContext &irctx,
                                      int rubyBlockId, int bodyRubyBlockId, cfg::LocalRef exceptionValue);

    static void emitDebugLoc(CompilerState &gs, llvm::IRBuilderBase &builder, const IREmitterContext &irctx,
                             int rubyBlockId, core::Loc loc);

    static void emitReturn(CompilerState &gs, llvm::IRBuilderBase &builder, const IREmitterContext &irctx,
                           int rubyBlockId, llvm::Value *retVal);
};
} // namespace sorbet::compiler
#endif
