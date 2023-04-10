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
#include "compiler/IREmitter/Payload.h"
#include "compiler/IREmitter/SymbolBasedIntrinsicMethod.h"
#include <optional>
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

class CMethod final {
public:
    string cMethod;
    core::ClassOrModuleRef resultType;

    CMethod(string cMethod, core::ClassOrModuleRef resultType = core::Symbols::noClassOrModule())
        : cMethod{cMethod}, resultType{resultType} {}

    llvm::Function *getFunction(CompilerState &cs) const {
        return cs.module->getFunction(cMethod);
    }

    void assertResultType(CompilerState &cs, llvm::IRBuilderBase &build, llvm::Value *res) const {
        if (resultType.exists()) {
            Payload::assumeType(cs, build, res, resultType);
        }
    }

    void sanityCheck(const core::GlobalState &gs, core::MethodRef primaryMethod) const {
        if (resultType.exists()) {
            auto intrinsicResultType = resultType.data(gs)->externalType();

            // We can only reasonably add type assertions for methods that have signatures
            ENFORCE(primaryMethod.data(gs)->hasSig());

            // test all overloads to see if we can find a sig that produces this type
            if (core::Types::isSubType(gs, intrinsicResultType, primaryMethod.data(gs)->resultType)) {
                return;
            }

            int i = 0;
            auto methodName = primaryMethod.data(gs)->name;
            auto current = primaryMethod;
            while (current.data(gs)->flags.isOverloaded) {
                i++;
                auto overloadName = gs.lookupNameUnique(core::UniqueNameKind::Overload, methodName, i);
                auto overload = primaryMethod.data(gs)->owner.data(gs)->findMethod(gs, overloadName);
                ENFORCE(overload.exists());
                if (core::Types::isSubType(gs, intrinsicResultType, overload.data(gs)->resultType)) {
                    return;
                }

                current = overload;
            }

            ENFORCE(false, "The method `{}` (or an overload) does not return `{}`", primaryMethod.show(gs),
                    intrinsicResultType.show(gs));
        }
    }
};

class CallCMethod : public SymbolBasedIntrinsicMethod {
protected:
    core::ClassOrModuleRef rubyClass;
    string_view rubyMethod;
    CMethod cMethod;
    optional<CMethod> cMethodWithBlock;
    vector<KnownFunction> expectedRubyCFuncs;

private:
    // Generate a one-off function that looks like the following:
    //
    // > define VALUE @<fresh-name>(VALUE %env) {
    // >     VALUE %res = call @sorbet_inlineIntrinsicEnv_apply(%env, @cMethod, %blkArg)
    // >     ret VALUE %res
    // > }
    llvm::Function *generateForwarder(MethodCallContext &mcctx) const {
        auto &cs = mcctx.cs;
        auto &builder = mcctx.builder;

        // function signature
        auto linkage = llvm::Function::InternalLinkage;
        llvm::Twine name{"forward_" + llvm::Twine{cMethodWithBlock->cMethod}};
        auto *fn = llvm::Function::Create(cs.getInlineForwarderType(), linkage, name, cs.module);
        auto *env = fn->arg_begin();

        // function body
        auto *entry = llvm::BasicBlock::Create(cs, "entry", fn);
        auto ip = builder.saveIP();
        builder.SetInsertPoint(entry);
        auto *cfunc = cMethodWithBlock->getFunction(cs);
        auto *blk = mcctx.blkAsFunction();
        ENFORCE(blk != nullptr);
        ENFORCE(mcctx.blk.has_value());
        int blockRubyBlockId = *mcctx.blk;
        const auto &argsFlags = mcctx.irctx.blockLinks[blockRubyBlockId]->argFlags;
        int maxPositionalArgs = 0;
        for (auto &flag : argsFlags) {
            if (flag.isKeyword) {
                continue;
            }
            if (flag.isRepeated) {
                continue;
            }
            if (flag.isDefault) {
                maxPositionalArgs += 1;
                continue;
            }
            if (flag.isBlock) {
                continue;
            }
            maxPositionalArgs += 1;
        }
        auto *result =
            builder.CreateCall(cs.module->getFunction("sorbet_inlineIntrinsicEnv_apply"),
                               {env, cfunc, blk, IREmitterHelpers::buildS4(cs, maxPositionalArgs)}, "result");
        builder.CreateRet(result);
        builder.restoreIP(ip);

        return fn;
    }

public:
    CallCMethod(core::ClassOrModuleRef rubyClass, string_view rubyMethod, CMethod cMethod,
                optional<CMethod> cMethodWithBlock = nullopt, vector<KnownFunction> expectedRubyCFuncs = {})
        : SymbolBasedIntrinsicMethod(cMethodWithBlock.has_value() ? Intrinsics::HandleBlock::Handled
                                                                  : Intrinsics::HandleBlock::Unhandled),
          rubyClass(rubyClass), rubyMethod(rubyMethod), cMethod(cMethod), cMethodWithBlock(cMethodWithBlock),
          expectedRubyCFuncs(expectedRubyCFuncs){};

    virtual llvm::Value *makeCall(MethodCallContext &mcctx) const override {
        auto &cs = mcctx.cs;
        auto &builder = mcctx.builder;
        auto *send = mcctx.send;

        auto *recv = mcctx.varGetRecv();
        auto *id = Payload::idIntern(cs, builder, send->fun.shortName(cs));
        auto *offset = Payload::buildLocalsOffset(cs);

        // kwsplat is used by the vm only, and we don't use the vm's api for calling an intrinsic directly.
        auto args = IREmitterHelpers::fillSendArgArray(mcctx);

        llvm::Value *res{nullptr};
        if (auto *blk = mcctx.blkAsFunction()) {
            if (!cMethodWithBlock.has_value()) {
                core::Loc loc{mcctx.cs.file, send->argLocs.back()};
                failCompilation(cs, loc, "Unable to handle a block with this intrinsic");
            }
            auto *forwarder = generateForwarder(mcctx);

            auto blkId = mcctx.blk.value();

            // NOTE: The ruby stack doesn't need to be managed here because the known c intrinsics don't expect to be
            // called by the vm.
            bool usesBreak = mcctx.irctx.blockUsesBreak[blkId];
            auto *blkIfunc = Payload::getOrBuildBlockIfunc(cs, builder, mcctx.irctx, blkId);
            if (usesBreak) {
                res = builder.CreateCall(cs.module->getFunction("sorbet_callIntrinsicInlineBlock"),
                                         {forwarder, recv, id, args.argc, args.argv, blkIfunc, offset},
                                         "rawSendResultWithBlock");
            } else {
                // Since the block doesn't use break we can make two optimizations:
                //
                // 1. Use the version of sorbet_callIntrinsicInlineBlock that doesn't use rb_iterate and will inline
                //    better
                // 2. Emit a type assertion on the result of the function, as we know that there won't be non-local
                //    control flow based on the use of `break` that could change the type of the returned value
                res = builder.CreateCall(cs.module->getFunction("sorbet_callIntrinsicInlineBlock_noBreak"),
                                         {forwarder, recv, id, args.argc, args.argv, blkIfunc, offset},
                                         "rawSendResultWithBlock");
                cMethodWithBlock->assertResultType(cs, builder, res);
            }
        } else {
            auto *blkPtr = llvm::ConstantPointerNull::get(cs.getRubyBlockFFIType()->getPointerTo());
            res = builder.CreateCall(cMethod.getFunction(cs), {recv, id, args.argc, args.argv, blkPtr, offset},
                                     "rawSendResult");
            cMethod.assertResultType(cs, builder, res);
        }

        return res;
    };

