#include "common/Exception.h"
#include "compiler/Core/ForwardDeclarations.h"

namespace sorbet::compiler {
class PayloadLoader {
public:
    static std::unique_ptr<llvm::Module> readDefaultModule(const char *name, llvm::LLVMContext &);
};

class AbortCompilation : public sorbet::SorbetException {
public:
    AbortCompilation(const std::string &message) : SorbetException(message){};
    AbortCompilation(const char *message) : SorbetException(message){};
};
} // namespace sorbet::compiler
