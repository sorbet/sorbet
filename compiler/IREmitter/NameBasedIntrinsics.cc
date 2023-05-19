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
#include "common/sort/sort.h"
#include "compiler/Core/CompilerState.h"
#include "compiler/Core/FailCompilation.h"
#include "compiler/Errors/Errors.h"
#include "compiler/IREmitter/IREmitter.h"
#include "compiler/IREmitter/IREmitterContext.h"
#include "compiler/IREmitter/IREmitterHelpers.h"
#include "compiler/IREmitter/MethodCallContext.h"
#include "compiler/IREmitter/NameBasedIntrinsics.h"
#include "compiler/IREmitter/Payload.h"
#include "core/Names.h"
#include <string_view>
using namespace std;
namespace sorbet::compiler {
namespace {
core::ClassOrModuleRef typeToSym(const core::GlobalState &gs, core::TypePtr typ) {
    core::ClassOrModuleRef sym;
    if (core::isa_type<core::ClassType>(typ)) {
        sym = core::cast_type_nonnull<core::ClassType>(typ).symbol;
    } else if (auto appliedType = core::cast_type<core::AppliedType>(typ)) {
        sym = appliedType->klass;
    } else {
        ENFORCE(false);
    }
    sym = IREmitterHelpers::fixupOwningSymbol(gs, sym).asClassOrModuleRef();
    return sym;
}

class DoNothingIntrinsic : public NameBasedIntrinsicMethod {
public:
    DoNothingIntrinsic() : NameBasedIntrinsicMethod(Intrinsics::HandleBlock::Handled){};
    virtual llvm::Value *makeCall(MethodCallContext &mcctx) const override {
        return Payload::rubyNil(mcctx.cs, mcctx.builder);
    }
    virtual InlinedVector<core::NameRef, 2> applicableMethods(CompilerState &cs) const override {
        return {core::Names::keepForCfg(), core::Names::nilForSafeNavigation()};
    }
} DoNothingIntrinsic;

class ShouldNeverSeeIntrinsic : public NameBasedIntrinsicMethod {
public:
    ShouldNeverSeeIntrinsic() : NameBasedIntrinsicMethod(Intrinsics::HandleBlock::Handled){};
    virtual llvm::Value *makeCall(MethodCallContext &mcctx) const override {
        failCompilation(mcctx.cs, core::Loc(core::FileRef(), mcctx.send->receiverLoc),
                        "Emitting intrinsic that should have been deleted!");
    }
    virtual InlinedVector<core::NameRef, 2> applicableMethods(CompilerState &cs) const override {
        return {core::Names::keepForIde()};
    }
} ShouldNeverSeeIntrinsic;

class DefineClassIntrinsic : public NameBasedIntrinsicMethod {
public:
    DefineClassIntrinsic() : NameBasedIntrinsicMethod(Intrinsics::HandleBlock::Unhandled){};
    virtual llvm::Value *makeCall(MethodCallContext &mcctx) const override {
        auto &cs = mcctx.cs;
        auto &builder = mcctx.builder;
        auto *send = mcctx.send;
        auto sym = typeToSym(cs, send->args[0].type);
        auto attachedClass = sym.data(cs)->attachedClass(cs);

        if (attachedClass.data(cs)->name.isTEnumName(cs)) {
            // T::Enum classes like `class X$1 < MyEnum; end` are fake and for the type system only
            // (We don't define them at runtime, because classes are expensive compared to how many
            // individual enum values there are.)
            return Payload::rubyNil(cs, builder);
        }

        llvm::Value *module = nullptr;

        auto funcSym = cs.gs.lookupStaticInitForClass(attachedClass);
        if (!attachedClass.data(cs)->isSingletonClass(cs)) {
            auto classNameCStr = Payload::toCString(cs, IREmitterHelpers::showClassNameWithoutOwner(cs, sym), builder);
            auto isModule = sym.data(cs)->superClass() == core::Symbols::Module();

            if (!IREmitterHelpers::isRootishSymbol(cs, sym.data(cs)->owner)) {
                auto getOwner = Payload::getRubyConstant(cs, sym.data(cs)->owner, builder);
                if (isModule) {
                    module = builder.CreateCall(cs.getFunction("sorbet_defineNestedModule"), {getOwner, classNameCStr});
                } else {
                    auto rawCall = Payload::getRubyConstant(cs, sym.data(cs)->superClass(), builder);
                    module = builder.CreateCall(cs.getFunction("sorbet_defineNestedClass"),
                                                {getOwner, classNameCStr, rawCall});
                }
            } else {
                if (isModule) {
                    module = builder.CreateCall(cs.getFunction("sorbet_defineTopLevelModule"), {classNameCStr});
                } else {
                    auto rawCall = Payload::getRubyConstant(cs, sym.data(cs)->superClass(), builder);
                    module =
                        builder.CreateCall(cs.getFunction("sorbet_defineTopClassOrModule"), {classNameCStr, rawCall});
                }
            }
        } else {
            module = Payload::getRubyConstant(cs, attachedClass, builder);
            module = builder.CreateCall(cs.getFunction("sorbet_singleton_class"), {module}, "singletonClass");
        }
        builder.CreateCall(cs.getFunction("sorbet_callStaticInitDirect"),
                           {IREmitterHelpers::getOrCreateStaticInit(cs, funcSym, send->receiverLoc),
                            llvm::ConstantInt::get(cs, llvm::APInt(32, 0, true)),
                            llvm::ConstantPointerNull::get(llvm::Type::getInt64PtrTy(cs)), module});
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
        return Payload::varGet(mcctx.cs, mcctx.send->args[0].variable, mcctx.builder, mcctx.irctx, mcctx.rubyRegionId);
    }
    virtual InlinedVector<core::NameRef, 2> applicableMethods(CompilerState &cs) const override {
        return {core::Names::suggestConstantType(), core::Names::suggestFieldType()};
    }
} IdentityIntrinsic;

llvm::Value *prepareBlockHandler(MethodCallContext &mcctx, cfg::VariableUseSite &blkVar) {
    auto &cs = mcctx.cs;
    auto &irctx = mcctx.irctx;
    auto rubyRegionId = mcctx.rubyRegionId;

    if (IREmitterHelpers::canPassThroughBlockViaRubyVM(mcctx, blkVar.variable)) {
        return Payload::getPassedBlockHandler(cs, mcctx.builder);
    } else {
        // TODO(perf) `makeBlockHandlerProc` uses `to_proc` under the hood, and could be rewritten here to make an
        // inline cache.
        auto *block = Payload::varGet(cs, blkVar.variable, mcctx.builder, irctx, rubyRegionId);
        return Payload::makeBlockHandlerProc(cs, mcctx.builder, block);
    }
}

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
        auto &irctx = mcctx.irctx;
        auto rubyRegionId = mcctx.rubyRegionId;
        auto *send = mcctx.send;
        auto &builder = mcctx.builder;

