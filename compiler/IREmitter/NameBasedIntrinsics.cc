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
#include "compiler/Core/CompilerState.h"
#include "compiler/Errors/Errors.h"
#include "compiler/IREmitter/IREmitter.h"
#include "compiler/IREmitter/IREmitterContext.h"
#include "compiler/IREmitter/IREmitterHelpers.h"
#include "compiler/IREmitter/MethodCallContext.h"
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
    if (auto classType = core::cast_type<core::ClassType>(typ)) {
        sym = classType->symbol;
    } else if (auto appliedType = core::cast_type<core::AppliedType>(typ)) {
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
    virtual llvm::Value *makeCall(MethodCallContext &mcctx) const override {
        return Payload::rubyNil(mcctx.cs, mcctx.build);
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
    virtual llvm::Value *makeCall(MethodCallContext &mcctx) const override {
        auto &cs = mcctx.cs;
        auto &builder = builderCast(mcctx.build);
        auto *send = mcctx.send;
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
        builder.CreateCall(IREmitterHelpers::getOrCreateStaticInit(cs, funcSym, send->receiverLoc),
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
    virtual llvm::Value *makeCall(MethodCallContext &mcctx) const override {
        return Payload::varGet(mcctx.cs, mcctx.send->args[0].variable, mcctx.build, mcctx.irctx, mcctx.rubyBlockId);
    }
    virtual InlinedVector<core::NameRef, 2> applicableMethods(CompilerState &cs) const override {
        return {core::Names::suggestType()};
    }
} IdentityIntrinsic;

class CallWithBlock : public NameBasedIntrinsicMethod {
public:
    CallWithBlock() : NameBasedIntrinsicMethod(Intrinsics::HandleBlock::Unhandled){};
    virtual llvm::Value *makeCall(MethodCallContext &mcctx) const override {
        // args[0] is the receiver
        // args[1] is the method
        // args[2] is the block
        // args[3...] are the remaining arguements
        // equivalent to (args[0]).args[1](*args[3..], &args[2])

        auto &cs = mcctx.cs;
        auto &builder = builderCast(mcctx.build);
        auto &irctx = mcctx.irctx;
        auto rubyBlockId = mcctx.rubyBlockId;
        auto *send = mcctx.send;

        // TODO: this implementation generates code that is stupidly slow, we should be able to reuse instrinsics here
        // one day
        auto recv = Payload::varGet(cs, send->args[0].variable, builder, irctx, rubyBlockId);
        auto lit = core::cast_type<core::LiteralType>(send->args[1].type);
        ENFORCE(lit->literalKind == core::LiteralType::LiteralTypeKind::Symbol);
        core::NameRef funName(cs, lit->value);
        auto name = funName.data(cs)->shortName(cs);
        auto rawId = Payload::idIntern(cs, builder, name);
        auto block = Payload::varGet(cs, send->args[2].variable, builder, irctx, rubyBlockId);
        auto blockAsProc = IREmitterHelpers::callViaRubyVMSimple(
            cs, builder, irctx, block, llvm::ConstantPointerNull::get(llvm::Type::getInt64PtrTy(cs)),
            llvm::ConstantInt::get(cs, llvm::APInt(32, 0, true)), llvm::ConstantInt::get(cs, llvm::APInt(32, 0, true)),
            "to_proc");

        auto [argc, argv, kw_splat] = IREmitterHelpers::fillSendArgArray(mcctx, 3);

        auto slowFunctionName = "callWithProc" + (string)name;
        auto *cache = IREmitterHelpers::makeInlineCache(cs, slowFunctionName);

        return builder.CreateCall(cs.module->getFunction("sorbet_callFuncProcWithCache"),
                                  {recv, rawId, argc, argv, kw_splat, blockAsProc, cache}, slowFunctionName);
    }
    virtual InlinedVector<core::NameRef, 2> applicableMethods(CompilerState &cs) const override {
        return {core::Names::callWithBlock()};
    }
} CallWithBlock;

class ExceptionRetry : public NameBasedIntrinsicMethod {
public:
    ExceptionRetry() : NameBasedIntrinsicMethod(Intrinsics::HandleBlock::Unhandled){};

    virtual llvm::Value *makeCall(MethodCallContext &mcctx) const override {
        auto &cs = mcctx.cs;
        auto &builder = static_cast<llvm::IRBuilder<> &>(mcctx.build);
        auto &irctx = mcctx.irctx;
        auto rubyBlockId = mcctx.rubyBlockId;

        auto *retrySingleton = Payload::retrySingleton(cs, builder, irctx);
        IREmitterHelpers::emitReturn(cs, builder, irctx, rubyBlockId, retrySingleton);

        auto *dead = llvm::BasicBlock::Create(cs, "dead-retry", irctx.rubyBlocks2Functions[rubyBlockId]);
        builder.SetInsertPoint(dead);

        return retrySingleton;
    }

    virtual InlinedVector<core::NameRef, 2> applicableMethods(CompilerState &cs) const override {
        return {core::Names::retry()};
    }
} ExceptionRetry;

enum ShouldTakeReceiver {
    TakesReceiver,
    NoReceiver,
};

llvm::Value *buildCMethodCall(MethodCallContext &mcctx, const string &cMethod, ShouldTakeReceiver takesReceiver) {
    auto &cs = mcctx.cs;
    auto &builder = builderCast(mcctx.build);
    auto &irctx = mcctx.irctx;
    auto rubyBlockId = mcctx.rubyBlockId;
    auto *send = mcctx.send;

    auto [argc, argv, _] = IREmitterHelpers::fillSendArgArray(mcctx);

    llvm::Value *recv;
    if (takesReceiver == TakesReceiver) {
        recv = Payload::varGet(cs, send->recv.variable, builder, irctx, rubyBlockId);
    } else {
        recv = Payload::rubyNil(cs, builder);
    }

    llvm::Value *blkPtr;
    if (mcctx.blk != nullptr) {
        blkPtr = mcctx.blk;
    } else {
        blkPtr = llvm::ConstantPointerNull::get(cs.getRubyBlockFFIType()->getPointerTo());
    }

    auto fun = Payload::idIntern(cs, builder, send->fun.data(cs)->shortName(cs));
    return builder.CreateCall(cs.module->getFunction(cMethod),
                              {recv, fun, argc, argv, blkPtr, irctx.localsOffset[rubyBlockId]}, "rawSendResult");
}

class CallCMethod : public NameBasedIntrinsicMethod {
protected:
    string_view rubyMethod;
    string cMethod;
    ShouldTakeReceiver takesReceiver;

public:
    CallCMethod(string_view rubyMethod, string cMethod, ShouldTakeReceiver takesReceiver,
                Intrinsics::HandleBlock supportsBlocks)
        : NameBasedIntrinsicMethod(supportsBlocks), rubyMethod(rubyMethod), cMethod(cMethod),
          takesReceiver(takesReceiver){};

    virtual llvm::Value *makeCall(MethodCallContext &mcctx) const override {
        return buildCMethodCall(mcctx, cMethod, takesReceiver);
    }
    virtual InlinedVector<core::NameRef, 2> applicableMethods(CompilerState &cs) const override {
        return {cs.gs.lookupNameUTF8(rubyMethod)};
    }
};

class BuildHash : public NameBasedIntrinsicMethod {
public:
    BuildHash() : NameBasedIntrinsicMethod(Intrinsics::HandleBlock::Unhandled) {}

    bool isLiteralish(CompilerState &cs, const core::TypePtr &t) const {
        // See IREmitterHelpers::emitLiteralish; we put the expected fast test first.
        return core::isa_type<core::LiteralType>(t) || t->derivesFrom(cs, core::Symbols::FalseClass()) ||
               t->derivesFrom(cs, core::Symbols::TrueClass()) || t->derivesFrom(cs, core::Symbols::NilClass());
    }

    virtual llvm::Value *makeCall(MethodCallContext &mcctx) const override {
        bool literalHash = absl::c_all_of(mcctx.send->args, [&](auto &v) { return isLiteralish(mcctx.cs, v.type); });

        // Building an empty hash at runtime is just as cheap as duplicating an
        // empty hash, and we don't have to waste space on the extra pre-built
        // hash.
        if (mcctx.send->args.empty() || !literalHash) {
            return buildCMethodCall(mcctx, "sorbet_buildHashIntrinsic", NoReceiver);
        }

        // We're going to build a literal hash at initialization time, and then
        // duplicate that hash wherever we need it.  This arrangement saves
        // re-hashing the keys every time the hash literal is constructed.
        static unsigned int counter = 0;
        auto &builder = builderCast(mcctx.build);
        string rawName = fmt::format("ruby_hashLiteral{}", ++counter);
        auto tp = llvm::Type::getInt64Ty(mcctx.cs);
        auto zero = llvm::ConstantInt::get(mcctx.cs, llvm::APInt(64, 0));
        llvm::Constant *indices[] = {zero};

        auto oldInsertPoint = builder.saveIP();
        auto globalDeclaration =
            static_cast<llvm::GlobalVariable *>(mcctx.cs.module->getOrInsertGlobal(rawName, tp, [&] {
                llvm::IRBuilder<> globalInitBuilder(mcctx.cs);
                auto ret = new llvm::GlobalVariable(*mcctx.cs.module, tp, false, llvm::GlobalVariable::InternalLinkage,
                                                    zero, rawName);
                ret->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);
                ret->setAlignment(8);

                auto voidTy = llvm::Type::getVoidTy(mcctx.cs);
                std::vector<llvm::Type *> NoArgs(0, voidTy);
                auto ft = llvm::FunctionType::get(voidTy, NoArgs, false);
                auto constr =
                    llvm::Function::Create(ft, llvm::Function::InternalLinkage, {"Constr_", rawName}, *mcctx.cs.module);

                auto bb = llvm::BasicBlock::Create(mcctx.cs, "constrHashLiteral", constr);
                globalInitBuilder.SetInsertPoint(bb);
                auto argArray = globalInitBuilder.CreateAlloca(llvm::ArrayType::get(tp, mcctx.send->args.size()),
                                                               nullptr, "argArray");

                int i = -1;
                for (auto &v : mcctx.send->args) {
                    i++;
                    llvm::Value *argIndices[] = {llvm::ConstantInt::get(mcctx.cs, llvm::APInt(32, 0, true)),
                                                 llvm::ConstantInt::get(mcctx.cs, llvm::APInt(64, i, true))};
                    llvm::Value *val = IREmitterHelpers::emitLiteralish(mcctx.cs, globalInitBuilder, v.type);
                    globalInitBuilder.CreateStore(
                        val, globalInitBuilder.CreateGEP(argArray, argIndices, fmt::format("hashArgs{}Addr", i)));
                }

                llvm::Value *argIndices[] = {llvm::ConstantInt::get(mcctx.cs, llvm::APInt(64, 0, true)),
                                             llvm::ConstantInt::get(mcctx.cs, llvm::APInt(64, 0, true))};
                auto hashValue = globalInitBuilder.CreateCall(
                    mcctx.cs.module->getFunction("sorbet_hashBuild"),
                    {llvm::ConstantInt::get(mcctx.cs, llvm::APInt(32, mcctx.send->args.size(), true)),
                     globalInitBuilder.CreateGEP(argArray, argIndices)},
                    "builtHash");

                globalInitBuilder.CreateStore(
                    hashValue, llvm::ConstantExpr::getInBoundsGetElementPtr(ret->getValueType(), ret, indices));
                globalInitBuilder.CreateRetVoid();
                globalInitBuilder.SetInsertPoint(mcctx.cs.globalConstructorsEntry);
                globalInitBuilder.CreateCall(constr, {});

                return ret;
            }));
        builder.restoreIP(oldInsertPoint);

        auto global = builder.CreateLoad(
            llvm::ConstantExpr::getInBoundsGetElementPtr(globalDeclaration->getValueType(), globalDeclaration, indices),
            "hashLiteral");
        auto copy = builder.CreateCall(mcctx.cs.module->getFunction("sorbet_hashDup"), {global}, "duplicatedHash");
        return copy;
    }

    virtual InlinedVector<core::NameRef, 2> applicableMethods(CompilerState &cs) const override {
        return {core::Names::buildHash()};
    }
} BuildHash;

static const vector<CallCMethod> knownCMethods{
    {"<expand-splat>", "sorbet_splatIntrinsic", NoReceiver, Intrinsics::HandleBlock::Unhandled},
    {"defined?", "sorbet_definedIntrinsic", NoReceiver, Intrinsics::HandleBlock::Unhandled},
    {"<build-keyword-args>", "sorbet_buildHashIntrinsic", NoReceiver, Intrinsics::HandleBlock::Unhandled},
    {"<build-array>", "sorbet_buildArrayIntrinsic", NoReceiver, Intrinsics::HandleBlock::Unhandled},
    {"<build-range>", "sorbet_buildRangeIntrinsic", NoReceiver, Intrinsics::HandleBlock::Unhandled},
    {"<string-interpolate>", "sorbet_stringInterpolate", NoReceiver, Intrinsics::HandleBlock::Unhandled},
    {"<self-new>", "sorbet_selfNew", NoReceiver, Intrinsics::HandleBlock::Unhandled},
    {"<block-break>", "sorbet_block_break", NoReceiver, Intrinsics::HandleBlock::Unhandled},
    {"!", "sorbet_bang", TakesReceiver, Intrinsics::HandleBlock::Unhandled},
};

vector<const NameBasedIntrinsicMethod *> computeNameBasedIntrinsics() {
    vector<const NameBasedIntrinsicMethod *> ret{
        &DoNothingIntrinsic, &DefineClassIntrinsic, &IdentityIntrinsic, &CallWithBlock, &ExceptionRetry, &BuildHash,
    };
    for (auto &method : knownCMethods) {
        ret.emplace_back(&method);
    }
    return ret;
}

} // namespace

const vector<const NameBasedIntrinsicMethod *> &NameBasedIntrinsicMethod::definedIntrinsics() {
    static vector<const NameBasedIntrinsicMethod *> ret = computeNameBasedIntrinsics();
    return ret;
}
} // namespace sorbet::compiler
