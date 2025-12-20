#include "compiler/Core/ForwardDeclarations.h"
#include <memory>

namespace sorbet::compiler {
class PayloadLoader {
public:
    // When accumulating all the functions from file into a new module, we initialize the module with the payload.
    // (Unfortunately, this means that the payload currently gets copied into every generated C extension.)
    //
    // Among other things, this means that we can do `cs.module->getFunction("...")` and look up the LLVM data
    // structure corresponding to any C function written in the payload.
    static std::unique_ptr<llvm::Module> readDefaultModule(llvm::LLVMContext &);
};
} // namespace sorbet::compiler