        // TODO: this implementation generates code that is stupidly slow, we should be able to reuse instrinsics here
        // one day
        auto recv = send->args[0].variable;
        auto lit = core::cast_type_nonnull<core::NamedLiteralType>(send->args[1].type);
        ENFORCE(lit.literalKind == core::NamedLiteralType::LiteralTypeKind::Symbol);
        auto methodName = lit.asName();

        llvm::Value *blockHandler = prepareBlockHandler(mcctx, send->args[2]);

        auto shortName = methodName.shortName(cs);

        auto [stack, keywords, flags] = IREmitterHelpers::buildSendArgs(mcctx, recv, 3);
        flags.blockarg = true;
        auto *cfp = Payload::getCFPForBlock(cs, builder, irctx, rubyRegionId);
        Payload::pushRubyStackVector(cs, builder, cfp, Payload::varGet(cs, recv, builder, irctx, rubyRegionId), stack);
        auto *cache = IREmitterHelpers::makeInlineCache(cs, builder, string(shortName), flags, stack.size(), keywords);
        if (methodName == core::Names::super()) {
            return Payload::callSuperFuncWithCache(mcctx.cs, mcctx.builder, cache, blockHandler);
        }
        return Payload::callFuncWithCache(mcctx.cs, mcctx.builder, cache, blockHandler);
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
        auto &builder = mcctx.builder;
        auto &irctx = mcctx.irctx;
        auto rubyRegionId = mcctx.rubyRegionId;

        auto *retrySingleton = Payload::retrySingleton(cs, builder, irctx);
        IREmitterHelpers::emitReturn(cs, builder, irctx, rubyRegionId, retrySingleton);

        auto *dead = llvm::BasicBlock::Create(cs, "dead-retry", irctx.rubyBlocks2Functions[rubyRegionId]);
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

llvm::Value *buildCMethodCall(MethodCallContext &mcctx, const string &cMethod, ShouldTakeReceiver takesReceiver,
                              core::ClassOrModuleRef klass) {
    auto &cs = mcctx.cs;
    auto &builder = mcctx.builder;

    auto args = IREmitterHelpers::fillSendArgArray(mcctx);

    llvm::Value *recv;
    if (takesReceiver == TakesReceiver) {
        recv = mcctx.varGetRecv();
    } else {
        recv = Payload::rubyNil(cs, builder);
    }

    llvm::Value *blkPtr;
    if (auto *blk = mcctx.blkAsFunction()) {
        blkPtr = blk;
    } else {
        blkPtr = llvm::ConstantPointerNull::get(cs.getRubyBlockFFIType()->getPointerTo());
    }

    llvm::Value *offset = Payload::buildLocalsOffset(cs);

    auto fun = Payload::idIntern(cs, builder, mcctx.send->fun.shortName(cs));
    auto *value =
        builder.CreateCall(cs.getFunction(cMethod), {recv, fun, args.argc, args.argv, blkPtr, offset}, "rawSendResult");
    if (klass.exists()) {
        Payload::assumeType(cs, builder, value, klass);
    }
    return value;
}

class CallCMethod : public NameBasedIntrinsicMethod {
protected:
    core::NameRef rubyMethod;
    string cMethod;
    ShouldTakeReceiver takesReceiver;
    core::ClassOrModuleRef klass;

public:
    CallCMethod(core::NameRef rubyMethod, string cMethod, ShouldTakeReceiver takesReceiver,
                Intrinsics::HandleBlock supportsBlocks, core::ClassOrModuleRef klass = core::ClassOrModuleRef{})
        : NameBasedIntrinsicMethod(supportsBlocks), rubyMethod(rubyMethod), cMethod(cMethod),
          takesReceiver(takesReceiver), klass(klass){};

    virtual llvm::Value *makeCall(MethodCallContext &mcctx) const override {
        return buildCMethodCall(mcctx, cMethod, takesReceiver, klass);
    }
    virtual InlinedVector<core::NameRef, 2> applicableMethods(CompilerState &cs) const override {
        return {rubyMethod};
    }
};

class BuildHash : public NameBasedIntrinsicMethod {
public:
    BuildHash() : NameBasedIntrinsicMethod(Intrinsics::HandleBlock::Unhandled) {}