    virtual InlinedVector<core::ClassOrModuleRef, 2> applicableClasses(const core::GlobalState &gs) const override {
        return {rubyClass};
    };
    virtual InlinedVector<core::NameRef, 2> applicableMethods(const core::GlobalState &gs) const override {
        return {gs.lookupNameUTF8(rubyMethod)};
    };
    virtual llvm::Value *receiverFastPathTest(MethodCallContext &mcctx,
                                              core::ClassOrModuleRef potentialClass) const override {
        if (!this->expectedRubyCFuncs.empty()) {
            return IREmitterHelpers::receiverFastPathTestWithCache(mcctx, this->expectedRubyCFuncs, string(rubyMethod));
        } else {
            return SymbolBasedIntrinsicMethod::receiverFastPathTest(mcctx, potentialClass);
        }
    }

    virtual void sanityCheck(const core::GlobalState &gs) const override {
        CallCMethod::sanityCheckInternal(gs, this->rubyClass, *this);
    }

protected:
    static void sanityCheckInternal(const core::GlobalState &gs, core::ClassOrModuleRef klass,
                                    const CallCMethod &call) {
        auto methodName = gs.lookupNameUTF8(call.rubyMethod);
        ENFORCE(methodName.exists());

        auto methodSym = klass.data(gs)->findMemberTransitive(gs, methodName);
        ENFORCE(methodSym.exists());

        auto primaryMethod = methodSym.asMethodRef();
        ENFORCE(primaryMethod.exists());

        call.cMethod.sanityCheck(gs, primaryMethod);
        if (call.cMethodWithBlock.has_value()) {
            call.cMethodWithBlock->sanityCheck(gs, primaryMethod);
        }

        // Determine if the primary, or any overload, accepts a block argument.
        int i = 0;
        bool acceptsBlock = false;
        auto current = primaryMethod;
        while (current.data(gs)->flags.isOverloaded) {
            const auto &args = current.data(gs)->arguments;
            if (!args.empty() && !args.back().isSyntheticBlockArgument()) {
                acceptsBlock = true;
                break;
            }

            i++;
            auto overloadName = gs.lookupNameUnique(core::UniqueNameKind::Overload, methodName, i);
            auto overloadSym = primaryMethod.data(gs)->owner.data(gs)->findMember(gs, overloadName);
            ENFORCE(overloadSym.exists());

            current = overloadSym.asMethodRef();
            ENFORCE(current.exists());
        }

        ENFORCE(!acceptsBlock || call.cMethodWithBlock.has_value(),
                "the intrinsic for `{}` needs to have a block variant added", primaryMethod.show(gs));
    }
};

void emitParamInitialization(CompilerState &cs, llvm::IRBuilderBase &builder, const IREmitterContext &irctx,
                             core::MethodRef funcSym, int rubyRegionId, llvm::Value *param) {
    // Following the comment in vm_core.h:
    // https://github.com/ruby/ruby/blob/344a824ef9d4b6152703d02d7ffa042abd4252c1/vm_core.h#L321-L342
    // Comment reproduced here to make things somewhat easier to follow.

    /*
     * parameter information
     *
     *  def m(a1, a2, ..., aM,                    # mandatory
     *        b1=(...), b2=(...), ..., bN=(...),  # optional
     *        *c,                                 # rest
     *        d1, d2, ..., dO,                    # post
     *        e1:(...), e2:(...), ..., eK:(...),  # keyword
     *        **f,                                # keyword_rest
     *        &g)                                 # block
     * =>
     *
     *  lead_num     = M
     *  opt_num      = N
     *  rest_start   = M+N
     *  post_start   = M+N+(*1)
     *  post_num     = O
     *  keyword_num  = K
     *  block_start  = M+N+(*1)+O+K
     *  keyword_bits = M+N+(*1)+O+K+(&1)
     *  size         = M+N+O+(*1)+K+(&1)+(**1) // parameter size.
     */

    int leadNum = 0;    // # of required arguments (M)
    int optNum = 0;     // # of optional arguments (N)
    int restStart = 0;  // M + N
    int postStart = 0;  // M + N + 1
    int postNum = 0;    // # of required arguments after rest (O)
    int kwNum = 0;      // # of keyword argments (K)
    int blockStart = 0; // M + N + 1 + O + K
    int reqKwNum = 0;   // # of required keyword arguments
    bool hasRest = false;
    bool hasPost = false;
    bool hasKw = false;
    bool hasKwRest = false;
    bool hasBlock = false;

    InlinedVector<const core::ArgInfo *, 4> nonKeywordArgInfo;
    InlinedVector<const core::ArgInfo *, 4> keywordArgInfo;

    int i = -1;
    for (auto &argInfo : funcSym.data(cs)->arguments) {
        ++i;
        auto &flags = argInfo.flags;
        if (flags.isBlock) {
            if (argInfo.loc.exists()) {
                hasBlock = true;
                blockStart = nonKeywordArgInfo.size();
                nonKeywordArgInfo.emplace_back(&argInfo);
            }
        } else if (flags.isKeyword) {
            if (flags.isRepeated) {
                hasKwRest = true;
            } else {
                hasKw = true;
                kwNum++;

                if (!flags.isDefault) {
                    reqKwNum++;
                }
            }
            keywordArgInfo.emplace_back(&argInfo);
        } else if (flags.isRepeated) {
            hasRest = true;
            restStart = i;
            nonKeywordArgInfo.emplace_back(&argInfo);
        } else if (flags.isDefault) {
            optNum++;
            nonKeywordArgInfo.emplace_back(&argInfo);
        } else {
            if (hasRest) {
                // This is the first post-rest required argument we have seen.
                if (postNum == 0) {
                    hasPost = true;
                    postStart = i;
                }
                postNum++;
            } else {
                leadNum++;
            }
            nonKeywordArgInfo.emplace_back(&argInfo);
        }
    }

    // Construct all the necessary LLVM values to make the call.  We name them
    // according to what arguments they correspond to on the C side.

    // Flags structure.
    auto *has_lead = builder.getInt1(leadNum != 0);
    auto *has_opt = builder.getInt1(optNum != 0);
    auto *has_rest = builder.getInt1(hasRest);
    auto *has_post = builder.getInt1(hasPost);
    auto *has_kw = builder.getInt1(hasKw);
    auto *has_kwrest = builder.getInt1(hasKwRest);
    auto *has_block = builder.getInt1(hasBlock);
    // TODO: Sorbet doesn't supply enough information to be able to track this correctly:
    // it is not !(has_kw || has_kwrest) but whether **nil was supplied in the arglist
    auto *accepts_no_kwarg = builder.getInt1(false);

    // Fields according to the diagram above.
    auto *lead_num = IREmitterHelpers::buildS4(cs, leadNum);
    auto *opt_num = IREmitterHelpers::buildS4(cs, optNum);
    auto *rest_start = IREmitterHelpers::buildS4(cs, restStart);
    auto *post_start = IREmitterHelpers::buildS4(cs, postStart);
    auto *post_num = IREmitterHelpers::buildS4(cs, postNum);
    auto *block_start = IREmitterHelpers::buildS4(cs, blockStart);
    auto *size = IREmitterHelpers::buildU4(cs, leadNum + optNum + hasRest + postNum + hasBlock + kwNum + hasKwRest);

    builder.CreateCall(cs.getFunction("sorbet_setParamInfo"),
                       {param, has_lead, has_opt, has_rest, has_post, has_kw, has_kwrest, has_block, accepts_no_kwarg,
                        lead_num, opt_num, rest_start, post_start, post_num, block_start, size});

    if (!nonKeywordArgInfo.empty()) {
        // Create a table for all the positional argument IDs.
        auto *table = builder.CreateAlloca(llvm::Type::getInt64Ty(cs),
                                           IREmitterHelpers::buildS4(cs, nonKeywordArgInfo.size()), "positional_table");

        int i = -1;
        for (auto info : nonKeywordArgInfo) {
            ++i;
            auto *id = Payload::idIntern(cs, builder, info->argumentName(cs));
            builder.CreateStore(id, builder.CreateConstGEP1_32(table, i));
        }

        auto *tableSize = IREmitterHelpers::buildS4(cs, nonKeywordArgInfo.size());
        builder.CreateCall(cs.getFunction("sorbet_setupParamPositional"), {param, tableSize, table});
    }

    if (hasKw || hasKwRest) {
        ENFORCE(kwNum > 0 || hasKwRest);
        ENFORCE(reqKwNum <= kwNum);
        ENFORCE(keywordArgInfo.size() == (kwNum + hasKwRest));

        // Create a table for all the keyword argument IDs.
        auto *table = builder.CreateAlloca(llvm::Type::getInt64Ty(cs),
                                           IREmitterHelpers::buildS4(cs, keywordArgInfo.size()), "keyword_table");

        int i = -1;
        for (auto info : keywordArgInfo) {
            ++i;
            auto *id = Payload::idIntern(cs, builder, info->argumentName(cs));
            builder.CreateStore(id, builder.CreateConstGEP1_32(table, i));
        }

        auto *kw_num = IREmitterHelpers::buildS4(cs, kwNum);
        auto *required_num = IREmitterHelpers::buildS4(cs, reqKwNum);
        auto *tableSize = IREmitterHelpers::buildS4(cs, keywordArgInfo.size());

        builder.CreateCall(cs.getFunction("sorbet_setupParamKeywords"),
                           {param, kw_num, required_num, tableSize, table});
    }
}

