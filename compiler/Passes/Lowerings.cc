// These violate our poisons so have to happen first
#include "llvm/IR/DerivedTypes.h" // FunctionType
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/MDBuilder.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include "Passes.h"
#include "common/typecase.h"
#include "compiler/Core/CompilerState.h"
#include "core/core.h"
#include <string>

using namespace std;
namespace sorbet::compiler {
namespace {

class IRIntrinsic {
public:
    virtual vector<string> implementedFunctionCall() = 0;
    virtual llvm::Value *replaceCall(llvm::LLVMContext &lctx, llvm::Module &module, llvm::CallInst *instr) = 0;
    virtual ~IRIntrinsic() = default;
};

// TODO: add more from https://git.corp.stripe.com/stripe-internal/ruby/blob/48bf9833/include/ruby/ruby.h#L1962. Will
// need to modify core sorbet for it.
const vector<pair<string, string>> knownSymbolMapping = {
    {"Kernel", "rb_mKernel"},
    {"Enumerable", "rb_mEnumerable"},
    {"Comparable", "rb_mComparable"},
    {"BasicObject", "rb_cBasicObject"},
    {"Object", "rb_cObject"},
    {"Array", "rb_cArray"},
    {"Class", "rb_cClass"},
    {"Method", "rb_cMethod"},
    {"Module", "rb_cModule"},
    {"Numeric", "rb_cNumeric"},
    {"NillClass", "rb_cNillClass"},
    {"FalseClass", "rb_cFalseClass"},
    {"TrueClass", "rb_cTrueClass"},
    {"Float", "rb_cFloat"},
    {"Hash", "rb_cHash"},
    {"Integer", "rb_cInteger"},
    {"Module", "rb_cModule"},
    {"NilClass", "rb_cNilClass"},
    {"Proc", "rb_cProc"},
    {"Range", "rb_cRange"},
    {"Rational", "rb_cRational"},
    {"Regexp", "rb_cRegexp"},
    {"String", "rb_cString"},
    {"Struct", "rb_cStruct"},
    {"Symbol", "rb_cSymbol"},
    {"StandardError", "rb_eStandardError"},
    {"NameError", "rb_eNameError"},
    {"NoMethodError", "rb_eNoMethodError"},
};

class ConstantLoading : public IRIntrinsic {
public:
    virtual vector<string> implementedFunctionCall() override {
        return {"sorbet_i_getRubyClass", "sorbet_i_getRubyConstant"};
    }