    bool isLiteralish(CompilerState &cs, const core::TypePtr &t) const {
        // See IREmitterHelpers::emitLiteralish; we put the expected fast test first.
        if (core::isa_type<core::NamedLiteralType>(t)) {
            return true;
        }

        if (core::isa_type<core::IntegerLiteralType>(t)) {
            return true;
        }

        if (core::isa_type<core::FloatLiteralType>(t)) {
            return true;
        }

        // IREmitterHelpers::emitLiteralish knows that its TypePtr argument is
        // restricted.  We have no such guarantees, so we have to be more defensive.
        if (t.hasUntyped()) {
            return false;
        }

        return t.derivesFrom(cs, core::Symbols::FalseClass()) || t.derivesFrom(cs, core::Symbols::TrueClass()) ||
               t.derivesFrom(cs, core::Symbols::NilClass());
    }

    virtual llvm::Value *makeCall(MethodCallContext &mcctx) const override {
        bool literalHash = absl::c_all_of(mcctx.send->args, [&](auto &v) { return isLiteralish(mcctx.cs, v.type); });

        // Building an empty hash at runtime is just as cheap as duplicating an
        // empty hash, and we don't have to waste space on the extra pre-built
        // hash.
        if (mcctx.send->args.empty() || !literalHash) {
            return buildCMethodCall(mcctx, "sorbet_buildHashIntrinsic", NoReceiver, core::Symbols::Hash());
        }

        // We're going to build a literal hash at initialization time, and then
        // duplicate that hash wherever we need it.  This arrangement saves
        // re-hashing the keys every time the hash literal is constructed.
        static unsigned int counter = 0;
        auto &builder = mcctx.builder;
        string rawName = fmt::format("ruby_hashLiteral{}", ++counter);
        auto tp = llvm::Type::getInt64Ty(mcctx.cs);
        auto zero = llvm::ConstantInt::get(mcctx.cs, llvm::APInt(64, 0));

        auto oldInsertPoint = builder.saveIP();
        auto globalDeclaration =
            static_cast<llvm::GlobalVariable *>(mcctx.cs.module->getOrInsertGlobal(rawName, tp, [&] {
                llvm::IRBuilder<> globalInitBuilder(mcctx.cs);
                auto ret = new llvm::GlobalVariable(*mcctx.cs.module, tp, false, llvm::GlobalVariable::InternalLinkage,
                                                    zero, rawName);
                ret->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);
                ret->setAlignment(llvm::MaybeAlign(8));

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
                    llvm::Value *val = IREmitterHelpers::emitLiteralish(mcctx.cs, globalInitBuilder, v.type);
                    globalInitBuilder.CreateStore(
                        val, globalInitBuilder.CreateConstGEP2_64(argArray, 0, i, fmt::format("hashArgs{}Addr", i)));
                }

                auto hashValue = globalInitBuilder.CreateCall(
                    mcctx.cs.getFunction("sorbet_literalHashBuild"),
                    {llvm::ConstantInt::get(mcctx.cs, llvm::APInt(32, mcctx.send->args.size(), true)),
                     globalInitBuilder.CreateConstGEP2_64(argArray, 0, 0)},
                    "builtHash");

                globalInitBuilder.CreateStore(hashValue, ret);
                globalInitBuilder.CreateRetVoid();
                globalInitBuilder.SetInsertPoint(mcctx.cs.globalConstructorsEntry);
                globalInitBuilder.CreateCall(constr, {});

                return ret;
            }));
        builder.restoreIP(oldInsertPoint);

        auto *index = builder.CreateLoad(globalDeclaration, "hashLiteral");
        auto *copy = builder.CreateCall(mcctx.cs.getFunction("sorbet_globalConstDupHash"), {index}, "duplicatedHash");
        Payload::assumeType(mcctx.cs, builder, copy, core::Symbols::Hash());
        return copy;
    }

    virtual InlinedVector<core::NameRef, 2> applicableMethods(CompilerState &cs) const override {
        return {core::Names::buildHash()};
    }
} BuildHash;

