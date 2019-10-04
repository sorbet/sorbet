#include "IRHelpers.h"
#include "llvm/Bitcode/BitcodeReader.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/MemoryBuffer.h"
#include <string_view>
using namespace std;
namespace sorbet::compiler {
string_view getDefaultModuleBitcode();
std::unique_ptr<llvm::Module> IRHelpers::readDefaultModule(const char *name, llvm::LLVMContext &lctx) {
    auto bitCode = getDefaultModuleBitcode();
    llvm::StringRef bitcodeAsStringRef(bitCode.data(), bitCode.size());
    auto memoryBuffer = llvm::MemoryBuffer::getMemBuffer(bitcodeAsStringRef);
    auto ret = llvm::parseBitcodeFile(*memoryBuffer, lctx);
    return move(ret.get());
}
} // namespace sorbet::compiler
