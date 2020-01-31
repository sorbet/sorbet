// These violate our poisons so have to happen first
#include "llvm/IR/Attributes.h"
#include "llvm/IR/DerivedTypes.h" // FunctionType, StructType
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Verifier.h"

#include "absl/base/casts.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "cfg/CFG.h"
#include "common/FileOps.h"
#include "common/sort.h"
#include "common/typecase.h"
#include "compiler/Core/CompilerState.h"
#include "compiler/Errors/Errors.h"
#include "compiler/IREmitter/BasicBlockMap.h"
#include "compiler/IREmitter/IREmitter.h"
#include "compiler/IREmitter/IREmitterHelpers.h"
#include "compiler/IREmitter/Payload.h"
#include "compiler/IREmitter/SymbolBasedIntrinsicMethod.h"
#include "compiler/Names/Names.h"
#include <string_view>

using namespace std;
namespace sorbet::compiler {
namespace {
llvm::IRBuilder<> &builderCast(llvm::IRBuilderBase &builder) {
    return static_cast<llvm::IRBuilder<> &>(builder);
};

class CallCMethod : public SymbolBasedIntrinsicMethod {
protected:
    core::SymbolRef rubyClass;
    string_view rubyMethod;
    string cMethod;

public:
    CallCMethod(core::SymbolRef rubyClass, string_view rubyMethod, string cMethod, bool handleBlocks)
        : SymbolBasedIntrinsicMethod(handleBlocks), rubyClass(rubyClass), rubyMethod(rubyMethod), cMethod(cMethod){};

    virtual llvm::Value *makeCall(CompilerState &cs, cfg::Send *i, llvm::IRBuilderBase &build,
                                  const BasicBlockMap &blockMap,
                                  const UnorderedMap<core::LocalVariable, Alias> &aliases, int rubyBlockId,
                                  llvm::Function *blk) const override {
        auto &builder = builderCast(build);

        // fill in args
        {
            int argId = -1;
            for (auto &arg : i->args) {
                argId += 1;
                llvm::Value *indices[] = {llvm::ConstantInt::get(cs, llvm::APInt(32, 0, true)),
                                          llvm::ConstantInt::get(cs, llvm::APInt(64, argId, true))};
                auto var = Payload::varGet(cs, arg.variable, builder, blockMap, aliases, rubyBlockId);
                builder.CreateStore(
                    var, builder.CreateGEP(blockMap.sendArgArrayByBlock[rubyBlockId], indices, "callArgsAddr"));
            }
        }
        llvm::Value *indices[] = {llvm::ConstantInt::get(cs, llvm::APInt(64, 0, true)),
                                  llvm::ConstantInt::get(cs, llvm::APInt(64, 0, true))};

        auto var = Payload::varGet(cs, i->recv.variable, builder, blockMap, aliases, rubyBlockId);
        llvm::Value *blkPtr;
        if (blk != nullptr) {
            blkPtr = blk;
        } else {
            blkPtr = llvm::ConstantPointerNull::get(cs.getRubyBlockFFIType()->getPointerTo());
        }

        return builder.CreateCall(cs.module->getFunction(cMethod),
                                  {var, llvm::ConstantInt::get(cs, llvm::APInt(32, i->args.size(), true)),
                                   builder.CreateGEP(blockMap.sendArgArrayByBlock[rubyBlockId], indices), blkPtr,

                                   blockMap.escapedClosure[rubyBlockId]},
                                  "rawSendResult");
    };

    virtual InlinedVector<core::SymbolRef, 2> applicableClasses(CompilerState &cs) const override {
        return {rubyClass};
    };
    virtual InlinedVector<core::NameRef, 2> applicableMethods(CompilerState &cs) const override {
        return {cs.gs.lookupNameUTF8(rubyMethod)};
    };
};

class Module_tripleEq : public SymbolBasedIntrinsicMethod {
public:
    Module_tripleEq() : SymbolBasedIntrinsicMethod(false) {}
    virtual llvm::Value *makeCall(CompilerState &cs, cfg::Send *send, llvm::IRBuilderBase &build,
                                  const BasicBlockMap &blockMap,
                                  const UnorderedMap<core::LocalVariable, Alias> &aliases, int rubyBlockId,
                                  llvm::Function *blk) const override {
        auto ctx = core::Context(cs, core::Symbols::root());
        auto representedClass = core::Types::getRepresentedClass(ctx, send->recv.type.get());
        if (!representedClass.exists()) {
            return IREmitterHelpers::emitMethodCallViaRubyVM(cs, build, send, blockMap, aliases, rubyBlockId, blk);
        }
        auto recvType = representedClass.data(cs)->externalType(cs);
        auto &arg0 = send->args[0];

        auto &builder = builderCast(build);

        auto recvValue = Payload::varGet(cs, send->recv.variable, builder, blockMap, aliases, rubyBlockId);
        auto representedClassValue = Payload::getRubyConstant(cs, representedClass, builder);
        auto classEq = builder.CreateICmpEQ(recvValue, representedClassValue, "Module_tripleEq_shortCircuit");

        auto fastStart = llvm::BasicBlock::Create(cs, "Module_tripleEq_fast", builder.GetInsertBlock()->getParent());
        auto slowStart = llvm::BasicBlock::Create(cs, "Module_tripleEq_slow", builder.GetInsertBlock()->getParent());
        auto cont = llvm::BasicBlock::Create(cs, "Module_tripleEq_cont", builder.GetInsertBlock()->getParent());

        auto expected = Payload::setExpectedBool(cs, builder, classEq, true);
        builder.CreateCondBr(expected, fastStart, slowStart);

        builder.SetInsertPoint(fastStart);
        auto arg0Value = Payload::varGet(cs, arg0.variable, builder, blockMap, aliases, rubyBlockId);
        auto typeTest = Payload::typeTest(cs, builder, arg0Value, recvType);
        auto fastPath = Payload::boolToRuby(cs, builder, typeTest);
        auto fastEnd = builder.GetInsertBlock();
        builder.CreateBr(cont);

        builder.SetInsertPoint(slowStart);
        auto slowPath = IREmitterHelpers::emitMethodCallViaRubyVM(cs, build, send, blockMap, aliases, rubyBlockId, blk);
        auto slowEnd = builder.GetInsertBlock();
        builder.CreateBr(cont);

        builder.SetInsertPoint(cont);
        auto incomingEdges = 2;
        auto phi = builder.CreatePHI(builder.getInt64Ty(), incomingEdges, "Module_tripleEq_result");
        phi->addIncoming(fastPath, fastEnd);
        phi->addIncoming(slowPath, slowEnd);

        return phi;
    };

