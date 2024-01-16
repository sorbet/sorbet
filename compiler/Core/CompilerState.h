#ifndef SORBET_COMPILER_CORE_COMPILER_STATE_H
#define SORBET_COMPILER_CORE_COMPILER_STATE_H

#include "compiler/Core/ForwardDeclarations.h"
#include "core/Files.h"
#include <memory>
#include <string_view>

namespace sorbet::compiler {

struct StringTable {
    struct Entry {
        uint32_t offset = 0;
        llvm::GlobalVariable *addrVar = nullptr;
    };

    UnorderedMap<std::string, Entry> map;
    uint32_t size = 0;

    void clear() {
        this->map.clear();
        this->size = 0;
    }

    void defineGlobalVariables(llvm::LLVMContext &lctx, llvm::Module &module);
};

struct IDTable {
    struct Entry {
        uint32_t offset;
        uint32_t stringTableOffset = 0;
        uint32_t stringLength = 0;
        llvm::GlobalVariable *addrVar = nullptr;
    };

    UnorderedMap<std::string, Entry> map;

    void clear() {
        this->map.clear();
    }

    void defineGlobalVariables(llvm::LLVMContext &lctx, llvm::Module &module, llvm::IRBuilderBase &builder);
};

struct RubyStringTable {
    struct Entry {
        uint32_t offset;
        uint32_t stringTableOffset = 0;
        uint32_t stringLength = 0;
        llvm::GlobalVariable *addrVar = nullptr;
    };

    UnorderedMap<std::string, Entry> map;

    void clear() {
        this->map.clear();
    }

    void defineGlobalVariables(llvm::LLVMContext &lctx, llvm::Module &module, llvm::IRBuilderBase &builder);
};

// Like GlobalState, but for the Sorbet Compiler.
class CompilerState {
public:
    // Things created and managed outside of us (by either Sorbet or plugin_injector)
    CompilerState(const core::GlobalState &gs, llvm::LLVMContext &lctx, llvm::Module *, llvm::DIBuilder *,
                  llvm::DICompileUnit *, core::FileRef, llvm::BasicBlock *allocRubyIdsEntry,
                  llvm::BasicBlock *globalConstructorsEntry, StringTable &stringTable, IDTable &idTable,
                  RubyStringTable &rubyStringTable);

    const core::GlobalState &gs;
    llvm::LLVMContext &lctx;
    llvm::Module *module;
    llvm::BasicBlock *allocRubyIdsEntry;
    llvm::BasicBlock *globalConstructorsEntry;

    // Debug info
    llvm::DIBuilder *debug;
    llvm::DICompileUnit *compileUnit;

    // Our own state

    // TODO(jez) Only used by IREmitter, thus this probably shouldn't live in CompilerState longer term.
    // (A better option might be to create an IREmitterContext of some sort.)
    llvm::BasicBlock *functionEntryInitializers;

    core::FileRef file;
    StringTable &stringTable;
    IDTable &idTable;
    RubyStringTable &rubyStringTable;

private:
    StringTable::Entry insertIntoStringTable(std::string_view str);

public:
    llvm::Value *stringTableRef(std::string_view str);
    llvm::Value *idTableRef(std::string_view str);
    llvm::Value *rubyStringTableRef(std::string_view str);

    // useful apis for getting common types

    llvm::StructType *getValueType();
    llvm::FunctionType *getRubyFFIType();
    llvm::FunctionType *getRubyBlockFFIType();
    llvm::FunctionType *getRubyExceptionFFIType();
    llvm::FunctionType *getInlineForwarderType();
    llvm::FunctionType *getAnyRubyCApiFunctionType();
    llvm::FunctionType *getDirectWrapperFunctionType();

    // Run some cheap, per-function optimizations immediately after IR emission.
    void runCheapOptimizations(llvm::Function *);

    // Implicit conversion to Sorbet state
    operator const sorbet::core::GlobalState &() const {
        return gs;
    }

    // Implicit conversion to LLVMContext
    operator llvm::LLVMContext &() const {
        return lctx;
    }

    // Forwards to `GlobalState::trace`
    void trace(std::string_view) const;

    // A wrapper around `llvm::Module::getFunction` that displays useful errors when
    // the function is not found
    llvm::Function *getFunction(llvm::StringRef) const;

    CompilerState withFunctionEntry(llvm::BasicBlock *functionEntry);
};

} // namespace sorbet::compiler

#endif
