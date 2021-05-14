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
        auto &builder = builderCast(mcctx.build);
        auto *send = mcctx.send;
        auto sym = typeToSym(cs, send->args[0].type);
        auto attachedClass = sym.data(cs)->attachedClass(cs);

        if (attachedClass.data(cs)->name.isTEnumName(cs)) {
            // T::Enum classes like `class X$1 < MyEnum; end` are fake and for the type system only
            // (We don't define them at runtime, because classes are expensive compared to how many
            // individual enum values there are.)
            return Payload::rubyNil(cs, builder);
        }

        // this is wrong and will not work for `class <<self`
        auto classNameCStr = Payload::toCString(cs, IREmitterHelpers::showClassNameWithoutOwner(cs, sym), builder);
        auto isModule = sym.data(cs)->superClass() == core::Symbols::Module();
        auto funcSym = cs.gs.lookupStaticInitForClass(attachedClass);

        llvm::Value *module = nullptr;

        if (!IREmitterHelpers::isRootishSymbol(cs, sym.data(cs)->owner)) {
            auto getOwner = Payload::getRubyConstant(cs, sym.data(cs)->owner, builder);
            if (isModule) {
                module = builder.CreateCall(cs.getFunction("sorbet_defineNestedModule"), {getOwner, classNameCStr});
            } else {
                auto rawCall = Payload::getRubyConstant(cs, sym.data(cs)->superClass(), builder);
                module =
                    builder.CreateCall(cs.getFunction("sorbet_defineNestedClass"), {getOwner, classNameCStr, rawCall});
            }
        } else {
            if (isModule) {
                module = builder.CreateCall(cs.getFunction("sorbet_defineTopLevelModule"), {classNameCStr});
            } else {
                auto rawCall = Payload::getRubyConstant(cs, sym.data(cs)->superClass(), builder);
                module = builder.CreateCall(cs.getFunction("sorbet_defineTopClassOrModule"), {classNameCStr, rawCall});
            }
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
        auto &irctx = mcctx.irctx;
        auto rubyBlockId = mcctx.rubyBlockId;
        auto *send = mcctx.send;

        // TODO: this implementation generates code that is stupidly slow, we should be able to reuse instrinsics here
        // one day
        auto recv = send->args[0].variable;
        auto lit = core::cast_type_nonnull<core::LiteralType>(send->args[1].type);
        ENFORCE(lit.literalKind == core::LiteralType::LiteralTypeKind::Symbol);
        auto name = lit.asName(cs).shortName(cs);

        llvm::Value *blockHandler = nullptr;

        // If our current block has a block argument and we are passing it though, we don't have to reify it into a full
        // proc; we can set things up so the Ruby VM will pass the block argument along and avoid extra allocations.
        if (IREmitterHelpers::hasBlockArgument(cs, rubyBlockId, irctx.cfg.symbol, irctx) &&
            send->args[2].variable == irctx.rubyBlockArgs[rubyBlockId].back()) {
            blockHandler = Payload::getPassedBlockHandler(cs, mcctx.build);
        } else {
            // TODO(perf) `makeBlockHandlerProc` uses `to_proc` under the hood, and could be rewritten here to make an
            // inline cache.
            auto *block = Payload::varGet(cs, send->args[2].variable, mcctx.build, irctx, rubyBlockId);
            blockHandler = Payload::makeBlockHandlerProc(cs, mcctx.build, block);
        }

        auto *cache = IREmitterHelpers::pushSendArgs(mcctx, recv, string(name), 3);
        return Payload::callFuncWithCache(mcctx.cs, mcctx.build, cache, blockHandler);
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

    llvm::Value *offset = Payload::buildLocalsOffset(cs);

    auto fun = Payload::idIntern(cs, builder, send->fun.shortName(cs));
    return builder.CreateCall(cs.getFunction(cMethod), {recv, fun, argc, argv, blkPtr, offset}, "rawSendResult");
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
        return copy;
    }

    virtual InlinedVector<core::NameRef, 2> applicableMethods(CompilerState &cs) const override {
        return {core::Names::buildHash()};
    }
} BuildHash;

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
        auto *send = mcctx.send;

        auto recv = send->args[0].variable;

        auto lit = core::cast_type_nonnull<core::LiteralType>(send->args[1].type);
        ENFORCE(lit.literalKind == core::LiteralType::LiteralTypeKind::Symbol);
        auto methodName = lit.asName(cs).shortName(cs);

        auto &builder = builderCast(mcctx.build);

        auto *cfp = Payload::getCFPForBlock(cs, builder, irctx, mcctx.rubyBlockId);

        // Push receiver.
        Payload::pushRubyStack(cs, builder, cfp,
                               Payload::varGet(mcctx.cs, recv, mcctx.build, irctx, mcctx.rubyBlockId));

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
        auto splatArgsVar = send->args[2].variable;
        auto *splatArgs = Payload::varGet(mcctx.cs, splatArgsVar, mcctx.build, irctx, mcctx.rubyBlockId);

        auto kwArgsVar = send->args[3].variable;
        auto kwArgsType = send->args[3].type;

        // TODO(perf) we can avoid duplicating the array here if we know that it was created specifically for this
        // splat.
        llvm::Value *splatArray = builder.CreateCall(cs.getFunction("sorbet_arrayDup"), {splatArgs}, "splatArray");

        struct VMFlag flag;

        vector<VMFlag> flags;

        if (kwArgsType.derivesFrom(mcctx.cs, core::Symbols::NilClass())) {
            flags.emplace_back(Payload::VM_CALL_ARGS_SPLAT);
        } else if (auto *ptt = core::cast_type<core::TupleType>(kwArgsType)) {
            flags.emplace_back(Payload::VM_CALL_ARGS_SPLAT);
            flags.emplace_back(Payload::VM_CALL_KW_SPLAT);

            auto *kwArgArray = Payload::varGet(mcctx.cs, kwArgsVar, mcctx.build, irctx, mcctx.rubyBlockId);

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
            flags.emplace_back(Payload::VM_CALL_FCALL);
        }

        // Push the splat array.
        Payload::pushRubyStack(cs, builder, cfp, splatArray);

        // Call the receiver.
        auto *cache = IREmitterHelpers::makeInlineCache(cs, builder, std::string(methodName), flags, 1, {});

        if (mcctx.blk != nullptr) {
            auto *closure = Payload::buildLocalsOffset(cs);
            return Payload::callFuncBlockWithCache(mcctx.cs, mcctx.build, cache, mcctx.blk, closure);
        } else {
            auto *blockHandler = Payload::vmBlockHandlerNone(mcctx.cs, mcctx.build);
            return Payload::callFuncWithCache(mcctx.cs, mcctx.build, cache, blockHandler);
        }
    }
    virtual InlinedVector<core::NameRef, 2> applicableMethods(CompilerState &cs) const override {
        return {core::Names::callWithSplat()};
    }
} CallWithSplat;

