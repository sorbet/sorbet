// These violate our poisons so have to happen first
#include "llvm/IR/DIBuilder.h"
#include "llvm/IR/DerivedTypes.h" // FunctionType
#include "llvm/IR/IRBuilder.h"

#include "Payload.h"
#include "absl/base/casts.h"
#include "absl/strings/match.h"
#include "absl/strings/str_replace.h"
#include "absl/strings/str_split.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "cfg/CFG.h"
#include "common/sort.h"
#include "compiler/Core/CompilerState.h"
#include "compiler/IREmitter/IREmitterContext.h"
#include "compiler/IREmitter/IREmitterHelpers.h"
#include "compiler/IREmitter/MethodCallContext.h"

using namespace std;
namespace sorbet::compiler {

namespace {
string getFunctionNamePrefix(CompilerState &cs, core::ClassOrModuleRef sym) {
    auto maybeAttached = sym.data(cs)->attachedClass(cs);
    if (maybeAttached.exists()) {
        return getFunctionNamePrefix(cs, maybeAttached) + ".singleton_class";
    }
    string suffix;
    auto name = sym.data(cs)->name;
    if (name.kind() == core::NameKind::CONSTANT && name.dataCnst(cs)->original.kind() == core::NameKind::UTF8) {
        suffix = string(name.shortName(cs));
    } else {
        suffix = name.toString(cs);
    }
    string prefix = IREmitterHelpers::isRootishSymbol(cs, sym.data(cs)->owner)
                        ? ""
                        : getFunctionNamePrefix(cs, sym.data(cs)->owner) + "::";

    return prefix + suffix;
}
} // namespace

string IREmitterHelpers::getFunctionName(CompilerState &cs, core::MethodRef sym) {
    auto owner = sym.data(cs)->owner;
    auto maybeAttachedOwner = owner.data(cs)->attachedClass(cs);
    string prefix = "func_";
    if (maybeAttachedOwner.exists()) {
        prefix = prefix + getFunctionNamePrefix(cs, maybeAttachedOwner) + ".";
    } else {
        prefix = prefix + getFunctionNamePrefix(cs, owner) + "#";
    }

    auto name = sym.data(cs)->name;
    string suffix;
    if (name.kind() == core::NameKind::UTF8) {
        suffix = string(name.shortName(cs));
    } else {
        suffix = name.toString(cs);
    }

    // '@' in symbol names means special things to the dynamic linker with regards
    // to symbol versioning.  Since we're not using symbol versioning, we need to
    // mangle function names to avoid '@'.
    string mangled;
    bool isFirst = true;
    for (auto part : absl::StrSplit(suffix, '@')) {
        if (!isFirst) {
            mangled += "at";
        } else {
            isFirst = false;
        }
        mangled += to_string(part.size());
        mangled += part;
    }

    return prefix + mangled;
}

bool IREmitterHelpers::isFileStaticInit(const core::GlobalState &gs, core::MethodRef sym) {
    auto name = sym.data(gs)->name;
    if (name.kind() != core::NameKind::UNIQUE) {
        return false;
    }
    return name.dataUnique(gs)->original == core::Names::staticInit();
}

bool IREmitterHelpers::isClassStaticInit(const core::GlobalState &gs, core::MethodRef sym) {
    return sym.data(gs)->name == core::Names::staticInit();
}

bool IREmitterHelpers::isFileOrClassStaticInit(const core::GlobalState &gs, core::MethodRef sym) {
    return isFileStaticInit(gs, sym) || isClassStaticInit(gs, sym);
}

namespace {

llvm::Function *
getOrCreateFunctionWithName(CompilerState &cs, std::string name, llvm::FunctionType *ft,
                            llvm::GlobalValue::LinkageTypes linkageType = llvm::Function::InternalLinkage,
                            bool overrideLinkage = false) {
    auto func = cs.module->getFunction(name);
    if (func) {
        if (overrideLinkage) {
            func->setLinkage(linkageType);
        }
        return func;
    }
    return llvm::Function::Create(ft, linkageType, name, *cs.module);
}

}; // namespace

llvm::Function *IREmitterHelpers::lookupFunction(CompilerState &cs, core::MethodRef sym) {
    ENFORCE(!isClassStaticInit(cs, sym), "use special helper instead");
    auto *func = cs.module->getFunction(IREmitterHelpers::getFunctionName(cs, sym));
    return func;
}

llvm::Function *IREmitterHelpers::getOrCreateDirectWrapper(CompilerState &cs, core::MethodRef sym) {
    auto name = "direct_" + IREmitterHelpers::getFunctionName(cs, sym);
    return getOrCreateFunctionWithName(cs, name, cs.getDirectWrapperFunctionType(), llvm::Function::ExternalLinkage,
                                       true);
}

llvm::Function *IREmitterHelpers::getOrCreateFunction(CompilerState &cs, core::MethodRef sym) {
    ENFORCE(!isClassStaticInit(cs, sym), "use special helper instead");
    return getOrCreateFunctionWithName(cs, IREmitterHelpers::getFunctionName(cs, sym), cs.getRubyFFIType(),
                                       llvm::Function::InternalLinkage, true);
}

llvm::Function *IREmitterHelpers::getOrCreateStaticInit(CompilerState &cs, core::MethodRef sym, core::LocOffsets loc) {
    ENFORCE(isClassStaticInit(cs, sym), "use general helper instead");
    auto name = IREmitterHelpers::getFunctionName(cs, sym) + "L" + to_string(loc.beginPos());
    return getOrCreateFunctionWithName(cs, name, cs.getRubyFFIType(), llvm::Function::InternalLinkage, true);
}
llvm::Function *IREmitterHelpers::getInitFunction(CompilerState &cs, core::MethodRef sym) {
    std::vector<llvm::Type *> NoArgs(0, llvm::Type::getVoidTy(cs));
    auto linkageType = llvm::Function::InternalLinkage;
    auto baseName = IREmitterHelpers::getFunctionName(cs, sym);

    auto ft = llvm::FunctionType::get(llvm::Type::getVoidTy(cs), NoArgs, false);
    return getOrCreateFunctionWithName(cs, "Init_" + baseName, ft, linkageType);
}

// TODO(froydnj): LLVM datatypes don't really have the concept of signedness, only
// LLVM operations.  Does that mean we should just be using IRBuilder::getInt32 etc.?
llvm::Value *IREmitterHelpers::buildU4(CompilerState &cs, uint32_t i) {
    return llvm::ConstantInt::get(cs, llvm::APInt(32, i));
}

llvm::Value *IREmitterHelpers::buildS4(CompilerState &cs, int i) {
    return llvm::ConstantInt::get(cs, llvm::APInt(32, i, /*signed=*/true));
}

llvm::Function *IREmitterHelpers::cleanFunctionBody(CompilerState &cs, llvm::Function *func) {
    for (auto &bb : *func) {
        bb.dropAllReferences();
    }

    // Delete all basic blocks. They are now unused, except possibly by
    // blockaddresses, but BasicBlock's destructor takes care of those.
    while (!func->empty()) {
        func->begin()->eraseFromParent();
    }
    return func;
}

void IREmitterHelpers::emitDebugLoc(CompilerState &cs, llvm::IRBuilderBase &builder, const IREmitterContext &irctx,
                                    int rubyRegionId, core::Loc loc) {
    auto *scope = irctx.blockScopes[rubyRegionId];
    unsigned line, column;

    if (!loc.exists()) {
        // This location seems less useful than no debug location, but it ensures
        // that we have a "real" debug location to set for LLVM.  LLVM verification
        // will assert if we have calls in debug-info-laden functions that can be
        // inlined and are not tagged with proper debug information.
        line = 0;
        column = 0;
    } else {
        auto start = loc.position(cs).first;
        line = start.line;
        column = start.column;
    }

    builder.SetCurrentDebugLocation(llvm::DILocation::get(cs, line, column, scope));
}

void IREmitterHelpers::emitUncheckedReturn(CompilerState &cs, llvm::IRBuilderBase &builder,
                                           const IREmitterContext &irctx, int rubyRegionId, llvm::Value *retVal) {
    if (functionTypePushesFrame(irctx.rubyBlockType[rubyRegionId])) {
        builder.CreateCall(cs.getFunction("sorbet_popFrame"), {});
    }
    builder.CreateRet(retVal);
}

void IREmitterHelpers::emitReturnAcrossBlock(CompilerState &cs, cfg::CFG &cfg, llvm::IRBuilderBase &builder,
                                             const IREmitterContext &irctx, int rubyRegionId, llvm::Value *retVal) {
    auto *ec = builder.CreateCall(cs.getFunction("sorbet_getEC"), {}, "ec");
    builder.CreateCall(cs.getFunction("sorbet_throwReturn"), {ec, retVal});
    builder.CreateUnreachable();
}

void IREmitterHelpers::emitReturn(CompilerState &cs, llvm::IRBuilderBase &builder, const IREmitterContext &irctx,
                                  int rubyRegionId, llvm::Value *retVal) {
    if (functionTypeNeedsPostprocessing(irctx.rubyBlockType[rubyRegionId])) {
        auto returnValue = irctx.cfg.enterLocal({core::Names::returnValue(), 1});
        Payload::varSet(cs, returnValue, retVal, builder, irctx, rubyRegionId);
        builder.CreateBr(irctx.postProcessBlock);
    } else {
        emitUncheckedReturn(cs, builder, irctx, rubyRegionId, retVal);
    }
}

llvm::Value *IREmitterHelpers::maybeCheckReturnValue(CompilerState &cs, cfg::CFG &cfg, llvm::IRBuilderBase &builder,
                                                     const IREmitterContext &irctx, llvm::Value *returnValue) {
    auto expectedType = cfg.symbol.data(cs)->resultType;
    if (expectedType == nullptr) {
        return returnValue;
    }

    if (core::isa_type<core::ClassType>(expectedType) &&
        core::cast_type_nonnull<core::ClassType>(expectedType).symbol == core::Symbols::void_()) {
        return Payload::voidSingleton(cs, builder, irctx);
    }

    // sorbet-runtime doesn't check this type for abstract methods, so we won't either.
    // TODO(froydnj): we should check this type.
    if (!cfg.symbol.data(cs)->flags.isAbstract) {
        IREmitterHelpers::emitTypeTest(cs, builder, returnValue, expectedType, "Return value");
    }

    return returnValue;
}

namespace {
void buildTypeTestPassFailBlocks(CompilerState &cs, llvm::IRBuilderBase &builder, llvm::Value *value,
                                 llvm::Value *testResult, const core::TypePtr &expectedType,
                                 std::string_view description) {
    auto successBlock = llvm::BasicBlock::Create(cs, "typeTestSuccess", builder.GetInsertBlock()->getParent());

    auto failBlock = llvm::BasicBlock::Create(cs, "typeTestFail", builder.GetInsertBlock()->getParent());

    auto expected = Payload::setExpectedBool(cs, builder, testResult, true);
    builder.CreateCondBr(expected, successBlock, failBlock);
    builder.SetInsertPoint(failBlock);
    // this will throw exception
    builder.CreateCall(cs.getFunction("sorbet_cast_failure"), {value, Payload::toCString(cs, description, builder),
                                                               Payload::toCString(cs, expectedType.show(cs), builder)});
    builder.CreateUnreachable();
    builder.SetInsertPoint(successBlock);
}
} // namespace

void IREmitterHelpers::emitTypeTest(CompilerState &cs, llvm::IRBuilderBase &builder, llvm::Value *value,
                                    const core::TypePtr &expectedType, std::string_view description) {
    auto *typeTest = Payload::typeTest(cs, builder, value, expectedType);
    buildTypeTestPassFailBlocks(cs, builder, value, typeTest, expectedType, description);
}

void IREmitterHelpers::emitTypeTestForRestArg(CompilerState &cs, llvm::IRBuilderBase &builder, llvm::Value *value,
                                              const core::TypePtr &expectedType, std::string_view description) {
    auto *fun = builder.GetInsertBlock()->getParent();
    auto *initBlock = llvm::BasicBlock::Create(cs, "restTypeTestInit", fun);
    auto *headerBlock = llvm::BasicBlock::Create(cs, "restTypeTestHeader", fun);
    auto *bodyBlock = llvm::BasicBlock::Create(cs, "restTypeTestBody", fun);
    auto *continuationBlock = llvm::BasicBlock::Create(cs, "restTypeTestFinished", fun);

    builder.CreateBr(initBlock);
    builder.SetInsertPoint(initBlock);
    auto *loopIndex = buildS4(cs, 0);
    auto *loopEnd = builder.CreateCall(cs.getFunction("sorbet_rubyArrayLen"), {value}, "restArgLength");
    builder.CreateBr(headerBlock);

    builder.SetInsertPoint(headerBlock);
    auto *indexPhi = builder.CreatePHI(builder.getInt32Ty(), 2, "loopIndexPhi");
    auto *moreToGo = builder.CreateICmpSLT(indexPhi, loopEnd, "moreToGo");
    builder.CreateCondBr(moreToGo, bodyBlock, continuationBlock);

    builder.SetInsertPoint(bodyBlock);
    auto *element = builder.CreateCall(cs.getFunction("sorbet_rubyArrayArefUnchecked"), {value, indexPhi}, "element");
    auto *typeTest = Payload::typeTest(cs, builder, element, expectedType);
    buildTypeTestPassFailBlocks(cs, builder, element, typeTest, expectedType, description);
    auto *incrementedIndex = builder.CreateAdd(indexPhi, buildS4(cs, 1));
    indexPhi->addIncoming(loopIndex, initBlock);
    indexPhi->addIncoming(incrementedIndex, builder.GetInsertBlock());
    builder.CreateBr(headerBlock);

    builder.SetInsertPoint(continuationBlock);
}

bool IREmitterHelpers::isAliasToSingleton(const core::GlobalState &gs, const IREmitterContext &irctx, cfg::LocalRef var,
                                          core::ClassOrModuleRef klass) {
    auto aliasit = irctx.aliases.find(var);
    if (aliasit == irctx.aliases.end()) {
        return false;
    }

    if (aliasit->second.kind != Alias::AliasKind::Constant) {
        return false;
    }

    ENFORCE(klass.data(gs)->isSingletonClass(gs));
    auto attachedClass = klass.data(gs)->attachedClass(gs);
    return aliasit->second.constantSym == attachedClass;
}

llvm::Value *IREmitterHelpers::emitLiteralish(CompilerState &cs, llvm::IRBuilderBase &builder,
                                              const core::TypePtr &lit) {
    if (lit.derivesFrom(cs, core::Symbols::FalseClass())) {
        return Payload::rubyFalse(cs, builder);
    }
    if (lit.derivesFrom(cs, core::Symbols::TrueClass())) {
        return Payload::rubyTrue(cs, builder);
    }
    if (lit.derivesFrom(cs, core::Symbols::NilClass())) {
        return Payload::rubyNil(cs, builder);
    }

    auto litType = core::cast_type_nonnull<core::LiteralType>(lit);
    switch (litType.literalKind) {
        case core::LiteralType::LiteralTypeKind::Integer: {
            auto *value = Payload::longToRubyValue(cs, builder, litType.asInteger());
            Payload::assumeType(cs, builder, value, core::Symbols::Integer());
            return value;
        }
        case core::LiteralType::LiteralTypeKind::Float: {
            auto *value = Payload::doubleToRubyValue(cs, builder, litType.asFloat());
            Payload::assumeType(cs, builder, value, core::Symbols::Float());
            return value;
        }
        case core::LiteralType::LiteralTypeKind::Symbol: {
            auto str = litType.asName(cs).shortName(cs);
            auto rawId = Payload::idIntern(cs, builder, str);
            auto *value = builder.CreateCall(cs.getFunction("rb_id2sym"), {rawId}, "rawSym");
            Payload::assumeType(cs, builder, value, core::Symbols::Symbol());
            return value;
        }
        case core::LiteralType::LiteralTypeKind::String: {
            auto str = litType.asName(cs).shortName(cs);
            auto *value = Payload::cPtrToRubyString(cs, builder, str, true);
            Payload::assumeType(cs, builder, value, core::Symbols::String());
            return value;
        }
    }
}

bool IREmitterHelpers::hasBlockArgument(CompilerState &cs, int blockId, core::MethodRef method,
                                        const IREmitterContext &irctx) {
    auto ty = irctx.rubyBlockType[blockId];
    if (!(ty == FunctionType::Block || ty == FunctionType::Method || ty == FunctionType::StaticInitFile ||
          ty == FunctionType::StaticInitModule)) {
        return false;
    }

    if (ty == FunctionType::Block) {
        auto blockLink = irctx.blockLinks[blockId];
        if (blockLink->argFlags.empty()) {
            return false;
        }

        return blockLink->argFlags.back().isBlock;
    }

    auto &args = method.data(cs)->arguments;
    if (args.empty()) {
        return false;
    }

    return args.back().flags.isBlock;
}

core::SymbolRef IREmitterHelpers::fixupOwningSymbol(const core::GlobalState &gs, core::SymbolRef sym) {
    if (isRootishSymbol(gs, sym)) {
        // Root methods end up going on Object.
        return core::Symbols::Object();
    }

    return sym;
}

std::string IREmitterHelpers::showClassNameWithoutOwner(const core::GlobalState &gs, core::SymbolRef sym) {
    std::string withoutOwnerStr;

    auto name = sym.name(gs);
    if (name.kind() == core::NameKind::UNIQUE) {
        withoutOwnerStr = name.dataUnique(gs)->original.show(gs);
    } else {
        withoutOwnerStr = name.show(gs);
    };

    // This is a little bit gross.  Symbol performs this sort of logic itself, but
    // the above calls are done inside NameRef, which doesn't have the necessary
    // symbol ownership information to do this sort of munging.  So we have to
    // duplicate the Symbol logic here.
    if (sym.owner(gs) != core::Symbols::PackageRegistry() || !name.isPackagerName(gs)) {
        return withoutOwnerStr;
    }

    if (name.isPackagerPrivateName(gs)) {
        // Remove _Package_Private before de-munging
        return absl::StrReplaceAll(
            withoutOwnerStr.substr(0, withoutOwnerStr.size() - core::PACKAGE_PRIVATE_SUFFIX.size()), {{"_", "::"}});
    }

    // Remove _Package before de-munging
    return absl::StrReplaceAll(withoutOwnerStr.substr(0, withoutOwnerStr.size() - core::PACKAGE_SUFFIX.size()),
                               {{"_", "::"}});
}

bool IREmitterHelpers::isRootishSymbol(const core::GlobalState &gs, core::SymbolRef sym) {
    if (!sym.exists()) {
        return false;
    }

    // These are the obvious cases.
    if (sym == core::Symbols::root() || sym == core::Symbols::rootSingleton()) {
        return true;
    }

    // --stripe-packages interposes its own set of symbols at the toplevel.
    // Absent any runtime support, we need to consider these as rootish.
    if (sym == core::Symbols::PackageRegistry() || sym.name(gs).isPackagerName(gs)) {
        return true;
    }

    return false;
}

std::optional<IREmitterHelpers::FinalMethodInfo>
IREmitterHelpers::isFinalMethod(const core::GlobalState &gs, core::TypePtr recvType, core::NameRef fun) {
    core::ClassOrModuleRef recvSym;
    if (core::isa_type<core::ClassType>(recvType)) {
        recvSym = core::cast_type_nonnull<core::ClassType>(recvType).symbol;
    } else if (auto *app = core::cast_type<core::AppliedType>(recvType)) {
        recvSym = app->klass;
    }

    if (!recvSym.exists()) {
        return std::nullopt;
    }

    auto funSym = recvSym.data(gs)->findMethod(gs, fun);
    if (!funSym.exists()) {
        return std::nullopt;
    }

    if (!funSym.data(gs)->flags.isFinal) {
        return std::nullopt;
    }

    auto file = funSym.data(gs)->loc().file();
    if (file.data(gs).compiledLevel != core::CompiledLevel::True) {
        return std::nullopt;
    }

    return IREmitterHelpers::FinalMethodInfo{recvSym, funSym, file};
}

llvm::Value *KnownFunction::getFunction(CompilerState &cs, llvm::IRBuilderBase &builder) const {
    auto *fun = cs.getFunction(this->name);
    auto *type = cs.getAnyRubyCApiFunctionType()->getPointerTo();

    switch (this->type) {
        case KnownFunction::Type::Symbol:
            return builder.CreatePointerCast(fun, type, "fnPtrCast");

        case KnownFunction::Type::CachedSymbol: {
            // allocate a global to store the return value of the forwarding function
            string cacheName = "symbolCache_" + this->name;

            auto *cache = static_cast<llvm::GlobalVariable *>(
                cs.module->getOrInsertGlobal(cacheName, type, [fun, type, &cs, &cacheName] {
                    auto *null = llvm::ConstantPointerNull::get(type);
                    auto *ret = new llvm::GlobalVariable(*cs.module, type, false, llvm::GlobalVariable::InternalLinkage,
                                                         null, cacheName);

                    ret->setAlignment(llvm::MaybeAlign(8));

                    auto globalInitBuilder = llvm::IRBuilder<>(cs);
                    globalInitBuilder.SetInsertPoint(cs.globalConstructorsEntry);

                    auto *res = globalInitBuilder.CreateCall(fun, {}, "init_" + cacheName);
                    globalInitBuilder.CreateStore(globalInitBuilder.CreatePointerCast(res, type, "fnPtrCast"), ret);

                    return ret;
                }));

            return builder.CreateLoad(cache);
        }
    }
}

llvm::Value *IREmitterHelpers::receiverFastPathTestWithCache(MethodCallContext &mcctx,
                                                             const vector<KnownFunction> &expectedRubyCFuncs,
                                                             const string &methodNameForDebug) {
    auto &cs = mcctx.cs;
    auto &builder = mcctx.builder;
    auto *cache = mcctx.getInlineCache();
    auto *recv = mcctx.varGetRecv();
    mcctx.emitMethodSearch();

    // We could initialize result with the first result (because expectedRubyCFunc is
    // non-empty), but this makes the code slightly cleaner, and LLVM will optimize.
    llvm::Value *result = builder.getInt1(false);
    for (const auto &knownFunc : expectedRubyCFuncs) {
        auto *fnPtrAsAnyFn = knownFunc.getFunction(cs, builder);
        auto current =
            builder.CreateCall(cs.getFunction("sorbet_isCachedMethod"), {cache, fnPtrAsAnyFn, recv}, "isCached");
        result = builder.CreateOr(result, current);
    }

    return result;
}

llvm::Value *CallCacheFlags::build(CompilerState &cs, llvm::IRBuilderBase &builder) {
    static struct {
        bool CallCacheFlags::*field;
        string_view functionName;
        llvm::StringRef flagName;
    } flags[] = {
        {&CallCacheFlags::args_simple, "sorbet_vmCallArgsSimple", "VM_CALL_ARGS_SIMPLE"},
        {&CallCacheFlags::args_splat, "sorbet_vmCallArgsSplat", "VM_CALL_ARGS_SPLAT"},
        {&CallCacheFlags::kwarg, "sorbet_vmCallKwarg", "VM_CALL_KWARG"},
        {&CallCacheFlags::kw_splat, "sorbet_vmCallKwSplat", "VM_CALL_KW_SPLAT"},
        {&CallCacheFlags::fcall, "sorbet_vmCallFCall", "VM_CALL_FCALL"},
        {&CallCacheFlags::blockarg, "sorbet_vmCallArgsBlockarg", "VM_CALL_ARGS_BLOCKARG"},
    };

    llvm::Value *acc = llvm::ConstantInt::get(cs, llvm::APInt(32, 0, false));
    for (auto &flag : flags) {
        if (this->*flag.field) {
            auto *flagVal = builder.CreateCall(cs.getFunction(flag.functionName), {}, flag.flagName);
            acc = builder.CreateBinOp(llvm::Instruction::Or, acc, flagVal);
        }
    }

    return acc;
}

bool IREmitterHelpers::canPassThroughBlockViaRubyVM(MethodCallContext &mcctx, cfg::LocalRef blkVar) {
    auto &irctx = mcctx.irctx;
    auto rubyRegionId = mcctx.rubyRegionId;

    // TODO: all of this logic needs to be modified for blocks that take blocks.
    if (irctx.blockArgUsage != BlockArgUsage::SameFrameAsTopLevel) {
        return false;
    }

    ENFORCE(IREmitterHelpers::hasBlockArgument(mcctx.cs, 0, irctx.cfg.symbol, irctx));

    // The block for the send is not at the same level as the toplevel.
    if (irctx.rubyBlockLevel[rubyRegionId] != 0) {
        return false;
    }

    ENFORCE(!irctx.rubyBlockArgs.empty());

    return blkVar == irctx.rubyBlockArgs[0].back();
}

} // namespace sorbet::compiler
