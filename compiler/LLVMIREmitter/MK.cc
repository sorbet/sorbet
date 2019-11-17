#include "llvm/IR/DerivedTypes.h" // FunctionType
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/ModuleUtils.h" // appendToGlobalCtors
// ^^^ violate our poisons

// needef for LLVMIREmitterHelpers
#include "common/typecase.h"
#include "core/core.h"

#include "LLVMIREmitterHelpers.h"
#include "compiler/IRHelpers/IRHelpers.h"
#include <string>

using namespace std;

namespace sorbet::compiler {
namespace {
llvm::IRBuilder<> &builderCast(llvm::IRBuilderBase &builder) {
    return static_cast<llvm::IRBuilder<> &>(builder);
};
} // namespace

llvm::Value *MK::setExpectedBool(CompilerState &cs, llvm::IRBuilderBase &builder, llvm::Value *value, bool expected) {
    return builderCast(builder).CreateIntrinsic(llvm::Intrinsic::ID::expect, {llvm::Type::getInt1Ty(cs)},
                                                {value, builder.getInt1(expected)});
}

void MK::boxRawValue(CompilerState &cs, llvm::IRBuilderBase &builder, llvm::AllocaInst *target, llvm::Value *rawData) {
    builderCast(builder).CreateStore(rawData, builderCast(builder).CreateStructGEP(target, 0));
}

llvm::Value *MK::unboxRawValue(CompilerState &cs, llvm::IRBuilderBase &builder, llvm::AllocaInst *target) {
    return builderCast(builder).CreateLoad(builderCast(builder).CreateStructGEP(target, 0), "rawRubyValue");
}

llvm::Value *MK::getRubyNilRaw(CompilerState &cs, llvm::IRBuilderBase &builder) {
    return builderCast(builder).CreateCall(cs.module->getFunction("sorbet_rubyNil"), {}, "nilValueRaw");
}

llvm::Value *MK::getRubyFalseRaw(CompilerState &cs, llvm::IRBuilderBase &builder) {
    return builderCast(builder).CreateCall(cs.module->getFunction("sorbet_rubyFalse"), {}, "falseValueRaw");
}

llvm::Value *MK::getRubyTrueRaw(CompilerState &cs, llvm::IRBuilderBase &builder) {
    return builderCast(builder).CreateCall(cs.module->getFunction("sorbet_rubyTrue"), {}, "trueValueRaw");
}

void MK::emitArgumentMismatch(CompilerState &cs, llvm::IRBuilderBase &builder, llvm::Value *currentArgCount,
                              int minArgs, int maxArgs) {
    builderCast(builder).CreateCall(cs.module->getFunction("sorbet_rb_error_arity"),
                                    {currentArgCount, llvm::ConstantInt::get(cs, llvm::APInt(32, minArgs, true)),
                                     llvm::ConstantInt::get(cs, llvm::APInt(32, maxArgs, true))

                                    });
    builderCast(builder).CreateUnreachable();
}
llvm::Value *MK::getRubyIntRaw(CompilerState &cs, llvm::IRBuilderBase &builder, long num) {
    return builderCast(builder).CreateCall(cs.module->getFunction("sorbet_longToRubyValue"),
                                           {llvm::ConstantInt::get(cs, llvm::APInt(64, num, true))}, "rawRubyInt");
}

llvm::Value *MK::getRubyFloatRaw(CompilerState &cs, llvm::IRBuilderBase &builder, double num) {
    return builderCast(builder).CreateCall(cs.module->getFunction("sorbet_doubleToRubyValue"),
                                           {llvm::ConstantFP::get(llvm::Type::getDoubleTy(cs), num)}, "rawRubyInt");
}

llvm::Value *MK::getRubyStringRaw(CompilerState &cs, llvm::IRBuilderBase &builder, std::string_view str) {
    llvm::StringRef userStr(str.data(), str.length());
    auto rawCString = MK::toCString(cs, str, builder);
    return builderCast(builder).CreateCall(
        cs.module->getFunction("sorbet_CPtrToRubyString"),
        {rawCString, llvm::ConstantInt::get(cs, llvm::APInt(64, str.length(), true))}, "rawRubyStr");
}

llvm::Value *MK::getIsTruthyU1(CompilerState &cs, llvm::IRBuilderBase &builder, llvm::Value *val) {
    return builderCast(builder).CreateCall(cs.module->getFunction("sorbet_testIsTruthy"), {val}, "cond");
}

llvm::Value *MK::getRubyIdFor(CompilerState &cs, llvm::IRBuilderBase &builder, std::string_view idName) {
    auto zero = llvm::ConstantInt::get(cs, llvm::APInt(64, 0));
    auto name = llvm::StringRef(idName.data(), idName.length());
    llvm::Constant *indices[] = {zero};
    string rawName = "rubyIdPrecomputed_" + (string)idName;
    auto tp = llvm::Type::getInt64Ty(cs);
    llvm::IRBuilder<> globalInitBuilder(cs);
    auto globalDeclaration = static_cast<llvm::GlobalVariable *>(cs.module->getOrInsertGlobal(rawName, tp, [&] {
        auto ret =
            new llvm::GlobalVariable(*cs.module, tp, false, llvm::GlobalVariable::InternalLinkage, zero, rawName);
        ret->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);
        ret->setAlignment(llvm::MaybeAlign(8));
        // create constructor
        std::vector<llvm::Type *> NoArgs(0, llvm::Type::getVoidTy(cs));
        auto ft = llvm::FunctionType::get(llvm::Type::getVoidTy(cs), NoArgs, false);
        auto constr = llvm::Function::Create(ft, llvm::Function::InternalLinkage, {"Constr_", rawName}, *cs.module);

        auto bb = llvm::BasicBlock::Create(cs, "constr", constr);
        globalInitBuilder.SetInsertPoint(bb);
        auto rawCString = MK::toCString(cs, idName, builder);
        auto rawID = globalInitBuilder.CreateCall(
            cs.module->getFunction("sorbet_IDIntern"),
            {rawCString, llvm::ConstantInt::get(cs, llvm::APInt(64, idName.length()))}, "rawId");
        globalInitBuilder.CreateStore(rawID,
                                      llvm::ConstantExpr::getInBoundsGetElementPtr(ret->getValueType(), ret, indices));
        globalInitBuilder.CreateRetVoid();
        llvm::appendToGlobalCtors(*cs.module, constr, 0, ret);

        return ret;
    }));

