#include "llvm/IR/DerivedTypes.h" // FunctionType, StructType
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
// ^^^ violate poisons
#include "common/formatting.h"
#include "common/sort.h"
#include "compiler/Core/AbortCompilation.h"
#include "compiler/Core/CompilerState.h"
#include "compiler/Errors/Errors.h"
#include "core/core.h"
#include <string_view>

using namespace std;
namespace sorbet::compiler {

CompilerState::CompilerState(const core::GlobalState &gs, llvm::LLVMContext &lctx, llvm::Module *module,
                             llvm::DIBuilder *debug, llvm::DICompileUnit *compileUnit, core::FileRef file,
                             llvm::BasicBlock *allocRubyIdsEntry, llvm::BasicBlock *globalConstructorsEntry,
                             StringTable &stringTable)
    : gs(gs), lctx(lctx), module(module), allocRubyIdsEntry(allocRubyIdsEntry),
      globalConstructorsEntry(globalConstructorsEntry), debug(debug), compileUnit(compileUnit),
      functionEntryInitializers(nullptr), file(file), stringTable(stringTable) {
}

llvm::StructType *CompilerState::getValueType() {
    auto intType = llvm::Type::getInt64Ty(lctx);
    return llvm::StructType::create(lctx, intType, "RV");
};

void CompilerState::trace(string_view msg) const {
    gs.trace(msg);
}

llvm::Value *CompilerState::stringTableRef(std::string_view str) {
    auto it = this->stringTable.map.find(str);

    // We would like to return &sorbet_moduleStringTable[offset], but that would
    // require knowing the length of the string table at this point (we would
    // need to know the length to declare the global variable holding the string
    // table properly).  So instead what we're going to do is return a
    // (as-yet uninitialized) variable that would hold &sorbet_moduleStringTable[offset].
    //
    // Then, when we're finished compiling the module, we can declare the string
    // table with the proper length (i.e. type) and go back and properly initialize
    // all of the temporary variables we created along the way.
    if (it != this->stringTable.map.end()) {
        return it->second.addrVar;
    }

    auto offset = this->stringTable.size;
    auto globalName = fmt::format("addr_str_{}", str);
    const auto isConstant = false;
    auto *type = llvm::Type::getInt8PtrTy(this->lctx);
    llvm::Constant *initializer = llvm::ConstantPointerNull::get(type);
    auto *global = new llvm::GlobalVariable(*this->module, type, isConstant,
                                            llvm::GlobalVariable::InternalLinkage,
                                            initializer, globalName);
    global->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);
    global->setAlignment(llvm::MaybeAlign(8));
    this->stringTable.map[str] = StringTable::StringTableEntry{offset, global};
    this->stringTable.size += str.size();

    return global;
}

void StringTable::defineGlobalVariables(llvm::LLVMContext &lctx, llvm::Module &module) {
    vector<pair<string_view, StringTable::StringTableEntry>> tableElements;
    tableElements.reserve(this->map.size());
    absl::c_copy(this->map, std::back_inserter(tableElements));
    fast_sort(tableElements, [](const auto &l, const auto &r) -> bool { return l.second.offset < r.second.offset; });
    string tableInitializer = fmt::format("{}", fmt::map_join(tableElements, "", [&](const auto &e) -> std::string_view { return e.first; }));
    ENFORCE(tableInitializer.size() == this->size);

    auto *arrayType = llvm::ArrayType::get(llvm::Type::getInt8Ty(lctx), this->size);
    const auto isConstant = true;
    const auto addNull = false;
    auto *initializer = llvm::ConstantDataArray::getString(lctx, tableInitializer, addNull);
    auto *table = new llvm::GlobalVariable(module, arrayType, isConstant,
                                           llvm::GlobalVariable::InternalLinkage,
                                           initializer, "sorbet_moduleStringTable");
    table->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);
    table->setAlignment(llvm::MaybeAlign(1));

    for (auto &elem : tableElements) {
        auto *var = elem.second.addrVar;
        auto *zero = llvm::ConstantInt::get(lctx, llvm::APInt(64, 0));
        auto *index = llvm::ConstantInt::get(lctx, llvm::APInt(64, elem.second.offset));
        llvm::Constant *indices[] = {zero, index};
        auto *initializer = llvm::ConstantExpr::getInBoundsGetElementPtr(table->getValueType(), table, indices);
        var->setInitializer(initializer);
        var->setConstant(true);
    }
}

