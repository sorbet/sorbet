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
                bind.value.get(), [&](cfg::Ident *i) { printf("Ident\n"); }, [&](cfg::Alias *i) { printf("Alias\n"); },
                [&](cfg::SolveConstraint *i) { printf("SolveConstraint\n"); }, [&](cfg::Send *i) { printf("Send\n"); },
                [&](cfg::Return *i) { printf("Return\n"); }, [&](cfg::BlockReturn *i) { printf("BlockReturn\n"); },
                [&](cfg::LoadSelf *i) { printf("LoadSelf\n"); }, [&](cfg::Literal *i) { printf("Literal\n"); },
                [&](cfg::Unanalyzable *i) { printf("Unanalyzable\n"); }, [&](cfg::LoadArg *i) { printf("LoadArg\n"); },
                [&](cfg::LoadYieldParams *i) { printf("LoadYieldParams\n"); }, [&](cfg::Cast *i) { printf("Cast\n"); },
                [&](cfg::TAbsurd *i) { printf("TAbsurd\n"); });
        }
    }
}

} // namespace sorbet::compiler
