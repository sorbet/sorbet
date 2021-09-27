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
core::SymbolRef typeToSym(const core::GlobalState &gs, core::TypePtr typ) {
    core::SymbolRef sym;
    if (core::isa_type<core::ClassType>(typ)) {
        sym = core::cast_type_nonnull<core::ClassType>(typ).symbol;
    } else if (auto appliedType = core::cast_type<core::AppliedType>(typ)) {
        sym = appliedType->klass;
    } else {
        ENFORCE(false);
    }
    sym = IREmitterHelpers::fixupOwningSymbol(gs, sym);
    ENFORCE(sym.data(gs)->isClassOrModule());
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
        return {core::Names::keepForIde(), core::Names::keepForTypechecking()};
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
        return Payload::varGet(mcctx.cs, mcctx.send->args[0].variable, mcctx.builder, mcctx.irctx, mcctx.rubyBlockId);
    }
    virtual InlinedVector<core::NameRef, 2> applicableMethods(CompilerState &cs) const override {
        return {core::Names::suggestType()};
    }
} IdentityIntrinsic;

llvm::Value *prepareBlockHandler(MethodCallContext &mcctx, cfg::VariableUseSite &blkVar) {
    auto &cs = mcctx.cs;
    auto &irctx = mcctx.irctx;
    auto rubyBlockId = mcctx.rubyBlockId;

    // If our current block has a block argument and we are passing it though, we don't have to reify it into a full
    // proc; we can set things up so the Ruby VM will pass the block argument along and avoid extra allocations.
    if (IREmitterHelpers::hasBlockArgument(cs, rubyBlockId, irctx.cfg.symbol, irctx) &&
        blkVar.variable == irctx.rubyBlockArgs[rubyBlockId].back()) {
        return Payload::getPassedBlockHandler(cs, mcctx.builder);
    } else {
        // TODO(perf) `makeBlockHandlerProc` uses `to_proc` under the hood, and could be rewritten here to make an
        // inline cache.
        auto *block = Payload::varGet(cs, blkVar.variable, mcctx.builder, irctx, rubyBlockId);
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
        auto rubyBlockId = mcctx.rubyBlockId;
        auto *send = mcctx.send;

        // TODO: this implementation generates code that is stupidly slow, we should be able to reuse instrinsics here
        // one day
        auto recv = send->args[0].variable;
        auto lit = core::cast_type_nonnull<core::LiteralType>(send->args[1].type);
        ENFORCE(lit.literalKind == core::LiteralType::LiteralTypeKind::Symbol);
        auto name = lit.asName(cs).shortName(cs);

        llvm::Value *blockHandler = prepareBlockHandler(mcctx, send->args[2]);

        auto [stack, keywords, flags] = IREmitterHelpers::buildSendArgs(mcctx, recv, 3);
        auto &builder = mcctx.builder;
        auto *cfp = Payload::getCFPForBlock(cs, builder, irctx, rubyBlockId);
        Payload::pushRubyStackVector(cs, builder, cfp, Payload::varGet(cs, recv, builder, irctx, rubyBlockId), stack);
        auto *cache = IREmitterHelpers::makeInlineCache(cs, builder, string(name), flags, stack.size(), keywords);
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
    string_view rubyMethod;
    string cMethod;
    ShouldTakeReceiver takesReceiver;
    core::ClassOrModuleRef klass;

public:
    CallCMethod(string_view rubyMethod, string cMethod, ShouldTakeReceiver takesReceiver,
                Intrinsics::HandleBlock supportsBlocks, core::ClassOrModuleRef klass = core::ClassOrModuleRef{})
        : NameBasedIntrinsicMethod(supportsBlocks), rubyMethod(rubyMethod), cMethod(cMethod),
          takesReceiver(takesReceiver), klass(klass){};

    virtual llvm::Value *makeCall(MethodCallContext &mcctx) const override {
        return buildCMethodCall(mcctx, cMethod, takesReceiver, klass);
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
        if (core::isa_type<core::LiteralType>(t)) {
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
        llvm::Constant *indices[] = {zero};

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
                    llvm::Value *argIndices[] = {llvm::ConstantInt::get(mcctx.cs, llvm::APInt(32, 0, true)),
                                                 llvm::ConstantInt::get(mcctx.cs, llvm::APInt(64, i, true))};
                    llvm::Value *val = IREmitterHelpers::emitLiteralish(mcctx.cs, globalInitBuilder, v.type);
                    globalInitBuilder.CreateStore(
                        val, globalInitBuilder.CreateGEP(argArray, argIndices, fmt::format("hashArgs{}Addr", i)));
                }

                llvm::Value *argIndices[] = {llvm::ConstantInt::get(mcctx.cs, llvm::APInt(64, 0, true)),
                                             llvm::ConstantInt::get(mcctx.cs, llvm::APInt(64, 0, true))};
                auto hashValue = globalInitBuilder.CreateCall(
                    mcctx.cs.getFunction("sorbet_literalHashBuild"),
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

        auto *index = builder.CreateLoad(
            llvm::ConstantExpr::getInBoundsGetElementPtr(globalDeclaration->getValueType(), globalDeclaration, indices),
            "hashLiteral");
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
    auto *splatArgs = Payload::varGet(mcctx.cs, splatArgsVar.variable, builder, irctx, mcctx.rubyBlockId);

    // TODO(perf) we can avoid duplicating the array here if we know that it was created specifically for this
    // splat.
    llvm::Value *splatArray = builder.CreateCall(cs.getFunction("sorbet_arrayDup"), {splatArgs}, "splatArray");

    CallCacheFlags flags;

    if (kwArgsVar.type.derivesFrom(mcctx.cs, core::Symbols::NilClass())) {
        flags.args_splat = true;
    } else if (auto *ptt = core::cast_type<core::TupleType>(kwArgsVar.type)) {
        flags.args_splat = true;
        flags.kw_splat = true;

        auto *kwArgArray = Payload::varGet(mcctx.cs, kwArgsVar.variable, mcctx.builder, irctx, mcctx.rubyBlockId);

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
                failCompilation(cs, core::Loc(irctx.cfg.file, send->receiverLoc),
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
        failCompilation(cs, core::Loc(irctx.cfg.file, send->receiverLoc),
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
        auto *cfp = Payload::getCFPForBlock(cs, builder, irctx, mcctx.rubyBlockId);

        auto [flags, splatArray] = prepareSplatArgs(mcctx, send->args[2], send->args[3]);

        // setup the inline cache
        auto lit = core::cast_type_nonnull<core::LiteralType>(send->args[1].type);
        ENFORCE(lit.literalKind == core::LiteralType::LiteralTypeKind::Symbol);
        auto methodName = lit.asName(cs).shortName(cs);
        auto *cache = IREmitterHelpers::makeInlineCache(cs, builder, std::string(methodName), flags, 1, {});

        // Push receiver and the splat array.
        // For the receiver, we can't use MethodCallContext::varGetRecv here because the real receiver
        // is actually the first arg of the callWithSplat intrinsic method.
        Payload::pushRubyStackVector(
            cs, builder, cfp, Payload::varGet(mcctx.cs, recv, mcctx.builder, irctx, mcctx.rubyBlockId), {splatArray});

        // Call the receiver.
        if (auto *blk = mcctx.blkAsFunction()) {
            auto *closure = Payload::buildLocalsOffset(cs);
            auto arity = irctx.rubyBlockArity[mcctx.blk.value()];
            auto usesBreak = irctx.blockUsesBreak[mcctx.blk.value()];
            return Payload::callFuncBlockWithCache(mcctx.cs, mcctx.builder, cache, usesBreak, blk, arity.min, arity.max,
                                                   closure);
        } else {
            auto *blockHandler = Payload::vmBlockHandlerNone(mcctx.cs, mcctx.builder);
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
        auto *blockHandler = prepareBlockHandler(mcctx, send->args[4]);

        // setup the inline cache
        auto lit = core::cast_type_nonnull<core::LiteralType>(send->args[1].type);
        ENFORCE(lit.literalKind == core::LiteralType::LiteralTypeKind::Symbol);
        auto methodName = lit.asName(cs).shortName(cs);
        auto *cache = IREmitterHelpers::makeInlineCache(cs, builder, std::string(methodName), flags, 1, {});

        // Push receiver and the splat array.
        // For the receiver, we can't use MethodCallContext::varGetRecv here because the real receiver
        // is actually the first arg of the callWithSplat intrinsic method.
        auto *cfp = Payload::getCFPForBlock(cs, builder, irctx, mcctx.rubyBlockId);
        Payload::pushRubyStackVector(
            cs, builder, cfp, Payload::varGet(mcctx.cs, recv, mcctx.builder, irctx, mcctx.rubyBlockId), {splatArray});

        return Payload::callFuncWithCache(mcctx.cs, mcctx.builder, cache, blockHandler);
    }

    virtual InlinedVector<core::NameRef, 2> applicableMethods(CompilerState &cs) const override {
        return {core::Names::callWithSplatAndBlock()};
    }
} CallWithSplatAndBlock;

class DefinedClassVar : public NameBasedIntrinsicMethod {
public:
    DefinedClassVar() : NameBasedIntrinsicMethod{Intrinsics::HandleBlock::Unhandled} {};

    virtual llvm::Value *makeCall(MethodCallContext &mcctx) const override {
        auto &cs = mcctx.cs;
        auto &builder = mcctx.builder;
        auto &irctx = mcctx.irctx;
        int rubyBlockId = mcctx.rubyBlockId;

        auto *klass = Payload::getClassVariableStoreClass(cs, builder, irctx);
        // TODO(froydnj): figure out how to access the ID of the argument directly.
        auto *var = Payload::varGet(cs, mcctx.send->args[0].variable, builder, irctx, rubyBlockId);
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
        int rubyBlockId = mcctx.rubyBlockId;

        auto *self = Payload::varGet(cs, cfg::LocalRef::selfVariable(), builder, irctx, rubyBlockId);
        // TODO(froydnj): figure out how to access the ID of the argument directly.
        auto *var = Payload::varGet(cs, mcctx.send->args[0].variable, builder, irctx, rubyBlockId);
        return builder.CreateCall(cs.getFunction("sorbet_instanceVariableDefined"), {self, var}, "is_cvar_defined");
    }

    virtual InlinedVector<core::NameRef, 2> applicableMethods(CompilerState &cs) const override {
        return {core::Names::definedInstanceVar()};
    }
} DefinedInstanceVar;

static const vector<CallCMethod> knownCMethods{
    {"<expand-splat>", "sorbet_expandSplatIntrinsic", NoReceiver, Intrinsics::HandleBlock::Unhandled,
     core::Symbols::Array()},
    {"<splat>", "sorbet_splatIntrinsic", NoReceiver, Intrinsics::HandleBlock::Unhandled, core::Symbols::Array()},
    {"defined?", "sorbet_definedIntrinsic", NoReceiver, Intrinsics::HandleBlock::Unhandled},
    {"<build-keyword-args>", "sorbet_buildHashIntrinsic", NoReceiver, Intrinsics::HandleBlock::Unhandled,
     core::Symbols::Hash()},
    {"<build-array>", "sorbet_buildArrayIntrinsic", NoReceiver, Intrinsics::HandleBlock::Unhandled,
     core::Symbols::Array()},
    {"<build-range>", "sorbet_buildRangeIntrinsic", NoReceiver, Intrinsics::HandleBlock::Unhandled,
     core::ClassOrModuleRef()},
    {"<string-interpolate>", "sorbet_stringInterpolate", NoReceiver, Intrinsics::HandleBlock::Unhandled,
     core::Symbols::String()},
    {"<self-new>", "sorbet_selfNew", NoReceiver, Intrinsics::HandleBlock::Unhandled},
    {"<block-break>", "sorbet_block_break", NoReceiver, Intrinsics::HandleBlock::Unhandled},
    {"!", "sorbet_bang", TakesReceiver, Intrinsics::HandleBlock::Unhandled},
    {"nil?", "sorbet_nil_p", TakesReceiver, Intrinsics::HandleBlock::Unhandled},
    {"<check-match-array>", "sorbet_check_match_array", NoReceiver, Intrinsics::HandleBlock::Unhandled,
     core::ClassOrModuleRef()},

    // for kwsplat building
    {"<to-hash-dup>", "sorbet_magic_toHashDup", NoReceiver, Intrinsics::HandleBlock::Unhandled, core::Symbols::Hash()},
    {"<to-hash-nodup>", "sorbet_magic_toHashNoDup", NoReceiver, Intrinsics::HandleBlock::Unhandled,
     core::Symbols::Hash()},
    {"<merge-hash>", "sorbet_magic_mergeHash", NoReceiver, Intrinsics::HandleBlock::Unhandled, core::Symbols::Hash()},
    {"<merge-hash-values>", "sorbet_magic_mergeHashValues", NoReceiver, Intrinsics::HandleBlock::Unhandled,
     core::Symbols::Hash()},
};

vector<const NameBasedIntrinsicMethod *> computeNameBasedIntrinsics() {
    vector<const NameBasedIntrinsicMethod *> ret{&DoNothingIntrinsic, &DefineClassIntrinsic,  &IdentityIntrinsic,
                                                 &CallWithBlock,      &ExceptionRetry,        &BuildHash,
                                                 &CallWithSplat,      &CallWithSplatAndBlock, &ShouldNeverSeeIntrinsic,
                                                 &DefinedClassVar,    &DefinedInstanceVar};
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
