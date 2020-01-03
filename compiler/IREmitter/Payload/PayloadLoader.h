#include "compiler/Core/ForwardDeclarations.h"
#include <memory>

namespace sorbet::compiler {
class PayloadLoader {
public:
    static std::unique_ptr<llvm::Module> readDefaultModule(llvm::LLVMContext &);
};
} // namespace sorbet::compiler
