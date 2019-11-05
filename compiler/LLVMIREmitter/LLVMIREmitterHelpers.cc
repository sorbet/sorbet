#include "compiler/IRHelpers/IRHelpers.h"
#include "llvm/IR/DerivedTypes.h" // FunctionType
#include "llvm/IR/IRBuilder.h"
// ^^^ violate our poisons
#include "LLVMIREmitterHelpers.h"
#include "cfg/CFG.h"

using namespace std;
namespace sorbet::compiler {

vector<llvm::Function *> LLVMIREmitterHelpers::getRubyBlocks2FunctionsMapping(CompilerState &cs, cfg::CFG &cfg,
                                                                              llvm::Function *func) {
    vector<llvm::Function *> res;
    res.emplace_back(func);
    llvm::Type *args[] = {
        llvm::Type::getInt64Ty(cs),    // first yielded argument(first argument is both here and in argArray
        llvm::Type::getInt64Ty(cs),    // data
        llvm::Type::getInt32Ty(cs),    // arg count
        llvm::Type::getInt64PtrTy(cs), // argArray
        llvm::Type::getInt64Ty(cs),    // blockArg
    };
    auto ft = llvm::FunctionType::get(llvm::Type::getInt64Ty(cs), args, false /*not varargs*/);

    for (int i = 1; i <= cfg.maxRubyBlockId; i++) {
        auto fp = llvm::Function::Create(ft, llvm::Function::InternalLinkage,
                                         llvm::Twine{func->getName()} + "$block_" + llvm::Twine(i), *cs.module);
        {
            // setup argument names
            // setup function argument names
            fp->arg_begin()->setName("firstYieldArgRaw");
            (fp->arg_begin() + 1)->setName("captures");
            (fp->arg_begin() + 2)->setName("argc");
            (fp->arg_begin() + 3)->setName("argArray");
            (fp->arg_begin() + 4)->setName("blockArg");
        }
        res.emplace_back(fp);
    }
    return res;
};
} // namespace sorbet::compiler
