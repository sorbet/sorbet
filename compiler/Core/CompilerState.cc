#include "llvm/IR/DerivedTypes.h" // FunctionType, StructType
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
// ^^^ violate poisons
#include "common/sort/sort.h"
#include "common/strings/formatting.h"
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
                             StringTable &stringTable, IDTable &idTable, RubyStringTable &rubyStringTable)
    : gs(gs), lctx(lctx), module(module), allocRubyIdsEntry(allocRubyIdsEntry),
      globalConstructorsEntry(globalConstructorsEntry), debug(debug), compileUnit(compileUnit),
      functionEntryInitializers(nullptr), file(file), stringTable(stringTable), idTable(idTable),
      rubyStringTable(rubyStringTable) {}

llvm::StructType *CompilerState::getValueType() {
    auto intType = llvm::Type::getInt64Ty(lctx);
    return llvm::StructType::create(lctx, intType, "RV");
};

void CompilerState::trace(string_view msg) const {
    gs.trace(msg);
}

namespace {
llvm::GlobalVariable *declareNullptrPlaceholder(llvm::Module &module, llvm::Type *type, const string &name) {
    // This variable is conceptually `const` (even though we're going to change its
    // initializer during compilation, at runtime its value doesn't change), but
    // if we declare that up front, LLVM will fold loads of its value, which
    // causes problems.  So save declaring it as `const` until later.
    const auto isConstant = false;
    llvm::Constant *initializer = llvm::Constant::getNullValue(type);
    auto *global =
        new llvm::GlobalVariable(module, type, isConstant, llvm::GlobalVariable::InternalLinkage, initializer, name);
    global->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);
    global->setAlignment(llvm::MaybeAlign(8));
    return global;
}
} // namespace

StringTable::Entry CompilerState::insertIntoStringTable(std::string_view str) {
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
        return it->second;
    }

    auto offset = this->stringTable.size;
    auto globalName = fmt::format("addr_str_{}", str);
    auto *type = llvm::Type::getInt8PtrTy(this->lctx);
    auto *global = declareNullptrPlaceholder(*this->module, type, globalName);
    auto &entry = this->stringTable.map[str];
    entry = StringTable::Entry{offset, global};
    // +1 for the null terminator.
    this->stringTable.size += str.size() + 1;

    return entry;
}

llvm::Value *CompilerState::stringTableRef(std::string_view str) {
    auto entry = this->insertIntoStringTable(str);
    return entry.addrVar;
}

llvm::Value *CompilerState::idTableRef(std::string_view str) {
    auto it = this->idTable.map.find(str);

    if (it != this->idTable.map.end()) {
        return it->second.addrVar;
    }

    auto entry = this->insertIntoStringTable(str);
    auto offset = static_cast<uint32_t>(this->idTable.map.size());
    auto globalName = fmt::format("addr_id_{}", str);
    // TODO(froydnj): IDs are 32 bits, but Payload.cc was declaring them as 64?
    auto *type = llvm::Type::getInt64PtrTy(this->lctx);
    auto *global = declareNullptrPlaceholder(*this->module, type, globalName);
    auto strSize = static_cast<uint32_t>(str.size());
    this->idTable.map[str] = IDTable::Entry{offset, entry.offset, strSize, global};

    return global;
}

llvm::Value *CompilerState::rubyStringTableRef(std::string_view str) {
    auto it = this->rubyStringTable.map.find(str);

    if (it != this->rubyStringTable.map.end()) {
        return it->second.addrVar;
    }

    auto entry = this->insertIntoStringTable(str);
    auto offset = static_cast<uint32_t>(this->rubyStringTable.map.size());
    auto globalName = fmt::format("addr_rubystr_{}", str);
    auto *type = llvm::Type::getInt64PtrTy(this->lctx);
    auto *global = declareNullptrPlaceholder(*this->module, type, globalName);
    auto strSize = static_cast<uint32_t>(str.size());
    this->rubyStringTable.map[str] = RubyStringTable::Entry{offset, entry.offset, strSize, global};

    return global;
}

