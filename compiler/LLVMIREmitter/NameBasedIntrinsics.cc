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
#include "compiler/LLVMIREmitter/NameBasedIntrinsics.h"
#include "compiler/Names/Names.h"
#include <string_view>
using namespace std;
namespace sorbet::compiler {
namespace {
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

llvm::IRBuilder<> &builderCast(llvm::IRBuilderBase &builder) {
    return static_cast<llvm::IRBuilder<> &>(builder);
};

class DoNothingIntrinsic : public NameBasedIntrinsicMethod {
public:
    virtual llvm::Value *makeCall(CompilerState &cs, cfg::Send *i, llvm::IRBuilderBase &builder,
                                  const BasicBlockMap &blockMap,
                                  const UnorderedMap<core::LocalVariable, Alias> &aliases,
                                  int currentRubyBlockId) const override {
        return MK::getRubyNilRaw(cs, builder);
    }
    virtual InlinedVector<core::NameRef, 2> applicableMethods(CompilerState &cs) const override {
        return {core::Names::keepForIde(), core::Names::keepForTypechecking()};
    }
} DoNothingIntrinsic;

class DefineMethodIntrinsic : public NameBasedIntrinsicMethod {
public:
    virtual llvm::Value *makeCall(CompilerState &cs, cfg::Send *i, llvm::IRBuilderBase &build,
                                  const BasicBlockMap &blockMap,
                                  const UnorderedMap<core::LocalVariable, Alias> &aliases,
                                  int currentRubyBlockId) const override {
        auto &builder = builderCast(build);
        bool isSelf = i->fun == Names::sorbet_defineMethodSingleton(cs);
        ENFORCE(i->args.size() == 2);
        auto ownerSym = typeToSym(cs, i->args[0].type);

        auto lit = core::cast_type<core::LiteralType>(i->args[1].type.get());
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
        auto llvmFuncName = LLVMIREmitterHelpers::getFunctionName(cs, funcSym);
        auto funcHandle = LLVMIREmitterHelpers::getOrCreateFunction(cs, funcSym);
        auto universalSignature =
            llvm::PointerType::getUnqual(llvm::FunctionType::get(llvm::Type::getInt64Ty(cs), true));
        auto ptr = builder.CreateBitCast(funcHandle, universalSignature);

        auto rubyFunc = cs.module->getFunction(isSelf ? "sorbet_defineMethodSingleton" : "sorbet_defineMethod");
        ENFORCE(rubyFunc);
        builder.CreateCall(rubyFunc, {MK::getRubyConstantValueRaw(cs, ownerSym, builder),
                                      MK::toCString(cs, funcNameRef.show(cs), builder), ptr,
                                      llvm::ConstantInt::get(cs, llvm::APInt(32, -1, true))});

        builder.CreateCall(LLVMIREmitterHelpers::getInitFunction(cs, funcSym), {});
        return MK::getRubyNilRaw(cs, builder);
    }

