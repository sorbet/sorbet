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
#include "compiler/IREmitter/Payload.h"
#include "compiler/IREmitter/SymbolBasedIntrinsicMethod.h"
#include "compiler/Names/Names.h"
#include <optional>
#include <string_view>

using namespace std;
namespace sorbet::compiler {
namespace {
llvm::IRBuilder<> &builderCast(llvm::IRBuilderBase &builder) {
    return static_cast<llvm::IRBuilder<> &>(builder);
};

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

class CallCMethod : public SymbolBasedIntrinsicMethod {
protected:
    core::ClassOrModuleRef rubyClass;
    string_view rubyMethod;
    string cMethod;
    optional<string> cMethodWithBlock;

private:
    // Generate a one-off function that looks like the following:
    //
    // > define VALUE @<fresh-name>(VALUE %env) {
    // >     VALUE %res = call @sorbet_inlineIntrinsicEnv_apply(%env, @cMethod, %blkArg)
    // >     ret VALUE %res
    // > }
    llvm::Function *generateForwarder(MethodCallContext &mcctx) const {
        ENFORCE(cMethodWithBlock != "");

        auto &cs = mcctx.cs;
        auto &builder = builderCast(mcctx.build);

        // function signature
        auto linkage = llvm::Function::InternalLinkage;
        llvm::Twine name{"forward_" + llvm::Twine{cMethod}};
        auto *fn = llvm::Function::Create(cs.getInlineForwarderType(), linkage, name, cs.module);
        auto *env = fn->arg_begin();

        // function body
        auto *entry = llvm::BasicBlock::Create(cs, "entry", fn);
        auto ip = builder.saveIP();
        builder.SetInsertPoint(entry);
        auto *cfunc = cs.module->getFunction(*cMethodWithBlock);
        auto *result = builder.CreateCall(cs.module->getFunction("sorbet_inlineIntrinsicEnv_apply"),
                                          {env, cfunc, mcctx.blk}, "result");
        builder.CreateRet(result);
        builder.restoreIP(ip);

        return fn;
    }

public:
    CallCMethod(core::ClassOrModuleRef rubyClass, string_view rubyMethod, string cMethod)
        : SymbolBasedIntrinsicMethod(Intrinsics::HandleBlock::Unhandled), rubyClass(rubyClass), rubyMethod(rubyMethod),
          cMethod(cMethod), cMethodWithBlock(){};

    CallCMethod(core::ClassOrModuleRef rubyClass, string_view rubyMethod, string cMethod, string cMethodWithBlock)
        : SymbolBasedIntrinsicMethod(Intrinsics::HandleBlock::Handled), rubyClass(rubyClass), rubyMethod(rubyMethod),
          cMethod(cMethod), cMethodWithBlock(cMethodWithBlock){};

    virtual llvm::Value *makeCall(MethodCallContext &mcctx) const override {
        auto &cs = mcctx.cs;
        auto &builder = builderCast(mcctx.build);
        auto *send = mcctx.send;
        auto rubyBlockId = mcctx.rubyBlockId;

        auto *recv = Payload::varGet(cs, send->recv.variable, builder, mcctx.irctx, rubyBlockId);
        auto *id = Payload::idIntern(cs, builder, send->fun.shortName(cs));
        auto *offset = Payload::buildLocalsOffset(cs);

        // kwsplat is used by the vm only, and we don't use the vm's api for calling an intrinsic directly.
        auto [argc, argv, _kwSplat] = IREmitterHelpers::fillSendArgArray(mcctx);

        llvm::Value *res{nullptr};
        if (mcctx.blk != nullptr) {
            if (!cMethodWithBlock.has_value()) {
                core::Loc loc{mcctx.irctx.cfg.file, send->argLocs.back()};
                compiler::failCompilation(cs, loc, "Unable to handle a block with this intrinsic");
            }
            auto *forwarder = generateForwarder(mcctx);

            // The ruby stack doens't need to be managed here because the known c intrinsics don't expect to be called
            // by the vm.
            res = builder.CreateCall(cs.module->getFunction("sorbet_callIntrinsicInlineBlock"),
                                     {forwarder, recv, id, argc, argv, mcctx.blk, offset}, "rawSendResultWithBlock");
        } else {
            auto *blkPtr = llvm::ConstantPointerNull::get(cs.getRubyBlockFFIType()->getPointerTo());
            res = builder.CreateCall(cs.getFunction(cMethod), {recv, id, argc, argv, blkPtr, offset}, "rawSendResult");
        }

        return res;
    };

