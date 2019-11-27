#include "common/Exception.h"
#include "compiler/Core/ForwardDeclarations.h"

namespace sorbet::compiler {
class PayloadLoader {
public:
    static std::unique_ptr<llvm::Module> readDefaultModule(const char *name, llvm::LLVMContext &);
};
} // namespace sorbet::compiler
