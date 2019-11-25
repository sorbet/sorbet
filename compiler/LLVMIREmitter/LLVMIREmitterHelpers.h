#ifndef SORBET_COMPILER_LLVM_IR_EMITTER_IMPL_H
#define SORBET_COMPILER_LLVM_IR_EMITTER_IMPL_H
#include "LLVMIREmitter.h"
#include <string_view>
#include <vector>

namespace sorbet::cfg {
class Send;
}; // namespace sorbet::cfg

namespace llvm {
class Function;
class BasicBlock;
class AllocaInst;
class IRBuilderBase;
}; // namespace llvm
namespace sorbet::compiler {
struct BasicBlockMap {
    core::SymbolRef forMethod;
    std::vector<llvm::BasicBlock *> functionInitializersByFunction;
    std::vector<llvm::BasicBlock *> argumentSetupBlocksByFunction;
    std::vector<llvm::BasicBlock *> userEntryBlockByFunction;
    std::vector<llvm::BasicBlock *> llvmBlocksBySorbetBlocks;
    std::vector<int> basicBlockJumpOverrides;
    std::vector<llvm::AllocaInst *> sendArgArrayByBlock;
    std::vector<llvm::Value *> escapedClosure;
    UnorderedMap<core::LocalVariable, int> escapedVariableIndeces;
    llvm::BasicBlock *sigVerificationBlock;
    llvm::BasicBlock *postProcessBlock;
    std::vector<std::shared_ptr<core::SendAndBlockLink>> blockLinks;
    std::vector<std::vector<core::LocalVariable>> rubyBlockArgs;
    std::vector<llvm::Function *> rubyBlocks2Functions;
    UnorderedMap<core::LocalVariable, llvm::AllocaInst *> llvmVariables;
    bool usesBlockArgs;
};

struct Alias {
    enum class AliasKind { Constant, InstanceField, ClassField, GlobalField };
    AliasKind kind;
    core::SymbolRef constantSym;
    core::NameRef instanceField;
    core::NameRef classField;
    core::SymbolRef globalField;
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
    static Alias forGlobalField(core::SymbolRef sym) {
        Alias ret;
        ret.kind = AliasKind::GlobalField;
        ret.globalField = sym;
        return ret;
    }
};

class LLVMIREmitterHelpers {
public:
    static bool isStaticInit(CompilerState &cs, core::SymbolRef sym);
    static std::string getFunctionName(CompilerState &cs, core::SymbolRef sym);
    static llvm::Function *lookupFunction(CompilerState &cs, core::SymbolRef sym);
    static llvm::Function *getOrCreateFunctionWeak(CompilerState &cs, core::SymbolRef sym);
    static llvm::Function *cleanFunctionBody(CompilerState &cs, llvm::Function *func);
    static llvm::Function *getOrCreateFunction(CompilerState &cs, core::SymbolRef sym);

    static llvm::Function *getInitFunction(CompilerState &cs, core::SymbolRef sym);

    static llvm::Value *emitMethodCall(CompilerState &cs, llvm::IRBuilderBase &builder, cfg::Send *send,
                                       const BasicBlockMap &blockMap, UnorderedMap<core::LocalVariable, Alias> &aliases,
                                       int rubyBlockId);

    static llvm::Value *emitMethodCallDirrect(CompilerState &cs, llvm::IRBuilderBase &builder, core::SymbolRef funSym,
                                              cfg::Send *send, const BasicBlockMap &blockMap,
                                              UnorderedMap<core::LocalVariable, Alias> &aliases, int rubyBlockId);

    static llvm::Value *emitMethodCallViaRubyVM(CompilerState &cs, llvm::IRBuilderBase &builder, cfg::Send *send,
                                                const BasicBlockMap &blockMap,
                                                UnorderedMap<core::LocalVariable, Alias> &aliases, int rubyBlockId);

    static BasicBlockMap getSorbetBlocks2LLVMBlockMapping(CompilerState &cs, cfg::CFG &cfg,
                                                          std::unique_ptr<ast::MethodDef> &md,
                                                          UnorderedMap<core::LocalVariable, Alias> &aliases,
                                                          llvm::Function *mainFunc);
};

class MK {
public:
    // api for actual code emission
    static llvm::Value *getRubyIdFor(CompilerState &cs, llvm::IRBuilderBase &builder, std::string_view idName);
    static llvm::Value *setExpectedBool(CompilerState &cs, llvm::IRBuilderBase &builder, llvm::Value *boolean,
                                        bool expected);
    // boxed raw value from rawData into target. Assumes that types are compatible.
    static void boxRawValue(CompilerState &cs, llvm::IRBuilderBase &builder, llvm::AllocaInst *storeTarget,
                            llvm::Value *rawData);
    static llvm::Value *unboxRawValue(CompilerState &cs, llvm::IRBuilderBase &builder, llvm::AllocaInst *storeTarget);

    static llvm::Value *getRubyNilRaw(CompilerState &cs, llvm::IRBuilderBase &builder);
    static llvm::Value *getRubyFalseRaw(CompilerState &cs, llvm::IRBuilderBase &builder);
    static llvm::Value *getRubyTrueRaw(CompilerState &cs, llvm::IRBuilderBase &builder);
    static void emitArgumentMismatch(CompilerState &cs, llvm::IRBuilderBase &builder, llvm::Value *currentArgCount,
                                     int minArgs, int maxArgs);
    static llvm::Value *getRubyIntRaw(CompilerState &cs, llvm::IRBuilderBase &builder, long num);
    static llvm::Value *doubleToRubyValue(CompilerState &cs, llvm::IRBuilderBase &builder, double num);
    static llvm::Value *getRubyStringRaw(CompilerState &cs, llvm::IRBuilderBase &builder, std::string_view str);
    static llvm::Value *getIsTruthyU1(CompilerState &cs, llvm::IRBuilderBase &builder, llvm::Value *val);
    static llvm::Value *getRubyConstant(CompilerState &cs, core::SymbolRef sym, llvm::IRBuilderBase &builder);
    static llvm::Value *toCString(CompilerState &cs, std::string_view str, llvm::IRBuilderBase &builder);
    static llvm::Value *createTypeTestU1(CompilerState &cs, llvm::IRBuilderBase &builder, llvm::Value *val,
                                         const core::TypePtr &type);
    static void pushControlFrame(CompilerState &cs, llvm::IRBuilderBase &builder, core::SymbolRef sym);
    static void popControlFrame(CompilerState &cs, llvm::IRBuilderBase &builder);

    static llvm::Value *varGet(CompilerState &cs, core::LocalVariable local, llvm::IRBuilderBase &builder,
                               const BasicBlockMap &blockMap, const UnorderedMap<core::LocalVariable, Alias> &aliases,
                               int rubyBlockId);
    static void varSet(CompilerState &cs, core::LocalVariable local, llvm::Value *var, llvm::IRBuilderBase &builder,
                       const BasicBlockMap &blockMap, UnorderedMap<core::LocalVariable, Alias> &aliases,
                       int rubyBlockId);
};
} // namespace sorbet::compiler
#endif