std::tuple<CallCacheFlags, llvm::Value *> prepareSplatArgs(MethodCallContext &mcctx, cfg::VariableUseSite &splatArgsVar,
                                                           cfg::VariableUseSite &kwArgsVar) {
    auto &cs = mcctx.cs;
    auto &irctx = mcctx.irctx;
    auto *send = mcctx.send;
    auto &builder = mcctx.builder;

    // For the VM send there will be two cases:
    //
    // 1. We do not have keyword args (args[3] is nil). Then we can just dup args[2] and use VM_CALL_ARGS_SPLAT.
    //
    // 2. We do have keyword args (args[3] is not nil). Then we'll need to construct an array of this form:
    //
    //      [posarg0, posarg1, ..., posargn, kwhash]
    //
    //    where args[2] = [posarg0, posarg1, ..., posargsn], and use VM_CALL_ARGS_SPLAT | VM_CALL_KW_SPLAT.
    //
    //    There are two subcases:
    //
    //    2a. All keywords were inline (args[3] has even length, and its contents will be of the form
    //        [sym,val,sym,val,...,sym,val]). Then we can just construct kwhash by build_hash'ing args[3].
    //    2b. Not all keywords were inline (args[3] has odd length, and its contents will be of the form)
    //        [sym,val,sym,val,...,sym,val,kwhash1]). Then we dup args[3], pop kwhash1 from it, construct
    //        kwhash0 from what's remaining, and update kwhash0 with kwhash1 to obtain kwhash.
    //
    //        Note that for now, the desugarer does not produce (2b) cases where the array is not simply [kwhash1].
    //        Thus we can't really test this case, so we throw an error, even though the code that is there does
    //        attempt to do the right thing.
    //
    // TODO(perf): We can probably save quite a bit of intermediate dupping, popping, etc., by cleverer addressing
    // of the array contents.
    auto *splatArgs = Payload::varGet(mcctx.cs, splatArgsVar.variable, builder, irctx, mcctx.rubyRegionId);

    // TODO(perf) we can avoid duplicating the array here if we know that it was created specifically for this
    // splat.
    llvm::Value *splatArray = builder.CreateCall(cs.getFunction("sorbet_arrayDup"), {splatArgs}, "splatArray");

    CallCacheFlags flags;

    if (kwArgsVar.type.derivesFrom(mcctx.cs, core::Symbols::NilClass())) {
        flags.args_splat = true;
    } else if (auto *ptt = core::cast_type<core::TupleType>(kwArgsVar.type)) {
        flags.args_splat = true;
        flags.kw_splat = true;

        auto *kwArgArray = Payload::varGet(mcctx.cs, kwArgsVar.variable, mcctx.builder, irctx, mcctx.rubyRegionId);

        llvm::Value *kwHash;

        if (ptt->elems.size() & 0x1) {
            auto *kwHash1 = builder.CreateCall(cs.getFunction("sorbet_arrayPop"), {kwArgArray}, "kwHash1");
            if (ptt->elems.size() > 1) {
                auto *size = llvm::ConstantInt::get(cs, llvm::APInt(32, ptt->elems.size() - 1, true));
                auto *innerPtr = builder.CreateCall(cs.getFunction("sorbet_rubyArrayInnerPtr"), {kwArgArray});
                kwHash = builder.CreateCall(cs.getFunction("sorbet_hashBuild"), {size, innerPtr}, "kwHash");
                builder.CreateCall(cs.getFunction("sorbet_hashUpdate"), {kwHash, kwHash1});

                // Failing compilation because as of this writing, this case is not produced by the desugarer, so
                // the above code is untested. In theory, once the case is implemented in the desugarer, it should
                // be okay to remove this.
                failCompilation(cs, core::Loc(cs.file, send->receiverLoc),
                                "internal error: arg 3 to call-with-splat has odd length > 1");
            } else {
                kwHash = builder.CreateCall(cs.getFunction("sorbet_hashDup"), {kwHash1}, "kwHash");
            }
        } else {
            auto *size = llvm::ConstantInt::get(cs, llvm::APInt(32, ptt->elems.size(), true));
            auto *innerPtr = builder.CreateCall(cs.getFunction("sorbet_rubyArrayInnerPtr"), {kwArgArray});
            kwHash = builder.CreateCall(cs.getFunction("sorbet_hashBuild"), {size, innerPtr}, "kwHash");
        }

        builder.CreateCall(cs.getFunction("sorbet_arrayPush"), {splatArray, kwHash});
    } else {
        // This should not be possible (desugarer will only pass nil or a tuple).
        failCompilation(cs, core::Loc(cs.file, send->receiverLoc),
                        "internal error: arg 3 to call-with-splat has neither nil nor tuple type");
    }

    if (send->isPrivateOk) {
        flags.fcall = true;
    }

    return {flags, splatArray};
}

class CallWithSplat : public NameBasedIntrinsicMethod {
public:
    CallWithSplat() : NameBasedIntrinsicMethod(Intrinsics::HandleBlock::Handled){};

    virtual llvm::Value *makeCall(MethodCallContext &mcctx) const override {
        // args[0] is the receiver
        // args[1] is the method
        // args[2] are the splat arguments
        // args[3] are the keyword args
        auto &cs = mcctx.cs;
        auto &irctx = mcctx.irctx;
        auto &builder = mcctx.builder;
        auto *send = mcctx.send;
        auto recv = send->args[0].variable;
        auto *cfp = Payload::getCFPForBlock(cs, builder, irctx, mcctx.rubyRegionId);

        auto [flags, splatArray] = prepareSplatArgs(mcctx, send->args[2], send->args[3]);

        auto lit = core::cast_type_nonnull<core::NamedLiteralType>(send->args[1].type);
        ENFORCE(lit.literalKind == core::NamedLiteralType::LiteralTypeKind::Symbol);
        auto methodName = lit.asName();

        // setup the inline cache
        // Note that in the case of calling `super`, the VM's search mechanism will
        // fetch the method name ID from the method definition itself, not the call
        // cache, so it's OK that we're saying the cache is for `super`.
        auto shortName = methodName.shortName(cs);
        auto *cache = IREmitterHelpers::makeInlineCache(cs, builder, std::string(shortName), flags, 1, {});

        // Push receiver and the splat array.
        // For the receiver, we can't use MethodCallContext::varGetRecv here because the real receiver
        // is actually the first arg of the callWithSplat intrinsic method.
        Payload::pushRubyStackVector(
            cs, builder, cfp, Payload::varGet(mcctx.cs, recv, mcctx.builder, irctx, mcctx.rubyRegionId), {splatArray});

        // Call the receiver.
        if (auto *blk = mcctx.blkAsFunction()) {
            auto blkId = mcctx.blk.value();
            auto usesBreak = irctx.blockUsesBreak[blkId];
            auto *ifunc = Payload::getOrBuildBlockIfunc(cs, builder, irctx, blkId);
            if (methodName == core::Names::super()) {
                return Payload::callSuperFuncBlockWithCache(mcctx.cs, mcctx.builder, cache, usesBreak, ifunc);
            }
            return Payload::callFuncBlockWithCache(mcctx.cs, mcctx.builder, cache, usesBreak, ifunc);
        } else {
            auto *blockHandler = Payload::vmBlockHandlerNone(mcctx.cs, mcctx.builder);
            if (methodName == core::Names::super()) {
                return Payload::callSuperFuncWithCache(mcctx.cs, mcctx.builder, cache, blockHandler);
            }
            return Payload::callFuncWithCache(mcctx.cs, mcctx.builder, cache, blockHandler);
        }
    }
    virtual InlinedVector<core::NameRef, 2> applicableMethods(CompilerState &cs) const override {
        return {core::Names::callWithSplat()};
    }
} CallWithSplat;