// TODO(froydnj): we need to do something like this for blocks as well.
llvm::Value *buildParamInfo(CompilerState &cs, llvm::IRBuilderBase &builder, const IREmitterContext &irctx,
                            core::MethodRef funcSym, int rubyRegionId) {
    auto *paramInfo = builder.CreateCall(cs.getFunction("sorbet_allocateParamInfo"), {}, "parameterInfo");

    emitParamInitialization(cs, builder, irctx, funcSym, rubyRegionId, paramInfo);

    return paramInfo;
}

class DefineMethodIntrinsic : public SymbolBasedIntrinsicMethod {
public:
    DefineMethodIntrinsic() : SymbolBasedIntrinsicMethod(Intrinsics::HandleBlock::Unhandled){};
    virtual llvm::Value *makeCall(MethodCallContext &mcctx) const override {
        auto &cs = mcctx.cs;
        auto &builder = mcctx.builder;
        auto *send = mcctx.send;

        bool isSelf = send->fun == core::Names::keepSelfDef();

        ENFORCE(send->args.size() == 3, "Invariant established by rewriter/Flatten.cc");

        // First arg: define method on what
        auto ownerSym = typeToSym(cs, send->args[0].type);
        llvm::Value *klass;
        // If we're defining the method on `T.class_of(T.class_of(X))`, we need to
        // programatically access the class, rather than letting getRubyConstant do
        // that work for us.
        if (ownerSym.data(cs)->isSingletonClass(cs)) {
            auto attachedClass = ownerSym.data(cs)->attachedClass(cs);
            ENFORCE(attachedClass.exists());
            if (attachedClass.data(cs)->isSingletonClass(cs)) {
                klass = Payload::getRubyConstant(cs, attachedClass, builder);
                klass = builder.CreateCall(cs.getFunction("sorbet_singleton_class"), {klass}, "singletonClass");
            } else {
                klass = Payload::getRubyConstant(cs, ownerSym, builder);
            }
        } else {
            klass = Payload::getRubyConstant(cs, ownerSym, builder);
        }

        // Second arg: name of method to define
        auto litName = core::cast_type_nonnull<core::NamedLiteralType>(send->args[1].type);
        ENFORCE(litName.literalKind == core::NamedLiteralType::LiteralTypeKind::Symbol);
        auto funcNameRef = litName.asName();
        auto name = Payload::toCString(cs, funcNameRef.show(cs), builder);

        // Third arg: method kind (normal, attr_reader, or genericPropGetter)
        auto litMethodKind = core::cast_type_nonnull<core::NamedLiteralType>(send->args[2].type);
        ENFORCE(litMethodKind.literalKind == core::NamedLiteralType::LiteralTypeKind::Symbol);
        auto methodKind = litMethodKind.asName();

        auto lookupSym = isSelf ? ownerSym : ownerSym.data(cs)->attachedClass(cs);
        if (ownerSym == core::Symbols::Object() && !isSelf) {
            // TODO Figure out if this speicial case is right
            lookupSym = core::Symbols::Object();
        }
        auto funcSym = lookupSym.data(cs)->findMethod(cs, funcNameRef);
        ENFORCE(funcSym.exists());

        // We are going to rely on compiled final methods having their return values checked.
        const bool needsTypechecking = funcSym.data(cs)->flags.isFinal;

        if (methodKind == core::Names::attrReader() && !needsTypechecking) {
            const char *payloadFuncName = isSelf ? "sorbet_defineIvarMethodSingleton" : "sorbet_defineIvarMethod";
            auto payloadFunc = cs.getFunction(payloadFuncName);

            builder.CreateCall(payloadFunc, {klass, name});
        } else {
            const bool isPropGetter = methodKind == core::Names::genericPropGetter();

            ENFORCE(methodKind == core::Names::normal() || isPropGetter ||
                        (methodKind == core::Names::attrReader() && needsTypechecking),
                    "Unknown method kind: {}", methodKind.show(cs));

            if (isPropGetter) {
                ENFORCE(!isSelf);
            }
            auto funcHandle = IREmitterHelpers::getOrCreateFunction(cs, funcSym);
            auto *stackFrameVar = Payload::rubyStackFrameVar(cs, builder, mcctx.irctx, funcSym);
            auto *stackFrame = builder.CreateLoad(stackFrameVar, "stackFrame");

            // If a prop getter doesn't necessarily need to be typechecked, then we can
            // decide at runtime whether to use the fully-general compiled version of
            // the getter or whether to use a fast attr_reader-based version.
            const char *payloadFuncName = isSelf                                 ? "sorbet_defineMethodSingleton"
                                          : (isPropGetter && !needsTypechecking) ? "sorbet_definePropGetter"
                                                                                 : "sorbet_defineMethod";
            auto rubyFunc = cs.getFunction(payloadFuncName);
            auto *paramInfo = buildParamInfo(cs, builder, mcctx.irctx, funcSym, mcctx.rubyRegionId);
            builder.CreateCall(rubyFunc, {klass, name, funcHandle, paramInfo, stackFrame});

            builder.CreateCall(IREmitterHelpers::getInitFunction(cs, funcSym), {});
        }

        // Return the symbol of the method name even if we don't emit a definition. This will be a problem if there are
        // meta-progrmaming methods applied to an abstract method definition, see
        // https://github.com/stripe/sorbet_llvm/issues/115 for more information.
        return Payload::varGet(cs, send->args[1].variable, builder, mcctx.irctx, mcctx.rubyRegionId);
    }

