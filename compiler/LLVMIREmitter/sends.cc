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

void defineMethod(CompilerState &cs, cfg::Send *i, bool isSelf, llvm::IRBuilder<> &builder) {
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
    auto universalSignature = llvm::PointerType::getUnqual(llvm::FunctionType::get(llvm::Type::getInt64Ty(cs), true));
    auto ptr = builder.CreateBitCast(funcHandle, universalSignature);

    auto rubyFunc = cs.module->getFunction(isSelf ? "sorbet_defineMethodSingleton" : "sorbet_defineMethod");
    ENFORCE(rubyFunc);
    builder.CreateCall(rubyFunc, {MK::getRubyConstantValueRaw(cs, ownerSym, builder),
                                  MK::toCString(cs, funcNameRef.show(cs), builder), ptr,
                                  llvm::ConstantInt::get(cs, llvm::APInt(32, -1, true))});

    builder.CreateCall(LLVMIREmitterHelpers::getInitFunction(cs, funcSym), {});
}

std::string showClassNameWithoutOwner(const core::GlobalState &gs, core::SymbolRef sym) {
    auto name = sym.data(gs)->name;
    if (name.data(gs)->kind == core::NameKind::UNIQUE) {
        return name.data(gs)->unique.original.data(gs)->show(gs);
    }
    return name.data(gs)->show(gs);
}

void defineClass(CompilerState &cs, cfg::Send *i, llvm::IRBuilder<> &builder) {
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
            builder.CreateCall(cs.module->getFunction("sorbet_defineNestedClass"), {getOwner, classNameCStr, rawCall});
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
}

}; // namespace

llvm::Value *LLVMIREmitterHelpers::emitMethodCall(CompilerState &cs, llvm::IRBuilderBase &build, cfg::Send *i,
                                                  const BasicBlockMap &blockMap,
                                                  UnorderedMap<core::LocalVariable, Alias> &aliases,
                                                  int currentRubyBlockId) {
    auto &builder = static_cast<llvm::IRBuilder<> &>(build);
    auto str = i->fun.data(cs)->shortName(cs);
    if (i->fun == core::Names::keepForIde() || i->fun == core::Names::keepForTypechecking()) {
        return MK::getRubyNilRaw(cs, builder);
    }
    if (i->fun == core::Names::buildHash()) {
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
    if (i->fun == core::Names::buildArray()) {
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
    if (i->fun == Names::sorbet_defineTopClassOrModule(cs)) {
        defineClass(cs, i, builder);
        return MK::getRubyNilRaw(cs, builder);
    }
    if (i->fun == Names::sorbet_defineMethod(cs)) {
        defineMethod(cs, i, false, builder);
        return MK::getRubyNilRaw(cs, builder);
    }
    if (i->fun == Names::sorbet_defineMethodSingleton(cs)) {
        defineMethod(cs, i, true, builder);
        return MK::getRubyNilRaw(cs, builder);
    }
    auto rawId = MK::getRubyIdFor(cs, builder, str);

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

    // TODO(perf): call
    // https://github.com/ruby/ruby/blob/3e3cc0885a9100e9d1bfdb77e136416ec803f4ca/internal.h#L2372
    // to get inline caching.
    // before this, perf will not be good
    auto var = MK::varGet(cs, i->recv.variable, builder, aliases, blockMap, currentRubyBlockId);
    llvm::Value *rawCall;
    if (i->link != nullptr) {
        // this send has a block!
        rawCall = builder.CreateCall(cs.module->getFunction("sorbet_callFuncBlock"),
                                     {var, rawId, llvm::ConstantInt::get(cs, llvm::APInt(32, i->args.size(), true)),
                                      builder.CreateGEP(blockMap.sendArgArrayByBlock[currentRubyBlockId], indices),
                                      blockMap.rubyBlocks2Functions[i->link->rubyBlockId],
                                      blockMap.escapedClosure[currentRubyBlockId]},
                                     "rawSendResult");

    } else {
        rawCall = builder.CreateCall(cs.module->getFunction("sorbet_callFunc"),
                                     {var, rawId, llvm::ConstantInt::get(cs, llvm::APInt(32, i->args.size(), true)),
                                      builder.CreateGEP(blockMap.sendArgArrayByBlock[currentRubyBlockId], indices)},
                                     "rawSendResult");
    }
    return rawCall;
};
} // namespace sorbet::compiler
