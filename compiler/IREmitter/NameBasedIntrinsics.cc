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
#include "compiler/IREmitter/NameBasedIntrinsics.h"
#include "compiler/IREmitter/Payload.h"
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
    DoNothingIntrinsic() : NameBasedIntrinsicMethod(Intrinsics::HandleBlock::Handled){};
    virtual llvm::Value *makeCall(CompilerState &cs, cfg::Send *send, llvm::IRBuilderBase &build,
                                  const BasicBlockMap &blockMap,
                                  const UnorderedMap<core::LocalVariable, Alias> &aliases, int rubyBlockId,
                                  llvm::Function *blk) const override {
        return Payload::rubyNil(cs, build);
    }
    virtual InlinedVector<core::NameRef, 2> applicableMethods(CompilerState &cs) const override {
        return {core::Names::keepForIde(), core::Names::keepForTypechecking()};
    }
} DoNothingIntrinsic;

std::string showClassNameWithoutOwner(const core::GlobalState &gs, core::SymbolRef sym) {
    auto name = sym.data(gs)->name;
    if (name.data(gs)->kind == core::NameKind::UNIQUE) {
        return name.data(gs)->unique.original.data(gs)->show(gs);
    }
    return name.data(gs)->show(gs);
}

class DefineClassIntrinsic : public NameBasedIntrinsicMethod {
public:
    DefineClassIntrinsic() : NameBasedIntrinsicMethod(Intrinsics::HandleBlock::Unhandled){};
    virtual llvm::Value *makeCall(CompilerState &cs, cfg::Send *send, llvm::IRBuilderBase &build,
                                  const BasicBlockMap &blockMap,
                                  const UnorderedMap<core::LocalVariable, Alias> &aliases, int rubyBlockId,
                                  llvm::Function *blk) const override {
        auto &builder = builderCast(build);
        auto sym = typeToSym(cs, send->args[0].type);
        // this is wrong and will not work for `class <<self`
        auto classNameCStr = Payload::toCString(cs, showClassNameWithoutOwner(cs, sym), builder);
        auto isModule = sym.data(cs)->superClass() == core::Symbols::Module();
        auto funcSym = cs.gs.lookupStaticInitForClass(sym.data(cs)->attachedClass(cs));

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
        builder.CreateCall(IREmitterHelpers::getOrCreateStaticInit(cs, funcSym, send->receiverLoc),
                           {llvm::ConstantInt::get(cs, llvm::APInt(32, 0, true)),
                            llvm::ConstantPointerNull::get(llvm::Type::getInt64PtrTy(cs)),
                            Payload::getRubyConstant(cs, sym, builder)});
        return Payload::rubyNil(cs, builder);
    }
    virtual InlinedVector<core::NameRef, 2> applicableMethods(CompilerState &cs) const override {
        return {Names::defineTopClassOrModule(cs)};
    }
} DefineClassIntrinsic;

class IdentityIntrinsic : public NameBasedIntrinsicMethod {
public:
    IdentityIntrinsic() : NameBasedIntrinsicMethod(Intrinsics::HandleBlock::Unhandled){};
    virtual llvm::Value *makeCall(CompilerState &cs, cfg::Send *send, llvm::IRBuilderBase &build,
                                  const BasicBlockMap &blockMap,
                                  const UnorderedMap<core::LocalVariable, Alias> &aliases, int rubyBlockId,
                                  llvm::Function *blk) const override {
        return Payload::varGet(cs, send->args[0].variable, build, blockMap, aliases, rubyBlockId);
    }
    virtual InlinedVector<core::NameRef, 2> applicableMethods(CompilerState &cs) const override {
        return {core::Names::suggestType()};
    }
} IdentityIntrinsic;

