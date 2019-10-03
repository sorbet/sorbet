#include "compiler/llvm_ir_emitter/llvm_ir_emitter.h"
#include "llvm/IR/IRBuilder.h"
#include <string_view>
#include "ast/ast.h"
#include "cfg/CFG.h"

using namespace std;
namespace sorbet::compiler {

void LLVMIREmitter::run(spdlog::logger &logger, llvm::LLVMContext &lctx, cfg::CFG &cfg, std::unique_ptr<ast::MethodDef> &md) {}

} // namespace sorbet::compiler
