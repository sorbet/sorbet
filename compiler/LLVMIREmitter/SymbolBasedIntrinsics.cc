// These violate our poisons so have to happen first
#include "llvm/IR/Attributes.h"
#include "llvm/IR/DerivedTypes.h" // FunctionType, StructType
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Verifier.h"
// ^^^ violate our poisons
#include "absl/base/casts.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "cfg/CFG.h"
#include "common/FileOps.h"
#include "common/sort.h"
#include "common/typecase.h"
#include "compiler/Errors/Errors.h"
#include "compiler/IRHelpers/IRHelpers.h"
#include "compiler/LLVMIREmitter/LLVMIREmitter.h"
#include "compiler/LLVMIREmitter/LLVMIREmitterHelpers.h"
#include "compiler/LLVMIREmitter/SymbolBasedIntrinsicMethod.h"
#include "compiler/Names/Names.h"
#include <string_view>
using namespace std;
namespace sorbet::compiler {
namespace {
llvm::IRBuilder<> &builderCast(llvm::IRBuilderBase &builder) {
    return static_cast<llvm::IRBuilder<> &>(builder);
};

class CallCMethod : public SymbolBasedIntrinsicMethod {
    core::SymbolRef rubyClass;
    string_view rubyMethod;
    string cMethod;

public:
    CallCMethod(core::SymbolRef rubyClass, string_view rubyMethod, string cMethod)
        : rubyClass(rubyClass), rubyMethod(rubyMethod), cMethod(cMethod){};

    virtual llvm::Value *makeCall(CompilerState &cs, cfg::Send *i, llvm::IRBuilderBase &build,
                                  const BasicBlockMap &blockMap,
                                  const UnorderedMap<core::LocalVariable, Alias> &aliases,
                                  int currentRubyBlockId) const override {
        auto &builder = builderCast(build);

        // fill in args
        {
            int argId = -1;
            for (auto &arg : i->args) {
                argId += 1;
                llvm::Value *indices[] = {llvm::ConstantInt::get(cs, llvm::APInt(32, 0, true)),
                                          llvm::ConstantInt::get(cs, llvm::APInt(64, argId, true))};
                auto var = MK::varGet(cs, arg.variable, builder, aliases, blockMap, currentRubyBlockId);
                builder.CreateStore(
                    var, builder.CreateGEP(blockMap.sendArgArrayByBlock[currentRubyBlockId], indices, "callArgsAddr"));
            }
        }
        llvm::Value *indices[] = {llvm::ConstantInt::get(cs, llvm::APInt(64, 0, true)),
                                  llvm::ConstantInt::get(cs, llvm::APInt(64, 0, true))};

        auto var = MK::varGet(cs, i->recv.variable, builder, aliases, blockMap, currentRubyBlockId);
        return builder.CreateCall(cs.module->getFunction(cMethod),
                                  {var, llvm::ConstantInt::get(cs, llvm::APInt(32, i->args.size(), true)),
                                   builder.CreateGEP(blockMap.sendArgArrayByBlock[currentRubyBlockId], indices)},
                                  "rawSendResult");
    };

    virtual InlinedVector<core::SymbolRef, 2> applicableClasses(CompilerState &cs) const override {
        return {rubyClass};
    };
    virtual InlinedVector<core::NameRef, 2> applicableMethods(CompilerState &cs) const override {
        return {cs.gs.lookupNameUTF8(rubyMethod)};
    };
};

static const vector<CallCMethod> knownCMethods{
    {core::Symbols::Array(), "size", "sorbet_rb_array_len"}, {core::Symbols::Integer(), "+", "sorbet_rb_int_plus"},
    {core::Symbols::Integer(), "-", "sorbet_rb_int_minus"},  {core::Symbols::Integer(), ">", "sorbet_rb_int_gt"},
    {core::Symbols::Integer(), "<", "sorbet_rb_int_lt"},     {core::Symbols::Integer(), "==", "sorbet_rb_int_equal"},
    {core::Symbols::Integer(), "!=", "sorbet_rb_int_neq"},
};

vector<const SymbolBasedIntrinsicMethod *> getKnownCMethodPtrs() {
    vector<const SymbolBasedIntrinsicMethod *> res;
    for (auto &method : knownCMethods) {
        res.emplace_back(&method);
    }
    return res;
}

// stuff
}; // namespace
vector<const SymbolBasedIntrinsicMethod *> &SymbolBasedIntrinsicMethod::definedIntrinsics() {
    static vector<const SymbolBasedIntrinsicMethod *> ret = getKnownCMethodPtrs();

    return ret;
}
}; // namespace sorbet::compiler