    virtual InlinedVector<core::SymbolRef, 2> applicableClasses(CompilerState &cs) const override {
        return {core::Symbols::Module()};
    };
    virtual InlinedVector<core::NameRef, 2> applicableMethods(CompilerState &cs) const override {
        return {core::Names::tripleEq()};
    };
} Module_tripleEq;

class CallCMethodSingleton : public CallCMethod {
public:
    CallCMethodSingleton(core::SymbolRef rubyClass, string_view rubyMethod, string cMethod, bool handleBlocks)
        : CallCMethod(rubyClass, rubyMethod, cMethod, handleBlocks){};

    virtual InlinedVector<core::SymbolRef, 2> applicableClasses(CompilerState &cs) const override {
        return {rubyClass.data(cs)->lookupSingletonClass(cs)};
    };
};

static const vector<CallCMethod> knownCMethodsInstance{
    {core::Symbols::Array(), "[]", "sorbet_rb_array_square_br", false},
    {core::Symbols::Array(), "empty", "sorbet_rb_array_empty", false},
    {core::Symbols::Array(), "[]=", "sorbet_rb_array_square_br_eq", false},
    {core::Symbols::Hash(), "[]", "sorbet_rb_hash_square_br", false},
    {core::Symbols::Hash(), "[]=", "sorbet_rb_hash_square_br_eq", false},
    {core::Symbols::Array(), "size", "sorbet_rb_array_len", false},
    {core::Symbols::TrueClass(), "|", "sorbet_int_bool_true", false},
    {core::Symbols::FalseClass(), "|", "sorbet_int_bool_and", false},
    {core::Symbols::TrueClass(), "&", "sorbet_int_bool_and", false},
    {core::Symbols::FalseClass(), "&", "sorbet_int_bool_false", false},
    {core::Symbols::TrueClass(), "!", "sorbet_int_bool_false", false},
    {core::Symbols::FalseClass(), "!", "sorbet_int_bool_true", false},
    {core::Symbols::TrueClass(), "^", "sorbet_int_bool_nand", false},
    {core::Symbols::FalseClass(), "^", "sorbet_int_bool_and", false},
    {core::Symbols::Integer(), "+", "sorbet_rb_int_plus", false},
    {core::Symbols::Integer(), "-", "sorbet_rb_int_minus", false},
    {core::Symbols::Integer(), "*", "sorbet_rb_int_mul", false},
    {core::Symbols::Integer(), "/", "sorbet_rb_int_div", false},
    {core::Symbols::Integer(), ">", "sorbet_rb_int_gt", false},
    {core::Symbols::Integer(), "<", "sorbet_rb_int_lt", false},
    {core::Symbols::Integer(), ">=", "sorbet_rb_int_ge", false},
    {core::Symbols::Integer(), "<=", "sorbet_rb_int_le", false},
    {core::Symbols::Integer(), "to_s", "sorbet_rb_int_to_s", false},
    {core::Symbols::Integer(), "==", "sorbet_rb_int_equal", false},
    {core::Symbols::Integer(), "!=", "sorbet_rb_int_neq", false},
#include "WrappedIntrinsics.h"
};

static const vector<CallCMethodSingleton> knownCMethodsSingleton{
    {core::Symbols::T(), "unsafe", "sorbet_T_unsafe", false},
    {core::Symbols::T_Hash(), "[]", "sorbet_T_Hash_squarebr", false},
    {core::Symbols::T_Array(), "[]", "sorbet_T_Array_squarebr", false},
};

vector<const SymbolBasedIntrinsicMethod *> getKnownCMethodPtrs() {
    vector<const SymbolBasedIntrinsicMethod *> res{&Module_tripleEq};
    for (auto &method : knownCMethodsInstance) {
        res.emplace_back(&method);
    }
    for (auto &method : knownCMethodsSingleton) {
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
