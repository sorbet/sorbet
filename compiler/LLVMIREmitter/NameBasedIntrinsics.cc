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
    virtual llvm::Value *makeCall(CompilerState &cs, cfg::Send *i, llvm::IRBuilderBase &build,
                                  const BasicBlockMap &blockMap,
                                  const UnorderedMap<core::LocalVariable, Alias> &aliases,
                                  int rubyBlockId) const override {
        return Payload::rubyNil(cs, build);
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
                                  int rubyBlockId) const override {
        auto &builder = builderCast(build);
        bool isSelf = i->fun == Names::defineMethodSingleton(cs);
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
        builder.CreateCall(rubyFunc, {Payload::getRubyConstant(cs, ownerSym, builder),
                                      Payload::toCString(cs, funcNameRef.show(cs), builder), ptr,
                                      llvm::ConstantInt::get(cs, llvm::APInt(32, -1, true))});

        builder.CreateCall(LLVMIREmitterHelpers::getInitFunction(cs, funcSym), {});
        return Payload::rubyNil(cs, builder);
    }

    virtual InlinedVector<core::NameRef, 2> applicableMethods(CompilerState &cs) const override {
        return {Names::defineMethod(cs), Names::defineMethodSingleton(cs)};
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
                                  int rubyBlockId) const override {
        auto &builder = builderCast(build);
        auto sym = typeToSym(cs, i->args[0].type);
        // this is wrong and will not work for `class <<self`
        auto classNameCStr = Payload::toCString(cs, showClassNameWithoutOwner(cs, sym), builder);
        auto isModule = sym.data(cs)->superClass() == core::Symbols::Module();

        if (sym.data(cs)->owner != core::Symbols::root()) {
            auto getOwner = Payload::getRubyConstant(cs, sym.data(cs)->owner, builder);
            if (isModule) {
                builder.CreateCall(cs.module->getFunction("sorbet_defineNestedModule"), {getOwner, classNameCStr});
            } else {
                auto rawCall = Payload::getRubyConstant(cs, sym.data(cs)->superClass(), builder);
                builder.CreateCall(cs.module->getFunction("sorbet_defineNestedClass"),
                                   {getOwner, classNameCStr, rawCall});
            }
        } else {
            if (isModule) {
                builder.CreateCall(cs.module->getFunction("sorbet_defineTopLevelModule"), {classNameCStr});
            } else {
                auto rawCall = Payload::getRubyConstant(cs, sym.data(cs)->superClass(), builder);
                builder.CreateCall(cs.module->getFunction("sorbet_defineTopClassOrModule"), {classNameCStr, rawCall});
            }
        }

        auto funcSym = cs.gs.lookupStaticInitForClass(sym.data(cs)->attachedClass(cs));
        auto llvmFuncName = LLVMIREmitterHelpers::getFunctionName(cs, funcSym);
        builder.CreateCall(LLVMIREmitterHelpers::getInitFunction(cs, funcSym), {});
        return Payload::rubyNil(cs, builder);
    }
    virtual InlinedVector<core::NameRef, 2> applicableMethods(CompilerState &cs) const override {
        return {Names::defineTopClassOrModule(cs)};
    }
} DefineClassIntrinsic;

class IdentityIntrinsic : public NameBasedIntrinsicMethod {
public:
    virtual llvm::Value *makeCall(CompilerState &cs, cfg::Send *i, llvm::IRBuilderBase &build,
                                  const BasicBlockMap &blockMap,
                                  const UnorderedMap<core::LocalVariable, Alias> &aliases,
                                  int rubyBlockId) const override {
        return Payload::varGet(cs, i->args[0].variable, build, blockMap, aliases, rubyBlockId);
    }
    virtual InlinedVector<core::NameRef, 2> applicableMethods(CompilerState &cs) const override {
        return {core::Names::suggestType()};
    }
} IdentityIntrinsic;