    virtual InlinedVector<core::ClassOrModuleRef, 2> applicableClasses(const core::GlobalState &gs) const override {
        return {rubyClass};
    };
    virtual InlinedVector<core::NameRef, 2> applicableMethods(const core::GlobalState &gs) const override {
        return {gs.lookupNameUTF8(rubyMethod)};
    };
};

void emitParamInitialization(CompilerState &cs, llvm::IRBuilder<> &builder, const IREmitterContext &irctx,
                             core::SymbolRef funcSym, int rubyBlockId, llvm::Value *param) {
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
    for (auto &argInfo : funcSym.data(cs)->arguments()) {
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
            auto *offset = IREmitterHelpers::buildU4(cs, i);
            llvm::Value *indices[] = {offset};
            builder.CreateStore(id, builder.CreateGEP(table, indices));
        }

        auto *tableSize = IREmitterHelpers::buildS4(cs, nonKeywordArgInfo.size());
        builder.CreateCall(cs.getFunction("sorbet_setupParamPositional"), {param, tableSize, table});
    }

    if (hasKw) {
        ENFORCE(kwNum > 0);
        ENFORCE(reqKwNum <= kwNum);
        ENFORCE(keywordArgInfo.size() == (kwNum + hasKwRest));

        // Create a table for all the keyword argument IDs.
        auto *table = builder.CreateAlloca(llvm::Type::getInt64Ty(cs),
                                           IREmitterHelpers::buildS4(cs, keywordArgInfo.size()), "keyword_table");

        int i = -1;
        for (auto info : keywordArgInfo) {
            ++i;
            auto *id = Payload::idIntern(cs, builder, info->argumentName(cs));
            auto *offset = IREmitterHelpers::buildU4(cs, i);
            llvm::Value *indices[] = {offset};
            builder.CreateStore(id, builder.CreateGEP(table, indices));
        }

        auto *kw_num = IREmitterHelpers::buildS4(cs, kwNum);
        auto *required_num = IREmitterHelpers::buildS4(cs, reqKwNum);
        auto *tableSize = IREmitterHelpers::buildS4(cs, keywordArgInfo.size());

        builder.CreateCall(cs.getFunction("sorbet_setupParamKeywords"),
                           {param, kw_num, required_num, tableSize, table});
    }
}

// TODO(froydnj): we need to do something like this for blocks as well.
llvm::Value *buildParamInfo(CompilerState &cs, llvm::IRBuilder<> &builderBase, const IREmitterContext &irctx,
                            core::SymbolRef funcSym, int rubyBlockId) {
    auto &builder = builderCast(builderBase);

    auto *paramInfo = builder.CreateCall(cs.getFunction("sorbet_allocateParamInfo"), {}, "parameterInfo");

    emitParamInitialization(cs, builder, irctx, funcSym, rubyBlockId, paramInfo);

    return paramInfo;
}

class DefineMethodIntrinsic : public SymbolBasedIntrinsicMethod {
public:
    DefineMethodIntrinsic() : SymbolBasedIntrinsicMethod(Intrinsics::HandleBlock::Unhandled){};
    virtual llvm::Value *makeCall(MethodCallContext &mcctx) const override {
        auto &cs = mcctx.cs;
        auto &builder = builderCast(mcctx.build);
        auto *send = mcctx.send;

        bool isSelf = send->fun == core::Names::keepSelfDef();

        ENFORCE(send->args.size() == 3, "Invariant established by rewriter/Flatten.cc");

        // First arg: define method on what
        auto ownerSym = typeToSym(cs, send->args[0].type);
        auto klass = Payload::getRubyConstant(cs, ownerSym, builder);

        // Second arg: name of method to define
        auto litName = core::cast_type_nonnull<core::LiteralType>(send->args[1].type);
        ENFORCE(litName.literalKind == core::LiteralType::LiteralTypeKind::Symbol);
        auto funcNameRef = litName.asName(cs);
        auto name = Payload::toCString(cs, funcNameRef.show(cs), builder);

        // Third arg: method kind (normal or attr_reader)
        auto litMethodKind = core::cast_type_nonnull<core::LiteralType>(send->args[2].type);
        ENFORCE(litMethodKind.literalKind == core::LiteralType::LiteralTypeKind::Symbol);
        auto methodKind = litMethodKind.asName(cs);

        switch (methodKind.rawId()) {
            case core::Names::attrReader().rawId(): {
                const char *payloadFuncName = isSelf ? "sorbet_defineIvarMethodSingleton" : "sorbet_defineIvarMethod";
                auto payloadFunc = cs.getFunction(payloadFuncName);

                builder.CreateCall(payloadFunc, {klass, name});
                break;
            }
            case core::Names::normal().rawId(): {
                auto lookupSym = isSelf ? ownerSym : ownerSym.data(cs)->attachedClass(cs);
                if (ownerSym == core::Symbols::Object() && !isSelf) {
                    // TODO Figure out if this speicial case is right
                    lookupSym = core::Symbols::Object();
                }
                auto funcSym = lookupSym.data(cs)->findMember(cs, funcNameRef);
                ENFORCE(funcSym.exists());
                ENFORCE(funcSym.data(cs)->isMethod());

                auto funcHandle = IREmitterHelpers::getOrCreateFunction(cs, funcSym);
                auto *stackFrameVar = Payload::rubyStackFrameVar(cs, builder, mcctx.irctx, funcSym);
                auto *stackFrame = builder.CreateLoad(stackFrameVar, "stackFrame");

                const char *payloadFuncName = isSelf ? "sorbet_defineMethodSingleton" : "sorbet_defineMethod";
                auto rubyFunc = cs.getFunction(payloadFuncName);
                auto *paramInfo = buildParamInfo(cs, builder, mcctx.irctx, funcSym, mcctx.rubyBlockId);
                builder.CreateCall(rubyFunc, {klass, name, funcHandle, paramInfo, stackFrame});

                builder.CreateCall(IREmitterHelpers::getInitFunction(cs, funcSym), {});
                break;
            }
            default:
                Exception::raise("Unknown method kind: {}", methodKind.show(cs));
        }

        // Return the symbol of the method name even if we don't emit a definition. This will be a problem if there are
        // meta-progrmaming methods applied to an abstract method definition, see
        // https://github.com/stripe/sorbet_llvm/issues/115 for more information.
        return Payload::varGet(cs, send->args[1].variable, builder, mcctx.irctx, mcctx.rubyBlockId);
    }

