#include "compiler/Core/ForwardDeclarations.h"
#include <memory>

namespace sorbet::compiler {
class PayloadLoader {
public:
    static std::unique_ptr<llvm::Module> readDefaultModule(const char *name, llvm::LLVMContext &);
};
} // namespace sorbet::compiler
