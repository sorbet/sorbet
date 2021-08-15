// These violate our poisons so have to happen first
#include "llvm/IR/DIBuilder.h"
#include "llvm/IR/DerivedTypes.h" // FunctionType
#include "llvm/IR/IRBuilder.h"

#include "Payload.h"
#include "absl/base/casts.h"
#include "absl/strings/match.h"
#include "absl/strings/str_replace.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "cfg/CFG.h"
#include "common/sort.h"
#include "compiler/Core/CompilerState.h"
#include "compiler/IREmitter/IREmitterContext.h"
#include "compiler/IREmitter/IREmitterHelpers.h"
#include "compiler/IREmitter/MethodCallContext.h"
#include "compiler/Names/Names.h"

using namespace std;
namespace sorbet::compiler {

namespace {
string getFunctionNamePrefix(CompilerState &cs, core::SymbolRef sym) {
    auto maybeAttached = sym.data(cs)->attachedClass(cs);
    if (maybeAttached.exists()) {
        return getFunctionNamePrefix(cs, maybeAttached) + ".singleton_class";
    }
    string suffix;
    auto name = sym.data(cs)->name;
    if (name.kind() == core::NameKind::CONSTANT && name.dataCnst(cs)->original.kind() == core::NameKind::UTF8) {
        suffix = (string)name.shortName(cs);
    } else {
        suffix = name.toString(cs);
    }
    string prefix = IREmitterHelpers::isRootishSymbol(cs, sym.data(cs)->owner)
                        ? ""
                        : getFunctionNamePrefix(cs, sym.data(cs)->owner) + "::";

    return prefix + suffix;
}
} // namespace

string IREmitterHelpers::getFunctionName(CompilerState &cs, core::SymbolRef sym) {
    auto maybeAttachedOwner = sym.data(cs)->owner.data(cs)->attachedClass(cs);
    string prefix = "func_";
    if (maybeAttachedOwner.exists()) {
        prefix = prefix + getFunctionNamePrefix(cs, maybeAttachedOwner) + ".";
    } else {
        prefix = prefix + getFunctionNamePrefix(cs, sym.data(cs)->owner) + "#";
    }

    auto name = sym.data(cs)->name;
    string suffix;
    if (name.kind() == core::NameKind::UTF8) {
        suffix = (string)name.shortName(cs);
    } else {
        suffix = name.toString(cs);
    }

    return prefix + suffix;
}

bool IREmitterHelpers::isFileStaticInit(const core::GlobalState &gs, core::SymbolRef sym) {
    auto name = sym.data(gs)->name;
    if (name.kind() != core::NameKind::UNIQUE) {
        return false;
    }
    return name.dataUnique(gs)->original == core::Names::staticInit();
}

bool IREmitterHelpers::isClassStaticInit(const core::GlobalState &gs, core::SymbolRef sym) {
    return sym.data(gs)->name == core::Names::staticInit();
}

bool IREmitterHelpers::isFileOrClassStaticInit(const core::GlobalState &gs, core::SymbolRef sym) {
    return isFileStaticInit(gs, sym) || isClassStaticInit(gs, sym);
}

namespace {
llvm::GlobalValue::LinkageTypes getFunctionLinkageType(CompilerState &cs, core::SymbolRef sym) {
    if (IREmitterHelpers::isFileOrClassStaticInit(cs, sym)) {
        // this is top level code that shoudln't be callable externally.
        // Even more, sorbet reuses symbols used for these and thus if we mark them non-private we'll get link errors
        return llvm::Function::InternalLinkage;
    }
    return llvm::Function::ExternalLinkage;
}

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

llvm::Function *IREmitterHelpers::lookupFunction(CompilerState &cs, core::SymbolRef sym) {
    ENFORCE(!isClassStaticInit(cs, sym), "use special helper instead");
    auto *func = cs.module->getFunction(IREmitterHelpers::getFunctionName(cs, sym));
    return func;
}
llvm::Function *IREmitterHelpers::getOrCreateFunctionWeak(CompilerState &cs, core::SymbolRef sym) {
    ENFORCE(!isClassStaticInit(cs, sym), "use special helper instead");
    auto *fn = getOrCreateFunctionWithName(cs, IREmitterHelpers::getFunctionName(cs, sym), cs.getRubyFFIType(),
                                           llvm::Function::WeakAnyLinkage);
    // Ensure that the arguments have consistent naming.
    fn->arg_begin()->setName("argc");
    (fn->arg_begin() + 1)->setName("argArray");
    (fn->arg_begin() + 2)->setName("selfRaw");
    (fn->arg_begin() + 3)->setName("cfp");

    return fn;
}

llvm::Function *IREmitterHelpers::getOrCreateFunction(CompilerState &cs, core::SymbolRef sym) {
    ENFORCE(!isClassStaticInit(cs, sym), "use special helper instead");
    return getOrCreateFunctionWithName(cs, IREmitterHelpers::getFunctionName(cs, sym), cs.getRubyFFIType(),
                                       getFunctionLinkageType(cs, sym), true);
}

llvm::Function *IREmitterHelpers::getOrCreateStaticInit(CompilerState &cs, core::SymbolRef sym, core::LocOffsets loc) {
    ENFORCE(isClassStaticInit(cs, sym), "use general helper instead");
    auto name = IREmitterHelpers::getFunctionName(cs, sym) + "L" + to_string(loc.beginPos());
    return getOrCreateFunctionWithName(cs, name, cs.getRubyFFIType(), getFunctionLinkageType(cs, sym), true);
}
llvm::Function *IREmitterHelpers::getInitFunction(CompilerState &cs, core::SymbolRef sym) {
    std::vector<llvm::Type *> NoArgs(0, llvm::Type::getVoidTy(cs));
    auto linkageType = llvm::Function::InternalLinkage;
    auto baseName = IREmitterHelpers::getFunctionName(cs, sym);

    auto ft = llvm::FunctionType::get(llvm::Type::getVoidTy(cs), NoArgs, false);
    return getOrCreateFunctionWithName(cs, "Init_" + baseName, ft, linkageType);
}

// TODO(froydnj): LLVM datatypes don't really have the concept of signedness, only
// LLVM operations.  Does that mean we should just be using IRBuilder::getInt32 etc.?
llvm::Value *IREmitterHelpers::buildU4(CompilerState &cs, u4 i) {
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

void IREmitterHelpers::emitDebugLoc(CompilerState &cs, llvm::IRBuilderBase &build, const IREmitterContext &irctx,
                                    int rubyBlockId, core::Loc loc) {
    auto &builder = static_cast<llvm::IRBuilder<> &>(build);

    auto *scope = irctx.blockScopes[rubyBlockId];
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

void IREmitterHelpers::emitUncheckedReturn(CompilerState &cs, llvm::IRBuilderBase &build, const IREmitterContext &irctx,
                                           int rubyBlockId, llvm::Value *retVal) {
    auto &builder = static_cast<llvm::IRBuilder<> &>(build);

    auto *func = irctx.rubyBlocks2Functions[rubyBlockId];

    auto *throwReturnBlock = llvm::BasicBlock::Create(cs, "throwReturn", func);
    auto *normalReturnBlock = llvm::BasicBlock::Create(cs, "normalReturn", func);

    if (functionTypePushesFrame(irctx.rubyBlockType[rubyBlockId])) {
        builder.CreateCall(cs.getFunction("sorbet_popFrame"), {});
    }
    if (rubyBlockId == 0 && irctx.returnFromBlockState.has_value()) {
        auto &state = *irctx.returnFromBlockState;
        builder.CreateCall(cs.getFunction("sorbet_teardownTagForThrowReturn"), {state.loadEC(cs, builder), state.ecTag});
    }
    auto *throwReturnFlag = builder.CreateLoad(irctx.throwReturnFlagByBlock[rubyBlockId]);
    builder.CreateCondBr(throwReturnFlag, throwReturnBlock, normalReturnBlock);

    builder.SetInsertPoint(throwReturnBlock);
    builder.CreateCall(cs.getFunction("sorbet_throwReturn"), {retVal});
    builder.CreateUnreachable();

    builder.SetInsertPoint(normalReturnBlock);
    builder.CreateRet(retVal);
}

void IREmitterHelpers::emitReturn(CompilerState &cs, llvm::IRBuilderBase &build, const IREmitterContext &irctx,
                                  int rubyBlockId, llvm::Value *retVal) {
    auto &builder = static_cast<llvm::IRBuilder<> &>(build);

    if (functionTypeNeedsPostprocessing(irctx.rubyBlockType[rubyBlockId])) {
        auto returnValue = irctx.cfg.enterLocal({Names::returnValue(cs), 1});
        Payload::varSet(cs, returnValue, retVal, builder, irctx, rubyBlockId);
        builder.CreateBr(irctx.postProcessBlock);
    } else {
        emitUncheckedReturn(cs, builder, irctx, rubyBlockId, retVal);
    }
}

void IREmitterHelpers::setThrowReturnFlag(CompilerState &cs, llvm::IRBuilderBase &build, const IREmitterContext &irctx,
                                          int rubyBlockId) {
    auto &builder = static_cast<llvm::IRBuilder<> &>(build);

    builder.CreateStore(builder.getTrue(), irctx.throwReturnFlagByBlock[rubyBlockId]);
}

namespace {
void buildTypeTestPassFailBlocks(CompilerState &cs, llvm::IRBuilderBase &build, llvm::Value *value,
                                 llvm::Value *testResult, const core::TypePtr &expectedType,
                                 std::string_view description) {
    auto &builder = static_cast<llvm::IRBuilder<> &>(build);

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

void IREmitterHelpers::emitTypeTest(CompilerState &cs, llvm::IRBuilderBase &build, llvm::Value *value,
                                    const core::TypePtr &expectedType, std::string_view description) {
    auto *typeTest = Payload::typeTest(cs, build, value, expectedType);
    buildTypeTestPassFailBlocks(cs, build, value, typeTest, expectedType, description);
}

void IREmitterHelpers::emitTypeTestForBlock(CompilerState &cs, llvm::IRBuilderBase &build, llvm::Value *value,
                                            const core::TypePtr &expectedType, std::string_view description) {
    // Checking for blocks is special.  We don't want to materialize the block (`value`)
    // unless we absolutely have to, so we check the type of blocks by poking at the
    // RubyVM.  (We obviously have materialized the block at this point since we have
    // `value` to inspect, but we have an LLVM optimization pass that will delete the
    // materialization if the result of the materialization is unused.  So we don't
    // want to add any more uses than we have to.)
    auto *typeTest = Payload::typeTestForBlock(cs, build, value, expectedType);
    buildTypeTestPassFailBlocks(cs, build, value, typeTest, expectedType, description);
}

llvm::Value *IREmitterHelpers::emitLiteralish(CompilerState &cs, llvm::IRBuilderBase &build, const core::TypePtr &lit) {
    auto &builder = static_cast<llvm::IRBuilder<> &>(build);
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

bool IREmitterHelpers::hasBlockArgument(CompilerState &cs, int blockId, core::SymbolRef method,
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

    auto &args = method.data(cs)->arguments();
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

    auto name = sym.data(gs)->name;
    if (name.kind() == core::NameKind::UNIQUE) {
        withoutOwnerStr = name.dataUnique(gs)->original.show(gs);
    } else {
        withoutOwnerStr = name.show(gs);
    };

    // This is a little bit gross.  Symbol performs this sort of logic itself, but
    // the above calls are done inside NameRef, which doesn't have the necessary
    // symbol ownership information to do this sort of munging.  So we have to
    // duplicate the Symbol logic here.
    if (sym.data(gs)->owner != core::Symbols::PackageRegistry()) {
        return withoutOwnerStr;
    }

    constexpr string_view packageNameSuffix = "_Package"sv;
    if (!absl::EndsWith(withoutOwnerStr, packageNameSuffix)) {
        return withoutOwnerStr;
    }

    return absl::StrReplaceAll(withoutOwnerStr.substr(0, withoutOwnerStr.size() - packageNameSuffix.size()),
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
    if (sym == core::Symbols::PackageRegistry() || sym.data(gs)->name.isPackagerName(gs)) {
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

    auto funSym = recvSym.data(gs)->findMember(gs, fun);
    if (!funSym.exists()) {
        return std::nullopt;
    }

    if (!funSym.data(gs)->isFinalMethod()) {
        return std::nullopt;
    }

    auto file = funSym.data(gs)->loc().file();
    bool isCompiled = file.data(gs).source().find("# compiled: true\n") != string_view::npos;
    return IREmitterHelpers::FinalMethodInfo{recvSym, funSym, isCompiled};
}

llvm::Value *IREmitterHelpers::receiverFastPathTestWithCache(MethodCallContext &mcctx,
                                                             const vector<string> &expectedRubyCFuncs,
                                                             const string &methodNameForDebug) {
    auto &cs = mcctx.cs;
    auto &builder = static_cast<llvm::IRBuilder<> &>(mcctx.build);

    auto *cache = mcctx.getInlineCache();
    auto *recv = mcctx.varGetRecv();
    mcctx.emitMethodSearch();

    // We could initialize result with the first result (because expectedRubyCFunc is
    // non-empty), but this makes the code slightly cleaner, and LLVM will optimize.
    llvm::Value *result = builder.getInt1(false);
    for (const auto &expectedFunc : expectedRubyCFuncs) {
        auto *expectedFnPtr = cs.getFunction(expectedFunc);
        if (expectedFnPtr == nullptr) {
            Exception::raise("Couldn't find expected Ruby C func `{}` in the current Module. Is it static (private)?",
                             expectedFunc);
        }
        auto *fnPtrAsAnyFn =
            builder.CreatePointerCast(expectedFnPtr, cs.getAnyRubyCApiFunctionType()->getPointerTo(), "fnPtrCast");

        auto current =
            builder.CreateCall(cs.getFunction("sorbet_isCachedMethod"), {cache, fnPtrAsAnyFn, recv}, "isCached");
        result = builder.CreateOr(result, current);
    }

    return result;
}

llvm::Value *CallCacheFlags::build(CompilerState &cs, llvm::IRBuilderBase &build) {
    auto &builder = static_cast<llvm::IRBuilder<> &>(build);

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

} // namespace sorbet::compiler
