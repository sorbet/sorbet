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

std::unique_ptr<llvm::Module> PayloadLoader::readDefaultModule(llvm::LLVMContext &lctx) {
    auto bitCode = getDefaultModuleBitcode();
    llvm::StringRef bitcodeAsStringRef(bitCode.data(), bitCode.size());
    auto memoryBuffer = llvm::MemoryBuffer::getMemBuffer(bitcodeAsStringRef, "payload", false);
    auto ret = llvm::parseBitcodeFile(*memoryBuffer, lctx);

    if (!ret) {
        // Log the error and abort
        auto err = ret.takeError();
        llvm::errs() << "Failed to parse payload bitcode: " << err << "\n";
        llvm::consumeError(std::move(err));
        return nullptr;
    }

    return std::move(*ret);
}

} // namespace sorbet::compiler
