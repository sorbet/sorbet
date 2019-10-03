// Sorbet doesn't like a library that llvm pulls in... so we have to pull it in
// beforehand
#include "llvm/IR/IRBuilder.h"

#include "ast/ast.h"
#include "cfg/CFG.h"
#include "common/typecase.h"
#include "compiler/llvm_ir_emitter/llvm_ir_emitter.h"
#include <string_view>

using namespace std;
namespace sorbet::compiler {

void LLVMIREmitter::run(spdlog::logger &logger, llvm::LLVMContext &lctx, cfg::CFG &cfg,
                        std::unique_ptr<ast::MethodDef> &md) {
    for (auto it = cfg.forwardsTopoSort.rbegin(); it != cfg.forwardsTopoSort.rend(); ++it) {
        cfg::BasicBlock *bb = *it;
        if (bb == cfg.deadBlock()) {
            continue;
        }
        for (cfg::Binding &bind : bb->exprs) {
            typecase(
                bind.value.get(), [&](cfg::Ident *i) {}, [&](cfg::Alias *i) {}, [&](cfg::SolveConstraint *i) {},
                [&](cfg::Send *i) {}, [&](cfg::Return *i) {}, [&](cfg::BlockReturn *i) {}, [&](cfg::LoadSelf *i) {},
                [&](cfg::Literal *i) {}, [&](cfg::Unanalyzable *i) {}, [&](cfg::LoadArg *i) {},
                [&](cfg::LoadYieldParams *i) {}, [&](cfg::Cast *i) {}, [&](cfg::TAbsurd *i) {});
        }
    }
}

} // namespace sorbet::compiler