class CallWithSplatAndBlock : public NameBasedIntrinsicMethod {
public:
    CallWithSplatAndBlock() : NameBasedIntrinsicMethod{Intrinsics::HandleBlock::Unhandled} {};

    virtual llvm::Value *makeCall(MethodCallContext &mcctx) const override {
        ENFORCE(mcctx.blkAsFunction() == nullptr);

        // args[0] is the receiver
        // args[1] is the method
        // args[2] are the splat arguments
        // args[3] are the keyword arguments
        // args[4] is the block
        auto &cs = mcctx.cs;
        auto &irctx = mcctx.irctx;
        auto &builder = mcctx.builder;
        auto *send = mcctx.send;
        auto recv = send->args[0].variable;

        auto [flags, splatArray] = prepareSplatArgs(mcctx, send->args[2], send->args[3]);
        flags.blockarg = true;
        auto *blockHandler = prepareBlockHandler(mcctx, send->args[4]);

        auto lit = core::cast_type_nonnull<core::NamedLiteralType>(send->args[1].type);
        ENFORCE(lit.literalKind == core::NamedLiteralType::LiteralTypeKind::Symbol);
        auto methodName = lit.asName();

        // setup the inline cache
        // Note that in the case of calling `super`, the VM's search mechanism will
        // fetch the method name ID from the method definition itself, not the call
        // cache, so it's OK that we're saying the cache is for `super`.
        auto shortName = methodName.shortName(cs);
        auto *cache = IREmitterHelpers::makeInlineCache(cs, builder, std::string(shortName), flags, 1, {});

        // Push receiver and the splat array.
        // For the receiver, we can't use MethodCallContext::varGetRecv here because the real receiver
        // is actually the first arg of the callWithSplat intrinsic method.
        auto *cfp = Payload::getCFPForBlock(cs, builder, irctx, mcctx.rubyRegionId);
        Payload::pushRubyStackVector(
            cs, builder, cfp, Payload::varGet(mcctx.cs, recv, mcctx.builder, irctx, mcctx.rubyRegionId), {splatArray});

        if (methodName == core::Names::super()) {
            return Payload::callSuperFuncWithCache(mcctx.cs, mcctx.builder, cache, blockHandler);
        }
        return Payload::callFuncWithCache(mcctx.cs, mcctx.builder, cache, blockHandler);
    }

    virtual InlinedVector<core::NameRef, 2> applicableMethods(CompilerState &cs) const override {
        return {core::Names::callWithSplatAndBlock()};
    }
} CallWithSplatAndBlock;

class NewIntrinsic : public NameBasedIntrinsicMethod {
public:
    NewIntrinsic() : NameBasedIntrinsicMethod{Intrinsics::HandleBlock::Unhandled} {};

    virtual llvm::Value *makeCall(MethodCallContext &mcctx) const override {
        auto &cs = mcctx.cs;
        auto &builder = mcctx.builder;
        auto &irctx = mcctx.irctx;
        int rubyRegionId = mcctx.rubyRegionId;

        auto *klass = mcctx.varGetRecv();
        auto *newCache = mcctx.getInlineCache();

        auto slowCall = llvm::BasicBlock::Create(cs, "slowNew", builder.GetInsertBlock()->getParent());
        auto fastCall = llvm::BasicBlock::Create(cs, "fastNew", builder.GetInsertBlock()->getParent());
        auto afterNew = llvm::BasicBlock::Create(cs, "afterNew", builder.GetInsertBlock()->getParent());

        auto *cfp = Payload::getCFPForBlock(cs, builder, irctx, rubyRegionId);
        auto *allocatedObject =
            builder.CreateCall(cs.getFunction("sorbet_maybeAllocateObjectFastPath"), {klass, newCache});
        auto *isUndef = Payload::testIsUndef(cs, builder, allocatedObject);
        builder.CreateCondBr(isUndef, slowCall, fastCall);

        // We're pushing these arguments always, the only question is what we actually
        // wind up calling with them.
        auto &rubyStackArgs = mcctx.getStackArgs();

        builder.SetInsertPoint(slowCall);
        Payload::pushRubyStackVector(cs, builder, cfp, klass, rubyStackArgs.stack);
        auto *nullBHForNew = Payload::vmBlockHandlerNone(cs, builder);
        auto *slowValue = builder.CreateCall(cs.getFunction("sorbet_callFuncWithCache"), {newCache, nullBHForNew});
        builder.CreateBr(afterNew);

        builder.SetInsertPoint(fastCall);

        // Whatever the flags on the `new` call were, the call to initialize is always
        // allowed to call a private method.
        CallCacheFlags flags = rubyStackArgs.flags;
        flags.fcall = true;

        auto *initializeCache = IREmitterHelpers::makeInlineCache(cs, builder, "initialize", flags,
                                                                  rubyStackArgs.stack.size(), rubyStackArgs.keywords);
        auto *nullBHForInitialize = Payload::vmBlockHandlerNone(cs, builder);
        Payload::pushRubyStackVector(cs, builder, cfp, allocatedObject, rubyStackArgs.stack);
        builder.CreateCall(cs.getFunction("sorbet_callFuncWithCache"), {initializeCache, nullBHForInitialize});
        builder.CreateBr(afterNew);

        builder.SetInsertPoint(afterNew);
        auto *objectPhi = builder.CreatePHI(builder.getInt64Ty(), 2, "initializedObject");
        objectPhi->addIncoming(slowValue, slowCall);
        objectPhi->addIncoming(allocatedObject, fastCall);

        return objectPhi;
    }

