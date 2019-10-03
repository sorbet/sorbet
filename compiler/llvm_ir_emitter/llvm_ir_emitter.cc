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

void LLVMIREmitter::run(const core::GlobalState &gs, llvm::LLVMContext &lctx, cfg::CFG &cfg,
                        std::unique_ptr<ast::MethodDef> &md, const string &functionName, llvm::Module *module) {
    for (auto it = cfg.forwardsTopoSort.rbegin(); it != cfg.forwardsTopoSort.rend(); ++it) {
        cfg::BasicBlock *bb = *it;
        if (bb == cfg.deadBlock()) {
            continue;
        }
        for (cfg::Binding &bind : bb->exprs) {
            typecase(
                bind.value.get(), [&](cfg::Ident *i) { gs.trace("Ident\n"); },
                [&](cfg::Alias *i) { gs.trace("Alias\n"); },
                [&](cfg::SolveConstraint *i) { gs.trace("SolveConstraint\n"); },
                [&](cfg::Send *i) { gs.trace("Send\n"); }, [&](cfg::Return *i) { gs.trace("Return\n"); },
                [&](cfg::BlockReturn *i) { gs.trace("BlockReturn\n"); },
                [&](cfg::LoadSelf *i) { gs.trace("LoadSelf\n"); }, [&](cfg::Literal *i) { gs.trace("Literal\n"); },
                [&](cfg::Unanalyzable *i) { gs.trace("Unanalyzable\n"); },
                [&](cfg::LoadArg *i) { gs.trace("LoadArg\n"); },
                [&](cfg::LoadYieldParams *i) { gs.trace("LoadYieldParams\n"); },
                [&](cfg::Cast *i) { gs.trace("Cast\n"); }, [&](cfg::TAbsurd *i) { gs.trace("TAbsurd\n"); });
        }
    }
}

} // namespace sorbet::compiler