    globalInitBuilder.SetInsertPoint(cs.functionEntryInitializers);
    auto global = globalInitBuilder.CreateLoad(
        llvm::ConstantExpr::getInBoundsGetElementPtr(globalDeclaration->getValueType(), globalDeclaration, indices),
        {"rubyId_", name});

    // todo(perf): mark these as immutable with https://llvm.org/docs/LangRef.html#llvm-invariant-start-intrinsic
    return global;
}

namespace {
core::SymbolRef removeRoot(core::SymbolRef sym) {
    if (sym == core::Symbols::root() || sym == core::Symbols::rootSingleton()) {
        // Root methods end up going on object
        sym = core::Symbols::Object();
    }
    return sym;
}

std::string showClassNameWithoutOwner(const core::GlobalState &gs, core::SymbolRef sym) {
    auto name = sym.data(gs)->name;
    if (name.data(gs)->kind == core::NameKind::UNIQUE) {
        return name.data(gs)->unique.original.data(gs)->show(gs);
    }
    return name.data(gs)->show(gs);
}

std::string showClassName(const core::GlobalState &gs, core::SymbolRef sym) {
    bool includeOwner = sym.data(gs)->owner.exists() && sym.data(gs)->owner != core::Symbols::root();
    string owner = includeOwner ? showClassName(gs, sym.data(gs)->owner) + "::" : "";
    return owner + showClassNameWithoutOwner(gs, sym);
}

} // namespace

llvm::Value *MK::getRubyConstantValueRaw(CompilerState &cs, core::SymbolRef sym, llvm::IRBuilderBase &build) {
    auto &builder = builderCast(build);
    sym = removeRoot(sym);
    auto str = showClassName(cs, sym);
    ENFORCE(str.length() < 2 || (str[0] != ':'), "implementation assumes that strings dont start with ::");
    auto functionName = sym.data(cs)->isClassOrModule() ? "sorbet_i_getRubyClass" : "sorbet_i_getRubyConstant";
    return builder.CreateCall(
        cs.module->getFunction(functionName),
        {MK::toCString(cs, str, builder), llvm::ConstantInt::get(cs, llvm::APInt(64, str.length()))});
}

llvm::Constant *MK::toCString(CompilerState &cs, string_view str, llvm::IRBuilderBase &builder) {
    llvm::StringRef nameRef(str.data(), str.length());
    return builderCast(builder).CreateGlobalStringPtr(nameRef, llvm::Twine("str_") + nameRef);
}