    virtual bool skipFastPathTest(MethodCallContext &mcctx, core::ClassOrModuleRef potentialClass) const override {
        return true;
    }
    virtual bool needsAfterIntrinsicProcessing() const override {
        return false;
    }
    virtual InlinedVector<core::ClassOrModuleRef, 2> applicableClasses(const core::GlobalState &gs) const override {
        return {core::Symbols::Sorbet_Private_Static().data(gs)->lookupSingletonClass(gs)};
    };
    virtual InlinedVector<core::NameRef, 2> applicableMethods(const core::GlobalState &gs) const override {
        return {core::Names::keepDef(), core::Names::keepSelfDef()};
    }
} DefineMethodIntrinsic;

class SorbetPrivateStaticResolvedSigIntrinsic : public SymbolBasedIntrinsicMethod {
public:
    SorbetPrivateStaticResolvedSigIntrinsic() : SymbolBasedIntrinsicMethod(Intrinsics::HandleBlock::Handled){};
    virtual llvm::Value *makeCall(MethodCallContext &mcctx) const override {
        auto &cs = mcctx.cs;
        auto &irctx = mcctx.irctx;
        auto &builder = mcctx.builder;
        auto *send = mcctx.send;
        auto rubyRegionId = mcctx.rubyRegionId;
        // args[0] = originalRecv
        // args[1] = arg to sig, if present
        // args[2] = isSelf
        // args[3] = methodName
        // args[4..] = originalArgs, if present
        //
        // Do nothing for non-self calls (e.g. T::Sig::WithoutRuntime.sig)
        if (send->args[0].variable != cfg::LocalRef::selfVariable()) {
            return Payload::rubyNil(mcctx.cs, builder);
        }

        ENFORCE(mcctx.blkAsFunction() != nullptr);
        llvm::Value *originalRecv = Payload::varGet(cs, send->args[0].variable, builder, irctx, rubyRegionId);
        llvm::Value *sigArg;
        cfg::VariableUseSite *remainingArgs;
        if (send->args.size() > 3) {
            sigArg = Payload::varGet(cs, send->args[1].variable, builder, irctx, rubyRegionId);
            remainingArgs = &send->args[2];
        } else {
            sigArg = Payload::rubyNil(cs, builder);
            remainingArgs = &send->args[1];
        }
        llvm::Value *isSelf = Payload::varGet(cs, remainingArgs[0].variable, builder, irctx, rubyRegionId);
        llvm::Value *methodName = Payload::varGet(cs, remainingArgs[1].variable, builder, irctx, rubyRegionId);
        builder.CreateCall(cs.getFunction("sorbet_vm_register_sig"),
                           {isSelf, methodName, originalRecv, sigArg, mcctx.blkAsFunction()});
        return Payload::rubyNil(mcctx.cs, builder);
    }

    virtual bool skipFastPathTest(MethodCallContext &mcctx, core::ClassOrModuleRef potentialClass) const override {
        return true;
    }
    virtual bool needsAfterIntrinsicProcessing() const override {
        return false;
    }
    virtual InlinedVector<core::ClassOrModuleRef, 2> applicableClasses(const core::GlobalState &gs) const override {
        return {core::Symbols::Sorbet_Private_Static_ResolvedSig().data(gs)->lookupSingletonClass(gs)};
    };
    virtual InlinedVector<core::NameRef, 2> applicableMethods(const core::GlobalState &gs) const override {
        return {core::Names::sig()};
    }
} SorbetPrivateStaticResolvedSigIntrinsic;

class SorbetPrivateStaticSigIntrinsic : public SymbolBasedIntrinsicMethod {
public:
    SorbetPrivateStaticSigIntrinsic() : SymbolBasedIntrinsicMethod(Intrinsics::HandleBlock::Handled){};
    virtual llvm::Value *makeCall(MethodCallContext &mcctx) const override {
        auto &builder = mcctx.builder;
        return Payload::rubyNil(mcctx.cs, builder);
    }

    virtual bool skipFastPathTest(MethodCallContext &mcctx, core::ClassOrModuleRef potentialClass) const override {
        return true;
    }
    virtual bool needsAfterIntrinsicProcessing() const override {
        return false;
    }
    virtual InlinedVector<core::ClassOrModuleRef, 2> applicableClasses(const core::GlobalState &gs) const override {
        return {core::Symbols::Sorbet_Private_Static().data(gs)->lookupSingletonClass(gs)};
    };
    virtual InlinedVector<core::NameRef, 2> applicableMethods(const core::GlobalState &gs) const override {
        return {core::Names::sig()};
    }
} SorbetPrivateStaticSigIntrinsic;