    virtual InlinedVector<core::ClassOrModuleRef, 2> applicableClasses(const core::GlobalState &gs) const override {
        return {core::Symbols::Sorbet_Private_Static().data(gs)->lookupSingletonClass(gs)};
    };
    virtual InlinedVector<core::NameRef, 2> applicableMethods(const core::GlobalState &gs) const override {
        return {core::Names::keepDef(), core::Names::keepSelfDef()};
    }
} DefineMethodIntrinsic;

class SorbetPrivateStaticSigIntrinsic : public SymbolBasedIntrinsicMethod {
public:
    SorbetPrivateStaticSigIntrinsic() : SymbolBasedIntrinsicMethod(Intrinsics::HandleBlock::Handled){};
    virtual llvm::Value *makeCall(MethodCallContext &mcctx) const override {
        auto &builder = builderCast(mcctx.build);
        return Payload::rubyNil(mcctx.cs, builder);
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

        auto &builder = builderCast(mcctx.build);

        auto recvValue = Payload::varGet(cs, send->recv.variable, builder, mcctx.irctx, mcctx.rubyBlockId);
        auto representedClassValue = Payload::getRubyConstant(cs, representedClass, builder);
        auto classEq = builder.CreateICmpEQ(recvValue, representedClassValue, "Module_tripleEq_shortCircuit");

        auto fastStart = llvm::BasicBlock::Create(cs, "Module_tripleEq_fast", builder.GetInsertBlock()->getParent());
        auto slowStart = llvm::BasicBlock::Create(cs, "Module_tripleEq_slow", builder.GetInsertBlock()->getParent());
        auto cont = llvm::BasicBlock::Create(cs, "Module_tripleEq_cont", builder.GetInsertBlock()->getParent());

        auto expected = Payload::setExpectedBool(cs, builder, classEq, true);
        builder.CreateCondBr(expected, fastStart, slowStart);

        builder.SetInsertPoint(fastStart);
        auto arg0Value = Payload::varGet(cs, arg0.variable, builder, mcctx.irctx, mcctx.rubyBlockId);
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
            if (!core::isa_type<core::LiteralType>(arg1.type)) {
                return IREmitterHelpers::emitMethodCallViaRubyVM(mcctx);
            }
            auto literalOptions = core::cast_type_nonnull<core::LiteralType>(arg1.type);
            if (literalOptions.literalKind != core::LiteralType::LiteralTypeKind::Integer) {
                return IREmitterHelpers::emitMethodCallViaRubyVM(mcctx);
            }
            options = literalOptions.asInteger();
        }