    virtual llvm::Value *replaceCall(llvm::LLVMContext &lctx, llvm::Module &module, llvm::CallInst *instr) override {
        auto elemPtr = llvm::dyn_cast<llvm::GEPOperator>(instr->getArgOperand(0));
        if (!elemPtr) {
            return llvm::UndefValue::get(instr->getType());
        }
        auto global = llvm::dyn_cast<llvm::GlobalVariable>(elemPtr->getOperand(0));

        if (!global) {
            return llvm::UndefValue::get(instr->getType());
        }
        auto initializer = llvm::dyn_cast<llvm::ConstantDataArray>(global->getInitializer());
        if (!initializer) {
            return llvm::UndefValue::get(instr->getType());
        }
        llvm::IRBuilder<> builder(instr);
        auto symName = initializer->getAsCString();
        for (const auto &[knownSym, name] : knownSymbolMapping) {
            if (symName == knownSym) {
                auto tp = llvm::Type::getInt64Ty(lctx);
                auto &nm = name; // C++ bindings don't play well with captures
                auto globalDeclaration = module.getOrInsertGlobal(name, tp, [&] {
                    auto ret =
                        new llvm::GlobalVariable(module, tp, true, llvm::GlobalVariable::ExternalLinkage, nullptr, nm);
                    return ret;
                });
                return builder.CreateLoad(globalDeclaration);
            }
        }
        auto str = (string)symName;
        ENFORCE(str.length() < 2 || (str[0] != ':'), "implementation assumes that strings dont start with ::");
        auto loaderName = "const_load" + str;
        auto guardEpochName = "guard_epoch_" + str;
        auto guardedConstName = "guarded_const_" + str;

        auto tp = llvm::Type::getInt64Ty(lctx);

        auto guardEpochDeclaration = module.getOrInsertGlobal(guardEpochName, tp, [&] {
            auto ret = new llvm::GlobalVariable(module, tp, false, llvm::GlobalVariable::LinkOnceAnyLinkage,
                                                llvm::ConstantInt::get(tp, 0), guardEpochName);
            return ret;
        });

        auto guardedConstDeclaration = module.getOrInsertGlobal(guardedConstName, tp, [&] {
            auto ret = new llvm::GlobalVariable(module, tp, false, llvm::GlobalVariable::LinkOnceAnyLinkage,
                                                llvm::ConstantInt::get(tp, 0), guardedConstName);
            return ret;
        });

        auto needTakeSlowPath =
            builder.CreateICmpNE(builder.CreateLoad(guardEpochDeclaration),
                                 builder.CreateCall(module.getFunction("sorbet_getConstantEpoch")), "needTakeSlowPath");

        auto splitPoint = builder.CreateLoad(guardedConstDeclaration);
        builder.CreateIntrinsic(
            llvm::Intrinsic::ID::assume, {},
            {builder.CreateICmpEQ(builder.CreateLoad(guardEpochDeclaration),
                                  builder.CreateCall(module.getFunction("sorbet_getConstantEpoch")), "guardUpdated")}

        );

        auto slowPathTerm = llvm::SplitBlockAndInsertIfThen(needTakeSlowPath, splitPoint, false,
                                                            llvm::MDBuilder(lctx).createBranchWeights(1, 10000));
        builder.SetInsertPoint(slowPathTerm);
        auto recomputeFunName = "const_recompute_" + str;
        auto recomputeFun = module.getFunction(recomputeFunName);
        if (!recomputeFun) {
            // generate something logically similar to
            // VALUE const_load_FOO() {
            //    if (const_guard == constant_epoch()) {
            //      return guarded_const;
            //    } else {
            //      guardedConst = sorbet_getConstant("FOO");
            //      const_guard = constant_epoch();
            //      return guarded_const;
            //    }
            //
            //    It's not exactly this as I'm doing some tunnings, more specifically, the "else" branch will be marked
            //    cold and shared across all modules

            auto recomputeFunT = llvm::FunctionType::get(llvm::Type::getVoidTy(lctx), {}, false /*not varargs*/);
            recomputeFun = llvm::Function::Create(recomputeFunT, llvm::Function::LinkOnceAnyLinkage,
                                                  llvm::Twine("const_recompute_") + str, module);
            llvm::IRBuilder<> functionBuilder(lctx);

            functionBuilder.SetInsertPoint(llvm::BasicBlock::Create(lctx, "", recomputeFun));

            auto zero = llvm::ConstantInt::get(lctx, llvm::APInt(64, 0));
            llvm::Constant *indicesString[] = {zero, zero};
            functionBuilder.CreateStore(
                functionBuilder.CreateCall(
                    module.getFunction("sorbet_getConstant"),
                    {llvm::ConstantExpr::getInBoundsGetElementPtr(global->getValueType(), global, indicesString),
                     llvm::ConstantInt::get(lctx, llvm::APInt(64, str.length()))}),
                guardedConstDeclaration);
            functionBuilder.CreateStore(functionBuilder.CreateCall(module.getFunction("sorbet_getConstantEpoch")),
                                        guardEpochDeclaration);
            functionBuilder.CreateRetVoid();
        }
        builder.CreateCall(recomputeFun);
        return splitPoint;
    };
} ClassAndModuleLoading;

vector<pair<string, IRIntrinsic *>> getIRIntrinsics() {
    vector<IRIntrinsic *> irIntrinsics{&ClassAndModuleLoading};

    vector<pair<string, IRIntrinsic *>> res;
    for (auto intrinsic : irIntrinsics) {
        for (auto &call : intrinsic->implementedFunctionCall()) {
            res.emplace_back(call, intrinsic);
        }
    }
    return res;
}
static const vector<pair<string, IRIntrinsic *>> irIntrinsics = getIRIntrinsics();

class LowerIntrinsicsPass : public llvm::ModulePass {
public:
    static char ID;
    LowerIntrinsicsPass() : llvm::ModulePass(ID){};
    struct CallInstVisitor : public llvm::InstVisitor<CallInstVisitor> {
        vector<pair<llvm::CallInst *, IRIntrinsic *>> result;
        UnorderedMap<llvm::Function *, IRIntrinsic *> lookups;
        void visitCallInst(llvm::CallInst &ci) {
            auto maybeFunc = ci.getCalledFunction();
            if (!maybeFunc) {
                return;
            }
            auto fnd = lookups.find(maybeFunc);
            if (fnd == lookups.end()) {
                return;
            }
            result.emplace_back(&ci, fnd->second);
        }
    };
    virtual bool runOnModule(llvm::Module &mod) override {
        CallInstVisitor visitor;
        mod.getFunction("__sorbet_only_exists_to_keep_functions_alive__")->eraseFromParent();
        for (const auto &[name, instr] : irIntrinsics) {
            auto fun = mod.getFunction(name);
            if (fun) {
                visitor.lookups.emplace(fun, instr);
            }
        }
        visitor.visit(mod);
        for (const auto &[instr, intrinsicHandler] : visitor.result) {
            auto newRes = intrinsicHandler->replaceCall(mod.getContext(), mod, instr);
            instr->replaceAllUsesWith(newRes);
            instr->eraseFromParent();
        }
        return true;
    };
    virtual ~LowerIntrinsicsPass() = default;
} LowerInrinsicsPass;
char LowerIntrinsicsPass::ID = 0;

static llvm::RegisterPass<LowerIntrinsicsPass> X("lowerSorbetIntrinsics", "Lower Sorbet Intrinsics",
                                                 false, // Only looks at CFG
                                                 false  // Analysis Pass
);

}; // namespace
const std::vector<llvm::ModulePass *> Passes::standardLowerings() {
    // LLVM pass manager is going to destuct them, so we need to allocate them every time
    return {new LowerIntrinsicsPass()};
};
} // namespace sorbet::compiler
