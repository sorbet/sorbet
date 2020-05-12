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

// TODO(jez) I copy pasted this from NameBasedIntrinsics.cc for sake of prototyping. DONT COMMIT
core::SymbolRef removeRoot(core::SymbolRef sym) {
    if (sym == core::Symbols::root() || sym == core::Symbols::rootSingleton()) {
        // Root methods end up going on object
        sym = core::Symbols::Object();
    }
    return sym;
}

core::SymbolRef typeToSym(const core::GlobalState &gs, core::TypePtr typ) {
    core::SymbolRef sym;
    if (auto classType = core::cast_type<core::ClassType>(typ.get())) {
        sym = classType->symbol;
    } else if (auto appliedType = core::cast_type<core::AppliedType>(typ.get())) {
        sym = appliedType->klass;
    } else {
        ENFORCE(false);
    }
    sym = removeRoot(sym);
    ENFORCE(sym.data(gs)->isClassOrModule());
    return sym;
}

class CallCMethod : public SymbolBasedIntrinsicMethod {
protected:
    core::SymbolRef rubyClass;
    string_view rubyMethod;
    string cMethod;

public:
    CallCMethod(core::SymbolRef rubyClass, string_view rubyMethod, string cMethod, Intrinsics::HandleBlock handleBlocks)
        : SymbolBasedIntrinsicMethod(handleBlocks), rubyClass(rubyClass), rubyMethod(rubyMethod), cMethod(cMethod){};