static const vector<CallCMethod> knownCMethods{
    {"<expand-splat>", "sorbet_expandSplatIntrinsic", NoReceiver, Intrinsics::HandleBlock::Unhandled},
    {"<splat>", "sorbet_splatIntrinsic", NoReceiver, Intrinsics::HandleBlock::Unhandled},
    {"defined?", "sorbet_definedIntrinsic", NoReceiver, Intrinsics::HandleBlock::Unhandled},
    {"<build-keyword-args>", "sorbet_buildHashIntrinsic", NoReceiver, Intrinsics::HandleBlock::Unhandled},
    {"<build-array>", "sorbet_buildArrayIntrinsic", NoReceiver, Intrinsics::HandleBlock::Unhandled},
    {"<build-range>", "sorbet_buildRangeIntrinsic", NoReceiver, Intrinsics::HandleBlock::Unhandled},
    {"<string-interpolate>", "sorbet_stringInterpolate", NoReceiver, Intrinsics::HandleBlock::Unhandled},
    {"<self-new>", "sorbet_selfNew", NoReceiver, Intrinsics::HandleBlock::Unhandled},
    {"<block-break>", "sorbet_block_break", NoReceiver, Intrinsics::HandleBlock::Unhandled},
    {"!", "sorbet_bang", TakesReceiver, Intrinsics::HandleBlock::Unhandled},
    {"<check-match-array>", "sorbet_check_match_array", NoReceiver, Intrinsics::HandleBlock::Unhandled},

    // for kwsplat building
    {"<to-hash-dup>", "sorbet_magic_toHashDup", NoReceiver, Intrinsics::HandleBlock::Unhandled},
    {"<to-hash-nodup>", "sorbet_magic_toHashNoDup", NoReceiver, Intrinsics::HandleBlock::Unhandled},
    {"<merge-hash>", "sorbet_magic_mergeHash", NoReceiver, Intrinsics::HandleBlock::Unhandled},
    {"<merge-hash-values>", "sorbet_magic_mergeHashValues", NoReceiver, Intrinsics::HandleBlock::Unhandled},
};

vector<const NameBasedIntrinsicMethod *> computeNameBasedIntrinsics() {
    vector<const NameBasedIntrinsicMethod *> ret{&DoNothingIntrinsic, &DefineClassIntrinsic,   &IdentityIntrinsic,
                                                 &CallWithBlock,      &ExceptionRetry,         &BuildHash,
                                                 &CallWithSplat,      &ShouldNeverSeeIntrinsic};
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