/* Reuse logic from typeTest to speedup SomeClass === someVal */
class Module_tripleEq : public SymbolBasedIntrinsicMethod {
public:
    Module_tripleEq() : SymbolBasedIntrinsicMethod(Intrinsics::HandleBlock::Unhandled) {}
    virtual llvm::Value *makeCall(MethodCallContext &mcctx) const override {
        auto &cs = mcctx.cs;
        auto *send = mcctx.send;
        auto representedClass = core::Types::getRepresentedClass(cs, send->recv.type);
        if (!representedClass.exists()) {
            return IREmitterHelpers::emitMethodCallViaRubyVM(mcctx);
        }
        auto recvType = representedClass.data(cs)->externalType();
        auto &arg0 = send->args[0];

        auto &builder = mcctx.builder;

        auto recvValue = mcctx.varGetRecv();
        auto representedClassValue = Payload::getRubyConstant(cs, representedClass, builder);
        auto classEq = builder.CreateICmpEQ(recvValue, representedClassValue, "Module_tripleEq_shortCircuit");

        auto fastStart = llvm::BasicBlock::Create(cs, "Module_tripleEq_fast", builder.GetInsertBlock()->getParent());
        auto slowStart = llvm::BasicBlock::Create(cs, "Module_tripleEq_slow", builder.GetInsertBlock()->getParent());
        auto cont = llvm::BasicBlock::Create(cs, "Module_tripleEq_cont", builder.GetInsertBlock()->getParent());

        auto expected = Payload::setExpectedBool(cs, builder, classEq, true);
        builder.CreateCondBr(expected, fastStart, slowStart);

        builder.SetInsertPoint(fastStart);
        auto arg0Value = Payload::varGet(cs, arg0.variable, builder, mcctx.irctx, mcctx.rubyRegionId);
        auto typeTest = Payload::typeTest(cs, builder, arg0Value, recvType);
        auto fastPath = Payload::boolToRuby(cs, builder, typeTest);
        auto fastEnd = builder.GetInsertBlock();
        builder.CreateBr(cont);

        builder.SetInsertPoint(slowStart);
        auto slowPath = IREmitterHelpers::emitMethodCallViaRubyVM(mcctx);
        auto slowEnd = builder.GetInsertBlock();
        builder.CreateBr(cont);

        builder.SetInsertPoint(cont);
        auto incomingEdges = 2;
        auto phi = builder.CreatePHI(builder.getInt64Ty(), incomingEdges, "Module_tripleEq_result");
        phi->addIncoming(fastPath, fastEnd);
        phi->addIncoming(slowPath, slowEnd);

        return phi;
    };

    virtual InlinedVector<core::ClassOrModuleRef, 2> applicableClasses(const core::GlobalState &gs) const override {
        return {core::Symbols::Module()};
    };
    virtual InlinedVector<core::NameRef, 2> applicableMethods(const core::GlobalState &gs) const override {
        return {core::Names::tripleEq()};
    };
} Module_tripleEq;

class Regexp_new : public SymbolBasedIntrinsicMethod {
public:
    Regexp_new() : SymbolBasedIntrinsicMethod(Intrinsics::HandleBlock::Unhandled) {}
    virtual llvm::Value *makeCall(MethodCallContext &mcctx) const override {
        auto &cs = mcctx.cs;
        auto *send = mcctx.send;
        if (send->args.size() < 1 || send->args.size() > 2) {
            // todo: make this work with options.
            return IREmitterHelpers::emitMethodCallViaRubyVM(mcctx);
        }
        auto options = 0;
        if (send->args.size() == 2) {
            auto &arg1 = send->args[1];
            if (!core::isa_type<core::IntegerLiteralType>(arg1.type)) {
                return IREmitterHelpers::emitMethodCallViaRubyVM(mcctx);
            }
            const auto &literalOptions = core::cast_type_nonnull<core::IntegerLiteralType>(arg1.type);
            options = literalOptions.value;
        }

        auto &arg0 = send->args[0];
        if (!core::isa_type<core::NamedLiteralType>(arg0.type)) {
            return IREmitterHelpers::emitMethodCallViaRubyVM(mcctx);
        }

        auto literal = core::cast_type_nonnull<core::NamedLiteralType>(arg0.type);
        if (literal.literalKind != core::NamedLiteralType::LiteralTypeKind::String) {
            return IREmitterHelpers::emitMethodCallViaRubyVM(mcctx);
        }
        auto &builder = mcctx.builder;
        auto str = literal.asName().shortName(cs);
        return Payload::cPtrToRubyRegexp(cs, builder, str, options);
    };

    virtual InlinedVector<core::ClassOrModuleRef, 2> applicableClasses(const core::GlobalState &gs) const override {
        return {core::Symbols::Regexp().data(gs)->lookupSingletonClass(gs)};
    };
    virtual InlinedVector<core::NameRef, 2> applicableMethods(const core::GlobalState &gs) const override {
        return {core::Names::new_()};
    };
} Regexp_new;

class TEnum_new : public SymbolBasedIntrinsicMethod {
public:
    TEnum_new() : SymbolBasedIntrinsicMethod(Intrinsics::HandleBlock::Unhandled) {}
    virtual llvm::Value *makeCall(MethodCallContext &mcctx) const override {
        auto &cs = mcctx.cs;
        auto *send = mcctx.send;
        // Instead of `MyEnum::X$1.new(...)`, we want to do `<self>.new(...)` to get back to what
        // would have happened at runtime. This is effecctively "undo-ing" the earlier DSL pass.
        auto appliedType = core::cast_type<core::AppliedType>(send->recv.type);
        if (appliedType == nullptr) {
            return IREmitterHelpers::emitMethodCallViaRubyVM(mcctx);
        }

        auto attachedClass = appliedType->klass.data(cs)->attachedClass(cs);
        ENFORCE(attachedClass.exists());
        if (!attachedClass.data(cs)->name.isTEnumName(cs)) {
            return IREmitterHelpers::emitMethodCallViaRubyVM(mcctx);
        }

        auto *blockHandler = Payload::vmBlockHandlerNone(cs, mcctx.builder);

        auto recv = cfg::LocalRef::selfVariable();
        auto [stack, keywords, flags] = IREmitterHelpers::buildSendArgs(mcctx, recv, 0);
        auto &irctx = mcctx.irctx;
        auto &builder = mcctx.builder;
        auto rubyRegionId = mcctx.rubyRegionId;
        auto *cfp = Payload::getCFPForBlock(cs, builder, irctx, rubyRegionId);
        Payload::pushRubyStackVector(cs, builder, cfp, Payload::varGet(cs, recv, builder, irctx, rubyRegionId), stack);
        auto *cache = IREmitterHelpers::makeInlineCache(cs, builder, "new", flags, stack.size(), keywords);
        return Payload::callFuncWithCache(cs, builder, cache, blockHandler);
    };