void StringTable::defineGlobalVariables(llvm::LLVMContext &lctx, llvm::Module &module) {
    vector<pair<string_view, StringTable::Entry>> tableElements;
    tableElements.reserve(this->map.size());
    absl::c_copy(this->map, std::back_inserter(tableElements));
    fast_sort(tableElements, [](const auto &l, const auto &r) -> bool { return l.second.offset < r.second.offset; });
    string tableInitializer;
    tableInitializer.reserve(this->size);
    for (auto &elem : tableElements) {
        tableInitializer += elem.first;
        tableInitializer += '\0';
    }
    ENFORCE(tableInitializer.size() == this->size);

    auto *arrayType = llvm::ArrayType::get(llvm::Type::getInt8Ty(lctx), this->size);
    const auto isConstant = true;
    const auto addNull = false;
    auto *initializer = llvm::ConstantDataArray::getString(lctx, tableInitializer, addNull);
    auto *table = new llvm::GlobalVariable(module, arrayType, isConstant, llvm::GlobalVariable::InternalLinkage,
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

void IDTable::defineGlobalVariables(llvm::LLVMContext &lctx, llvm::Module &module, llvm::IRBuilderBase &builder) {
    vector<pair<string_view, IDTable::Entry>> tableElements;
    tableElements.reserve(this->map.size());
    absl::c_copy(this->map, std::back_inserter(tableElements));
    fast_sort(tableElements, [](const auto &l, const auto &r) -> bool { return l.second.offset < r.second.offset; });

    llvm::GlobalVariable *table;

    {
        auto *arrayType = llvm::ArrayType::get(llvm::Type::getInt64Ty(lctx), this->map.size());
        const auto isConstant = false;
        auto *initializer = llvm::ConstantAggregateZero::get(arrayType);
        table = new llvm::GlobalVariable(module, arrayType, isConstant, llvm::GlobalVariable::InternalLinkage,
                                         initializer, "sorbet_moduleIDTable");
        table->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);
        table->setAlignment(llvm::MaybeAlign(8));
    }

    // Fill in all the addr vars that we indirected through.
    for (auto &elem : tableElements) {
        auto *var = elem.second.addrVar;
        auto *zero = llvm::ConstantInt::get(lctx, llvm::APInt(64, 0));
        auto *index = llvm::ConstantInt::get(lctx, llvm::APInt(64, elem.second.offset));
        llvm::Constant *indices[] = {zero, index};
        auto *initializer = llvm::ConstantExpr::getInBoundsGetElementPtr(table->getValueType(), table, indices);
        var->setInitializer(initializer);
        var->setConstant(true);
    }

    // Descriptors for the runtime initialization of members of the above.
    // These types are known to the runtime.
    auto *func = module.getFunction("sorbet_vm_intern_ids");
    ENFORCE(func != nullptr);
    auto *descOffsetTy = llvm::Type::getInt32Ty(lctx);
    auto *descLengthTy = descOffsetTy;
    auto *structType = llvm::dyn_cast<llvm::StructType>(
        func->getType()->getElementType()->getFunctionParamType(1)->getPointerElementType());
    ENFORCE(structType != nullptr);
    auto *arrayType = llvm::ArrayType::get(structType, this->map.size());
    const auto isConstant = true;
    std::vector<llvm::Constant *> descsConstants;
    descsConstants.reserve(this->map.size());

    for (auto &elem : tableElements) {
        auto *offset = llvm::ConstantInt::get(descOffsetTy, elem.second.stringTableOffset);
        auto *length = llvm::ConstantInt::get(descLengthTy, elem.second.stringLength);
        auto *desc = llvm::ConstantStruct::get(structType, offset, length);
        descsConstants.push_back(desc);
    }

    auto *initializer = llvm::ConstantArray::get(arrayType, descsConstants);
    auto *descTable = new llvm::GlobalVariable(module, arrayType, isConstant, llvm::GlobalVariable::InternalLinkage,
                                               initializer, "sorbet_moduleIDDescriptors");
    descTable->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);
    descTable->setAlignment(llvm::MaybeAlign(8));

    // Call into the runtime to initialize everything.
    const auto allowInternal = true;
    auto *stringTable = module.getGlobalVariable("sorbet_moduleStringTable", allowInternal);
    ENFORCE(stringTable != nullptr);
    builder.CreateCall(func, {builder.CreateConstGEP2_32(nullptr, table, 0, 0),
                              builder.CreateConstGEP2_32(nullptr, descTable, 0, 0),
                              llvm::ConstantInt::get(llvm::Type::getInt32Ty(lctx), this->map.size()),
                              builder.CreateConstGEP2_32(nullptr, stringTable, 0, 0)});
}

