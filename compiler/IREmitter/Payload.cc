#include "llvm/IR/DerivedTypes.h" // FunctionType
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/ModuleUtils.h" // appendToGlobalCtors
// ^^^ violate our poisons

// needef for IREmitterHelpers
#include "common/typecase.h"
#include "core/core.h"

#include "IREmitterHelpers.h"
#include "Payload.h"
#include "compiler/Payload/Payload.h"
#include <string>

using namespace std;

namespace sorbet::compiler {
namespace {
llvm::IRBuilder<> &builderCast(llvm::IRBuilderBase &builder) {
    return static_cast<llvm::IRBuilder<> &>(builder);
};
} // namespace

llvm::Value *Payload::setExpectedBool(CompilerState &cs, llvm::IRBuilderBase &builder, llvm::Value *value,
                                      bool expected) {
    return builderCast(builder).CreateIntrinsic(llvm::Intrinsic::ID::expect, {llvm::Type::getInt1Ty(cs)},
                                                {value, builder.getInt1(expected)});
}

void Payload::boxRawValue(CompilerState &cs, llvm::IRBuilderBase &builder, llvm::AllocaInst *target,
                          llvm::Value *rawData) {
    builderCast(builder).CreateStore(rawData, builderCast(builder).CreateStructGEP(target, 0));
}

llvm::Value *Payload::unboxRawValue(CompilerState &cs, llvm::IRBuilderBase &builder, llvm::AllocaInst *target) {
    return builderCast(builder).CreateLoad(builderCast(builder).CreateStructGEP(target, 0), "rawRubyValue");
}

llvm::Value *Payload::rubyNil(CompilerState &cs, llvm::IRBuilderBase &builder) {
    return builderCast(builder).CreateCall(cs.module->getFunction("sorbet_rubyNil"), {}, "nilValueRaw");
}

llvm::Value *Payload::rubyFalse(CompilerState &cs, llvm::IRBuilderBase &builder) {
    return builderCast(builder).CreateCall(cs.module->getFunction("sorbet_rubyFalse"), {}, "falseValueRaw");
}

llvm::Value *Payload::rubyTrue(CompilerState &cs, llvm::IRBuilderBase &builder) {
    return builderCast(builder).CreateCall(cs.module->getFunction("sorbet_rubyTrue"), {}, "trueValueRaw");
}

void Payload::raiseArity(CompilerState &cs, llvm::IRBuilderBase &builder, llvm::Value *currentArgCount, int minArgs,
                         int maxArgs) {
    builderCast(builder).CreateCall(cs.module->getFunction("sorbet_raiseArity"),
                                    {currentArgCount, llvm::ConstantInt::get(cs, llvm::APInt(32, minArgs, true)),
                                     llvm::ConstantInt::get(cs, llvm::APInt(32, maxArgs, true))

                                    });
    builderCast(builder).CreateUnreachable();
}
llvm::Value *Payload::longToRubyValue(CompilerState &cs, llvm::IRBuilderBase &builder, long num) {
    return builderCast(builder).CreateCall(cs.module->getFunction("sorbet_longToRubyValue"),
                                           {llvm::ConstantInt::get(cs, llvm::APInt(64, num, true))}, "rawRubyInt");
}

llvm::Value *Payload::doubleToRubyValue(CompilerState &cs, llvm::IRBuilderBase &builder, double num) {
    return builderCast(builder).CreateCall(cs.module->getFunction("sorbet_doubleToRubyValue"),
                                           {llvm::ConstantFP::get(llvm::Type::getDoubleTy(cs), num)}, "rawRubyInt");
}

llvm::Value *Payload::cPtrToRubyString(CompilerState &cs, llvm::IRBuilderBase &builder, std::string_view str) {
    llvm::StringRef userStr(str.data(), str.length());
    auto rawCString = Payload::toCString(cs, str, builder);
    return builderCast(builder).CreateCall(
        cs.module->getFunction("sorbet_cPtrToRubyString"),
        {rawCString, llvm::ConstantInt::get(cs, llvm::APInt(64, str.length(), true))}, "rawRubyStr");
}

llvm::Value *Payload::testIsTruthy(CompilerState &cs, llvm::IRBuilderBase &builder, llvm::Value *val) {
    return builderCast(builder).CreateCall(cs.module->getFunction("sorbet_testIsTruthy"), {val}, "cond");
}

llvm::Value *Payload::idIntern(CompilerState &cs, llvm::IRBuilderBase &builder, std::string_view idName) {
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
        auto rawCString = Payload::toCString(cs, idName, builder);
        auto rawID = globalInitBuilder.CreateCall(
            cs.module->getFunction("sorbet_idIntern"),
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

llvm::Value *Payload::getRubyConstant(CompilerState &cs, core::SymbolRef sym, llvm::IRBuilderBase &build) {
    auto &builder = builderCast(build);
    sym = removeRoot(sym);
    auto str = showClassName(cs, sym);
    ENFORCE(str.length() < 2 || (str[0] != ':'), "implementation assumes that strings dont start with ::");
    auto functionName = sym.data(cs)->isClassOrModule() ? "sorbet_i_getRubyClass" : "sorbet_i_getRubyConstant";
    return builder.CreateCall(
        cs.module->getFunction(functionName),
        {Payload::toCString(cs, str, builder), llvm::ConstantInt::get(cs, llvm::APInt(64, str.length()))});
}

llvm::Value *Payload::toCString(CompilerState &cs, string_view str, llvm::IRBuilderBase &builder) {
    llvm::StringRef valueRef(str.data(), str.length());
    auto globalName = "addr_str_" + (string)str;
    auto globalDeclaration =
        static_cast<llvm::GlobalVariable *>(cs.module->getOrInsertGlobal(globalName, builder.getInt8PtrTy(), [&] {
            auto valueGlobal = builder.CreateGlobalString(valueRef, llvm::Twine("str_") + valueRef);
            auto zero = llvm::ConstantInt::get(cs, llvm::APInt(64, 0));
            llvm::Constant *indicesString[] = {zero, zero};
            auto addrGlobalInitializer =
                llvm::ConstantExpr::getInBoundsGetElementPtr(valueGlobal->getValueType(), valueGlobal, indicesString);
            auto addrGlobal =
                new llvm::GlobalVariable(*cs.module, builder.getInt8PtrTy(), true,
                                         llvm::GlobalVariable::InternalLinkage, addrGlobalInitializer, globalName);
            addrGlobal->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);
            addrGlobal->setAlignment(llvm::MaybeAlign(8));

            return addrGlobal;
        }));

    return builderCast(builder).CreateLoad(globalDeclaration);
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

llvm::Value *Payload::typeTest(CompilerState &cs, llvm::IRBuilderBase &b, llvm::Value *val, const core::TypePtr &type) {
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
                                         {val, Payload::getRubyConstant(cs, attachedClass, builder)});
                return;
            }
            ret = builder.CreateCall(cs.module->getFunction("sorbet_isa"),
                                     {val, Payload::getRubyConstant(cs, ct->symbol, builder)});
        },
        [&](core::AppliedType *at) {
            auto base = typeTest(cs, builder, val, core::make_type<core::ClassType>(at->klass));
            ret = base;
            // todo: ranges, hashes, sets, enumerator, and, overall, enumerables
        },
        [&](core::OrType *ct) {
            // TODO: reoder types so that cheap test is done first
            auto left = typeTest(cs, builder, val, ct->left);
            auto rightBlockStart = llvm::BasicBlock::Create(cs, "orRight", builder.GetInsertBlock()->getParent());
            auto contBlock = llvm::BasicBlock::Create(cs, "orContinue", builder.GetInsertBlock()->getParent());
            auto leftEnd = builder.GetInsertBlock();
            builder.CreateCondBr(left, contBlock, rightBlockStart);
            builder.SetInsertPoint(rightBlockStart);
            auto right = typeTest(cs, builder, val, ct->right);
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
            auto left = typeTest(cs, builder, val, ct->left);
            auto rightBlockStart = llvm::BasicBlock::Create(cs, "andRight", builder.GetInsertBlock()->getParent());
            auto contBlock = llvm::BasicBlock::Create(cs, "andContinue", builder.GetInsertBlock()->getParent());
            auto leftEnd = builder.GetInsertBlock();
            builder.CreateCondBr(left, rightBlockStart, contBlock);
            builder.SetInsertPoint(rightBlockStart);
            auto right = typeTest(cs, builder, val, ct->right);
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

void Payload::pushControlFrame(CompilerState &cs, llvm::IRBuilderBase &build, core::SymbolRef sym) {
    auto &builder = builderCast(build);
    auto funcName = sym.data(cs)->name.data(cs)->shortName(cs);
    auto funcNameId = Payload::idIntern(cs, builder, funcName);
    auto recv = Payload::getRubyConstant(cs, sym.data(cs)->owner, builder);
    builder.CreateCall(cs.module->getFunction("sorbet_pushControlFrame"), {recv, funcNameId});
}

void Payload::popControlFrame(CompilerState &cs, llvm::IRBuilderBase &build) {
    auto &builder = builderCast(build);
    builder.CreateCall(cs.module->getFunction("sorbet_popControlFrame"), {});
}

namespace {
llvm::Value *getClassVariableStoreClass(CompilerState &cs, llvm::IRBuilder<> &builder, const BasicBlockMap &blockMap) {
    auto sym = blockMap.forMethod.data(cs)->owner;
    ENFORCE(sym.data(cs)->isClassOrModule());

    return Payload::getRubyConstant(cs, sym.data(cs)->topAttachedClass(cs), builder);
};

} // namespace

llvm::Value *Payload::varGet(CompilerState &cs, core::LocalVariable local, llvm::IRBuilderBase &build,
                             const BasicBlockMap &blockMap, const UnorderedMap<core::LocalVariable, Alias> &aliases,
                             int rubyBlockId) {
    auto &builder = builderCast(build);
    if (aliases.contains(local)) {
        // alias to a field or constant
        auto alias = aliases.at(local);

        if (alias.kind == Alias::AliasKind::Constant) {
            return Payload::getRubyConstant(cs, alias.constantSym, builder);
        } else if (alias.kind == Alias::AliasKind::GlobalField) {
            return builder.CreateCall(
                cs.module->getFunction("sorbet_globalVariableGet"),
                {Payload::toCString(cs, alias.globalField.data(cs)->name.data(cs)->shortName(cs), builder)});

        } else if (alias.kind == Alias::AliasKind::ClassField) {
            return builder.CreateCall(cs.module->getFunction("sorbet_classVariableGet"),
                                      {getClassVariableStoreClass(cs, builder, blockMap),
                                       Payload::idIntern(cs, builder, alias.classField.data(cs)->shortName(cs))});
        } else if (alias.kind == Alias::AliasKind::InstanceField) {
            return builder.CreateCall(
                cs.module->getFunction("sorbet_instanceVariableGet"),
                {varGet(cs, core::LocalVariable::selfVariable(), builder, blockMap, aliases, rubyBlockId),
                 Payload::idIntern(cs, builder, alias.instanceField.data(cs)->shortName(cs))});
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
    return Payload::unboxRawValue(cs, builder, blockMap.llvmVariables.at(local));
}

void Payload::varSet(CompilerState &cs, core::LocalVariable local, llvm::Value *var, llvm::IRBuilderBase &build,
                     const BasicBlockMap &blockMap, UnorderedMap<core::LocalVariable, Alias> &aliases,
                     int rubyBlockId) {
    auto &builder = builderCast(build);
    if (aliases.contains(local)) {
        // alias to a field or constant
        auto alias = aliases.at(local);
        if (alias.kind == Alias::AliasKind::Constant) {
            auto sym = aliases.at(local).constantSym;
            auto name = sym.data(cs.gs)->name.show(cs.gs);
            auto owner = sym.data(cs.gs)->owner;
            builder.CreateCall(cs.module->getFunction("sorbet_setConstant"),
                               {Payload::getRubyConstant(cs, owner, builder), Payload::toCString(cs, name, builder),
                                llvm::ConstantInt::get(cs, llvm::APInt(64, name.length())), var});
        } else if (alias.kind == Alias::AliasKind::GlobalField) {
            builder.CreateCall(
                cs.module->getFunction("sorbet_globalVariableSet"),
                {Payload::toCString(cs, alias.globalField.data(cs)->name.data(cs)->shortName(cs), builder), var});
        } else if (alias.kind == Alias::AliasKind::ClassField) {
            builder.CreateCall(cs.module->getFunction("sorbet_classVariableSet"),
                               {getClassVariableStoreClass(cs, builder, blockMap),
                                Payload::idIntern(cs, builder, alias.classField.data(cs)->shortName(cs)), var});
        } else if (alias.kind == Alias::AliasKind::InstanceField) {
            builder.CreateCall(
                cs.module->getFunction("sorbet_instanceVariableSet"),
                {Payload::varGet(cs, core::LocalVariable::selfVariable(), builder, blockMap, aliases, rubyBlockId),
                 Payload::idIntern(cs, builder, alias.instanceField.data(cs)->shortName(cs)), var});
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
    Payload::boxRawValue(cs, builder, blockMap.llvmVariables.at(local), var);
}
}; // namespace sorbet::compiler