    virtual InlinedVector<core::NameRef, 2> applicableMethods(CompilerState &cs) const override {
        return {core::Names::new_()};
    }
} NewIntrinsic;

class DefinedClassVar : public NameBasedIntrinsicMethod {
public:
    DefinedClassVar() : NameBasedIntrinsicMethod{Intrinsics::HandleBlock::Unhandled} {};

    virtual llvm::Value *makeCall(MethodCallContext &mcctx) const override {
        auto &cs = mcctx.cs;
        auto &builder = mcctx.builder;
        auto &irctx = mcctx.irctx;
        int rubyRegionId = mcctx.rubyRegionId;

        auto *klass = Payload::getClassVariableStoreClass(cs, builder, irctx);
        // TODO(froydnj): figure out how to access the ID of the argument directly.
        auto *var = Payload::varGet(cs, mcctx.send->args[0].variable, builder, irctx, rubyRegionId);
        return builder.CreateCall(cs.getFunction("sorbet_classVariableDefined"), {klass, var}, "is_cvar_defined");
    }

    virtual InlinedVector<core::NameRef, 2> applicableMethods(CompilerState &cs) const override {
        return {core::Names::definedClassVar()};
    }
} DefinedClassVar;

class DefinedInstanceVar : public NameBasedIntrinsicMethod {
public:
    DefinedInstanceVar() : NameBasedIntrinsicMethod{Intrinsics::HandleBlock::Unhandled} {};

    virtual llvm::Value *makeCall(MethodCallContext &mcctx) const override {
        auto &cs = mcctx.cs;
        auto &builder = mcctx.builder;
        auto &irctx = mcctx.irctx;
        int rubyRegionId = mcctx.rubyRegionId;

        auto *self = Payload::varGet(cs, cfg::LocalRef::selfVariable(), builder, irctx, rubyRegionId);
        // TODO(froydnj): figure out how to access the ID of the argument directly.
        auto *var = Payload::varGet(cs, mcctx.send->args[0].variable, builder, irctx, rubyRegionId);
        return builder.CreateCall(cs.getFunction("sorbet_instanceVariableDefined"), {self, var}, "is_cvar_defined");
    }

    virtual InlinedVector<core::NameRef, 2> applicableMethods(CompilerState &cs) const override {
        return {core::Names::definedInstanceVar()};
    }
} DefinedInstanceVar;

class InstanceVariableGet : public NameBasedIntrinsicMethod {
public:
    InstanceVariableGet() : NameBasedIntrinsicMethod{Intrinsics::HandleBlock::Unhandled} {};

    virtual llvm::Value *makeCall(MethodCallContext &mcctx) const override {
        if (mcctx.send->args.size() != 1) {
            return IREmitterHelpers::emitMethodCallViaRubyVM(mcctx);
        }

        auto &cs = mcctx.cs;
        auto &builder = mcctx.builder;
        auto &var = mcctx.send->args[0].type;
        if (!core::isa_type<core::NamedLiteralType>(var)) {
            return IREmitterHelpers::emitMethodCallViaRubyVM(mcctx);
        }
        auto lit = core::cast_type_nonnull<core::NamedLiteralType>(var);
        if (lit.literalKind != core::NamedLiteralType::LiteralTypeKind::Symbol) {
            return IREmitterHelpers::emitMethodCallViaRubyVM(mcctx);
        }

        auto varName = lit.asName();
        auto varNameStr = varName.shortName(cs);

        auto *callCache = mcctx.getInlineCache();
        auto *ivarCache = Payload::buildInstanceVariableCache(cs, varNameStr);
        auto *cfp = Payload::getCFPForBlock(cs, builder, mcctx.irctx, mcctx.rubyRegionId);
        auto *recv = mcctx.varGetRecv();
        auto *ivarID = Payload::idIntern(cs, builder, varNameStr);

        return builder.CreateCall(cs.getFunction("sorbet_vm_instance_variable_get"),
                                  {callCache, ivarCache, cfp, recv, ivarID});
    }

    virtual InlinedVector<core::NameRef, 2> applicableMethods(CompilerState &cs) const override {
        return {core::Names::instanceVariableGet()};
    }
} InstanceVariableGet;

class InstanceVariableSet : public NameBasedIntrinsicMethod {
public:
    InstanceVariableSet() : NameBasedIntrinsicMethod{Intrinsics::HandleBlock::Unhandled} {};

    virtual llvm::Value *makeCall(MethodCallContext &mcctx) const override {
        if (mcctx.send->args.size() != 2) {
            return IREmitterHelpers::emitMethodCallViaRubyVM(mcctx);
        }

        auto &cs = mcctx.cs;
        auto &builder = mcctx.builder;
        auto &var = mcctx.send->args[0].type;
        if (!core::isa_type<core::NamedLiteralType>(var)) {
            return IREmitterHelpers::emitMethodCallViaRubyVM(mcctx);
        }
        auto lit = core::cast_type_nonnull<core::NamedLiteralType>(var);
        if (lit.literalKind != core::NamedLiteralType::LiteralTypeKind::Symbol) {
            return IREmitterHelpers::emitMethodCallViaRubyVM(mcctx);
        }

        auto varName = lit.asName();
        auto varNameStr = varName.shortName(cs);

        auto *callCache = mcctx.getInlineCache();
        auto *ivarCache = Payload::buildInstanceVariableCache(cs, varNameStr);
        auto *cfp = Payload::getCFPForBlock(cs, builder, mcctx.irctx, mcctx.rubyRegionId);
        auto *recv = mcctx.varGetRecv();
        auto *ivarID = Payload::idIntern(cs, builder, varNameStr);
        auto *value = Payload::varGet(cs, mcctx.send->args[1].variable, builder, mcctx.irctx, mcctx.rubyRegionId);

        return builder.CreateCall(cs.getFunction("sorbet_vm_instance_variable_set"),
                                  {callCache, ivarCache, cfp, recv, ivarID, value});
    }

