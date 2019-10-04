#include <memory>
namespace llvm {
class Module;
class LLVMContext;
}; // namespace llvm

namespace sorbet::compiler {
class IRHelpers {
public:
    static std::unique_ptr<llvm::Module> readDefaultModule(const char *name, llvm::LLVMContext &);
};
} // namespace sorbet::compiler
