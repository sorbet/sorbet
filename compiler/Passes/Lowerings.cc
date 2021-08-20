// These violate our poisons so have to happen first
#include "llvm/IR/DerivedTypes.h" // FunctionType
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/MDBuilder.h"
#include "llvm/Pass.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"

#include "Passes.h"
#include "compiler/Core/OptimizerException.h"
#include "core/core.h"
#include <string>

using namespace std;
namespace sorbet::compiler {
namespace {

class IRIntrinsic {
public:
    virtual vector<llvm::StringRef> implementedFunctionCall() const = 0;

    // The contract you're expected to implement here is basically:
    //
    // - You're given an llvm::CallInst (which is a subclass of llvm::Value) that corresponds to the
    //   `call` instruction matching one of the names in implementedFunctionCall()
    // - You can look at that instruction and all the rest of the `module` to do literally anything
    //   LLVM allows you to do.
    // - You return a new `llvm::Value *` (like a cfg::Binding in Sorbet--contains the instruction and
    //   the variable to write that instruction into)
    // - The framework will then update all reads from the variable of the `instr` assigns to to
    //   instead read from the variable of the `llvm::Value *` you just returned.
    // - The frameowrk will delete `instr` from the module. This might unlock further optimization
    //   opportunities in later phases.
    //
    // You cannot (currently at least):
    //
    // - return `nullptr`
    // - return `instr` unchanged
    //
    // We might change this (I don't know if this decision was intentional or accidental).
    virtual llvm::Value *replaceCall(llvm::LLVMContext &lctx, llvm::ModulePass *pass, llvm::Module &module,
                                     llvm::CallInst *instr) const = 0;

    virtual ~IRIntrinsic() = default;
};

// TODO: add more from https://git.corp.stripe.com/stripe-internal/ruby/blob/48bf9833/include/ruby/ruby.h#L1962. Will
// need to modify core sorbet for it.
const vector<pair<llvm::StringRef, llvm::StringRef>> knownSymbolMapping = {
    {"Array", "rb_cArray"},
    {"BasicObject", "rb_cBasicObject"},
    {"Class", "rb_cClass"},
    {"Comparable", "rb_mComparable"},
    {"Enumerable", "rb_mEnumerable"},
    {"FalseClass", "rb_cFalseClass"},
    {"Float", "rb_cFloat"},
    {"Hash", "rb_cHash"},
    {"Integer", "rb_cInteger"},
    {"Kernel", "rb_mKernel"},
    {"Method", "rb_cMethod"},
    {"Module", "rb_cModule"},
    {"NameError", "rb_eNameError"},
    {"NilClass", "rb_cNilClass"},
    {"NoMethodError", "rb_eNoMethodError"},
    {"Numeric", "rb_cNumeric"},
    {"Object", "rb_cObject"},
    {"Proc", "rb_cProc"},
    {"Range", "rb_cRange"},
    {"Rational", "rb_cRational"},
    {"Regexp", "rb_cRegexp"},
    {"StandardError", "rb_eStandardError"},
    {"String", "rb_cString"},
    {"Struct", "rb_cStruct"},
    {"Symbol", "rb_cSymbol"},
    {"Thread", "rb_cThread"},
    {"TrueClass", "rb_cTrueClass"},
};

class ClassAndModuleLoading : public IRIntrinsic {
public:
    virtual vector<llvm::StringRef> implementedFunctionCall() const override {
        return {"sorbet_i_getRubyClass", "sorbet_i_getRubyConstant"};
    }