namespace {
const vector<pair<core::SymbolRef, string>> optimizedTypeTests = {
    {core::Symbols::untyped(), "sorbet_isa_Untyped"},
    {core::Symbols::Array(), "sorbet_isa_Array"},
    {core::Symbols::FalseClass(), "sorbet_isa_FalseClass"},
    {core::Symbols::TrueClass(), "sorbet_isa_TrueClass"},
    {core::Symbols::Float(), "sorbet_isa_Float"},
    {core::Symbols::Hash(), "sorbet_isa_Hash"},
    {core::Symbols::Integer(), "sorbet_isa_Integer"},
    {core::Symbols::NilClass(), "sorbet_isa_NilClass"},
    {core::Symbols::Proc(), "sorbet_isa_Proc"},
    {core::Symbols::Rational(), "sorbet_isa_Rational"},
    {core::Symbols::Regexp(), "sorbet_isa_Regexp"},
    {core::Symbols::String(), "sorbet_isa_String"},
    {core::Symbols::Symbol(), "sorbet_isa_Symbol"},
    {core::Symbols::Proc(), "sorbet_isa_Proc"},
};
}

llvm::Value *MK::createTypeTestU1(CompilerState &cs, llvm::IRBuilderBase &b, llvm::Value *val,
                                  const core::TypePtr &type) {
    auto &builder = builderCast(b);
    llvm::Value *ret = nullptr;
    typecase(
        type.get(),
        [&](core::ClassType *ct) {
            for (const auto &[candidate, specializedCall] : optimizedTypeTests) {
                if (ct->symbol == candidate) {
                    ret = builder.CreateCall(cs.module->getFunction(specializedCall), {val});
                    return;
                }
            }
            auto attachedClass = ct->symbol.data(cs)->attachedClass(cs);
            // todo: handle attached of attached class
            if (attachedClass.exists()) {
                ret = builder.CreateCall(cs.module->getFunction("sorbet_isa_class_of"),
                                         {val, MK::getRubyConstantValueRaw(cs, attachedClass, builder)});
                return;
            }
            ret = builder.CreateCall(cs.module->getFunction("sorbet_isa"),
                                     {val, MK::getRubyConstantValueRaw(cs, ct->symbol, builder)});
        },
        [&](core::AppliedType *at) {
            auto base = createTypeTestU1(cs, builder, val, core::make_type<core::ClassType>(at->klass));
            ret = base;
            // todo: ranges, hashes, sets, enumerator, and, overall, enumerables
        },
        [&](core::OrType *ct) {
            // TODO: reoder types so that cheap test is done first
            auto left = createTypeTestU1(cs, builder, val, ct->left);
            auto rightBlockStart = llvm::BasicBlock::Create(cs, "orRight", builder.GetInsertBlock()->getParent());
            auto contBlock = llvm::BasicBlock::Create(cs, "orContinue", builder.GetInsertBlock()->getParent());
            auto leftEnd = builder.GetInsertBlock();
            builder.CreateCondBr(left, contBlock, rightBlockStart);
            builder.SetInsertPoint(rightBlockStart);
            auto right = createTypeTestU1(cs, builder, val, ct->right);
            auto rightEnd = builder.GetInsertBlock();
            builder.CreateBr(contBlock);
            builder.SetInsertPoint(contBlock);
            auto phi = builder.CreatePHI(builder.getInt1Ty(), 2, "orTypeTest");
            phi->addIncoming(left, leftEnd);
            phi->addIncoming(right, rightEnd);
            ret = phi;
        },
        [&](core::AndType *ct) {
            // TODO: reoder types so that cheap test is done first
            auto left = createTypeTestU1(cs, builder, val, ct->left);
            auto rightBlockStart = llvm::BasicBlock::Create(cs, "andRight", builder.GetInsertBlock()->getParent());
            auto contBlock = llvm::BasicBlock::Create(cs, "andContinue", builder.GetInsertBlock()->getParent());
            auto leftEnd = builder.GetInsertBlock();
            builder.CreateCondBr(left, rightBlockStart, contBlock);
            builder.SetInsertPoint(rightBlockStart);
            auto right = createTypeTestU1(cs, builder, val, ct->right);
            auto rightEnd = builder.GetInsertBlock();
            builder.CreateBr(contBlock);
            builder.SetInsertPoint(contBlock);
            auto phi = builder.CreatePHI(builder.getInt1Ty(), 2, "andTypeTest");
            phi->addIncoming(left, leftEnd);
            phi->addIncoming(right, rightEnd);
            ret = phi;
        },
        [&](core::Type *_default) { ret = builder.getInt1(true); });
    ENFORCE(ret != nullptr);
    return ret;
}

namespace {
llvm::Value *getClassVariableStoreClass(CompilerState &cs, llvm::IRBuilder<> &builder, const BasicBlockMap &blockMap) {
    auto sym = blockMap.forMethod.data(cs)->owner;
    ENFORCE(sym.data(cs)->isClassOrModule());

    return MK::getRubyConstantValueRaw(cs, sym.data(cs)->topAttachedClass(cs), builder);
};

} // namespace