    virtual InlinedVector<core::ClassOrModuleRef, 2> applicableClasses(const core::GlobalState &gs) const override {
        return {core::Symbols::T_Enum().data(gs)->lookupSingletonClass(gs)};
    };
    virtual InlinedVector<core::NameRef, 2> applicableMethods(const core::GlobalState &gs) const override {
        return {core::Names::new_()};
    };
    virtual llvm::Value *receiverFastPathTest(MethodCallContext &mcctx, core::ClassOrModuleRef c) const override {
        return mcctx.builder.getInt1(true);
    };
} TEnum_new;

class TEnum_abstract : public SymbolBasedIntrinsicMethod {
public:
    TEnum_abstract() : SymbolBasedIntrinsicMethod(Intrinsics::HandleBlock::Unhandled) {}
    virtual llvm::Value *makeCall(MethodCallContext &mcctx) const override {
        auto &builder = mcctx.builder;
        return Payload::rubyNil(mcctx.cs, builder);
    };

    virtual InlinedVector<core::ClassOrModuleRef, 2> applicableClasses(const core::GlobalState &gs) const override {
        return {core::Symbols::T_Enum().data(gs)->lookupSingletonClass(gs)};
    };
    virtual InlinedVector<core::NameRef, 2> applicableMethods(const core::GlobalState &gs) const override {
        return {core::Names::declareAbstract()};
    };
} TEnum_abstract;

class TPrivateCompiler_runningCompiled_p : public SymbolBasedIntrinsicMethod {
public:
    TPrivateCompiler_runningCompiled_p() : SymbolBasedIntrinsicMethod(Intrinsics::HandleBlock::Unhandled) {}
    virtual llvm::Value *makeCall(MethodCallContext &mcctx) const override {
        auto &builder = mcctx.builder;
        return Payload::rubyTrue(mcctx.cs, builder);
    };

    virtual InlinedVector<core::ClassOrModuleRef, 2> applicableClasses(const core::GlobalState &gs) const override {
        return {core::Symbols::T_Private_CompilerSingleton()};
    };
    virtual InlinedVector<core::NameRef, 2> applicableMethods(const core::GlobalState &gs) const override {
        return {core::Names::runningCompiled_p()};
    };
} TPrivateCompiler_runningCompiled_p;

class TPrivateCompiler_compilerVersion : public SymbolBasedIntrinsicMethod {
public:
    TPrivateCompiler_compilerVersion() : SymbolBasedIntrinsicMethod(Intrinsics::HandleBlock::Unhandled) {}
    virtual llvm::Value *makeCall(MethodCallContext &mcctx) const override {
        auto &builder = mcctx.builder;
        auto frozen = true;
        return Payload::cPtrToRubyString(mcctx.cs, builder, sorbet_full_version_string, frozen);
    };

    virtual InlinedVector<core::ClassOrModuleRef, 2> applicableClasses(const core::GlobalState &gs) const override {
        return {core::Symbols::T_Private_CompilerSingleton()};
    };
    virtual InlinedVector<core::NameRef, 2> applicableMethods(const core::GlobalState &gs) const override {
        return {core::Names::compilerVersion()};
    };
} TPrivateCompiler_compilerVersion;

class Thread_squareBrackets : public CallCMethod {
public:
    // This sorbet_Thread_square_br is a slower version that will do arity checking and convert the
    // argument to a symbol. If our `makeCall` fast path doesn't apply, we'll fall back to calling
    // the slow version (via super class `makeCall`).
    Thread_squareBrackets() : CallCMethod(core::Symbols::Thread(), "[]"sv, CMethod{"sorbet_Thread_square_br"}) {}

    virtual llvm::Value *makeCall(MethodCallContext &mcctx) const override {
        auto &cs = mcctx.cs;
        auto &builder = mcctx.builder;
        auto &irctx = mcctx.irctx;
        auto *send = mcctx.send;

        if (send->args.size() != 1 || send->numPosArgs != 1) {
            return CallCMethod::makeCall(mcctx);
        }

        auto &arg = send->args[0];
        auto symit = irctx.symbols.find(arg.variable);
        if (symit == irctx.symbols.end()) {
            return CallCMethod::makeCall(mcctx);
        }

        auto *recv = mcctx.varGetRecv();
        auto *id = Payload::idIntern(cs, builder, symit->second);
        return builder.CreateCall(cs.getFunction("sorbet_Thread_square_br_symarg"), {recv, id});
    }
} Thread_squareBrackets;

class Thread_squareBracketsEq : public CallCMethod {
public:
    // This sorbet_Thread_square_br_eq is a slower version that will do arity checking and convert the
    // argument to a symbol. If our `makeCall` fast path doesn't apply, we'll fall back to calling
    // the slow version (via super class).
    Thread_squareBracketsEq() : CallCMethod(core::Symbols::Thread(), "[]="sv, CMethod{"sorbet_Thread_square_br_eq"}) {}

    virtual llvm::Value *makeCall(MethodCallContext &mcctx) const override {
        auto &cs = mcctx.cs;
        auto &builder = mcctx.builder;
        auto &irctx = mcctx.irctx;
        auto *send = mcctx.send;

        if (send->args.size() != 2 || send->numPosArgs != 2) {
            return CallCMethod::makeCall(mcctx);
        }

        auto &arg = send->args[0];
        auto symit = irctx.symbols.find(arg.variable);
        if (symit == irctx.symbols.end()) {
            return CallCMethod::makeCall(mcctx);
        }

        auto *recv = mcctx.varGetRecv();
        auto *id = Payload::idIntern(cs, builder, symit->second);
        return builder.CreateCall(
            cs.getFunction("sorbet_Thread_square_br_eq_symarg"),
            {recv, id, Payload::varGet(cs, send->args[1].variable, builder, irctx, mcctx.rubyRegionId)});
    }
} Thread_squareBracketsEq;

class CallCMethodSingleton : public CallCMethod {
public:
    CallCMethodSingleton(core::ClassOrModuleRef rubyClass, string_view rubyMethod, CMethod cMethod)
        : CallCMethod(rubyClass, rubyMethod, cMethod){};

    CallCMethodSingleton(core::ClassOrModuleRef rubyClass, string_view rubyMethod, CMethod cMethod,
                         string cMethodWithBlock)
        : CallCMethod(rubyClass, rubyMethod, cMethod, cMethodWithBlock){};

    // It is safe to skip the test if the receiver is a constant of the given class.
    virtual bool skipFastPathTest(MethodCallContext &mcctx, core::ClassOrModuleRef potentialClass) const override {
        auto &cs = mcctx.cs;
        auto &irctx = mcctx.irctx;
        auto &recv = mcctx.send->recv;

        return IREmitterHelpers::isAliasToSingleton(cs, irctx, recv.variable, potentialClass);
    }
    virtual InlinedVector<core::ClassOrModuleRef, 2> applicableClasses(const core::GlobalState &gs) const override {
        return {rubyClass.data(gs)->lookupSingletonClass(gs)};
    };