    virtual InlinedVector<core::NameRef, 2> applicableMethods(CompilerState &cs) const override {
        return {core::Names::instanceVariableSet()};
    }
} InstanceVariableSet;

class ClassIntrinsic : public NameBasedIntrinsicMethod {
public:
    ClassIntrinsic() : NameBasedIntrinsicMethod{Intrinsics::HandleBlock::Unhandled} {};

    virtual llvm::Value *makeCall(MethodCallContext &mcctx) const override {
        if (mcctx.send->args.size() != 0) {
            return IREmitterHelpers::emitMethodCallViaRubyVM(mcctx);
        }

        auto &cs = mcctx.cs;
        auto &builder = mcctx.builder;
        auto *callCache = mcctx.getInlineCache();
        auto *cfp = Payload::getCFPForBlock(cs, builder, mcctx.irctx, mcctx.rubyRegionId);
        auto *recv = mcctx.varGetRecv();
        return builder.CreateCall(cs.getFunction("sorbet_vm_class"), {callCache, cfp, recv});
    }

    virtual InlinedVector<core::NameRef, 2> applicableMethods(CompilerState &cs) const override {
        return {core::Names::class_()};
    }
} ClassIntrinsic;

class BangIntrinsic : public NameBasedIntrinsicMethod {
public:
    BangIntrinsic() : NameBasedIntrinsicMethod{Intrinsics::HandleBlock::Unhandled} {};

    virtual llvm::Value *makeCall(MethodCallContext &mcctx) const override {
        if (mcctx.send->args.size() != 0) {
            return IREmitterHelpers::emitMethodCallViaRubyVM(mcctx);
        }

        auto &cs = mcctx.cs;
        auto &builder = mcctx.builder;
        auto *callCache = mcctx.getInlineCache();
        auto *cfp = Payload::getCFPForBlock(cs, builder, mcctx.irctx, mcctx.rubyRegionId);
        auto *recv = mcctx.varGetRecv();
        return builder.CreateCall(cs.getFunction("sorbet_vm_bang"), {callCache, cfp, recv});
    }

    virtual InlinedVector<core::NameRef, 2> applicableMethods(CompilerState &cs) const override {
        return {core::Names::bang()};
    }
} BangIntrinsic;

class FreezeIntrinsic : public NameBasedIntrinsicMethod {
public:
    FreezeIntrinsic() : NameBasedIntrinsicMethod{Intrinsics::HandleBlock::Unhandled} {}

    virtual llvm::Value *makeCall(MethodCallContext &mcctx) const override {
        if (!mcctx.send->args.empty()) {
            return IREmitterHelpers::emitMethodCallViaRubyVM(mcctx);
        }

        auto &cs = mcctx.cs;
        auto &builder = mcctx.builder;
        auto *callCache = mcctx.getInlineCache();
        auto *cfp = Payload::getCFPForBlock(cs, builder, mcctx.irctx, mcctx.rubyRegionId);
        auto *recv = mcctx.varGetRecv();
        return builder.CreateCall(cs.getFunction("sorbet_vm_freeze"), {callCache, cfp, recv});
    }

    virtual InlinedVector<core::NameRef, 2> applicableMethods(CompilerState &cs) const override {
        return {core::Names::freeze()};
    }
} FreezeIntrinsic;

class IsAIntrinsic : public NameBasedIntrinsicMethod {
public:
    IsAIntrinsic() : NameBasedIntrinsicMethod{Intrinsics::HandleBlock::Unhandled} {};

    virtual llvm::Value *makeCall(MethodCallContext &mcctx) const override {
        if (mcctx.send->args.size() != 1) {
            return IREmitterHelpers::emitMethodCallViaRubyVM(mcctx);
        }

        auto &cs = mcctx.cs;
        auto &builder = mcctx.builder;
        auto *callCache = mcctx.getInlineCache();
        auto *cfp = Payload::getCFPForBlock(cs, builder, mcctx.irctx, mcctx.rubyRegionId);
        auto *recv = mcctx.varGetRecv();
        auto *var = Payload::varGet(cs, mcctx.send->args[0].variable, builder, mcctx.irctx, mcctx.rubyRegionId);
        return builder.CreateCall(cs.getFunction("sorbet_vm_isa_p"), {callCache, cfp, recv, var});
    }

    virtual InlinedVector<core::NameRef, 2> applicableMethods(CompilerState &cs) const override {
        return {core::Names::isA_p(), core::Names::kindOf_p()};
    }
} IsAIntrinsic;

class UntypedSpecialization : public NameBasedIntrinsicMethod {
    const core::NameRef rubyMethod;
    const uint32_t arity;
    const string_view cMethod;

public:
    UntypedSpecialization(core::NameRef rubyMethod, uint32_t arity, string_view cMethod)
        : NameBasedIntrinsicMethod{Intrinsics::HandleBlock::Unhandled}, rubyMethod(rubyMethod), arity(arity),
          cMethod(cMethod) {}