    virtual llvm::Value *makeCall(CompilerState &cs, cfg::Send *send, llvm::IRBuilderBase &build,
                                  const BasicBlockMap &blockMap,
                                  const UnorderedMap<core::LocalVariable, Alias> &aliases, int rubyBlockId,
                                  llvm::Function *blk) const override {
        auto &builder = builderCast(build);

        // fill in args
        {
            int argId = -1;
            for (auto &arg : send->args) {
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

        auto recv = Payload::varGet(cs, send->recv.variable, builder, blockMap, aliases, rubyBlockId);
        llvm::Value *blkPtr;
        if (blk != nullptr) {
            blkPtr = blk;
        } else {
            blkPtr = llvm::ConstantPointerNull::get(cs.getRubyBlockFFIType()->getPointerTo());
        }

        auto argc = llvm::ConstantInt::get(cs, llvm::APInt(32, send->args.size(), true));
        auto argv = builder.CreateGEP(blockMap.sendArgArrayByBlock[rubyBlockId], indices);
        auto fun = Payload::idIntern(cs, builder, send->fun.data(cs)->shortName(cs));
        return builder.CreateCall(cs.module->getFunction(cMethod),
                                  {recv, fun, argc, argv, blkPtr, blockMap.escapedClosure[rubyBlockId]},
                                  "rawSendResult");
    };

    virtual InlinedVector<core::SymbolRef, 2> applicableClasses(CompilerState &cs) const override {
        return {rubyClass};
    };
    virtual InlinedVector<core::NameRef, 2> applicableMethods(CompilerState &cs) const override {
        return {cs.gs.lookupNameUTF8(rubyMethod)};
    };
};

class DefineMethodIntrinsic : public SymbolBasedIntrinsicMethod {
public:
    DefineMethodIntrinsic() : SymbolBasedIntrinsicMethod(Intrinsics::HandleBlock::Unhandled){};
    virtual llvm::Value *makeCall(CompilerState &cs, cfg::Send *send, llvm::IRBuilderBase &build,
                                  const BasicBlockMap &blockMap,
                                  const UnorderedMap<core::LocalVariable, Alias> &aliases, int rubyBlockId,
                                  llvm::Function *blk) const override {
        auto &builder = builderCast(build);
        bool isSelf = send->fun == core::Names::keepSelfDef();
        ENFORCE(send->args.size() == 2, "Invariant established by rewriter/Flatten.cc");
        auto ownerSym = typeToSym(cs, send->args[0].type);

        auto lit = core::cast_type<core::LiteralType>(send->args[1].type.get());
        ENFORCE(lit->literalKind == core::LiteralType::LiteralTypeKind::Symbol);
        core::NameRef funcNameRef(cs, lit->value);

        auto lookupSym = isSelf ? ownerSym : ownerSym.data(cs)->attachedClass(cs);
        if (ownerSym == core::Symbols::Object() && !isSelf) {
            // TODO Figure out if this speicial case is right
            lookupSym = core::Symbols::Object();
        }
        auto funcSym = lookupSym.data(cs)->findMember(cs, funcNameRef);
        ENFORCE(funcSym.exists());
        ENFORCE(funcSym.data(cs)->isMethod());
        auto funcHandle = IREmitterHelpers::getOrCreateFunction(cs, funcSym);
        auto universalSignature =
            llvm::PointerType::getUnqual(llvm::FunctionType::get(llvm::Type::getInt64Ty(cs), true));
        auto ptr = builder.CreateBitCast(funcHandle, universalSignature);

        auto rubyFunc = cs.module->getFunction(isSelf ? "sorbet_defineMethodSingleton" : "sorbet_defineMethod");
        ENFORCE(rubyFunc);
        builder.CreateCall(rubyFunc, {Payload::getRubyConstant(cs, ownerSym, builder),
                                      Payload::toCString(cs, funcNameRef.show(cs), builder), ptr,
                                      llvm::ConstantInt::get(cs, llvm::APInt(32, -1, true))});

        builder.CreateCall(IREmitterHelpers::getInitFunction(cs, funcSym), {});
        return Payload::varGet(cs, send->args[1].variable, builder, blockMap, aliases, rubyBlockId);
    }

    virtual InlinedVector<core::SymbolRef, 2> applicableClasses(CompilerState &cs) const override {
        return {core::Symbols::Sorbet_Private_Static().data(cs)->lookupSingletonClass(cs)};
    };
    virtual InlinedVector<core::NameRef, 2> applicableMethods(CompilerState &cs) const override {
        return {core::Names::keepDef(), core::Names::keepSelfDef()};
    }
} DefineMethodIntrinsic;

/* Reuse logic from typeTest to speedup SomeClass === someVal */
class Module_tripleEq : public SymbolBasedIntrinsicMethod {
public:
    Module_tripleEq() : SymbolBasedIntrinsicMethod(Intrinsics::HandleBlock::Unhandled) {}
    virtual llvm::Value *makeCall(CompilerState &cs, cfg::Send *send, llvm::IRBuilderBase &build,
                                  const BasicBlockMap &blockMap,
                                  const UnorderedMap<core::LocalVariable, Alias> &aliases, int rubyBlockId,
                                  llvm::Function *blk) const override {
        auto representedClass = core::Types::getRepresentedClass(cs, send->recv.type.get());
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

class Regexp_new : public SymbolBasedIntrinsicMethod {
public:
    Regexp_new() : SymbolBasedIntrinsicMethod(Intrinsics::HandleBlock::Unhandled) {}
    virtual llvm::Value *makeCall(CompilerState &cs, cfg::Send *send, llvm::IRBuilderBase &build,
                                  const BasicBlockMap &blockMap,
                                  const UnorderedMap<core::LocalVariable, Alias> &aliases, int rubyBlockId,
                                  llvm::Function *blk) const override {
        if (send->args.size() < 1 || send->args.size() > 2) {
            // todo: make this work with options.
            return IREmitterHelpers::emitMethodCallViaRubyVM(cs, build, send, blockMap, aliases, rubyBlockId, blk);
        }
        auto options = 0;
        if (send->args.size() == 2) {
            auto &arg1 = send->args[1];
            auto literalOptions = core::cast_type<core::LiteralType>(arg1.type.get());
            if (literalOptions == nullptr ||
                literalOptions->literalKind != core::LiteralType::LiteralTypeKind::Integer) {
                return IREmitterHelpers::emitMethodCallViaRubyVM(cs, build, send, blockMap, aliases, rubyBlockId, blk);
            }
            options = literalOptions->value;
        }

        auto &arg0 = send->args[0];
        auto literal = core::cast_type<core::LiteralType>(arg0.type.get());
        if (literal == nullptr || literal->literalKind != core::LiteralType::LiteralTypeKind::String) {
            return IREmitterHelpers::emitMethodCallViaRubyVM(cs, build, send, blockMap, aliases, rubyBlockId, blk);
        }
        auto &builder = builderCast(build);
        auto str = core::NameRef(cs, literal->value).data(cs)->shortName(cs);
        return Payload::cPtrToRubyRegexp(cs, builder, str, options);
    };

    virtual InlinedVector<core::SymbolRef, 2> applicableClasses(CompilerState &cs) const override {
        return {core::Symbols::Regexp().data(cs)->lookupSingletonClass(cs)};
    };
    virtual InlinedVector<core::NameRef, 2> applicableMethods(CompilerState &cs) const override {
        return {core::Names::new_()};
    };
} Regexp_new;

class CallCMethodSingleton : public CallCMethod {
public:
    CallCMethodSingleton(core::SymbolRef rubyClass, string_view rubyMethod, string cMethod,
                         Intrinsics::HandleBlock handleBlocks)
        : CallCMethod(rubyClass, rubyMethod, cMethod, handleBlocks){};

    virtual InlinedVector<core::SymbolRef, 2> applicableClasses(CompilerState &cs) const override {
        return {rubyClass.data(cs)->lookupSingletonClass(cs)};
    };
};

static const vector<CallCMethod> knownCMethodsInstance{
    {core::Symbols::Array(), "[]", "sorbet_rb_array_square_br", Intrinsics::HandleBlock::Unhandled},
    {core::Symbols::Array(), "empty?", "sorbet_rb_array_empty", Intrinsics::HandleBlock::Unhandled},
    {core::Symbols::Array(), "each", "sorbet_rb_array_each", Intrinsics::HandleBlock::Handled},
    {core::Symbols::Hash(), "[]", "sorbet_rb_hash_square_br", Intrinsics::HandleBlock::Unhandled},
    {core::Symbols::Hash(), "[]=", "sorbet_rb_hash_square_br_eq", Intrinsics::HandleBlock::Unhandled},
    {core::Symbols::Array(), "size", "sorbet_rb_array_len", Intrinsics::HandleBlock::Unhandled},
    {core::Symbols::TrueClass(), "|", "sorbet_int_bool_true", Intrinsics::HandleBlock::Unhandled},
    {core::Symbols::FalseClass(), "|", "sorbet_int_bool_and", Intrinsics::HandleBlock::Unhandled},
    {core::Symbols::TrueClass(), "&", "sorbet_int_bool_and", Intrinsics::HandleBlock::Unhandled},
    {core::Symbols::FalseClass(), "&", "sorbet_int_bool_false", Intrinsics::HandleBlock::Unhandled},
    {core::Symbols::TrueClass(), "!", "sorbet_int_bool_false", Intrinsics::HandleBlock::Unhandled},
    {core::Symbols::FalseClass(), "!", "sorbet_int_bool_true", Intrinsics::HandleBlock::Unhandled},
    {core::Symbols::TrueClass(), "^", "sorbet_int_bool_nand", Intrinsics::HandleBlock::Unhandled},
    {core::Symbols::FalseClass(), "^", "sorbet_int_bool_and", Intrinsics::HandleBlock::Unhandled},
    {core::Symbols::Integer(), "+", "sorbet_rb_int_plus", Intrinsics::HandleBlock::Unhandled},
    {core::Symbols::Integer(), "-", "sorbet_rb_int_minus", Intrinsics::HandleBlock::Unhandled},
    {core::Symbols::Integer(), "*", "sorbet_rb_int_mul", Intrinsics::HandleBlock::Unhandled},
    {core::Symbols::Integer(), "/", "sorbet_rb_int_div", Intrinsics::HandleBlock::Unhandled},
    {core::Symbols::Integer(), ">", "sorbet_rb_int_gt", Intrinsics::HandleBlock::Unhandled},
    {core::Symbols::Integer(), "<", "sorbet_rb_int_lt", Intrinsics::HandleBlock::Unhandled},
    {core::Symbols::Integer(), ">=", "sorbet_rb_int_ge", Intrinsics::HandleBlock::Unhandled},
    {core::Symbols::Integer(), "<=", "sorbet_rb_int_le", Intrinsics::HandleBlock::Unhandled},
    {core::Symbols::Integer(), "to_s", "sorbet_rb_int_to_s", Intrinsics::HandleBlock::Unhandled},
    {core::Symbols::Integer(), "==", "sorbet_rb_int_equal", Intrinsics::HandleBlock::Unhandled},
    {core::Symbols::Integer(), "!=", "sorbet_rb_int_neq", Intrinsics::HandleBlock::Unhandled},
#include "WrappedIntrinsics.h"
};

static const vector<CallCMethodSingleton> knownCMethodsSingleton{
    {core::Symbols::T(), "unsafe", "sorbet_T_unsafe", Intrinsics::HandleBlock::Unhandled},
};

vector<const SymbolBasedIntrinsicMethod *> getKnownCMethodPtrs() {
    vector<const SymbolBasedIntrinsicMethod *> res{&DefineMethodIntrinsic, &Module_tripleEq, &Regexp_new};
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