class CallWithBlock : public NameBasedIntrinsicMethod {
public:
    CallWithBlock() : NameBasedIntrinsicMethod(Intrinsics::HandleBlock::Unhandled){};
    virtual llvm::Value *makeCall(CompilerState &cs, cfg::Send *send, llvm::IRBuilderBase &build,
                                  const BasicBlockMap &blockMap,
                                  const UnorderedMap<core::LocalVariable, Alias> &aliases, int rubyBlockId,
                                  llvm::Function *blk) const override {
        // args[0] is the receiver
        // args[1] is the method
        // args[2] is the block
        // args[3...] are the remaining arguements
        // equivalent to (args[0]).args[1](*args[3..], &args[2])

        auto &builder = builderCast(build);
        // TODO: this implementation generates code that is stupidly slow, we should be able to reuse instrinsics here
        // one day
        auto recv = Payload::varGet(cs, send->args[0].variable, builder, blockMap, aliases, rubyBlockId);
        auto lit = core::cast_type<core::LiteralType>(send->args[1].type.get());
        ENFORCE(lit->literalKind == core::LiteralType::LiteralTypeKind::Symbol);
        core::NameRef funName(cs, lit->value);
        auto rawId = Payload::idIntern(cs, builder, funName.data(cs)->shortName(cs));
        auto block = Payload::varGet(cs, send->args[2].variable, builder, blockMap, aliases, rubyBlockId);
        auto blockAsProc =
            IREmitterHelpers::callViaRubyVMSimple(cs, builder, block,

                                                  llvm::ConstantPointerNull::get(llvm::Type::getInt64PtrTy(cs)),

                                                  llvm::ConstantInt::get(cs, llvm::APInt(32, 0, true)), "to_proc");
        {
            int argId = -1;
            for (auto &arg : send->args) {
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

        auto argc = llvm::ConstantInt::get(cs, llvm::APInt(32, send->args.size() - 3, true));
        auto argv = builder.CreateGEP(blockMap.sendArgArrayByBlock[rubyBlockId], indices);
        return builder.CreateCall(cs.module->getFunction("sorbet_callFuncProc"), {recv, rawId, argc, argv, blockAsProc},
                                  "rawSendWithProcResult");
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
    CallCMethod(string_view rubyMethod, string cMethod, ShouldTakeReciever takesReciever,
                Intrinsics::HandleBlock supportsBlocks)
        : NameBasedIntrinsicMethod(supportsBlocks), rubyMethod(rubyMethod), cMethod(cMethod),
          takesReciever(takesReciever){};

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

        llvm::Value *recv;
        if (takesReciever == TakesReciever) {
            recv = Payload::varGet(cs, send->recv.variable, builder, blockMap, aliases, rubyBlockId);
        } else {
            recv = Payload::rubyNil(cs, builder);
        }

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
    }
    virtual InlinedVector<core::NameRef, 2> applicableMethods(CompilerState &cs) const override {
        return {cs.gs.lookupNameUTF8(rubyMethod)};
    }
};

static const vector<CallCMethod> knownCMethods{
    {"<expand-splat>", "sorbet_splatIntrinsic", NoReciever, Intrinsics::HandleBlock::Unhandled},
    {"defined?", "sorbet_definedIntinsic", NoReciever, Intrinsics::HandleBlock::Unhandled},
    {"<build-hash>", "sorbet_buildHashIntrinsic", NoReciever, Intrinsics::HandleBlock::Unhandled},
    {"<build-array>", "sorbet_buildArrayIntrinsic", NoReciever, Intrinsics::HandleBlock::Unhandled},
    {"<string-interpolate>", "sorbet_stringInterpolate", NoReciever, Intrinsics::HandleBlock::Unhandled},
    {"<self-new>", "sorbet_selfNew", NoReciever, Intrinsics::HandleBlock::Unhandled},
};

vector<const NameBasedIntrinsicMethod *> computeNameBasedIntrinsics() {
    vector<const NameBasedIntrinsicMethod *> ret{
        &DoNothingIntrinsic,
        &DefineClassIntrinsic,
        &IdentityIntrinsic,
        &CallWithBlock,

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
