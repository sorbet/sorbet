#ifndef SORBET_COMPILER_LLVM_IR_EMITTER_IMPL_H
#define SORBET_COMPILER_LLVM_IR_EMITTER_IMPL_H
#include "LLVMIREmitter.h"
#include <vector>

namespace llvm {
class Function;
class BasicBlock;
class AllocaInst;
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
    std::vector<std::shared_ptr<core::SendAndBlockLink>> blockLinks;
    std::vector<std::vector<core::LocalVariable>> rubyBlockArgs;
    std::vector<llvm::Function *> rubyBlocks2Functions;
    UnorderedMap<core::LocalVariable, llvm::AllocaInst *> llvmVariables;
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
    static BasicBlockMap getSorbetBlocks2LLVMBlockMapping(CompilerState &cs, cfg::CFG &cfg,
                                                          std::unique_ptr<ast::MethodDef> &md,
                                                          UnorderedMap<core::LocalVariable, Alias> &aliases,
                                                          llvm::Function *mainFunc);
};
} // namespace sorbet::compiler
#endif
