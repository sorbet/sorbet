#include <memory>
#include <string_view>

namespace sorbet::core {
class GlobalState;
};
namespace llvm {
class Module;
class LLVMContext;
class Type;
class StructType;
class FunctionType;
class IRBuilderBase;
class BasicBlock;
class Value;
class AllocaInst;
}; // namespace llvm

namespace sorbet::compiler {
class CompilerState {
public:
    CompilerState(const core::GlobalState &gs, llvm::LLVMContext &lctx, llvm::Module *, llvm::BasicBlock *globalInits);

    const core::GlobalState &gs;
    llvm::LLVMContext &lctx;
    llvm::BasicBlock *globalInitializers;
    llvm::BasicBlock *functionEntryInitializers;
    llvm::Module *module;

    // useful apis for getting common types

    llvm::StructType *getValueType();
    llvm::FunctionType *getRubyFFIType();

    // api for actual code emission
    llvm::Value getRubyIdFor(llvm::IRBuilderBase &builder, std::string_view idName);
    void setExpectedBool(llvm::IRBuilderBase &builder, llvm::Value *boolean, bool expected);
    // boxed raw value from rawData into target. Assumes that types are compatible.
    void boxRawValue(llvm::IRBuilderBase &builder, llvm::AllocaInst *storeTarget, llvm::Value *rawData);
    llvm::Value *unboxRawValue(llvm::IRBuilderBase &builder, llvm::AllocaInst *storeTarget);

    // conversion to Sorbet state
    operator const sorbet::core::GlobalState &() const {
        return gs;
    }

    operator llvm::LLVMContext &() const {
        return lctx;
    }

    // tracing
    void trace(std::string_view) const;
};
class IRHelpers {
public:
    static std::unique_ptr<llvm::Module> readDefaultModule(const char *name, llvm::LLVMContext &);
};
} // namespace sorbet::compiler