void RubyStringTable::defineGlobalVariables(llvm::LLVMContext &lctx, llvm::Module &module,
                                            llvm::IRBuilderBase &builder) {
    vector<pair<string_view, RubyStringTable::Entry>> tableElements;
    tableElements.reserve(this->map.size());
    absl::c_copy(this->map, std::back_inserter(tableElements));
    fast_sort(tableElements, [](const auto &l, const auto &r) -> bool { return l.second.offset < r.second.offset; });

    llvm::GlobalVariable *table;

    {
        auto *arrayType = llvm::ArrayType::get(llvm::Type::getInt64Ty(lctx), this->map.size());
        const auto isConstant = false;
        auto *initializer = llvm::ConstantAggregateZero::get(arrayType);
        table = new llvm::GlobalVariable(module, arrayType, isConstant, llvm::GlobalVariable::InternalLinkage,
                                         initializer, "sorbet_moduleRubyStringTable");
        table->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);
        table->setAlignment(llvm::MaybeAlign(8));
    }

    // Fill in all the addr vars that we indirected through.
    for (auto &elem : tableElements) {
        auto *var = elem.second.addrVar;
        auto *zero = llvm::ConstantInt::get(lctx, llvm::APInt(64, 0));
        auto *index = llvm::ConstantInt::get(lctx, llvm::APInt(64, elem.second.offset));
        llvm::Constant *indices[] = {zero, index};
        auto *initializer = llvm::ConstantExpr::getInBoundsGetElementPtr(table->getValueType(), table, indices);
        var->setInitializer(initializer);
        var->setConstant(true);
    }

    // Descriptors for the runtime initialization of members of the above.
    // These types are known to the runtime.
    auto *func = module.getFunction("sorbet_vm_init_string_table");
    ENFORCE(func != nullptr);
    auto *descOffsetTy = llvm::Type::getInt32Ty(lctx);
    auto *descLengthTy = descOffsetTy;
    auto *structType = llvm::dyn_cast<llvm::StructType>(
        func->getType()->getElementType()->getFunctionParamType(1)->getPointerElementType());
    ENFORCE(structType != nullptr);
    auto *arrayType = llvm::ArrayType::get(structType, this->map.size());
    const auto isConstant = true;
    std::vector<llvm::Constant *> descsConstants;
    descsConstants.reserve(this->map.size());

    for (auto &elem : tableElements) {
        auto *offset = llvm::ConstantInt::get(descOffsetTy, elem.second.stringTableOffset);
        auto *length = llvm::ConstantInt::get(descLengthTy, elem.second.stringLength);
        auto *desc = llvm::ConstantStruct::get(structType, offset, length);
        descsConstants.push_back(desc);
    }

    auto *initializer = llvm::ConstantArray::get(arrayType, descsConstants);
    auto *descTable = new llvm::GlobalVariable(module, arrayType, isConstant, llvm::GlobalVariable::InternalLinkage,
                                               initializer, "sorbet_moduleRubyStringDescriptors");
    descTable->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);
    descTable->setAlignment(llvm::MaybeAlign(8));

    // Call into the runtime to initialize everything.
    const auto allowInternal = true;
    auto *stringTable = module.getGlobalVariable("sorbet_moduleStringTable", allowInternal);
    ENFORCE(stringTable != nullptr);
    builder.CreateCall(func, {builder.CreateConstGEP2_32(nullptr, table, 0, 0),
                              builder.CreateConstGEP2_32(nullptr, descTable, 0, 0),
                              llvm::ConstantInt::get(llvm::Type::getInt32Ty(lctx), this->map.size()),
                              builder.CreateConstGEP2_32(nullptr, stringTable, 0, 0)});
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