class CallWithBlock : public NameBasedIntrinsicMethod {
public:
    virtual llvm::Value *makeCall(CompilerState &cs, cfg::Send *i, llvm::IRBuilderBase &build,
                                  const BasicBlockMap &blockMap,
                                  const UnorderedMap<core::LocalVariable, Alias> &aliases,
                                  int rubyBlockId) const override {
        // args[0] is the receiver
        // args[1] is the method
        // args[2] is the block
        // args[3...] are the remaining arguements
        // equivalent to (args[0]).args[1](*args[3..], &args[2])

        auto &builder = builderCast(build);
        // TODO: this implementation generates code that is stupidly slow, we should be able to reuse instrinsics here
        // one day
        auto recv = Payload::varGet(cs, i->args[0].variable, builder, blockMap, aliases, rubyBlockId);
        auto lit = core::cast_type<core::LiteralType>(i->args[1].type.get());
        ENFORCE(lit->literalKind == core::LiteralType::LiteralTypeKind::Symbol);
        core::NameRef funName(cs, lit->value);
        auto rawId = Payload::idIntern(cs, builder, funName.data(cs)->shortName(cs));
        auto block = Payload::varGet(cs, i->args[2].variable, builder, blockMap, aliases, rubyBlockId);
        auto blockAsProc = builder.CreateCall(cs.module->getFunction("sorbet_callFunc"),
                                              {
                                                  block,
                                                  Payload::idIntern(cs, builder, "to_proc"),
                                                  llvm::ConstantInt::get(cs, llvm::APInt(32, 0, true)),
                                                  llvm::ConstantPointerNull::get(llvm::Type::getInt64PtrTy(cs)),
                                              });

        {
            int argId = -1;
            for (auto &arg : i->args) {
                if (argId < 3) {
                    continue;
                }
                argId += 1;
                llvm::Value *indices[] = {llvm::ConstantInt::get(cs, llvm::APInt(32, 0, true)),
                                          llvm::ConstantInt::get(cs, llvm::APInt(64, argId - 3, true))};
                auto var = Payload::varGet(cs, arg.variable, builder, blockMap, aliases, rubyBlockId);
                builder.CreateStore(
                    var, builder.CreateGEP(blockMap.sendArgArrayByBlock[rubyBlockId], indices, "callArgsAddr"));
            }
        }

        llvm::Value *indices[] = {llvm::ConstantInt::get(cs, llvm::APInt(64, 0, true)),
                                  llvm::ConstantInt::get(cs, llvm::APInt(64, 0, true))};

        auto rawCall =
            builder.CreateCall(cs.module->getFunction("sorbet_callFuncProc"),
                               {recv, rawId, llvm::ConstantInt::get(cs, llvm::APInt(32, i->args.size() - 3, true)),
                                builder.CreateGEP(blockMap.sendArgArrayByBlock[rubyBlockId], indices), blockAsProc},
                               "rawSendWithProcResult");

        return rawCall;
    }
    virtual InlinedVector<core::NameRef, 2> applicableMethods(CompilerState &cs) const override {
        return {core::Names::callWithBlock()};
    }
} CallWithBlock;

enum ShouldTakeReciever {
    TakesReciever,
    NoReciever,
};

class CallCMethod : public NameBasedIntrinsicMethod {
protected:
    string_view rubyMethod;
    string cMethod;
    ShouldTakeReciever takesReciever;

public:
    CallCMethod(string_view rubyMethod, string cMethod, ShouldTakeReciever takesReciever)
        : rubyMethod(rubyMethod), cMethod(cMethod), takesReciever(takesReciever){};

    virtual llvm::Value *makeCall(CompilerState &cs, cfg::Send *i, llvm::IRBuilderBase &build,
                                  const BasicBlockMap &blockMap,
                                  const UnorderedMap<core::LocalVariable, Alias> &aliases,
                                  int rubyBlockId) const override {
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

        llvm::Value *var;
        if (takesReciever == TakesReciever) {
            var = Payload::varGet(cs, i->recv.variable, builder, blockMap, aliases, rubyBlockId);
        } else {
            var = Payload::rubyNil(cs, builder);
        }

        return builder.CreateCall(cs.module->getFunction(cMethod),
                                  {var, llvm::ConstantInt::get(cs, llvm::APInt(32, i->args.size(), true)),
                                   builder.CreateGEP(blockMap.sendArgArrayByBlock[rubyBlockId], indices)},
                                  "rawSendResult");
    }
    virtual InlinedVector<core::NameRef, 2> applicableMethods(CompilerState &cs) const override {
        return {cs.gs.lookupNameUTF8(rubyMethod)};
    }
};

static const vector<CallCMethod> knownCMethods{
    {"<expand-splat>", "sorbet_splatIntrinsic", NoReciever},
    {"defined?", "sorbet_definedIntinsic", NoReciever},
    {"<build-hash>", "sorbet_buildHashIntrinsic", NoReciever},
    {"<build-array>", "sorbet_buildArrayIntrinsic", NoReciever},
};

vector<const NameBasedIntrinsicMethod *> computeNameBasedIntrinsics() {
    vector<const NameBasedIntrinsicMethod *> ret{
        &DoNothingIntrinsic, &DefineMethodIntrinsic, &DefineClassIntrinsic, &IdentityIntrinsic, &CallWithBlock,

    };
    for (auto &method : knownCMethods) {
        ret.emplace_back(&method);
    }
    return ret;
}

}; // namespace
const vector<const NameBasedIntrinsicMethod *> &NameBasedIntrinsicMethod::definedIntrinsics() {
    static vector<const NameBasedIntrinsicMethod *> ret = computeNameBasedIntrinsics();
    return ret;
}
}; // namespace sorbet::compiler