        auto &arg0 = send->args[0];
        if (!core::isa_type<core::LiteralType>(arg0.type)) {
            return IREmitterHelpers::emitMethodCallViaRubyVM(mcctx);
        }

        auto literal = core::cast_type_nonnull<core::LiteralType>(arg0.type);
        if (literal.literalKind != core::LiteralType::LiteralTypeKind::String) {
            return IREmitterHelpers::emitMethodCallViaRubyVM(mcctx);
        }
        auto &builder = builderCast(mcctx.build);
        auto str = literal.asName(cs).shortName(cs);
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

        auto *blockHandler = Payload::vmBlockHandlerNone(cs, mcctx.build);
        auto *cache = IREmitterHelpers::pushSendArgs(mcctx, cfg::LocalRef::selfVariable(), "new", 0);
        return Payload::callFuncWithCache(cs, mcctx.build, cache, blockHandler);
    };

    virtual InlinedVector<core::ClassOrModuleRef, 2> applicableClasses(const core::GlobalState &gs) const override {
        return {core::Symbols::T_Enum().data(gs)->lookupSingletonClass(gs)};
    };
    virtual InlinedVector<core::NameRef, 2> applicableMethods(const core::GlobalState &gs) const override {
        return {core::Names::new_()};
    };
    virtual bool skipReceiverTypeTest() const override {
        return true;
    };
} TEnum_new;

class TEnum_abstract : public SymbolBasedIntrinsicMethod {
public:
    TEnum_abstract() : SymbolBasedIntrinsicMethod(Intrinsics::HandleBlock::Unhandled) {}
    virtual llvm::Value *makeCall(MethodCallContext &mcctx) const override {
        auto &builder = builderCast(mcctx.build);
        return Payload::rubyNil(mcctx.cs, builder);
    };

    virtual InlinedVector<core::ClassOrModuleRef, 2> applicableClasses(const core::GlobalState &gs) const override {
        return {core::Symbols::T_Enum().data(gs)->lookupSingletonClass(gs)};
    };
    virtual InlinedVector<core::NameRef, 2> applicableMethods(const core::GlobalState &gs) const override {
        return {core::Names::declareAbstract()};
    };
} TEnum_abstract;

class CallCMethodSingleton : public CallCMethod {
public:
    CallCMethodSingleton(core::ClassOrModuleRef rubyClass, string_view rubyMethod, string cMethod)
        : CallCMethod(rubyClass, rubyMethod, cMethod){};

    CallCMethodSingleton(core::ClassOrModuleRef rubyClass, string_view rubyMethod, string cMethod,
                         string cMethodWithBlock)
        : CallCMethod(rubyClass, rubyMethod, cMethod, cMethodWithBlock){};