llvm::Value *MK::varGet(CompilerState &cs, core::LocalVariable local, llvm::IRBuilderBase &build,
                        const BasicBlockMap &blockMap, const UnorderedMap<core::LocalVariable, Alias> &aliases,
                        int rubyBlockId) {
    auto &builder = builderCast(build);
    if (aliases.contains(local)) {
        // alias to a field or constant
        auto alias = aliases.at(local);

        if (alias.kind == Alias::AliasKind::Constant) {
            return MK::getRubyConstantValueRaw(cs, alias.constantSym, builder);
        } else if (alias.kind == Alias::AliasKind::GlobalField) {
            return builder.CreateCall(
                cs.module->getFunction("sorbet_globalVariableGet"),
                {MK::toCString(cs, alias.globalField.data(cs)->name.data(cs)->shortName(cs), builder)});

        } else if (alias.kind == Alias::AliasKind::ClassField) {
            return builder.CreateCall(cs.module->getFunction("sorbet_classVariableGet"),
                                      {getClassVariableStoreClass(cs, builder, blockMap),
                                       MK::getRubyIdFor(cs, builder, alias.classField.data(cs)->shortName(cs))});
        } else if (alias.kind == Alias::AliasKind::InstanceField) {
            return builder.CreateCall(
                cs.module->getFunction("sorbet_instanceVariableGet"),
                {varGet(cs, core::LocalVariable::selfVariable(), builder, blockMap, aliases, rubyBlockId),
                 MK::getRubyIdFor(cs, builder, alias.instanceField.data(cs)->shortName(cs))});
        }
    }
    if (blockMap.escapedVariableIndeces.contains(local)) {
        auto id = blockMap.escapedVariableIndeces.at(local);
        auto store =
            builder.CreateCall(cs.module->getFunction("sorbet_getClosureElem"),
                               {blockMap.escapedClosure[rubyBlockId], llvm::ConstantInt::get(cs, llvm::APInt(32, id))});
        return builder.CreateLoad(store);
    }

    // normal local variable
    return MK::unboxRawValue(cs, builder, blockMap.llvmVariables.at(local));
}

void MK::varSet(CompilerState &cs, core::LocalVariable local, llvm::Value *var, llvm::IRBuilderBase &build,
                const BasicBlockMap &blockMap, UnorderedMap<core::LocalVariable, Alias> &aliases, int rubyBlockId) {
    auto &builder = builderCast(build);
    if (aliases.contains(local)) {
        // alias to a field or constant
        auto alias = aliases.at(local);
        if (alias.kind == Alias::AliasKind::Constant) {
            auto sym = aliases.at(local).constantSym;
            auto name = sym.data(cs.gs)->name.show(cs.gs);
            auto owner = sym.data(cs.gs)->owner;
            builder.CreateCall(cs.module->getFunction("sorbet_setConstant"),
                               {MK::getRubyConstantValueRaw(cs, owner, builder), MK::toCString(cs, name, builder),
                                llvm::ConstantInt::get(cs, llvm::APInt(64, name.length())), var});
        } else if (alias.kind == Alias::AliasKind::GlobalField) {
            builder.CreateCall(
                cs.module->getFunction("sorbet_globalVariableSet"),
                {MK::toCString(cs, alias.globalField.data(cs)->name.data(cs)->shortName(cs), builder), var});
        } else if (alias.kind == Alias::AliasKind::ClassField) {
            builder.CreateCall(cs.module->getFunction("sorbet_classVariableSet"),
                               {getClassVariableStoreClass(cs, builder, blockMap),
                                MK::getRubyIdFor(cs, builder, alias.classField.data(cs)->shortName(cs)), var});
        } else if (alias.kind == Alias::AliasKind::InstanceField) {
            builder.CreateCall(
                cs.module->getFunction("sorbet_instanceVariableSet"),
                {MK::varGet(cs, core::LocalVariable::selfVariable(), builder, blockMap, aliases, rubyBlockId),
                 MK::getRubyIdFor(cs, builder, alias.instanceField.data(cs)->shortName(cs)), var});
        }
        return;
    }
    if (blockMap.escapedVariableIndeces.contains(local)) {
        auto id = blockMap.escapedVariableIndeces.at(local);
        auto store =
            builder.CreateCall(cs.module->getFunction("sorbet_getClosureElem"),
                               {blockMap.escapedClosure[rubyBlockId], llvm::ConstantInt::get(cs, llvm::APInt(32, id))});
        builder.CreateStore(var, store);
        return;
    }

    // normal local variable
    MK::boxRawValue(cs, builder, blockMap.llvmVariables.at(local), var);
}
}; // namespace sorbet::compiler