    virtual InlinedVector<core::NameRef, 2> applicableMethods(CompilerState &cs) const override {
        return {Names::sorbet_defineMethod(cs), Names::sorbet_defineMethodSingleton(cs)};
    }
} DefineMethodIntrinsic;

std::string showClassNameWithoutOwner(const core::GlobalState &gs, core::SymbolRef sym) {
    auto name = sym.data(gs)->name;
    if (name.data(gs)->kind == core::NameKind::UNIQUE) {
        return name.data(gs)->unique.original.data(gs)->show(gs);
    }
    return name.data(gs)->show(gs);
}

class DefineClassIntrinsic : public NameBasedIntrinsicMethod {
public:
    virtual llvm::Value *makeCall(CompilerState &cs, cfg::Send *i, llvm::IRBuilderBase &build,
                                  const BasicBlockMap &blockMap,
                                  const UnorderedMap<core::LocalVariable, Alias> &aliases,
                                  int currentRubyBlockId) const override {
        auto &builder = builderCast(build);
        auto sym = typeToSym(cs, i->args[0].type);
        // this is wrong and will not work for `class <<self`
        auto classNameCStr = MK::toCString(cs, showClassNameWithoutOwner(cs, sym), builder);
        auto isModule = sym.data(cs)->superClass() == core::Symbols::Module();

        if (sym.data(cs)->owner != core::Symbols::root()) {
            auto getOwner = MK::getRubyConstantValueRaw(cs, sym.data(cs)->owner, builder);
            if (isModule) {
                builder.CreateCall(cs.module->getFunction("sorbet_defineNestedModule"), {getOwner, classNameCStr});
            } else {
                auto rawCall = MK::getRubyConstantValueRaw(cs, sym.data(cs)->superClass(), builder);
                builder.CreateCall(cs.module->getFunction("sorbet_defineNestedClass"),
                                   {getOwner, classNameCStr, rawCall});
            }
        } else {
            if (isModule) {
                builder.CreateCall(cs.module->getFunction("sorbet_defineTopLevelModule"), {classNameCStr});
            } else {
                auto rawCall = MK::getRubyConstantValueRaw(cs, sym.data(cs)->superClass(), builder);
                builder.CreateCall(cs.module->getFunction("sorbet_defineTopClassOrModule"), {classNameCStr, rawCall});
            }
        }

        auto funcSym = cs.gs.lookupStaticInitForClass(sym.data(cs)->attachedClass(cs));
        auto llvmFuncName = LLVMIREmitterHelpers::getFunctionName(cs, funcSym);
        builder.CreateCall(LLVMIREmitterHelpers::getInitFunction(cs, funcSym), {});
        return MK::getRubyNilRaw(cs, builder);
    }
    virtual InlinedVector<core::NameRef, 2> applicableMethods(CompilerState &cs) const override {
        return {Names::sorbet_defineTopClassOrModule(cs)};
    }
} DefineClassIntrinsic;

class BuildArrayIntrinsic : public NameBasedIntrinsicMethod {
public:
    virtual llvm::Value *makeCall(CompilerState &cs, cfg::Send *i, llvm::IRBuilderBase &build,
                                  const BasicBlockMap &blockMap,
                                  const UnorderedMap<core::LocalVariable, Alias> &aliases,
                                  int currentRubyBlockId) const override {
        auto &builder = builderCast(build);
        auto ret =
            builder.CreateCall(cs.module->getFunction("sorbet_newRubyArray"),
                               {llvm::ConstantInt::get(cs, llvm::APInt(64, i->args.size(), true))}, "rawArrayLiteral");
        for (int argc = 0; argc < i->args.size(); argc++) {
            auto value = i->args[argc].variable;
            builder.CreateCall(cs.module->getFunction("sorbet_arrayPush"),
                               {ret, MK::varGet(cs, value, builder, aliases, blockMap, currentRubyBlockId)});
        }
        return ret;
    }
    virtual InlinedVector<core::NameRef, 2> applicableMethods(CompilerState &cs) const override {
        return {core::Names::buildArray()};
    }
} BuildArrayIntrinsic;

class BuildHashIntrinsic : public NameBasedIntrinsicMethod {
public:
    virtual llvm::Value *makeCall(CompilerState &cs, cfg::Send *i, llvm::IRBuilderBase &build,
                                  const BasicBlockMap &blockMap,
                                  const UnorderedMap<core::LocalVariable, Alias> &aliases,
                                  int currentRubyBlockId) const override {
        auto &builder = builderCast(build);
        auto ret = builder.CreateCall(cs.module->getFunction("sorbet_newRubyHash"), {}, "rawHashLiteral");
        // TODO(perf): in 2.7 use rb_hash_bulk_insert will give 2x speedup
        int argc = 0;
        while (argc < i->args.size()) {
            auto key = i->args[argc].variable;
            auto value = i->args[argc + 1].variable;
            builder.CreateCall(cs.module->getFunction("sorbet_hashStore"),
                               {ret, MK::varGet(cs, key, builder, aliases, blockMap, currentRubyBlockId),
                                MK::varGet(cs, value, builder, aliases, blockMap, currentRubyBlockId)});
            argc += 2;
        }
        return ret;
    }

    virtual InlinedVector<core::NameRef, 2> applicableMethods(CompilerState &cs) const override {
        return {core::Names::buildHash()};
    }
} BuildHashIntrinsic;

class IdentityIntrinsic : public NameBasedIntrinsicMethod {
public:
    virtual llvm::Value *makeCall(CompilerState &cs, cfg::Send *i, llvm::IRBuilderBase &builder,
                                  const BasicBlockMap &blockMap,
                                  const UnorderedMap<core::LocalVariable, Alias> &aliases,
                                  int currentRubyBlockId) const override {
        return MK::varGet(cs, i->args[0].variable, builder, aliases, blockMap, currentRubyBlockId);
    }
    virtual InlinedVector<core::NameRef, 2> applicableMethods(CompilerState &cs) const override {
        return {core::Names::suggestType()};
    }
} IdentityIntrinsic;

class SplatIntrinsic : public NameBasedIntrinsicMethod {
public:
    virtual llvm::Value *makeCall(CompilerState &cs, cfg::Send *i, llvm::IRBuilderBase &build,
                                  const BasicBlockMap &blockMap,
                                  const UnorderedMap<core::LocalVariable, Alias> &aliases,
                                  int currentRubyBlockId) const override {
        auto &builder = builderCast(build);
        auto arr = MK::varGet(cs, i->args[0].variable, builder, aliases, blockMap, currentRubyBlockId);
        auto before = MK::varGet(cs, i->args[1].variable, builder, aliases, blockMap, currentRubyBlockId);
        auto after = MK::varGet(cs, i->args[2].variable, builder, aliases, blockMap, currentRubyBlockId);
        return builder.CreateCall(cs.module->getFunction("sorbet_splatIntrinsic"), {arr, before, after},
                                  "sorbet_splatIntrinsic");
    }
    virtual InlinedVector<core::NameRef, 2> applicableMethods(CompilerState &cs) const override {
        return {core::Names::expandSplat()};
    }
} SplatIntrinsic;

}; // namespace
const vector<const NameBasedIntrinsicMethod *> &NameBasedIntrinsicMethod::definedIntrinsics() {
    static vector<const NameBasedIntrinsicMethod *> ret{
        &DoNothingIntrinsic, &DefineMethodIntrinsic, &DefineClassIntrinsic, &BuildArrayIntrinsic,
        &BuildHashIntrinsic, &IdentityIntrinsic,     &SplatIntrinsic};
    return ret;
}
}; // namespace sorbet::compiler