    virtual llvm::Value *replaceCall(llvm::LLVMContext &lctx, llvm::ModulePass *pass, llvm::Module &module,
                                     llvm::CallInst *instr) const override {
        auto elemPtr = llvm::dyn_cast<llvm::GEPOperator>(instr->getArgOperand(0));
        if (elemPtr == nullptr) {
            throw OptimizerException("Unexpected argument to intrinsic");
        }
        auto global = llvm::dyn_cast<llvm::GlobalVariable>(elemPtr->getOperand(0));

        if (global == nullptr) {
            throw OptimizerException("Unexpected argument to intrinsic");
        }
        auto initializer = llvm::dyn_cast<llvm::ConstantDataArray>(global->getInitializer());
        if (initializer == nullptr) {
            throw OptimizerException("Unexpected argument to intrinsic");
        }

        llvm::IRBuilder<> builder(instr);

        auto symName = initializer->getAsCString();
        auto tp = llvm::Type::getInt64Ty(lctx);

        for (const auto &[rubySourceName, rubyCApiName] : knownSymbolMapping) {
            if (symName == rubySourceName) {
                auto &nm = rubyCApiName; // C++ bindings don't play well with captures
                auto globalDeclaration = module.getOrInsertGlobal(rubyCApiName, tp, [&] {
                    auto isConstant = true;
                    llvm::Constant *initializer = nullptr;
                    auto ret = new llvm::GlobalVariable(module, tp, isConstant, llvm::GlobalVariable::ExternalLinkage,
                                                        initializer, nm);
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

        auto guardEpochDeclaration = module.getOrInsertGlobal(guardEpochName, tp, [&] {
            auto isConstant = false;
            auto ret = new llvm::GlobalVariable(module, tp, isConstant, llvm::GlobalVariable::LinkOnceAnyLinkage,
                                                llvm::ConstantInt::get(tp, 0), guardEpochName);
            return ret;
        });

        auto guardedConstDeclaration = module.getOrInsertGlobal(guardedConstName, tp, [&] {
            auto isConstant = false;
            auto ret = new llvm::GlobalVariable(module, tp, isConstant, llvm::GlobalVariable::LinkOnceAnyLinkage,
                                                llvm::ConstantInt::get(tp, 0), guardedConstName);
            return ret;
        });

        auto needTakeSlowPath =
            builder.CreateICmpNE(builder.CreateLoad(guardEpochDeclaration),
                                 builder.CreateCall(module.getFunction("sorbet_getConstantEpoch")), "needTakeSlowPath");

        auto splitPoint = builder.CreateLoad(guardedConstDeclaration);
        builder.CreateIntrinsic(
            llvm::Intrinsic::IndependentIntrinsics::assume, {},
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
    }
} ClassAndModuleLoading;

class ObjIsKindOf : public IRIntrinsic {
    llvm::Value *fallbackToVM(llvm::Module &module, llvm::CallInst *instr) const {
        llvm::IRBuilder<> builder(instr);
        return builder.CreateCall(module.getFunction("rb_obj_is_kind_of"),
                                  {instr->getArgOperand(0), instr->getArgOperand(1)});
    }

public:
    virtual vector<llvm::StringRef> implementedFunctionCall() const override {
        return {"sorbet_i_objIsKindOf"};
    }

    // Detects code that looks like this:
    //
    //     %44 = load i64, i64* @rb_cNilClass, align 8, !dbg !14
    //     %45 = load i64, i64* @rb_cModule, align 8, !dbg !14
    //     %46 = call i64 @sorbet_i_objIsKindOf(i64 %44, i64 %45) #1, !dbg !14
    //
    // and returns an instruction that looks like this:
    //
    //     %47 = call i64 @sorbet_rubyTrue()
    //
    // Then the LowerIntrinsicsPass harness below will update all reads from %46 to read from %47
    // instead and then delete the write to %46 entirely (which might unlock other optimizations).
    virtual llvm::Value *replaceCall(llvm::LLVMContext &lctx, llvm::ModulePass *pass, llvm::Module &module,
                                     llvm::CallInst *instr) const override {
        auto valueLoad = llvm::dyn_cast<llvm::LoadInst>(instr->getArgOperand(0));
        auto kindLoad = llvm::dyn_cast<llvm::LoadInst>(instr->getArgOperand(1));

        if (valueLoad == nullptr || kindLoad == nullptr) {
            return this->fallbackToVM(module, instr);
        }

        auto value = llvm::dyn_cast<llvm::GlobalVariable>(valueLoad->getPointerOperand());
        auto kind = llvm::dyn_cast<llvm::GlobalVariable>(kindLoad->getPointerOperand());

        if (value == nullptr || kind == nullptr) {
            return this->fallbackToVM(module, instr);
        }

        llvm::IRBuilder<> builder(instr);

        auto kindName = kind->getName();
        if (kindName != "rb_cModule") {
            return this->fallbackToVM(module, instr);
        }

        auto valueName = value->getName();
        for (const auto &[_rubySourceName, rubyCApiName] : knownSymbolMapping) {
            if (valueName == rubyCApiName) {
                // We could use Payload::rubyTrue, but it takes a CompilerState which we don't have.
                auto fn = module.getFunction("sorbet_rubyTrue");
                ENFORCE(fn != nullptr);
                return builder.CreateCall(fn, {}, "trueValueRaw");
            }
        }

        return this->fallbackToVM(module, instr);
    }
} ObjIsKindOf;

class TypeTest : public IRIntrinsic {
    static const vector<pair<llvm::StringRef, llvm::StringRef>> intrinsicMap;

public:
    virtual vector<llvm::StringRef> implementedFunctionCall() const override {
        vector<llvm::StringRef> methods;
        for (auto &[intrinsic, realMethod] : intrinsicMap) {
            methods.emplace_back(intrinsic);
        }
        return methods;
    }

    virtual llvm::Value *replaceCall(llvm::LLVMContext &lctx, llvm::ModulePass *pass, llvm::Module &module,
                                     llvm::CallInst *instr) const override {
        llvm::IRBuilder<> builder(instr);
        auto *arg = instr->getArgOperand(0);
        auto name = instr->getCalledFunction()->getName();

        auto realMethod = absl::c_find_if(intrinsicMap, [&name](const auto &pair) { return pair.first == name; });
        ENFORCE(realMethod != intrinsicMap.end());

        return builder.CreateCall(module.getFunction(realMethod->second), {arg});
    }

} TypeTest;

const vector<pair<llvm::StringRef, llvm::StringRef>> TypeTest::intrinsicMap{
    {"sorbet_i_isa_Array", "sorbet_isa_Array"},         {"sorbet_i_isa_Integer", "sorbet_isa_Integer"},
    {"sorbet_i_isa_TrueClass", "sorbet_isa_TrueClass"}, {"sorbet_i_isa_FalseClass", "sorbet_isa_FalseClass"},
    {"sorbet_i_isa_NilClass", "sorbet_isa_NilClass"},   {"sorbet_i_isa_Symbol", "sorbet_isa_Symbol"},
    {"sorbet_i_isa_Float", "sorbet_isa_Float"},         {"sorbet_i_isa_Untyped", "sorbet_isa_Untyped"},
    {"sorbet_i_isa_Hash", "sorbet_isa_Hash"},           {"sorbet_i_isa_Array", "sorbet_isa_Array"},
    {"sorbet_i_isa_Regexp", "sorbet_isa_Regexp"},       {"sorbet_i_isa_String", "sorbet_isa_String"},
    {"sorbet_i_isa_Proc", "sorbet_isa_Proc"},           {"sorbet_i_isa_RootSingleton", "sorbet_isa_RootSingleton"},
};

class SorbetSend : public IRIntrinsic {
public:
    virtual vector<llvm::StringRef> implementedFunctionCall() const override {
        return {"sorbet_i_send"};
    }

    // Original ruby code:
    //   puts "a"
    //
    // Detects code that looks like this:
    //
    //   %3 = call ... @sorbet_i_send(%struct.FunctionInlineCache* @ic_puts,
    //                                i1 false,
    //                                i64 (i64, i64, i32, i64*, i64)* null,
    //                                %struct.rb_control_frame_struct* %cfp,
    //                                i64 %selfRaw,
    //                                i64 %rubyStr_a
    //                               ), !dbg !121
    //
    // and replaces it with:
    //
    // (disabling clang-format here because it will otherwise remove all the newlines)
    // clang-format off
    //   (1) %18 = getelementptr inbounds %struct.rb_control_frame_struct, %struct.rb_control_frame_struct* %11, i64 0, i32 1, !dbg !8
    //   (2) %19 = load i64*, i64** %18, align 8, !dbg !8
    //   (3) store i64 %8, i64* %19, align 8, !dbg !8, !tbaa !4
    //   (4) %20 = getelementptr inbounds i64, i64* %19, i64 1, !dbg !8
    //   (5) store i64 %rubyStr_a.i, i64* %20, align 8, !dbg !8, !tbaa !4
    //   (6) %21 = getelementptr inbounds i64, i64* %20, i64 1, !dbg !8
    //   (7) store i64* %21, i64** %18, align 8, !dbg !8
    //   (8) %send = call i64 @sorbet_callFuncWithCache(%struct.FunctionInlineCache* @ic_puts, i64 0), !dbg !8
    // clang-format on
    //
    // Lines 1 & 2 correspond to the sorbet_get_sp call, and the sp load from spPtr
    // Lines 3 & 4 correspond to the sorbet_pushValueStack call for self, while 5 & 6 correspond to the
    // sorbet_pushValueStack call for "a".
    // Line 7 corresponds to the store to spPtr.
    // Line 8 corresponds to the sorbet_callFuncWithCache call.
    virtual llvm::Value *replaceCall(llvm::LLVMContext &lctx, llvm::ModulePass *pass, llvm::Module &module,
                                     llvm::CallInst *instr) const override {
        // Make sure cache, blk, closure, cfp and self are passed in.
        ENFORCE(instr->arg_size() >= 5);

        llvm::IRBuilder<> builder(instr);
        auto *cache = instr->getArgOperand(0);
        auto *blk = instr->getArgOperand(2);
        auto *closure = instr->getArgOperand(3);
        auto *cfp = instr->getArgOperand(4);

        auto *spPtr = builder.CreateCall(module.getFunction("sorbet_get_sp"), {cfp});
        auto spPtrType = llvm::dyn_cast<llvm::PointerType>(spPtr->getType());
        llvm::Value *sp = builder.CreateLoad(spPtrType->getElementType(), spPtr);
        for (auto iter = std::next(instr->arg_begin(), 5); iter < instr->arg_end(); ++iter) {
            sp = builder.CreateCall(module.getFunction("sorbet_pushValueStack"), {sp, iter->get()});
        }
        builder.CreateStore(sp, spPtr);

        if (llvm::isa<llvm::ConstantPointerNull>(blk)) {
            auto *blockHandler =
                builder.CreateCall(module.getFunction("sorbet_vmBlockHandlerNone"), {}, "VM_BLOCK_HANDLER_NONE");
            return builder.CreateCall(module.getFunction("sorbet_callFuncWithCache"), {cache, blockHandler}, "send");
        } else {
            auto *blkUsesBreak = llvm::dyn_cast<llvm::ConstantInt>(instr->getArgOperand(1));
            ENFORCE(blkUsesBreak);

            auto *callImpl = blkUsesBreak->equalsInt(1) ? module.getFunction("sorbet_callFuncBlockWithCache")
                                                        : module.getFunction("sorbet_callFuncBlockWithCache_noBreak");

            return builder.CreateCall(callImpl, {cache, blk, closure}, "sendWithBlock");
        }
    }
} SorbetSend;

vector<IRIntrinsic *> getIRIntrinsics() {
    vector<IRIntrinsic *> irIntrinsics{
        &ClassAndModuleLoading,
        &ObjIsKindOf,
        &TypeTest,
        &SorbetSend,
    };

    return irIntrinsics;
}
static const vector<IRIntrinsic *> irIntrinsics = getIRIntrinsics();

class LowerIntrinsicsPass : public llvm::ModulePass {
public:
    // The contents of this variable don't matter; LLVM just uses the pointer address of it as an ID.
    static char ID;
    LowerIntrinsicsPass() : llvm::ModulePass(ID){};

    void getAnalysisUsage(llvm::AnalysisUsage &AU) const override {
        AU.addRequired<llvm::DominatorTreeWrapperPass>();
    }

    struct CallInstVisitor : public llvm::InstVisitor<CallInstVisitor> {
        vector<llvm::CallInst *> result;
        UnorderedSet<llvm::Function *> lookups;

        void visitCallInst(llvm::CallInst &ci) {
            auto maybeFunc = ci.getCalledFunction();
            if (maybeFunc == nullptr) {
                return;
            }

            auto fnd = lookups.find(maybeFunc);
            if (fnd == lookups.end()) {
                return;
            }

            // We're taking the address of a llvm::CallInst & here to avoid having to deal with a
            // vector of references.
            //
            // This lives at least long enough (owned by the llvm::Module) to let the visitor finish
            // and also run the replaceCall callback.
            //
            // An alternative here would be to run the replaceCall callback and do the replacements
            // right here, but it's probably not great to have a visitor that simultaneously mutates
            // the structure it's visiting, which is why we accumulate a to-do list of results.
            result.emplace_back(&ci);
        }
    };

    virtual bool runOnModule(llvm::Module &mod) override {
        vector<llvm::Function *> functionsToRemove;

        for (auto &fn : mod.functions()) {
            if (fn.getName().startswith("sorbet_exists_to_keep_alive_")) {
                functionsToRemove.emplace_back(&fn);
            }
        }

        for (auto *fn : functionsToRemove) {
            fn->eraseFromParent();
        }

        // We run each lowering pass in sequence. Each does a full visit on the module.
        // We don't have that many lowering passes right now, so this cost is small.
        //
        // When we get around to optimizing compile-time performance in the future, this might need to change.
        // At the very least, it should be easy enough to schedule `ClassAndModuleLoading` and `ObjIsKindOf`
        // to run in sequence **at each call site**, but still in one `llvm::ModulePass`.
        for (const auto *intrinsic : irIntrinsics) {
            CallInstVisitor visitor;

            for (const auto &name : intrinsic->implementedFunctionCall()) {
                if (auto fun = mod.getFunction(name)) {
                    visitor.lookups.insert(fun);
                }
            }

            visitor.visit(mod);

            for (const auto &callInst : visitor.result) {
                auto newRes = intrinsic->replaceCall(mod.getContext(), this, mod, callInst);
                callInst->replaceAllUsesWith(newRes);
                callInst->eraseFromParent();
            }
        }

        return true;
    };

    virtual ~LowerIntrinsicsPass() = default;
} LowerIntrinsicsPass_;
char LowerIntrinsicsPass::ID = 0;

static llvm::RegisterPass<LowerIntrinsicsPass> X("lowerSorbetIntrinsics", "Lower Sorbet Intrinsics",
                                                 false, // Only looks at CFG
                                                 false  // Analysis Pass
);

// This pass is kind of a hack.  There are certain calls that we eagerly generate
// because it makes code generation simpler.  But after code generation, those
// calls may not actually be used and we know -- but LLVM doesn't -- that the
// calls are safe to delete.  There's not an LLVM attribute to describe this sort
// of behavior, so we have to write our own pass.
class DeleteUnusedSorbetIntrinsicsPass : public llvm::ModulePass {
public:
    static char ID;

    DeleteUnusedSorbetIntrinsicsPass() : llvm::ModulePass(ID) {}

    struct CallInstVisitor : public llvm::InstVisitor<CallInstVisitor> {
        vector<llvm::CallInst *> callsToDelete;
        UnorderedSet<llvm::Function *> functionsToDelete;

        CallInstVisitor(llvm::Module &m) {
            auto f = m.getFunction("sorbet_getMethodBlockAsProc");
            ENFORCE(f);
            functionsToDelete.insert(f);
        }

        void visitCallInst(llvm::CallInst &ci) {
            auto maybeFunc = ci.getCalledFunction();
            if (!maybeFunc) {
                return;
            }

            if (!functionsToDelete.contains(maybeFunc)) {
                return;
            }

            if (!ci.use_empty()) {
                return;
            }

            callsToDelete.emplace_back(&ci);
        }
    };

    virtual bool runOnModule(llvm::Module &mod) override {
        CallInstVisitor visitor(mod);
        ENFORCE(!visitor.functionsToDelete.empty());

        visitor.visit(mod);

        if (visitor.callsToDelete.empty()) {
            return false;
        }

        for (auto inst : visitor.callsToDelete) {
            inst->eraseFromParent();
        }

        return true;
    }
};
char DeleteUnusedSorbetIntrinsicsPass::ID = 0;

static llvm::RegisterPass<DeleteUnusedSorbetIntrinsicsPass> Y("deleteUnusuedSorbetIntrinsics",
                                                              "Delete Unused Sorbet Intrinsics",
                                                              false, // Only looks at CFG
                                                              false  // Analysis Pass
);

// When through optimization llvm is able to prune out sends that go through the VM, the inline cache that they
// allocated will still be initialized during the module's Init function, and the global will persist. This pass detects
// this case by finding globals whose type is `struct FunctionInlineCache`, and that only have a single user which is a
// call to the `sorbet_setupFunctionInlineCache` function.
class DeleteUnusedInlineCachesPass : public llvm::ModulePass {
public:
    static char ID;

    DeleteUnusedInlineCachesPass() : llvm::ModulePass(ID) {}

    virtual bool runOnModule(llvm::Module &mod) override {
        auto *setupInlineCacheFun = mod.getFunction("sorbet_setupFunctionInlineCache");
        if (setupInlineCacheFun == nullptr) {
            // Unlikely, but this would mean that there were no uses of the function, and that it was removed. If this
            // function is missing, there would be no inline caches allocated in this module.
            return false;
        }

        auto *inlineCacheType = llvm::StructType::getTypeByName(mod.getContext(), "struct.FunctionInlineCache");
        ENFORCE(inlineCacheType != nullptr);

        std::vector<llvm::Instruction *> toRemove;
        for (auto &global : mod.globals()) {
            if (global.getValueType() != inlineCacheType) {
                continue;
            }

            int numUses = std::distance(global.user_begin(), global.user_end());
            if (numUses != 1) {
                continue;
            }

            auto *inst = llvm::dyn_cast<llvm::CallInst>(global.user_back());
            if (inst == nullptr || inst->getCalledFunction() != setupInlineCacheFun) {
                continue;
            }

            toRemove.emplace_back(inst);
        }

        if (toRemove.empty()) {
            return false;
        }

        for (auto *inst : toRemove) {
            inst->eraseFromParent();
        }

        return true;
    }
};

char DeleteUnusedInlineCachesPass::ID = 0;

static llvm::RegisterPass<DeleteUnusedInlineCachesPass> Z("deleteUnusuedInlineCaches", "Delete Unused Inline Caches",
                                                          false, // Only looks at CFG
                                                          false  // Analysis Pass
);

// Detects code that looks like this:
//
// > %allTypeTested.i = call i1 (i64, ...) @sorbet_i_allTypeTested(i64 %rubyStr_hello.i)
//
// and replaces all uses of `%allTypeTested.i` with a constant true/false value that indicates whether or not this use
// of `%rubyStr_hello.i` is dominated by a call to `sorbet_i_typeTested(%rubyStr_hello.i)`.
//
// This pass also removes all calls to `sorbet_i_typeTested`, as they are metadata that is used only by this pass.
class AllTypeTestedPass : public llvm::ModulePass {
public:
    static char ID;

    AllTypeTestedPass() : llvm::ModulePass(ID) {}

    // Register that we need the dominator tree
    void getAnalysisUsage(llvm::AnalysisUsage &AU) const override {
        AU.addRequired<llvm::DominatorTreeWrapperPass>();
    }

    struct CallInstVisitor : public llvm::InstVisitor<CallInstVisitor> {
        llvm::Function *typeTested;
        llvm::Function *allTypeTested;

        vector<llvm::CallInst *> typeTestedCalls;
        vector<llvm::CallInst *> allTypeTestedCalls;

        void visitCallInst(llvm::CallInst &ci) {
            auto maybeFunc = ci.getCalledFunction();
            if (maybeFunc == nullptr) {
                return;
            }

            if (maybeFunc == typeTested) {
                typeTestedCalls.emplace_back(&ci);
                return;
            }

            if (maybeFunc == allTypeTested) {
                allTypeTestedCalls.emplace_back(&ci);
                return;
            }

            return;
        }
    };

    bool runOnModule(llvm::Module &mod) override {
        CallInstVisitor visitor;

        visitor.typeTested = mod.getFunction("sorbet_i_typeTested");
        visitor.allTypeTested = mod.getFunction("sorbet_i_allTypeTested");

        if (visitor.typeTested == nullptr || visitor.allTypeTested == nullptr) {
            return false;
        }

        visitor.visit(mod);

        if (visitor.typeTestedCalls.empty()) {
            return false;
        }

        // Translate all uses of `sorbet_i_allTypeTested`
        for (auto *ci : visitor.allTypeTestedCalls) {
            ci->dump();
            auto &domTree = getAnalysis<llvm::DominatorTreeWrapperPass>(*ci->getParent()->getParent()).getDomTree();

            bool allTypeTested = true;

            for (auto &arg : ci->args()) {
                bool localTypeTested = false;
                for (auto *user : arg->users()) {
                    if (auto *call = llvm::dyn_cast<llvm::CallInst>(user)) {
                        if (call->getCalledFunction() == visitor.typeTested) {
                            if (domTree.dominates(call, ci)) {
                                localTypeTested = true;
                                break;
                            }
                        }
                    }
                }

                if (!localTypeTested) {
                    allTypeTested = false;
                    break;
                }
            }

            // constant-fold away all uses of the original intrinsic
            ci->replaceAllUsesWith(llvm::ConstantInt::get(mod.getContext(), llvm::APInt(1, allTypeTested)));
            ci->eraseFromParent();
        }

        // Remove all uses of `sorbet_i_typeTested`, as the metadata is no longer needed
        for (auto *ci : visitor.typeTestedCalls) {
            ci->eraseFromParent();
        }

        return false;
    }
};

char AllTypeTestedPass::ID = 0;

static llvm::RegisterPass<AllTypeTestedPass> AllTypeTestedPass_("allTypeTested",
                                                                "Propagate type tests through final method calls",
                                                                false, // Only looks at CFG
                                                                false  // Analysis Pass
);

class RemoveUnnecessaryHashDupsPass : public llvm::ModulePass {
public:
    static char ID;

    RemoveUnnecessaryHashDupsPass() : llvm::ModulePass(ID) {}

    bool detectLiteralHash(llvm::Module &mod, llvm::Function *sorbetGlobalConstDupHashFn, llvm::CallInst *toHashCall) {
        // Checks that the argument to rb_to_hash_type is the result of a sorbet_globalConstDupHash,
        // and that it only has one user.
        // This is the case for code written as follows:
        //   args = { a: 1 }
        //   foo(**args)
        ENFORCE(toHashCall->getNumArgOperands() == 1);
        auto *constDupCall = llvm::dyn_cast<llvm::CallInst>(toHashCall->getOperand(0));
        if (constDupCall == nullptr || constDupCall->getCalledFunction() != sorbetGlobalConstDupHashFn ||
            constDupCall->getParent() != toHashCall->getParent() || !constDupCall->hasOneUser()) {
            return false;
        }

        return true;
    }

    bool detectPassThroughCase(llvm::Module &mod, llvm::Function *rbHashDupFn, llvm::Function *rbHashNewFn,
                               llvm::CallInst *toHashCall) {
        // Checks that the argument to rb_to_hash_type is the result of a phi,
        // where one branch is a call to rb_hash_new, and the other is a call to
        // rb_hash_dup on the result of a load from the arg array.
        // This is the case for code written as follows:
        //   def foo(**args)
        //     bar(**args)
        //   end

        auto *phiNode = llvm::dyn_cast<llvm::PHINode>(toHashCall->getOperand(0));
        if (phiNode == nullptr || phiNode->getParent() != toHashCall->getParent() || !phiNode->hasOneUser()) {
            return false;
        }
        for (auto &u : phiNode->incoming_values()) {
            auto *phiArg = llvm::dyn_cast<llvm::CallInst>(u);
            if (phiArg == nullptr) {
                return false;
            }
            // If we see a Hash.new, we can skip processing this branch of the phi, since there's nothing else to follow
            // backwards.
            if (phiArg->getCalledFunction() == rbHashNewFn) {
                continue;
            }
            // If we see a function other than Hash.new or rb_hash_dup, we should abort, and not do anything,
            // because the expected phi node only has calls to those 2.
            if (phiArg->getCalledFunction() != rbHashDupFn) {
                return false;
            }

            ENFORCE(phiArg->getNumArgOperands() == 1);
            auto *loadInst = llvm::dyn_cast<llvm::LoadInst>(phiArg->getOperand(0));
            if (loadInst == nullptr) {
                return false;
            }
            auto *gepInst = llvm::dyn_cast<llvm::GetElementPtrInst>(loadInst->getOperand(0));
            if (gepInst == nullptr) {
                return false;
            }
            // Check that the operand to the GEP is the 2nd arg to the function.
            // gepInst->getParent() returns the basic block the GEP is part of,
            // and ->getParent() of that returns the function.
            if (gepInst->getOperand(0) != gepInst->getParent()->getParent()->getArg(1)) {
                return false;
            }
        }
        return true;
    }

    bool runOnModule(llvm::Module &mod) override {
        bool modifiedCode = false;

        auto *rbHashDupFn = mod.getFunction("rb_hash_dup");
        auto *rbHashNewFn = mod.getFunction("rb_hash_new");
        auto *rbToHashTypeFn = mod.getFunction("rb_to_hash_type");
        auto *sorbetGlobalConstDupHashFn = mod.getFunction("sorbet_globalConstDupHash");
        auto *sorbetSendFn = mod.getFunction("sorbet_i_send");

        if (rbHashDupFn == nullptr || rbToHashTypeFn == nullptr || sorbetSendFn == nullptr) {
            return false;
        }

        for (auto &function : mod) {
            for (auto &block : function) {
                for (auto insn_iter = block.begin(); insn_iter != block.end();) {
                    llvm::Instruction *insn = &*insn_iter;
                    // This increment needs to happen outside of the loop header, because we are
                    // potentially deleting this instruction in this loop body, so incrementing at
                    // the end of the loop body could fail.
                    insn_iter++;

                    // Look for the following instructions:
                    //   %2 = call i64 @rb_to_hash_type(i64 %1)
                    //   %3 = call i64 @rb_hash_dup(i64 %2)
                    // And ensure that both %2 and %3 have exactly one user, and are in the same basic block.
                    auto *hashDupCall = llvm::dyn_cast<llvm::CallInst>(insn);
                    if (hashDupCall == nullptr || hashDupCall->getCalledFunction() != rbHashDupFn ||
                        !hashDupCall->hasOneUser()) {
                        continue;
                    }
                    ENFORCE(hashDupCall->getNumArgOperands() == 1);
                    auto *toHashCall = llvm::dyn_cast<llvm::CallInst>(hashDupCall->getOperand(0));
                    if (toHashCall == nullptr || toHashCall->getCalledFunction() != rbToHashTypeFn ||
                        toHashCall->getParent() != hashDupCall->getParent() || !toHashCall->hasOneUser()) {
                        continue;
                    }

                    // Get the use for the rb_hash_dup value, and ensure that it is a send, in the same basic block.
                    auto use = hashDupCall->use_begin();
                    llvm::User *user = use->getUser();
                    auto *sendCall = llvm::dyn_cast<llvm::CallInst>(user);
                    if (sendCall == nullptr || sendCall->getCalledFunction() != sorbetSendFn ||
                        sendCall->getParent() != hashDupCall->getParent()) {
                        continue;
                    }

                    // Check that the rb_hash_dup is the last arg to send,
                    // The last operand is the definition,
                    // which means that getNumOperands - getOperandNo == 2
                    if (user->getNumOperands() - use->getOperandNo() != 2) {
                        continue;
                    }

                    bool shouldRemoveDup = false;
                    if (sorbetGlobalConstDupHashFn && detectLiteralHash(mod, sorbetGlobalConstDupHashFn, toHashCall)) {
                        shouldRemoveDup = true;
                    }
                    if (rbHashNewFn && detectPassThroughCase(mod, rbHashDupFn, rbHashNewFn, toHashCall)) {
                        shouldRemoveDup = true;
                    }
                    if (!shouldRemoveDup) {
                        continue;
                    }

                    user->setOperand(use->getOperandNo(), toHashCall);
                    hashDupCall->eraseFromParent();
                    modifiedCode = true;
                }
            }
        }
        return modifiedCode;
    }
};

char RemoveUnnecessaryHashDupsPass::ID = 0;

static llvm::RegisterPass<RemoveUnnecessaryHashDupsPass> AA("removeUnnecessaryHashDupsPass",
                                                            "Remove Unnecessary Hash Dups",
                                                            false, // Only looks at CFG
                                                            false  // Analysis Pass
);

} // namespace
const std::vector<llvm::ModulePass *> Passes::standardLowerings() {
    // LLVM pass manager is going to destuct them, so we need to allocate them every time
    return {new LowerIntrinsicsPass()};
};

llvm::ModulePass *Passes::createDeleteUnusedSorbetIntrinsticsPass() {
    return new DeleteUnusedSorbetIntrinsicsPass();
}

llvm::ModulePass *Passes::createDeleteUnusedInlineCachesPass() {
    return new DeleteUnusedInlineCachesPass();
}

llvm::ModulePass *Passes::createRemoveUnnecessaryHashDupsPass() {
    return new RemoveUnnecessaryHashDupsPass();
}

llvm::ModulePass *Passes::createAllTypeTestedPass() {
    return new AllTypeTestedPass();
}

} // namespace sorbet::compiler
