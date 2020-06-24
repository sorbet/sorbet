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
#include "compiler/IREmitter/IREmitter.h"
#include "compiler/IREmitter/IREmitterContext.h"
#include "compiler/IREmitter/IREmitterHelpers.h"
#include "compiler/IREmitter/NameBasedIntrinsics.h"
#include "compiler/IREmitter/Payload.h"
#include "compiler/Names/Names.h"
#include "core/Names.h"
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
                                  const IREmitterContext &irctx,
                                  const UnorderedMap<core::LocalVariable, Alias> &aliases, int rubyBlockId,
                                  llvm::Function *blk) const override {
        return Payload::rubyNil(cs, build);
    }
    virtual InlinedVector<core::NameRef, 2> applicableMethods(CompilerState &cs) const override {
        return {core::Names::keepForIde(), core::Names::keepForTypechecking(), core::Names::keepForCfg()};
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
                                  const IREmitterContext &irctx,
                                  const UnorderedMap<core::LocalVariable, Alias> &aliases, int rubyBlockId,
                                  llvm::Function *blk) const override {
        auto &builder = builderCast(build);
        auto sym = typeToSym(cs, send->args[0].type);
        auto attachedClass = sym.data(cs)->attachedClass(cs);

        if (attachedClass.data(cs)->name.data(cs)->isTEnumName(cs)) {
            // T::Enum classes like `class X$1 < MyEnum; end` are fake and for the type system only
            // (We don't define them at runtime, because classes are expensive compared to how many
            // individual enum values there are.)
            return Payload::rubyNil(cs, builder);
        }

        // this is wrong and will not work for `class <<self`
        auto classNameCStr = Payload::toCString(cs, showClassNameWithoutOwner(cs, sym), builder);
        auto isModule = sym.data(cs)->superClass() == core::Symbols::Module();
        auto funcSym = cs.gs.lookupStaticInitForClass(attachedClass);

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
        builder.CreateCall(IREmitterHelpers::getOrCreateStaticInit(cs, funcSym, core::Loc(cs.file, send->receiverLoc)),
                           {llvm::ConstantInt::get(cs, llvm::APInt(32, 0, true)),
                            llvm::ConstantPointerNull::get(llvm::Type::getInt64PtrTy(cs)),
                            Payload::getRubyConstant(cs, sym, builder)});
        return Payload::rubyNil(cs, builder);
    }
    virtual InlinedVector<core::NameRef, 2> applicableMethods(CompilerState &cs) const override {
        return {core::Names::defineTopClassOrModule()};
    }
} DefineClassIntrinsic;

class IdentityIntrinsic : public NameBasedIntrinsicMethod {
public:
    IdentityIntrinsic() : NameBasedIntrinsicMethod(Intrinsics::HandleBlock::Unhandled){};
    virtual llvm::Value *makeCall(CompilerState &cs, cfg::Send *send, llvm::IRBuilderBase &build,
                                  const IREmitterContext &irctx,
                                  const UnorderedMap<core::LocalVariable, Alias> &aliases, int rubyBlockId,
                                  llvm::Function *blk) const override {
        return Payload::varGet(cs, send->args[0].variable, build, irctx, aliases, rubyBlockId);
    }
    virtual InlinedVector<core::NameRef, 2> applicableMethods(CompilerState &cs) const override {
        return {core::Names::suggestType()};
    }
} IdentityIntrinsic;

class CallWithBlock : public NameBasedIntrinsicMethod {
public:
    CallWithBlock() : NameBasedIntrinsicMethod(Intrinsics::HandleBlock::Unhandled){};
    virtual llvm::Value *makeCall(CompilerState &cs, cfg::Send *send, llvm::IRBuilderBase &build,
                                  const IREmitterContext &irctx,
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
        auto recv = Payload::varGet(cs, send->args[0].variable, builder, irctx, aliases, rubyBlockId);
        auto lit = core::cast_type<core::LiteralType>(send->args[1].type.get());
        ENFORCE(lit->literalKind == core::LiteralType::LiteralTypeKind::Symbol);
        core::NameRef funName(cs, lit->value);
        auto rawId = Payload::idIntern(cs, builder, funName.data(cs)->shortName(cs));
        auto block = Payload::varGet(cs, send->args[2].variable, builder, irctx, aliases, rubyBlockId);
        auto blockAsProc = IREmitterHelpers::callViaRubyVMSimple(
            cs, builder, block, llvm::ConstantPointerNull::get(llvm::Type::getInt64PtrTy(cs)),
            llvm::ConstantInt::get(cs, llvm::APInt(32, 0, true)), "to_proc");

        auto numArgs = send->args.size() - 3;
        auto *argc = llvm::ConstantInt::get(cs, llvm::APInt(32, numArgs, true));
        auto *argv =
            IREmitterHelpers::fillSendArgArray(cs, builder, irctx, aliases, rubyBlockId, send->args, 3, numArgs);

        return builder.CreateCall(cs.module->getFunction("sorbet_callFuncProc"), {recv, rawId, argc, argv, blockAsProc},
                                  "rawSendWithProcResult");
    }
    virtual InlinedVector<core::NameRef, 2> applicableMethods(CompilerState &cs) const override {
        return {core::Names::callWithBlock()};
    }
} CallWithBlock;

class ExceptionRetry : public NameBasedIntrinsicMethod {
public:
    ExceptionRetry() : NameBasedIntrinsicMethod(Intrinsics::HandleBlock::Unhandled){};

    virtual llvm::Value *makeCall(CompilerState &cs, cfg::Send *send, llvm::IRBuilderBase &build,
                                  const IREmitterContext &irctx,
                                  const UnorderedMap<core::LocalVariable, Alias> &aliases, int rubyBlockId,
                                  llvm::Function *blk) const override {
        llvm::IRBuilder<> &builder = static_cast<llvm::IRBuilder<> &>(build);

        auto *retrySingleton = Payload::retrySingleton(cs, builder, irctx);
        builder.CreateRet(retrySingleton);

        auto *dead = llvm::BasicBlock::Create(cs, "dead-retry", irctx.rubyBlocks2Functions[rubyBlockId]);
        builder.SetInsertPoint(dead);

        return retrySingleton;
    }

    virtual InlinedVector<core::NameRef, 2> applicableMethods(CompilerState &cs) const override {
        return {core::Names::retry()};
    }
} ExceptionRetry;

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
                                  const IREmitterContext &irctx,
                                  const UnorderedMap<core::LocalVariable, Alias> &aliases, int rubyBlockId,
                                  llvm::Function *blk) const override {
        auto &builder = builderCast(build);

        auto argv = IREmitterHelpers::fillSendArgArray(cs, builder, irctx, aliases, rubyBlockId, send->args);

        llvm::Value *recv;
        if (takesReciever == TakesReciever) {
            recv = Payload::varGet(cs, send->recv.variable, builder, irctx, aliases, rubyBlockId);
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
        auto fun = Payload::idIntern(cs, builder, send->fun.data(cs)->shortName(cs));
        return builder.CreateCall(cs.module->getFunction(cMethod),
                                  {recv, fun, argc, argv, blkPtr, irctx.escapedClosure[rubyBlockId]}, "rawSendResult");
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
    {"<block-break>", "sorbet_block_break", NoReciever, Intrinsics::HandleBlock::Unhandled},
};

vector<const NameBasedIntrinsicMethod *> computeNameBasedIntrinsics() {
    vector<const NameBasedIntrinsicMethod *> ret{
        &DoNothingIntrinsic, &DefineClassIntrinsic, &IdentityIntrinsic, &CallWithBlock, &ExceptionRetry,
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