llvm::FunctionType *CompilerState::getRubyFFIType() {
    llvm::Type *args[] = {
        llvm::Type::getInt32Ty(lctx),    // arg count
        llvm::Type::getInt64PtrTy(lctx), // argArray
        llvm::Type::getInt64Ty(lctx),    // self
        llvm::StructType::getTypeByName(lctx, "struct.rb_control_frame_struct")->getPointerTo(),
        llvm::Type::getInt8PtrTy(lctx), // void* (struct rb_calling_info)
        llvm::Type::getInt8PtrTy(lctx), // void* (struct rb_call_data / struct rb_kwarg_call_data)
    };
    return llvm::FunctionType::get(llvm::Type::getInt64Ty(lctx), args, false /*not varargs*/);
}

llvm::FunctionType *CompilerState::getDirectWrapperFunctionType() {
    llvm::Type *args[] = {
        llvm::StructType::getTypeByName(lctx, "struct.FunctionInlineCache")->getPointerTo(), // cache
        llvm::Type::getInt32Ty(lctx),                                                        // arg count
        llvm::Type::getInt64PtrTy(lctx),                                                     // argArray
        llvm::Type::getInt64Ty(lctx),                                                        // self
    };
    return llvm::FunctionType::get(llvm::Type::getInt64Ty(lctx), args, false /*not varargs*/);
}

llvm::FunctionType *CompilerState::getRubyBlockFFIType() {
    llvm::Type *args[] = {
        llvm::Type::getInt64Ty(lctx),    // first yielded argument(first argument is both here and in argArray
        llvm::Type::getInt64Ty(lctx),    // data
        llvm::Type::getInt32Ty(lctx),    // arg count
        llvm::Type::getInt64PtrTy(lctx), // argArray
        llvm::Type::getInt64Ty(lctx),    // blockArg
    };
    return llvm::FunctionType::get(llvm::Type::getInt64Ty(lctx), args, false /*not varargs*/);
}

llvm::FunctionType *CompilerState::getRubyExceptionFFIType() {
    llvm::Type *args[] = {
        llvm::Type::getInt64PtrTy(lctx)->getPointerTo(),                                         // VALUE **pc
        llvm::Type::getInt64Ty(lctx),                                                            // VALUE captures
        llvm::StructType::getTypeByName(lctx, "struct.rb_control_frame_struct")->getPointerTo(), // cfp
    };
    return llvm::FunctionType::get(llvm::Type::getInt64Ty(lctx), args, false /*not varargs*/);
}

llvm::FunctionType *CompilerState::getInlineForwarderType() {
    llvm::Type *args[] = {
        llvm::Type::getInt64Ty(lctx), // VALUE val
    };
    return llvm::FunctionType::get(llvm::Type::getInt64Ty(lctx), args, false /*not varargs*/);
}

llvm::FunctionType *CompilerState::getAnyRubyCApiFunctionType() {
    auto args = vector<llvm::Type *>{};
    return llvm::FunctionType::get(llvm::Type::getInt64Ty(lctx), args, true /*IS varargs*/);
}

llvm::Function *CompilerState::getFunction(llvm::StringRef name) const {
    llvm::Function *f = module->getFunction(name);
    ENFORCE(f, "could not find {} in the payload", name.str());
    return f;
}

CompilerState CompilerState::withFunctionEntry(llvm::BasicBlock *entry) {
    auto res = CompilerState(*this);
    res.functionEntryInitializers = entry;
    return res;
}

void CompilerState::runCheapOptimizations(llvm::Function *func) {
    llvm::legacy::FunctionPassManager pm(module);
    llvm::PassManagerBuilder pmbuilder;
    int optLevel = 2;
    int sizeLevel = 0;
    pmbuilder.OptLevel = optLevel;
    pmbuilder.SizeLevel = sizeLevel;
    pmbuilder.Inliner = nullptr;
    pmbuilder.DisableUnrollLoops = false;
    pmbuilder.LoopVectorize = true;
    pmbuilder.SLPVectorize = true;
    pmbuilder.VerifyInput = debug_mode;
    pmbuilder.populateFunctionPassManager(pm);
    pm.run(*func);
}

} // namespace sorbet::compiler
