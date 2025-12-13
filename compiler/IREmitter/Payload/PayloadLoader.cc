#include "llvm/Bitcode/BitcodeReader.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/MemoryBuffer.h"
// ^^^ violate poisons
#include "PayloadLoader.h"
#include "compiler/Core/AbortCompilation.h"
#include "compiler/Errors/Errors.h"
#include "core/core.h"
#include <string_view>

using namespace std;
namespace sorbet::compiler {
string_view getDefaultModuleBitcode();

// If possible, plesase write attributes in C code. Unfortunately, not all attributes have C function equivalent
const vector<std::pair<string, llvm::Attribute::AttrKind>> additionalFunctionAttributes = {
    {"ruby_stack_check", llvm::Attribute::InaccessibleMemOnly},
};

std::unique_ptr<llvm::Module> PayloadLoader::readDefaultModule(llvm::LLVMContext &lctx) {
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