    virtual void sanityCheck(const core::GlobalState &gs) const override {
        auto singletonClass = this->rubyClass.data(gs)->lookupSingletonClass(gs);
        CallCMethodSingleton::sanityCheckInternal(gs, singletonClass, *this);
    }
};

static const vector<CallCMethod> knownCMethodsInstance{
    {core::Symbols::Array(), "[]", CMethod{"sorbet_rb_array_square_br"}},
    {core::Symbols::Array(), "[]=", CMethod{"sorbet_rb_array_square_br_eq"}},
    {core::Symbols::Array(), "empty?", CMethod{"sorbet_rb_array_empty"}},
    {core::Symbols::Array(), "each", CMethod{"sorbet_rb_array_each", core::Symbols::Enumerator()},
     CMethod{"sorbet_rb_array_each_withBlock", core::Symbols::Array()}},
    {core::Symbols::Array(), "each_with_object",
     CMethod{"sorbet_rb_array_each_with_object", core::Symbols::Enumerator()},
     CMethod{"sorbet_rb_array_each_with_object_withBlock"}},
    {core::Symbols::Array(), "select", CMethod{"sorbet_rb_array_select", core::Symbols::Enumerator()},
     CMethod{"sorbet_rb_array_select_withBlock", core::Symbols::Array()}},
    // filter is an alias for select, so we call the same intrinsic
    {core::Symbols::Array(), "filter", CMethod{"sorbet_rb_array_select", core::Symbols::Enumerator()},
     CMethod{"sorbet_rb_array_select_withBlock", core::Symbols::Array()}},
    {core::Symbols::Array(), "reject", CMethod{"sorbet_rb_array_reject", core::Symbols::Enumerator()},
     CMethod{"sorbet_rb_array_reject_withBlock", core::Symbols::Array()}},
    {core::Symbols::Array(), "reject!", CMethod{"sorbet_rb_array_reject_bang", core::Symbols::Enumerator()},
     CMethod{"sorbet_rb_array_reject_bang_withBlock", core::Symbols::Array()}},
    {core::Symbols::Array(), "find", CMethod{"sorbet_rb_array_find"}, CMethod{"sorbet_rb_array_find_withBlock"}},
    {core::Symbols::Array(), "collect", CMethod{"sorbet_rb_array_collect", core::Symbols::Enumerator()},
     CMethod{"sorbet_rb_array_collect_withBlock", core::Symbols::Array()}},
    // Ruby implements map and collect with the same function (named with "collect" in its name).
    // We do the same for consistency.
    {core::Symbols::Array(), "map", CMethod{"sorbet_rb_array_collect", core::Symbols::Enumerator()},
     CMethod{"sorbet_rb_array_collect_withBlock", core::Symbols::Array()}},
    {core::Symbols::Array(), "collect!", CMethod{"sorbet_rb_array_collect_bang", core::Symbols::Enumerator()},
     CMethod{"sorbet_rb_array_collect_bang_withBlock", core::Symbols::Array()}},
    {core::Symbols::Array(), "map!", CMethod{"sorbet_rb_array_collect_bang", core::Symbols::Enumerator()},
     CMethod{"sorbet_rb_array_collect_bang_withBlock", core::Symbols::Array()}},
    {core::Symbols::Array(), "any?", CMethod{"sorbet_rb_array_any"}, CMethod{"sorbet_rb_array_any_withBlock"}},
    {core::Symbols::Array(), "all?", CMethod{"sorbet_rb_array_all"}, CMethod{"sorbet_rb_array_all_withBlock"}},
    {core::Symbols::Array(), "compact", CMethod{"sorbet_rb_array_compact", core::Symbols::Array()}},
    {core::Symbols::Array(), "compact!", CMethod{"sorbet_rb_array_compact_bang"}},
    {core::Symbols::Array(), "to_ary", CMethod{"sorbet_returnRecv", core::Symbols::Array()}},
    {core::Symbols::Array(), "to_h", CMethod{"sorbet_rb_array_to_h", core::Symbols::Hash()}},
    {core::Symbols::Array(), "size", CMethod{"sorbet_rb_array_len", core::Symbols::Integer()}},
    {core::Symbols::Array(), "length", CMethod{"sorbet_rb_array_len", core::Symbols::Integer()}},
    {core::Symbols::Array(),
     "to_a",
     CMethod{"sorbet_int_ary_to_a", core::Symbols::Array()},
     std::nullopt,
     {KnownFunction::cached("sorbet_rb_ary_to_a_func")}},
    {core::Symbols::Array(), "uniq", CMethod("sorbet_rb_array_uniq", core::Symbols::Array()),
     CMethod("sorbet_rb_array_uniq_withBlock", core::Symbols::Array())},
    {core::Symbols::Hash(), "[]", CMethod{"sorbet_rb_hash_square_br"}},
    {core::Symbols::Hash(), "[]=", CMethod{"sorbet_rb_hash_square_br_eq"}},
    {core::Symbols::Hash(), "each_pair", CMethod{"sorbet_rb_hash_each_pair", core::Symbols::Enumerator()},
     CMethod{"sorbet_rb_hash_each_pair_withBlock", core::Symbols::Hash()}},
    {core::Symbols::Hash(), "each", CMethod{"sorbet_rb_hash_each_pair", core::Symbols::Enumerator()},
     CMethod{"sorbet_rb_hash_each_pair_withBlock", core::Symbols::Hash()}},
    {core::Symbols::Hash(), "each_with_object", CMethod{"sorbet_rb_hash_each_with_object", core::Symbols::Enumerator()},
     CMethod{"sorbet_rb_hash_each_with_object_withBlock"}},
    {core::Symbols::Hash(), "any?", CMethod{"sorbet_rb_hash_any"}, CMethod{"sorbet_rb_hash_any_withBlock"}},
    {core::Symbols::Hash(),
     "keys",
     CMethod{"sorbet_rb_hash_keys", core::Symbols::Array()},
     nullopt,
     {KnownFunction("rb_hash_keys")}},
    {core::Symbols::Hash(),
     "values",
     CMethod{"sorbet_rb_hash_values", core::Symbols::Array()},
     nullopt,
     {KnownFunction("rb_hash_values")}},
    {core::Symbols::Hash(),
     "to_h",
     CMethod{"sorbet_int_hash_to_h", core::Symbols::Hash()},
     CMethod{"sorbet_int_hash_to_h_withBlock", core::Symbols::Hash()},
     {KnownFunction::cached("sorbet_rb_hash_to_h_func")}},
    {core::Symbols::Hash(), "to_hash", CMethod{"sorbet_returnRecv", core::Symbols::Hash()}},
    {core::Symbols::Hash(), "fetch", CMethod{"sorbet_rb_hash_fetch_m"}, CMethod{"sorbet_rb_hash_fetch_m_withBlock"}},
    {core::Symbols::Hash(), "merge", CMethod{"sorbet_rb_hash_merge", core::Symbols::Hash()},
     CMethod{"sorbet_rb_hash_merge_withBlock", core::Symbols::Hash()}},
    {core::Symbols::Hash(), "merge!", CMethod{"sorbet_rb_hash_update", core::Symbols::Hash()},
     CMethod{"sorbet_rb_hash_update_withBlock", core::Symbols::Hash()}},
    {core::Symbols::Hash(), "update", CMethod{"sorbet_rb_hash_update", core::Symbols::Hash()},
     CMethod{"sorbet_rb_hash_update_withBlock", core::Symbols::Hash()}},
    {core::Symbols::Hash(), "select", CMethod{"sorbet_rb_hash_select", core::Symbols::Enumerator()},
     CMethod{"sorbet_rb_hash_select_withBlock", core::Symbols::Hash()}},
    {core::Symbols::Hash(), "delete", CMethod{"sorbet_rb_hash_delete_m"}, CMethod{"sorbet_rb_hash_delete_m_withBlock"}},
    {core::Symbols::Hash(), "empty?", CMethod{"sorbet_rb_hash_empty_p"}},
    {core::Symbols::Hash(), "transform_values", CMethod{"sorbet_rb_hash_transform_values", core::Symbols::Hash()},
     CMethod{"sorbet_rb_hash_transform_values_withBlock", core::Symbols::Hash()}},
    {core::Symbols::TrueClass(), "|", CMethod{"sorbet_int_bool_true"}},
    {core::Symbols::FalseClass(), "|", CMethod{"sorbet_int_bool_and"}},
    {core::Symbols::TrueClass(), "&", CMethod{"sorbet_int_bool_and"}},
    {core::Symbols::FalseClass(), "&", CMethod{"sorbet_int_bool_false"}},
    {core::Symbols::TrueClass(), "^", CMethod{"sorbet_int_bool_nand"}},
    {core::Symbols::FalseClass(), "^", CMethod{"sorbet_int_bool_and"}},
    {core::Symbols::Integer(), "+", CMethod{"sorbet_rb_int_plus"}},
    {core::Symbols::Integer(), "-", CMethod{"sorbet_rb_int_minus"}},
    {core::Symbols::Integer(), "*", CMethod{"sorbet_rb_int_mul"}},
    {core::Symbols::Integer(), "/", CMethod{"sorbet_rb_int_div"}},
    {core::Symbols::Integer(), ">", CMethod{"sorbet_rb_int_gt"}},
    {core::Symbols::Integer(), "<", CMethod{"sorbet_rb_int_lt"}},
    {core::Symbols::Integer(), ">=", CMethod{"sorbet_rb_int_ge"}},
    {core::Symbols::Integer(), "<=", CMethod{"sorbet_rb_int_le"}},
    {core::Symbols::Integer(), "to_i", CMethod{"sorbet_returnRecv", core::Symbols::Integer()}},
    {core::Symbols::Integer(), "to_int", CMethod{"sorbet_returnRecv", core::Symbols::Integer()}},
    {core::Symbols::Integer(), "to_s", CMethod{"sorbet_rb_int_to_s", core::Symbols::String()}},
    {core::Symbols::Integer(), "==", CMethod{"sorbet_rb_int_equal"}},
    {core::Symbols::Integer(), "!=", CMethod{"sorbet_rb_int_neq"}},
    {core::Symbols::Integer(), "times", CMethod{"sorbet_rb_int_dotimes", core::Symbols::Enumerator()},
     CMethod{"sorbet_rb_int_dotimes_withBlock", core::Symbols::Integer()}},
    {core::Symbols::String(), "+@", CMethod{"sorbet_int_str_uplus", core::Symbols::String()}},
    {core::Symbols::String(),
     "to_s",
     CMethod{"sorbet_int_str_to_s", core::Symbols::String()},
     std::nullopt,
     {KnownFunction::cached("sorbet_rb_str_to_s_func")}},
    {core::Symbols::Symbol(), "==", CMethod{"sorbet_rb_sym_equal"}},
    {core::Symbols::Symbol(), "===", CMethod{"sorbet_rb_sym_equal"}},
    {core::Symbols::Symbol(), "to_sym", CMethod{"sorbet_returnRecv"}},
#include "WrappedIntrinsics.h"
};

