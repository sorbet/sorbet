#include "Payload.h"
#include "llvm/Bitcode/BitcodeReader.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include <string_view>
// ^^^ violate poisons
#include "compiler/Errors/Errors.h"
#include "compiler/Payload/AbortCompilation.h"
#include "core/core.h"

using namespace std;

namespace sorbet::compiler {
string_view getDefaultModuleBitcode();

// If possible, plesase write attributes in C code. Unfortunately, not all attributes have C function equivalent
const vector<std::pair<string, llvm::Attribute::AttrKind>> additionalFunctionAttributes = {
    {"ruby_stack_check", llvm::Attribute::InaccessibleMemOnly},
};

std::unique_ptr<llvm::Module> PayloadLoader::readDefaultModule(const char *name, llvm::LLVMContext &lctx) {
    auto bitCode = getDefaultModuleBitcode();
    llvm::StringRef bitcodeAsStringRef(bitCode.data(), bitCode.size());
    auto memoryBuffer = llvm::MemoryBuffer::getMemBuffer(bitcodeAsStringRef, "payload", false);
    auto ret = llvm::parseBitcodeFile(*memoryBuffer, lctx);
    for (const auto &[funName, attr] : additionalFunctionAttributes) {
        ret.get()->getFunction(funName)->addFnAttr(attr);
    }
    return move(ret.get());
}

} // namespace sorbet::compiler