    virtual InlinedVector<core::ClassOrModuleRef, 2> applicableClasses(const core::GlobalState &gs) const override {
        return {rubyClass.data(gs)->lookupSingletonClass(gs)};
    };
};

static const vector<CallCMethod> knownCMethodsInstance{
    {core::Symbols::Array(), "[]", "sorbet_rb_array_square_br"},
    {core::Symbols::Array(), "[]=", "sorbet_rb_array_square_br_eq"},
    {core::Symbols::Array(), "empty?", "sorbet_rb_array_empty"},
    {core::Symbols::Array(), "each", "sorbet_rb_array_each", "sorbet_rb_array_each_withBlock"},
    {core::Symbols::Hash(), "[]", "sorbet_rb_hash_square_br"},
    {core::Symbols::Hash(), "[]=", "sorbet_rb_hash_square_br_eq"},
    {core::Symbols::Array(), "size", "sorbet_rb_array_len"},
    {core::Symbols::TrueClass(), "|", "sorbet_int_bool_true"},
    {core::Symbols::FalseClass(), "|", "sorbet_int_bool_and"},
    {core::Symbols::TrueClass(), "&", "sorbet_int_bool_and"},
    {core::Symbols::FalseClass(), "&", "sorbet_int_bool_false"},
    {core::Symbols::TrueClass(), "^", "sorbet_int_bool_nand"},
    {core::Symbols::FalseClass(), "^", "sorbet_int_bool_and"},
    {core::Symbols::Integer(), "+", "sorbet_rb_int_plus"},
    {core::Symbols::Integer(), "-", "sorbet_rb_int_minus"},
    {core::Symbols::Integer(), "*", "sorbet_rb_int_mul"},
    {core::Symbols::Integer(), "/", "sorbet_rb_int_div"},
    {core::Symbols::Integer(), ">", "sorbet_rb_int_gt"},
    {core::Symbols::Integer(), "<", "sorbet_rb_int_lt"},
    {core::Symbols::Integer(), ">=", "sorbet_rb_int_ge"},
    {core::Symbols::Integer(), "<=", "sorbet_rb_int_le"},
    {core::Symbols::Integer(), "to_s", "sorbet_rb_int_to_s"},
    {core::Symbols::Integer(), "==", "sorbet_rb_int_equal"},
    {core::Symbols::Integer(), "!=", "sorbet_rb_int_neq"},
    {core::Symbols::Integer(), "times", "sorbet_rb_int_dotimes", "sorbet_rb_int_dotimes_withBlock"},
    {core::Symbols::Thread(), "[]", "sorbet_Thread_square_br"},
    {core::Symbols::Thread(), "[]=", "sorbet_Thread_square_br_eq"},
#include "WrappedIntrinsics.h"
};

static const vector<CallCMethodSingleton> knownCMethodsSingleton{
    {core::Symbols::T(), "unsafe", "sorbet_T_unsafe"},
    {core::Symbols::T(), "must", "sorbet_T_must"},
    {core::Symbols::Thread(), "current", "sorbet_Thread_current"},
};

vector<const SymbolBasedIntrinsicMethod *> getKnownCMethodPtrs() {
    vector<const SymbolBasedIntrinsicMethod *> res{
        &DefineMethodIntrinsic, &SorbetPrivateStaticSigIntrinsic, &Module_tripleEq, &Regexp_new, &TEnum_new,
        &TEnum_abstract,
    };
    for (auto &method : knownCMethodsInstance) {
        res.emplace_back(&method);
    }
    for (auto &method : knownCMethodsSingleton) {
        res.emplace_back(&method);
    }
    return res;
}

// stuff
}; // namespace
vector<const SymbolBasedIntrinsicMethod *> &SymbolBasedIntrinsicMethod::definedIntrinsics() {
    static vector<const SymbolBasedIntrinsicMethod *> ret = getKnownCMethodPtrs();

    return ret;
}

}; // namespace sorbet::compiler