static const vector<CallCMethodSingleton> knownCMethodsSingleton{
    {core::Symbols::T(), "unsafe", CMethod{"sorbet_T_unsafe"}},
    {core::Symbols::T(), "must", CMethod{"sorbet_T_must"}},
    {core::Symbols::Thread(), "current", CMethod{"sorbet_Thread_current", core::Symbols::Thread()}},
    {core::Symbols::Thread(), "main", CMethod{"sorbet_Thread_main", core::Symbols::Thread()}},
};

vector<const SymbolBasedIntrinsicMethod *> getKnownCMethodPtrs(const core::GlobalState &gs) {
    vector<const SymbolBasedIntrinsicMethod *> res{
        &DefineMethodIntrinsic,
        &SorbetPrivateStaticResolvedSigIntrinsic,
        &SorbetPrivateStaticSigIntrinsic,
        &Module_tripleEq,
        &Regexp_new,
        &TEnum_new,
        &TEnum_abstract,
        &TPrivateCompiler_runningCompiled_p,
        &TPrivateCompiler_compilerVersion,
        &Thread_squareBrackets,
        &Thread_squareBracketsEq,
    };
    for (auto &method : knownCMethodsInstance) {
        if (debug_mode) {
            method.sanityCheck(gs);
        }
        res.emplace_back(&method);
    }
    for (auto &method : knownCMethodsSingleton) {
        if (debug_mode) {
            method.sanityCheck(gs);
        }
        res.emplace_back(&method);
    }
    return res;
}

// stuff
}; // namespace

llvm::Value *SymbolBasedIntrinsicMethod::receiverFastPathTest(MethodCallContext &mcctx,
                                                              core::ClassOrModuleRef potentialClass) const {
    auto *recv = mcctx.varGetRecv();
    return Payload::typeTest(mcctx.cs, mcctx.builder, recv, potentialClass);
}

bool SymbolBasedIntrinsicMethod::skipFastPathTest(MethodCallContext &mcctx,
                                                  core::ClassOrModuleRef potentialClass) const {
    return false;
}

bool SymbolBasedIntrinsicMethod::needsAfterIntrinsicProcessing() const {
    return true;
}

void SymbolBasedIntrinsicMethod::sanityCheck(const core::GlobalState &gs) const {}

vector<const SymbolBasedIntrinsicMethod *> &SymbolBasedIntrinsicMethod::definedIntrinsics(const core::GlobalState &gs) {
    static vector<const SymbolBasedIntrinsicMethod *> ret = getKnownCMethodPtrs(gs);

    return ret;
}

}; // namespace sorbet::compiler