    virtual llvm::Value *makeCall(MethodCallContext &mcctx) const override {
        auto *send = mcctx.send;
        if (send->args.size() != this->arity) {
            return IREmitterHelpers::emitMethodCallViaRubyVM(mcctx);
        }
        // Should be taken care of by specifying Intrinsics::HandleBlock::Unhandled.
        ENFORCE(!mcctx.blk.has_value());
        // If we had some kind of type information for the receiver, assume that we
        // have already tested for a fast path earlier; this way we don't waste
        // extra time doing another test that didn't work the first time.
        if (!send->recv.type.isUntyped()) {
            return IREmitterHelpers::emitMethodCallViaRubyVM(mcctx);
        }

        auto &cs = mcctx.cs;
        auto &builder = mcctx.builder;
        auto *cache = mcctx.getInlineCache();
        auto *recv = mcctx.varGetRecv();
        auto &args = mcctx.getStackArgs();
        ENFORCE(args.stack.size() == this->arity);
        ENFORCE(this->arity == 1 || this->arity == 2);

        auto *cFunction = cs.getFunction(llvm::StringRef{this->cMethod.data(), this->cMethod.size()});
        auto *cfp = Payload::getCFPForBlock(cs, builder, mcctx.irctx, mcctx.rubyRegionId);

        InlinedVector<llvm::Value *, 5> funcArgs{cfp, cache, recv, args.stack[0]};
        if (this->arity == 2) {
            funcArgs.emplace_back(args.stack[1]);
        }

        return builder.CreateCall(cFunction, llvm::ArrayRef{&funcArgs[0], funcArgs.size()});
    }

    virtual InlinedVector<core::NameRef, 2> applicableMethods(CompilerState &cs) const override {
        return {rubyMethod};
    }
};

static const vector<CallCMethod> knownCMethods{
    {core::Names::expandSplat(), "sorbet_expandSplatIntrinsic", NoReceiver, Intrinsics::HandleBlock::Unhandled,
     core::Symbols::Array()},
    {core::Names::splat(), "sorbet_splatIntrinsic", NoReceiver, Intrinsics::HandleBlock::Unhandled,
     core::Symbols::Array()},
    {core::Names::defined_p(), "sorbet_definedIntrinsic", NoReceiver, Intrinsics::HandleBlock::Unhandled},
    {core::Names::buildArray(), "sorbet_buildArrayIntrinsic", NoReceiver, Intrinsics::HandleBlock::Unhandled,
     core::Symbols::Array()},
    {core::Names::buildRange(), "sorbet_buildRangeIntrinsic", NoReceiver, Intrinsics::HandleBlock::Unhandled,
     core::ClassOrModuleRef()},
    {core::Names::stringInterpolate(), "sorbet_stringInterpolate", NoReceiver, Intrinsics::HandleBlock::Unhandled,
     core::Symbols::String()},
    {core::Names::blockBreak(), "sorbet_block_break", NoReceiver, Intrinsics::HandleBlock::Unhandled},
    {core::Names::nil_p(), "sorbet_nil_p", TakesReceiver, Intrinsics::HandleBlock::Unhandled},
    {core::Names::checkMatchArray(), "sorbet_check_match_array", NoReceiver, Intrinsics::HandleBlock::Unhandled,
     core::ClassOrModuleRef()},

    // for kwsplat building
    {core::Names::toHashDup(), "sorbet_magic_toHashDup", NoReceiver, Intrinsics::HandleBlock::Unhandled,
     core::Symbols::Hash()},
    {core::Names::toHashNoDup(), "sorbet_magic_toHashNoDup", NoReceiver, Intrinsics::HandleBlock::Unhandled,
     core::Symbols::Hash()},
    {core::Names::mergeHash(), "sorbet_magic_mergeHash", NoReceiver, Intrinsics::HandleBlock::Unhandled,
     core::Symbols::Hash()},
    {core::Names::mergeHashValues(), "sorbet_magic_mergeHashValues", NoReceiver, Intrinsics::HandleBlock::Unhandled,
     core::Symbols::Hash()},
};

static const vector<UntypedSpecialization> untypedSpecializations{
    {core::Names::squareBrackets(), 1, "sorbet_vm_aref"sv},
    {core::Names::plus(), 1, "sorbet_vm_plus"sv},
    {core::Names::minus(), 1, "sorbet_vm_minus"sv},
    {core::Names::eqeq(), 1, "sorbet_vm_eqeq"sv},
    {core::Names::neq(), 1, "sorbet_vm_neq"sv},
    {core::Names::leq(), 1, "sorbet_vm_leq"sv},
    {core::Names::lessThan(), 1, "sorbet_vm_lt"sv},
    {core::Names::geq(), 1, "sorbet_vm_geq"sv},
    {core::Names::greaterThan(), 1, "sorbet_vm_gt"sv},
};

vector<const NameBasedIntrinsicMethod *> computeNameBasedIntrinsics() {
    vector<const NameBasedIntrinsicMethod *> ret{
        &DoNothingIntrinsic, &DefineClassIntrinsic, &IdentityIntrinsic,     &CallWithBlock,           &ExceptionRetry,
        &BuildHash,          &CallWithSplat,        &CallWithSplatAndBlock, &ShouldNeverSeeIntrinsic, &DefinedClassVar,
        &DefinedInstanceVar, &NewIntrinsic,         &InstanceVariableGet,   &InstanceVariableSet,     &ClassIntrinsic,
        &BangIntrinsic,      &FreezeIntrinsic,      &IsAIntrinsic};
    for (auto &method : knownCMethods) {
        ret.emplace_back(&method);
    }
    for (auto &method : untypedSpecializations) {
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
