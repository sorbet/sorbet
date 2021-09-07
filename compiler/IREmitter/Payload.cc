// These violate our poisons so have to happen first
#include "llvm/IR/DerivedTypes.h" // FunctionType
#include "llvm/IR/IRBuilder.h"

#include "IREmitterHelpers.h"
#include "Payload.h"
#include "ast/Trees.h"
#include "common/sort.h"
#include "common/typecase.h"
#include "compiler/Core/CompilerState.h"
#include "compiler/IREmitter/IREmitterContext.h"
#include <string>

using namespace std;
namespace sorbet::compiler {
namespace {
llvm::IRBuilder<> &builderCast(llvm::IRBuilderBase &builder) {
    return static_cast<llvm::IRBuilder<> &>(builder);
};
} // namespace

llvm::Value *Payload::setExpectedBool(CompilerState &cs, llvm::IRBuilderBase &builder, llvm::Value *value,
                                      bool expected) {
    return builderCast(builder).CreateIntrinsic(llvm::Intrinsic::IndependentIntrinsics::expect,
                                                {llvm::Type::getInt1Ty(cs)}, {value, builder.getInt1(expected)});
}

void Payload::boxRawValue(CompilerState &cs, llvm::IRBuilderBase &builder, llvm::AllocaInst *target,
                          llvm::Value *rawData) {
    builderCast(builder).CreateStore(rawData, builderCast(builder).CreateStructGEP(target, 0));
}

llvm::Value *Payload::unboxRawValue(CompilerState &cs, llvm::IRBuilderBase &builder, llvm::AllocaInst *target) {
    return builderCast(builder).CreateLoad(builderCast(builder).CreateStructGEP(target, 0), "rawRubyValue");
}

llvm::Value *Payload::rubyUndef(CompilerState &cs, llvm::IRBuilderBase &builder) {
    return builderCast(builder).CreateCall(cs.getFunction("sorbet_rubyUndef"), {}, "undefValueRaw");
}

llvm::Value *Payload::rubyNil(CompilerState &cs, llvm::IRBuilderBase &builder) {
    return builderCast(builder).CreateCall(cs.getFunction("sorbet_rubyNil"), {}, "nilValueRaw");
}

llvm::Value *Payload::rubyFalse(CompilerState &cs, llvm::IRBuilderBase &builder) {
    return builderCast(builder).CreateCall(cs.getFunction("sorbet_rubyFalse"), {}, "falseValueRaw");
}

llvm::Value *Payload::rubyTopSelf(CompilerState &cs, llvm::IRBuilderBase &builder) {
    return builderCast(builder).CreateCall(cs.getFunction("sorbet_rubyTopSelf"), {}, "topSelf");
}

llvm::Value *Payload::rubyTrue(CompilerState &cs, llvm::IRBuilderBase &builder) {
    return builderCast(builder).CreateCall(cs.getFunction("sorbet_rubyTrue"), {}, "trueValueRaw");
}

void Payload::raiseArity(CompilerState &cs, llvm::IRBuilderBase &builder, llvm::Value *currentArgCount, int minArgs,
                         int maxArgs) {
    builderCast(builder).CreateCall(cs.getFunction("sorbet_raiseArity"),
                                    {currentArgCount, llvm::ConstantInt::get(cs, llvm::APInt(32, minArgs, true)),
                                     llvm::ConstantInt::get(cs, llvm::APInt(32, maxArgs, true))

                                    });
    builderCast(builder).CreateUnreachable();
}
llvm::Value *Payload::longToRubyValue(CompilerState &cs, llvm::IRBuilderBase &builder, long num) {
    return builderCast(builder).CreateCall(cs.getFunction("sorbet_longToRubyValue"),
                                           {llvm::ConstantInt::get(cs, llvm::APInt(64, num, true))}, "rawRubyInt");
}

llvm::Value *Payload::doubleToRubyValue(CompilerState &cs, llvm::IRBuilderBase &builder, double num) {
    return builderCast(builder).CreateCall(cs.getFunction("sorbet_doubleToRubyValue"),
                                           {llvm::ConstantFP::get(llvm::Type::getDoubleTy(cs), num)}, "rawRubyDouble");
}

llvm::Value *Payload::cPtrToRubyRegexp(CompilerState &cs, llvm::IRBuilderBase &build, std::string_view str,
                                       int options) {
    auto &builder = builderCast(build);
    // all regexp are frozen. We'll allocate it at load time and share it.
    string rawName = "rubyRegexpFrozen_" + (string)str;
    auto tp = llvm::Type::getInt64Ty(cs);
    auto zero = llvm::ConstantInt::get(cs, llvm::APInt(64, 0));
    llvm::Constant *indices[] = {zero};

    auto globalDeclaration = static_cast<llvm::GlobalVariable *>(cs.module->getOrInsertGlobal(rawName, tp, [&] {
        llvm::IRBuilder<> globalInitBuilder(cs);
        auto ret =
            new llvm::GlobalVariable(*cs.module, tp, false, llvm::GlobalVariable::InternalLinkage, zero, rawName);
        ret->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);
        ret->setAlignment(llvm::MaybeAlign(8));
        // create constructor
        std::vector<llvm::Type *> NoArgs(0, llvm::Type::getVoidTy(cs));
        auto ft = llvm::FunctionType::get(llvm::Type::getVoidTy(cs), NoArgs, false);
        auto constr = llvm::Function::Create(ft, llvm::Function::InternalLinkage, {"Constr_", rawName}, *cs.module);

        auto bb = llvm::BasicBlock::Create(cs, "constr", constr);
        globalInitBuilder.SetInsertPoint(bb);
        auto rawCString = Payload::toCString(cs, str, globalInitBuilder);
        auto rawStr =
            globalInitBuilder.CreateCall(cs.getFunction("sorbet_cPtrToRubyRegexpFrozen"),
                                         {rawCString, llvm::ConstantInt::get(cs, llvm::APInt(64, str.length())),

                                          llvm::ConstantInt::get(cs, llvm::APInt(32, options))});
        globalInitBuilder.CreateStore(rawStr,
                                      llvm::ConstantExpr::getInBoundsGetElementPtr(ret->getValueType(), ret, indices));
        globalInitBuilder.CreateRetVoid();
        globalInitBuilder.SetInsertPoint(cs.globalConstructorsEntry);
        globalInitBuilder.CreateCall(constr, {});

        return ret;
    }));

    ENFORCE(cs.functionEntryInitializers->getParent() == builder.GetInsertBlock()->getParent(),
            "you're calling this function from something low-level that passed a IRBuilder that points outside of "
            "function currently being generated");
    auto oldInsertPoint = builder.saveIP();
    builder.SetInsertPoint(cs.functionEntryInitializers);
    auto name = llvm::StringRef(str.data(), str.length());
    auto global = builder.CreateLoad(
        llvm::ConstantExpr::getInBoundsGetElementPtr(globalDeclaration->getValueType(), globalDeclaration, indices),
        {"rubyRegexp_", name});
    builder.restoreIP(oldInsertPoint);

    // todo(perf): mark these as immutable with https://llvm.org/docs/LangRef.html#llvm-invariant-start-intrinsic
    return global;
}

llvm::Value *Payload::cPtrToRubyString(CompilerState &cs, llvm::IRBuilderBase &build, std::string_view str,
                                       bool frozen) {
    auto &builder = builderCast(build);
    if (!frozen) {
        auto rawCString = Payload::toCString(cs, str, builder);
        return builder.CreateCall(cs.getFunction("sorbet_cPtrToRubyString"),
                                  {rawCString, llvm::ConstantInt::get(cs, llvm::APInt(64, str.length(), true))},
                                  "rawRubyStr");
    }
    // this is a frozen string. We'll allocate it at load time and share it.
    string rawName = "rubyStrFrozen_" + (string)str;
    auto tp = llvm::Type::getInt64Ty(cs);
    auto zero = llvm::ConstantInt::get(cs, llvm::APInt(64, 0));
    llvm::Constant *indices[] = {zero};

    auto globalDeclaration = static_cast<llvm::GlobalVariable *>(cs.module->getOrInsertGlobal(rawName, tp, [&] {
        llvm::IRBuilder<> globalInitBuilder(cs);
        auto ret =
            new llvm::GlobalVariable(*cs.module, tp, false, llvm::GlobalVariable::InternalLinkage, zero, rawName);
        ret->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);
        ret->setAlignment(llvm::MaybeAlign(8));
        // create constructor
        std::vector<llvm::Type *> NoArgs(0, llvm::Type::getVoidTy(cs));
        auto ft = llvm::FunctionType::get(llvm::Type::getVoidTy(cs), NoArgs, false);
        auto constr = llvm::Function::Create(ft, llvm::Function::InternalLinkage, {"Constr_", rawName}, *cs.module);

        auto bb = llvm::BasicBlock::Create(cs, "constr", constr);
        globalInitBuilder.SetInsertPoint(bb);
        auto rawCString = Payload::toCString(cs, str, globalInitBuilder);
        auto rawStr =
            globalInitBuilder.CreateCall(cs.getFunction("sorbet_cPtrToRubyStringFrozen"),
                                         {rawCString, llvm::ConstantInt::get(cs, llvm::APInt(64, str.length()))});
        globalInitBuilder.CreateStore(rawStr,
                                      llvm::ConstantExpr::getInBoundsGetElementPtr(ret->getValueType(), ret, indices));
        globalInitBuilder.CreateRetVoid();
        globalInitBuilder.SetInsertPoint(cs.globalConstructorsEntry);
        globalInitBuilder.CreateCall(constr, {});

        return ret;
    }));

    auto name = llvm::StringRef(str.data(), str.length());
    auto global = builder.CreateLoad(
        llvm::ConstantExpr::getInBoundsGetElementPtr(globalDeclaration->getValueType(), globalDeclaration, indices),
        {"rubyStr_", name});

    // todo(perf): mark these as immutable with https://llvm.org/docs/LangRef.html#llvm-invariant-start-intrinsic
    return global;
}

llvm::Value *Payload::testIsUndef(CompilerState &cs, llvm::IRBuilderBase &builder, llvm::Value *val) {
    return builderCast(builder).CreateCall(cs.getFunction("sorbet_testIsUndef"), {val}, "isUndef");
}

llvm::Value *Payload::testIsTruthy(CompilerState &cs, llvm::IRBuilderBase &builder, llvm::Value *val) {
    return builderCast(builder).CreateCall(cs.getFunction("sorbet_testIsTruthy"), {val}, "cond");
}

llvm::Value *Payload::idIntern(CompilerState &cs, llvm::IRBuilderBase &build, std::string_view idName) {
    auto &builder = builderCast(build);
    auto zero = llvm::ConstantInt::get(cs, llvm::APInt(64, 0));
    auto name = llvm::StringRef(idName.data(), idName.length());
    llvm::Constant *indices[] = {zero};
    string rawName = "rubyIdPrecomputed_" + (string)idName;
    auto tp = llvm::Type::getInt64Ty(cs);
    auto globalDeclaration = static_cast<llvm::GlobalVariable *>(cs.module->getOrInsertGlobal(rawName, tp, [&] {
        llvm::IRBuilder<> globalInitBuilder(cs);
        auto ret =
            new llvm::GlobalVariable(*cs.module, tp, false, llvm::GlobalVariable::InternalLinkage, zero, rawName);
        ret->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);
        ret->setAlignment(llvm::MaybeAlign(8));
        // create constructor
        std::vector<llvm::Type *> NoArgs(0, llvm::Type::getVoidTy(cs));
        auto ft = llvm::FunctionType::get(llvm::Type::getVoidTy(cs), NoArgs, false);
        auto constr = llvm::Function::Create(ft, llvm::Function::InternalLinkage, {"Constr_", rawName}, *cs.module);

        auto bb = llvm::BasicBlock::Create(cs, "constr", constr);
        globalInitBuilder.SetInsertPoint(bb);
        auto rawCString = Payload::toCString(cs, idName, globalInitBuilder);
        auto rawID = globalInitBuilder.CreateCall(
            cs.getFunction("sorbet_idIntern"),
            {rawCString, llvm::ConstantInt::get(cs, llvm::APInt(64, idName.length()))}, "rawId");
        globalInitBuilder.CreateStore(rawID,
                                      llvm::ConstantExpr::getInBoundsGetElementPtr(ret->getValueType(), ret, indices));
        globalInitBuilder.CreateRetVoid();
        globalInitBuilder.SetInsertPoint(cs.allocRubyIdsEntry);
        globalInitBuilder.CreateCall(constr, {});

        return ret;
    }));

    auto global = builder.CreateLoad(
        llvm::ConstantExpr::getInBoundsGetElementPtr(globalDeclaration->getValueType(), globalDeclaration, indices),
        {"rubyId_", name});

    // todo(perf): mark these as immutable with https://llvm.org/docs/LangRef.html#llvm-invariant-start-intrinsic
    return global;
}

namespace {
std::string showClassName(const core::GlobalState &gs, core::SymbolRef sym) {
    auto owner = sym.data(gs)->owner;
    bool includeOwner = !IREmitterHelpers::isRootishSymbol(gs, owner);
    string ownerStr = includeOwner ? showClassName(gs, owner) + "::" : "";
    return ownerStr + IREmitterHelpers::showClassNameWithoutOwner(gs, sym);
}

} // namespace

llvm::Value *Payload::getRubyConstant(CompilerState &cs, core::SymbolRef sym, llvm::IRBuilderBase &build) {
    ENFORCE(sym.data(cs)->isClassOrModule() || sym.data(cs)->isStaticField() || sym.data(cs)->isTypeMember());
    auto &builder = builderCast(build);
    sym = IREmitterHelpers::fixupOwningSymbol(cs, sym);
    auto str = showClassName(cs, sym);
    ENFORCE(str.length() < 2 || (str[0] != ':'), "implementation assumes that strings dont start with ::");
    auto functionName = sym.data(cs)->isClassOrModule() ? "sorbet_i_getRubyClass" : "sorbet_i_getRubyConstant";
    return builder.CreateCall(
        cs.getFunction(functionName),
        {Payload::toCString(cs, str, builder), llvm::ConstantInt::get(cs, llvm::APInt(64, str.length()))});
}

llvm::Value *Payload::toCString(CompilerState &cs, string_view str, llvm::IRBuilderBase &builder) {
    llvm::StringRef valueRef(str.data(), str.length());
    auto globalName = "addr_str_" + (string)str;
    auto globalDeclaration =
        static_cast<llvm::GlobalVariable *>(cs.module->getOrInsertGlobal(globalName, builder.getInt8PtrTy(), [&] {
            auto valueGlobal = builder.CreateGlobalString(valueRef, llvm::Twine("str_") + valueRef);
            auto zero = llvm::ConstantInt::get(cs, llvm::APInt(64, 0));
            llvm::Constant *indicesString[] = {zero, zero};
            auto addrGlobalInitializer =
                llvm::ConstantExpr::getInBoundsGetElementPtr(valueGlobal->getValueType(), valueGlobal, indicesString);
            auto addrGlobal =
                new llvm::GlobalVariable(*cs.module, builder.getInt8PtrTy(), true,
                                         llvm::GlobalVariable::InternalLinkage, addrGlobalInitializer, globalName);
            addrGlobal->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);
            addrGlobal->setAlignment(llvm::MaybeAlign(8));

            return addrGlobal;
        }));

    return builderCast(builder).CreateLoad(globalDeclaration);
}

namespace {
const vector<pair<core::ClassOrModuleRef, string>> optimizedTypeTests = {
    {core::Symbols::untyped(), "sorbet_i_isa_Untyped"},
    {core::Symbols::Array(), "sorbet_i_isa_Array"},
    {core::Symbols::FalseClass(), "sorbet_i_isa_FalseClass"},
    {core::Symbols::TrueClass(), "sorbet_i_isa_TrueClass"},
    {core::Symbols::Float(), "sorbet_i_isa_Float"},
    {core::Symbols::Hash(), "sorbet_i_isa_Hash"},
    {core::Symbols::Integer(), "sorbet_i_isa_Integer"},
    {core::Symbols::NilClass(), "sorbet_i_isa_NilClass"},
    {core::Symbols::Proc(), "sorbet_i_isa_Proc"},
    {core::Symbols::Rational(), "sorbet_i_isa_Rational"},
    {core::Symbols::Regexp(), "sorbet_i_isa_Regexp"},
    {core::Symbols::String(), "sorbet_i_isa_String"},
    {core::Symbols::Symbol(), "sorbet_i_isa_Symbol"},
    {core::Symbols::rootSingleton(), "sorbet_i_isa_RootSingleton"},
};

bool hasOptimizedTest(core::ClassOrModuleRef sym) {
    return absl::c_any_of(optimizedTypeTests, [sym](const auto &pair) { return pair.first == sym; });
}

bool hasOptimizedTest(const core::TypePtr &type) {
    bool res = false;
    typecase(
        type, [&res](const core::ClassType &ct) { res = hasOptimizedTest(ct.symbol); },
        [&res](const core::AppliedType &at) { res = hasOptimizedTest(at.klass); }, [](const core::TypePtr &def) {});

    return res;
}

void flattenAndType(vector<core::TypePtr> &results, const core::AndType &type) {
    typecase(
        type.left, [&results](const core::AndType &type) { flattenAndType(results, type); },
        [&results](const core::TypePtr &def) { results.emplace_back(def); });

    typecase(
        type.right, [&results](const core::AndType &type) { flattenAndType(results, type); },
        [&results](const core::TypePtr &def) { results.emplace_back(def); });
}

void flattenOrType(vector<core::TypePtr> &results, const core::OrType &type) {
    typecase(
        type.left, [&results](const core::OrType &type) { flattenOrType(results, type); },
        [&results](const core::TypePtr &def) { results.emplace_back(def); });

    typecase(
        type.right, [&results](const core::OrType &type) { flattenOrType(results, type); },
        [&results](const core::TypePtr &def) { results.emplace_back(def); });
}

} // namespace

static bool isProc(core::SymbolRef sym) {
    if (sym.kind() != core::SymbolRef::Kind::ClassOrModule) {
        return false;
    }
    auto id = sym.classOrModuleIndex();
    return id >= core::Symbols::Proc0().id() && id <= core::Symbols::last_proc().id();
}

llvm::Value *Payload::typeTest(CompilerState &cs, llvm::IRBuilderBase &b, llvm::Value *val, const core::TypePtr &type) {
    auto &builder = builderCast(b);
    llvm::Value *ret = nullptr;
    typecase(
        type,
        [&](const core::ClassType &ct) {
            for (const auto &[candidate, specializedCall] : optimizedTypeTests) {
                if (ct.symbol == candidate) {
                    ret = builder.CreateCall(cs.getFunction(specializedCall), {val});
                    return;
                }
            }

            if (ct.symbol.data(cs)->name.isTEnumName(cs)) {
                // T.let(..., MyEnum::X$1) is special. These are singleton values, so we can do a type
                // test with an object (reference) equality check.
                ret = builder.CreateCall(cs.getFunction("sorbet_testObjectEqual_p"),
                                         {Payload::getRubyConstant(cs, ct.symbol, builder), val});
                return;
            }

            auto attachedClass = ct.symbol.data(cs)->attachedClass(cs);
            // todo: handle attached of attached class
            if (attachedClass.exists()) {
                ret = builder.CreateCall(cs.getFunction("sorbet_isa_class_of"),
                                         {val, Payload::getRubyConstant(cs, attachedClass, builder)});
                return;
            }
            auto sym = isProc(ct.symbol) ? core::Symbols::Proc() : ct.symbol;
            ret = builder.CreateCall(cs.getFunction("sorbet_isa"), {val, Payload::getRubyConstant(cs, sym, builder)});
        },
        [&](const core::AppliedType &at) {
            core::ClassOrModuleRef klass = at.klass;
            auto base = typeTest(cs, builder, val, core::make_type<core::ClassType>(klass));
            ret = base;
            // todo: ranges, hashes, sets, enumerator, and, overall, enumerables
        },
        [&](const core::OrType &ct) {
            // flatten the or, and order it so that the optimized type tests show up first
            vector<core::TypePtr> parts;
            flattenOrType(parts, ct);
            absl::c_partition(parts, [](const auto &ty) { return hasOptimizedTest(ty); });

            // forward-declare the exit
            auto *fun = builder.GetInsertBlock()->getParent();
            auto *exitBlock = llvm::BasicBlock::Create(cs, "orContinue", fun);
            llvm::PHINode *phi;

            {
                auto ip = builder.saveIP();
                builder.SetInsertPoint(exitBlock);
                phi = builder.CreatePHI(builder.getInt1Ty(), 2, "orTypeTest");
                builder.restoreIP(ip);
            }

            llvm::Value *testResult = nullptr;
            for (const auto &part : parts) {
                // for all cases after the first, close the previous block by adding a conditional branch
                if (testResult != nullptr) {
                    auto *block = llvm::BasicBlock::Create(cs, "orCase", fun);
                    builder.CreateCondBr(testResult, exitBlock, block);
                    builder.SetInsertPoint(block);
                }

                testResult = typeTest(cs, builder, val, part);
                phi->addIncoming(testResult, builder.GetInsertBlock());
            }

            // close the last block by adding an unconditional branch to the exit block
            builder.CreateBr(exitBlock);
            builder.SetInsertPoint(exitBlock);
            ret = phi;
        },
        [&](const core::AndType &ct) {
            // flatten the and, and order it so that the optimized type tests show up first
            vector<core::TypePtr> parts;
            flattenAndType(parts, ct);
            absl::c_partition(parts, [](const auto &ty) { return hasOptimizedTest(ty); });

            // forward-declare the exit
            auto *fun = builder.GetInsertBlock()->getParent();
            auto *exitBlock = llvm::BasicBlock::Create(cs, "andContinue", fun);
            llvm::PHINode *phi;

            {
                auto ip = builder.saveIP();
                builder.SetInsertPoint(exitBlock);
                phi = builder.CreatePHI(builder.getInt1Ty(), 2, "andTypeTest");
                builder.restoreIP(ip);
            }

            llvm::Value *testResult = nullptr;
            for (const auto &part : parts) {
                // for all cases after the first, close the previous block by adding a conditional branch
                if (testResult != nullptr) {
                    auto *block = llvm::BasicBlock::Create(cs, "andCase", fun);
                    builder.CreateCondBr(testResult, block, exitBlock);
                    builder.SetInsertPoint(block);
                }

                testResult = typeTest(cs, builder, val, part);
                phi->addIncoming(testResult, builder.GetInsertBlock());
            }

            // close the last block by adding an unconditional branch to the exit block
            builder.CreateBr(exitBlock);
            builder.SetInsertPoint(exitBlock);
            ret = phi;
        },
        [&](const core::TypePtr &_default) { ret = builder.getInt1(true); });
    ENFORCE(ret != nullptr);
    return ret;
}

llvm::Value *Payload::typeTestForBlock(CompilerState &cs, llvm::IRBuilderBase &b, llvm::Value *val,
                                       const core::TypePtr &type) {
    auto &builder = builderCast(b);
    core::SymbolRef procSym;

    // We are deliberately not being exhaustive here and only handling the "easy"
    // Proc-like types.  T.any(proc-ish, proc-ish) and similar does not seem like
    // they are worth the trouble.
    if (core::isa_type<core::ClassType>(type)) {
        auto ct = core::cast_type_nonnull<core::ClassType>(type);
        procSym = ct.symbol;
    } else if (core::isa_type<core::AppliedType>(type)) {
        auto at = core::cast_type_nonnull<core::AppliedType>(type);
        procSym = at.klass;
    }

    if (procSym.exists() && (procSym == core::Symbols::Proc() || isProc(procSym))) {
        return builder.CreateCall(cs.getFunction("sorbet_block_isa_proc"), {});
    }

    return typeTest(cs, b, val, type);
}

// Emit an `llvm.assume` intrinsic with the result of a `Payload::typeTest` with the given symbol. For example, this can
// be used assert that a value will be an array.
//
// NOTE: this will make the optimizer behave as though the type test can never be false, and undefined behavior will
// arise if it is false at runtime. If it's not clear that the type test will always be true, err on the side of not
// adding an assertion.
void Payload::assumeType(CompilerState &cs, llvm::IRBuilderBase &builder, llvm::Value *val,
                         core::ClassOrModuleRef sym) {
    auto type = core::make_type<core::ClassType>(sym);
    auto *cond = Payload::typeTest(cs, builder, val, type);
    builderCast(builder).CreateIntrinsic(llvm::Intrinsic::IndependentIntrinsics::assume, {}, {cond});
    return;
}

llvm::Value *Payload::boolToRuby(CompilerState &cs, llvm::IRBuilderBase &builder, llvm::Value *u1) {
    return builderCast(builder).CreateCall(cs.getFunction("sorbet_boolToRuby"), {u1}, "rubyBool");
}

namespace {

llvm::Value *allocateRubyStackFrames(CompilerState &cs, llvm::IRBuilderBase &build, const IREmitterContext &irctx,
                                     const ast::MethodDef &md, int rubyBlockId);

llvm::Value *getIseqType(CompilerState &cs, llvm::IRBuilderBase &build, const IREmitterContext &irctx,
                         int rubyBlockId) {
    auto &builder = builderCast(build);
    switch (irctx.rubyBlockType[rubyBlockId]) {
        case FunctionType::Method:
            return builder.CreateCall(cs.getFunction("sorbet_rubyIseqTypeMethod"), {}, "ISEQ_TYPE_METHOD");

        case FunctionType::StaticInitFile:
            return builder.CreateCall(cs.getFunction("sorbet_rubyIseqTypeTop"), {}, "ISEQ_TYPE_TOP");

        case FunctionType::StaticInitModule:
            return builder.CreateCall(cs.getFunction("sorbet_rubyIseqTypeClass"), {}, "ISEQ_TYPE_CLASS");

        case FunctionType::Block:
            return builder.CreateCall(cs.getFunction("sorbet_rubyIseqTypeBlock"), {}, "ISEQ_TYPE_BLOCK");

        case FunctionType::Rescue:
            return builder.CreateCall(cs.getFunction("sorbet_rubyIseqTypeRescue"), {}, "ISEQ_TYPE_RESCUE");

        case FunctionType::Ensure:
            return builder.CreateCall(cs.getFunction("sorbet_rubyIseqTypeEnsure"), {}, "ISEQ_TYPE_ENSURE");

        case FunctionType::ExceptionBegin:
            // Exception body functions inherit the iseq entry for their containing context, so we should never be
            // generating an iseq entry for them.
            Exception::raise("Allocating an iseq for a FunctionType::ExceptionBegin function");
            break;

        case FunctionType::Unused:
            // This should never happen, as we should be skipping iseq initialization for unused functions.
            Exception::raise("Picking an ISEQ_TYPE for an unused function!");
            break;
    }
}

llvm::PointerType *iseqType(CompilerState &cs) {
    return llvm::PointerType::getUnqual(llvm::StructType::getTypeByName(cs, "struct.rb_iseq_struct"));
}

// Given a Ruby block, finds the block id of the nearest _proper_ ancestor of that block that allocates an iseq.
int getNearestIseqAllocatorBlock(const IREmitterContext &irctx, int rubyBlockId) {
    do {
        rubyBlockId = irctx.rubyBlockParent[rubyBlockId];
    } while (rubyBlockId > 0 && irctx.rubyBlockType[rubyBlockId] == FunctionType::ExceptionBegin);

    return rubyBlockId;
}

std::tuple<const string &, llvm::Value *> getIseqInfo(CompilerState &cs, llvm::IRBuilderBase &build,
                                                      const IREmitterContext &irctx, const ast::MethodDef &md,
                                                      int rubyBlockId) {
    auto &locationName = irctx.rubyBlockLocationNames[rubyBlockId];
    llvm::Value *parent = nullptr;
    switch (irctx.rubyBlockType[rubyBlockId]) {
        case FunctionType::Method:
        case FunctionType::StaticInitFile:
        case FunctionType::StaticInitModule:
            parent = llvm::Constant::getNullValue(iseqType(cs));
            break;

        case FunctionType::Block:
        case FunctionType::Rescue:
        case FunctionType::Ensure:
            parent = allocateRubyStackFrames(cs, build, irctx, md, getNearestIseqAllocatorBlock(irctx, rubyBlockId));
            break;

        case FunctionType::ExceptionBegin:
            // Exception body functions inherit the iseq entry for their containing context, so we should never be
            // generating an iseq entry for them.
            Exception::raise("Allocating an iseq for a FunctionType::ExceptionBegin function");
            break;

        case FunctionType::Unused:
            // This should never happen, as we should be skipping iseq initialization for unused functions.
            Exception::raise("Picking an ISEQ_TYPE for an unused function!");
            break;
    }

    // If we get here, we know we have a valid iseq and a valid name.
    ENFORCE(locationName.has_value());
    return {*locationName, parent};
}

// Fill the locals array with interned ruby IDs.
void fillLocals(CompilerState &cs, llvm::IRBuilderBase &build, const IREmitterContext &irctx, int rubyBlockId,
                int baseOffset, llvm::Value *locals) {
    auto &builder = builderCast(build);

    // The map used to store escaped variables isn't stable, so we first sort it into a vector. This isn't great, but
    // without this step the locals are processed in random order, making the llvm output unstable.
    vector<pair<cfg::LocalRef, int>> escapedVariables{};
    for (auto &entry : irctx.escapedVariableIndices) {
        escapedVariables.emplace_back(entry);
    }

    fast_sort(escapedVariables, [](const auto &left, const auto &right) -> bool { return left.second < right.second; });

    for (auto &entry : escapedVariables) {
        auto *id = Payload::idIntern(cs, builder, entry.first.data(irctx.cfg)._name.shortName(cs));
        auto *offset = llvm::ConstantInt::get(cs, llvm::APInt(32, baseOffset + entry.second, false));
        llvm::Value *indices[] = {offset};
        builder.CreateStore(id, builder.CreateGEP(locals, indices));
    }
}

// A description of where local variables' IDs are held prior to calling
// `sorbet_allocateRubyStackFrame`.
enum class LocalsIDStorage {
    // A normal method or static init function: we will allocate a temporar
    //  array on the C stack.
    Stack,

    // Blocks and exception-related functions: locals are inherited from the containing
    // method, which already allocated ID storage according to one of the other enum
    // values.  We don't need to allocate any space.
    Inherited,
};

LocalsIDStorage classifyStorageFor(CompilerState &cs, const IREmitterContext &irctx, const ast::MethodDef &md,
                                   int rubyBlockId) {
    switch (irctx.rubyBlockType[rubyBlockId]) {
        case FunctionType::Method:
        case FunctionType::StaticInitFile:
        case FunctionType::StaticInitModule:
            return LocalsIDStorage::Stack;

        case FunctionType::Block:
        case FunctionType::Rescue:
        case FunctionType::Ensure:
        case FunctionType::ExceptionBegin:
        case FunctionType::Unused:
            return LocalsIDStorage::Inherited;
    }
}

// Allocate an array to hold local variable ids before calling `sorbet_allocateRubyStackFrame`.
tuple<llvm::Value *, llvm::Value *> getLocals(CompilerState &cs, llvm::IRBuilderBase &build,
                                              const IREmitterContext &irctx, const ast::MethodDef &md,
                                              int rubyBlockId) {
    auto &builder = builderCast(build);
    llvm::Value *locals = nullptr;
    llvm::Value *numLocals = nullptr;
    auto *idType = llvm::Type::getInt64Ty(cs);
    auto *idPtrType = llvm::PointerType::getUnqual(idType);

    auto storageKind = classifyStorageFor(cs, irctx, md, rubyBlockId);
    switch (storageKind) {
        case LocalsIDStorage::Inherited:
            numLocals = llvm::ConstantInt::get(cs, llvm::APInt(32, 0, true));
            locals = llvm::ConstantPointerNull::get(idPtrType);
            break;

        case LocalsIDStorage::Stack:
            numLocals = llvm::ConstantInt::get(cs, llvm::APInt(32, irctx.escapedVariableIndices.size(), true));
            locals = builder.CreateAlloca(idType, numLocals, "locals");
            fillLocals(cs, builder, irctx, rubyBlockId, 0, locals);
            break;
    }

    return {locals, numLocals};
}

llvm::Function *allocateRubyStackFramesImpl(CompilerState &cs, const IREmitterContext &irctx, const ast::MethodDef &md,
                                            int rubyBlockId, llvm::GlobalVariable *store) {
    std::vector<llvm::Type *> argTys{llvm::Type::getInt64Ty(cs)};
    auto ft = llvm::FunctionType::get(llvm::Type::getVoidTy(cs), argTys, false);
    auto constr =
        llvm::Function::Create(ft, llvm::Function::InternalLinkage, {"Constr_", store->getName()}, *cs.module);

    auto realpath = constr->arg_begin();
    realpath->setName("realpath");

    auto bei = llvm::BasicBlock::Create(cs, "entryInitializers", constr);
    auto bb = llvm::BasicBlock::Create(cs, "constr", constr);

    llvm::IRBuilder<> builder(cs);
    builder.SetInsertPoint(bb);

    // We are building a new function. We should redefine where do function initializers go
    auto cs1 = cs.withFunctionEntry(bei);

    auto loc = md.symbol.data(cs)->loc();
    auto file = cs.file;
    auto *iseqType = getIseqType(cs1, builder, irctx, rubyBlockId);
    auto [funcName, parent] = getIseqInfo(cs1, builder, irctx, md, rubyBlockId);
    auto funcNameId = Payload::idIntern(cs1, builder, funcName);
    auto funcNameValue = Payload::cPtrToRubyString(cs1, builder, funcName, true);
    auto filename = file.data(cs).path();
    auto filenameValue = Payload::cPtrToRubyString(cs1, builder, filename, true);
    // The method might have been synthesized by Sorbet (e.g. in the case of packages).
    // Give such methods line numbers of 0.
    unsigned startLine;
    if (loc.exists()) {
        startLine = loc.position(cs).first.line;
    } else {
        startLine = 0;
    }
    auto [locals, numLocals] = getLocals(cs1, builder, irctx, md, rubyBlockId);
    auto sendMax = llvm::ConstantInt::get(cs, llvm::APInt(32, irctx.maxSendArgCount, true));
    auto *fileLineNumberInfo = Payload::getFileLineNumberInfo(cs, builder, file);
    auto *fn = cs.getFunction("sorbet_allocateRubyStackFrame");
    auto ret = builder.CreateCall(fn, {funcNameValue, funcNameId, filenameValue, realpath, parent, iseqType,
                                       llvm::ConstantInt::get(cs, llvm::APInt(32, startLine)), fileLineNumberInfo,
                                       locals, numLocals, sendMax});
    auto zero = llvm::ConstantInt::get(cs, llvm::APInt(64, 0));
    llvm::Constant *indices[] = {zero};
    builder.CreateStore(ret, llvm::ConstantExpr::getInBoundsGetElementPtr(store->getValueType(), store, indices));
    builder.CreateRetVoid();
    builder.SetInsertPoint(bei);
    builder.CreateBr(bb);
    return constr;
}

// The common suffix for stack frame related global names.
string getStackFrameGlobalName(CompilerState &cs, const IREmitterContext &irctx, core::SymbolRef methodSym,
                               int rubyBlockId) {
    auto name = IREmitterHelpers::getFunctionName(cs, methodSym);

    switch (irctx.rubyBlockType[rubyBlockId]) {
        case FunctionType::Method:
        case FunctionType::StaticInitFile:
        case FunctionType::StaticInitModule:
            return name;

        case FunctionType::Block:
        case FunctionType::Rescue:
        case FunctionType::Ensure:
        case FunctionType::ExceptionBegin:
        case FunctionType::Unused:
            return name + "$block_" + std::to_string(rubyBlockId);
    }
}

llvm::GlobalVariable *rubyStackFrameVar(CompilerState &cs, llvm::IRBuilderBase &build, const IREmitterContext &irctx,
                                        core::SymbolRef methodSym, int rubyBlockId) {
    auto tp = iseqType(cs);
    auto name = getStackFrameGlobalName(cs, irctx, methodSym, rubyBlockId);
    string rawName = "stackFramePrecomputed_" + name;
    auto *var = static_cast<llvm::GlobalVariable *>(cs.module->getOrInsertGlobal(rawName, tp, [&] {
        auto ret =
            new llvm::GlobalVariable(*cs.module, tp, false, llvm::GlobalVariable::InternalLinkage, nullptr, rawName);
        ret->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);
        ret->setAlignment(llvm::MaybeAlign(8));

        return ret;
    }));

    return var;
}

llvm::Value *allocateRubyStackFrames(CompilerState &cs, llvm::IRBuilderBase &build, const IREmitterContext &irctx,
                                     const ast::MethodDef &md, int rubyBlockId) {
    llvm::IRBuilder<> globalInitBuilder(cs);
    auto globalDeclaration = rubyStackFrameVar(cs, build, irctx, md.symbol, rubyBlockId);
    if (!globalDeclaration->hasInitializer()) {
        auto nullv = llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(globalDeclaration->getValueType()));
        globalDeclaration->setInitializer(nullv);

        // The realpath is the first argument to `sorbet_globalConstructors`
        auto realpath = cs.globalConstructorsEntry->getParent()->arg_begin();
        realpath->setName("realpath");

        // create constructor
        auto constr = allocateRubyStackFramesImpl(cs, irctx, md, rubyBlockId, globalDeclaration);
        globalInitBuilder.SetInsertPoint(cs.globalConstructorsEntry);
        globalInitBuilder.CreateCall(constr, {realpath});
    }

    globalInitBuilder.SetInsertPoint(cs.functionEntryInitializers);
    auto zero = llvm::ConstantInt::get(cs, llvm::APInt(64, 0));
    llvm::Constant *indices[] = {zero};
    auto global = globalInitBuilder.CreateLoad(
        llvm::ConstantExpr::getInBoundsGetElementPtr(globalDeclaration->getValueType(), globalDeclaration, indices),
        "stackFrame");

    // todo(perf): mark these as immutable with https://llvm.org/docs/LangRef.html#llvm-invariant-start-intrinsic
    return global;
}

} // namespace

llvm::Value *Payload::rubyStackFrameVar(CompilerState &cs, llvm::IRBuilderBase &build, const IREmitterContext &irctx,
                                        core::SymbolRef methodSym) {
    return ::sorbet::compiler::rubyStackFrameVar(cs, build, irctx, methodSym, 0);
}

llvm::Value *Payload::setRubyStackFrame(CompilerState &cs, llvm::IRBuilderBase &build, const IREmitterContext &irctx,
                                        const ast::MethodDef &md, int rubyBlockId) {
    auto &builder = builderCast(build);
    auto stackFrame = allocateRubyStackFrames(cs, builder, irctx, md, rubyBlockId);
    auto *iseqType = getIseqType(cs, builder, irctx, rubyBlockId);
    auto *isStaticInit = llvm::ConstantInt::get(
        cs, llvm::APInt(1, static_cast<int>(irctx.rubyBlockType[rubyBlockId] == FunctionType::StaticInitFile ||
                                            irctx.rubyBlockType[rubyBlockId] == FunctionType::StaticInitModule)));
    builder.CreateCall(cs.getFunction("sorbet_setRubyStackFrame"), {isStaticInit, iseqType, stackFrame});
    auto *cfp = getCFPForBlock(cs, builder, irctx, rubyBlockId);
    auto pc = builder.CreateCall(cs.getFunction("sorbet_getPc"), {cfp});
    return pc;
}

// Ensure that the retry singleton is present during module initialization, and store it in a module-local global.
llvm::Value *Payload::retrySingleton(CompilerState &cs, llvm::IRBuilderBase &build, const IREmitterContext &irctx) {
    auto tp = llvm::Type::getInt64Ty(cs);
    string rawName = "<retry-singleton>";
    auto *global = cs.module->getOrInsertGlobal(rawName, tp, [&] {
        auto globalInitBuilder = llvm::IRBuilder<>(cs);

        auto isConstant = false;
        auto zero = llvm::ConstantInt::get(cs, llvm::APInt(64, 0, true));
        auto global =
            new llvm::GlobalVariable(*cs.module, tp, isConstant, llvm::GlobalVariable::InternalLinkage, zero, rawName);

        globalInitBuilder.SetInsertPoint(cs.globalConstructorsEntry);
        auto *singletonValue = globalInitBuilder.CreateCall(cs.getFunction("sorbet_getTRetry"), {}, "retrySingleton");

        globalInitBuilder.CreateStore(singletonValue, global);

        return global;
    });

    return builderCast(build).CreateLoad(global, rawName);
}

// Ensure that the VOID singleton is present during module initialization, and store it in a module-local global.
llvm::Value *Payload::voidSingleton(CompilerState &cs, llvm::IRBuilderBase &build, const IREmitterContext &irctx) {
    auto tp = llvm::Type::getInt64Ty(cs);
    string rawName = "<void-singleton>";
    auto *global = cs.module->getOrInsertGlobal(rawName, tp, [&] {
        auto globalInitBuilder = llvm::IRBuilder<>(cs);

        auto isConstant = false;
        auto zero = llvm::ConstantInt::get(cs, llvm::APInt(64, 0, true));
        auto global =
            new llvm::GlobalVariable(*cs.module, tp, isConstant, llvm::GlobalVariable::InternalLinkage, zero, rawName);

        globalInitBuilder.SetInsertPoint(cs.globalConstructorsEntry);
        auto *singletonValue =
            globalInitBuilder.CreateCall(cs.getFunction("sorbet_getVoidSingleton"), {}, "voidSingleton");

        globalInitBuilder.CreateStore(singletonValue, global);

        return global;
    });

    return builderCast(build).CreateLoad(global, rawName);
}

// Lazily initialize a global that contains enough noops to represent all the lines in the file as an iseq_encoded
// array.
llvm::Value *Payload::getFileLineNumberInfo(CompilerState &cs, llvm::IRBuilderBase &build, core::FileRef file) {
    auto *iseqEncodedInitFn = cs.module->getFunction("sorbet_initLineNumberInfo");
    auto *infoPointerTy = iseqEncodedInitFn->getFunctionType()->params()[0];
    ENFORCE(infoPointerTy != nullptr);

    auto *globalTy = llvm::cast<llvm::PointerType>(infoPointerTy)->getElementType();

    auto *iseqEncoded = getIseqEncodedPointer(cs, build, file);
    const string rawName = "fileLineNumberInfo";
    auto *global = cs.module->getOrInsertGlobal(
        rawName, globalTy, [&cs, &rawName, &iseqEncoded, file, globalTy, iseqEncodedInitFn]() {
            auto globalInitBuilder = llvm::IRBuilder<>(cs);

            bool isConstant = false;
            auto *zero = llvm::ConstantAggregateZero::get(globalTy);
            auto *fileLineNumberInfo = new llvm::GlobalVariable(*cs.module, globalTy, isConstant,
                                                                llvm::GlobalVariable::InternalLinkage, zero, rawName);

            globalInitBuilder.SetInsertPoint(cs.globalConstructorsEntry);

            auto *numLines = llvm::ConstantInt::get(cs, llvm::APInt(32, file.data(cs).lineCount(), true));
            auto *intzero = llvm::ConstantInt::get(cs, llvm::APInt(32, 0));
            llvm::Value *indices[] = {intzero, intzero};
            globalInitBuilder.CreateCall(
                iseqEncodedInitFn, {fileLineNumberInfo, globalInitBuilder.CreateGEP(iseqEncoded, indices), numLines});

            return fileLineNumberInfo;
        });

    return global;
}

llvm::Value *Payload::getIseqEncodedPointer(CompilerState &cs, llvm::IRBuilderBase &builder, core::FileRef file) {
    auto *int64Ty = llvm::Type::getInt64Ty(cs);
    uint32_t lineCount = file.data(cs).lineCount();
    auto *globalTy = llvm::ArrayType::get(int64Ty, lineCount);

    const string rawName = "iseqEncodedArray";
    auto *global = cs.module->getOrInsertGlobal(rawName, globalTy, [&cs, &rawName, globalTy]() {
        auto globalInitBuilder = llvm::IRBuilder<>(cs);

        bool isConstant = false;
        auto *zero = llvm::ConstantAggregateZero::get(globalTy);
        auto *iseqEncodedArray = new llvm::GlobalVariable(*cs.module, globalTy, isConstant,
                                                          llvm::GlobalVariable::InternalLinkage, zero, rawName);

        return iseqEncodedArray;
    });

    return global;
}

core::Loc Payload::setLineNumber(CompilerState &cs, llvm::IRBuilderBase &build, core::Loc loc, core::Loc methodStart,
                                 core::Loc lastLoc, llvm::AllocaInst *lineNumberPtr) {
    if (!loc.exists()) {
        return lastLoc;
    }
    auto &builder = builderCast(build);
    auto lineno = loc.position(cs).first.line;
    if (lastLoc.exists() && lastLoc.position(cs).first.line == lineno) {
        return lastLoc;
    }
    if (!methodStart.exists()) {
        return lastLoc;
    }

    // turn the line number into an offset into the iseq_encoded global array
    auto *offset = llvm::ConstantInt::get(cs, llvm::APInt(32, lineno - 1));

    auto *encoded = Payload::getIseqEncodedPointer(cs, builder, loc.file());
    auto *intzero = llvm::ConstantInt::get(cs, llvm::APInt(32, 0));
    llvm::Value *indices[] = {intzero, intzero};
    builder.CreateCall(cs.getFunction("sorbet_setLineNumber"),
                       {offset, builder.CreateGEP(encoded, indices), builder.CreateLoad(lineNumberPtr)});
    return loc;
}

llvm::Value *Payload::readKWRestArg(CompilerState &cs, llvm::IRBuilderBase &build, llvm::Value *maybeHash) {
    auto &builder = builderCast(build);
    return builder.CreateCall(cs.getFunction("sorbet_readKWRestArgs"), {maybeHash});
}

llvm::Value *Payload::addMissingKWArg(CompilerState &cs, llvm::IRBuilderBase &build, llvm::Value *missing,
                                      llvm::Value *sym) {
    auto &builder = builderCast(build);
    return builder.CreateCall(cs.getFunction("sorbet_addMissingKWArg"), {missing, sym});
}

llvm::Value *Payload::assertAllRequiredKWArgs(CompilerState &cs, llvm::IRBuilderBase &build, llvm::Value *missing) {
    auto &builder = builderCast(build);
    return builder.CreateCall(cs.getFunction("sorbet_assertAllRequiredKWArgs"), {missing});
}

llvm::Value *Payload::assertNoExtraKWArg(CompilerState &cs, llvm::IRBuilderBase &build, llvm::Value *maybeHash,
                                         llvm::Value *numRequired, llvm::Value *optionalParsed) {
    auto &builder = builderCast(build);
    return builder.CreateCall(cs.getFunction("sorbet_assertNoExtraKWArg"), {maybeHash, numRequired, optionalParsed});
}

llvm::Value *Payload::getKWArg(CompilerState &cs, llvm::IRBuilderBase &build, llvm::Value *maybeHash,
                               llvm::Value *rubySym) {
    auto &builder = builderCast(build);
    return builder.CreateCall(cs.getFunction("sorbet_getKWArg"), {maybeHash, rubySym});
}

llvm::Value *Payload::readRestArgs(CompilerState &cs, llvm::IRBuilderBase &build, int maxPositionalArgCount,
                                   llvm::Value *argCountRaw, llvm::Value *argArrayRaw) {
    auto &builder = builderCast(build);
    return builder.CreateCall(
        cs.getFunction("sorbet_readRestArgs"),
        {llvm::ConstantInt::get(cs, llvm::APInt(32, maxPositionalArgCount)), argCountRaw, argArrayRaw});
}

namespace {
llvm::Value *getClassVariableStoreClass(CompilerState &cs, llvm::IRBuilder<> &builder, const IREmitterContext &irctx) {
    auto sym = irctx.cfg.symbol.data(cs)->owner;
    ENFORCE(sym.data(cs)->isClassOrModule());

    return Payload::getRubyConstant(cs, sym.data(cs)->topAttachedClass(cs), builder);
};

// For a variable that's escaped, compute its index into the locals from its unique id in the
// closure.
llvm::Value *indexForLocalVariable(CompilerState &cs, const IREmitterContext &irctx, int rubyBlockId, int escapeId) {
    return llvm::ConstantInt::get(cs, llvm::APInt(64, escapeId, true));
}

llvm::Value *buildInstanceVariableCache(CompilerState &cs, std::string_view name) {
    auto *cacheTy = llvm::StructType::getTypeByName(cs, "struct.iseq_inline_iv_cache_entry");
    ENFORCE(cacheTy != nullptr);
    auto *zero = llvm::ConstantAggregateZero::get(cacheTy);
    // No special initialization necessary, unlike function inline caches.
    return new llvm::GlobalVariable(*cs.module, cacheTy, false, llvm::GlobalVariable::InternalLinkage, zero,
                                    llvm::Twine("ivc_") + (string)name);
}

} // namespace

std::tuple<llvm::Value *, llvm::Value *> Payload::escapedVariableIndexAndLevel(CompilerState &cs, cfg::LocalRef local,
                                                                               const IREmitterContext &irctx,
                                                                               int rubyBlockId) {
    auto *index = indexForLocalVariable(cs, irctx, rubyBlockId, irctx.escapedVariableIndices.at(local));
    auto level = irctx.rubyBlockLevel[rubyBlockId];
    return {index, llvm::ConstantInt::get(cs, llvm::APInt(64, level, true))};
}

llvm::Value *Payload::varGet(CompilerState &cs, cfg::LocalRef local, llvm::IRBuilderBase &build,
                             const IREmitterContext &irctx, int rubyBlockId) {
    auto &builder = builderCast(build);
    if (local == cfg::LocalRef::selfVariable()) {
        return Payload::unboxRawValue(cs, builder, irctx.selfVariables.at(rubyBlockId));
    }
    if (irctx.aliases.contains(local)) {
        // alias to a field or constant
        auto alias = irctx.aliases.at(local);

        switch (alias.kind) {
            case Alias::AliasKind::Constant:
                return Payload::getRubyConstant(cs, alias.constantSym, builder);
            case Alias::AliasKind::GlobalField:
                return builder.CreateCall(cs.getFunction("sorbet_globalVariableGet"),
                                          {Payload::idIntern(cs, builder, alias.globalField.shortName(cs))});
            case Alias::AliasKind::ClassField:
                return builder.CreateCall(cs.getFunction("sorbet_classVariableGet"),
                                          {getClassVariableStoreClass(cs, builder, irctx),
                                           Payload::idIntern(cs, builder, alias.classField.shortName(cs))});
            case Alias::AliasKind::InstanceField: {
                // Each instance variable reference receives its own cache; this
                // is the easiest thing to do and is identical to what the VM does.
                //
                // TODO: attempt to create caches shared across variable references
                // for each instance variable for a given class. This is different
                // than what the VM does, but it ought to be a small size/performance
                // win.  This change would require Sorbet changes so we have the
                // necessary information at this point.
                auto name = alias.instanceField.shortName(cs);
                auto *cache = buildInstanceVariableCache(cs, name);
                return builder.CreateCall(cs.getFunction("sorbet_instanceVariableGet"),
                                          {varGet(cs, cfg::LocalRef::selfVariable(), builder, irctx, rubyBlockId),
                                           Payload::idIntern(cs, builder, name), cache});
            }
        }
    }
    if (irctx.escapedVariableIndices.contains(local)) {
        auto *cfp = getCFPForBlock(cs, builder, irctx, rubyBlockId);
        auto [index, level] = escapedVariableIndexAndLevel(cs, local, irctx, rubyBlockId);
        return builder.CreateCall(cs.getFunction("sorbet_readLocal"), {cfp, index, level});
    }

    // normal local variable
    return Payload::unboxRawValue(cs, builder, irctx.llvmVariables.at(local));
}

void Payload::varSet(CompilerState &cs, cfg::LocalRef local, llvm::Value *var, llvm::IRBuilderBase &build,
                     const IREmitterContext &irctx, int rubyBlockId) {
    auto &builder = builderCast(build);
    if (local == cfg::LocalRef::selfVariable()) {
        return Payload::boxRawValue(cs, builder, irctx.selfVariables.at(rubyBlockId), var);
    }
    if (irctx.aliases.contains(local)) {
        // alias to a field or constant
        auto alias = irctx.aliases.at(local);
        switch (alias.kind) {
            case Alias::AliasKind::Constant: {
                auto sym = alias.constantSym;
                auto name = sym.data(cs.gs)->name.show(cs.gs);
                auto owner = sym.data(cs.gs)->owner;
                builder.CreateCall(cs.getFunction("sorbet_setConstant"),
                                   {Payload::getRubyConstant(cs, owner, builder), Payload::toCString(cs, name, builder),
                                    llvm::ConstantInt::get(cs, llvm::APInt(64, name.length())), var});
            } break;
            case Alias::AliasKind::GlobalField:
                builder.CreateCall(cs.getFunction("sorbet_globalVariableSet"),
                                   {Payload::idIntern(cs, builder, alias.globalField.shortName(cs)), var});
                break;
            case Alias::AliasKind::ClassField:
                builder.CreateCall(cs.getFunction("sorbet_classVariableSet"),
                                   {getClassVariableStoreClass(cs, builder, irctx),
                                    Payload::idIntern(cs, builder, alias.classField.shortName(cs)), var});
                break;
            case Alias::AliasKind::InstanceField: {
                // Each instance variable reference receives its own cache; this
                // is the easiest thing to do and is identical to what the VM does.
                //
                // TODO: attempt to create caches shared across variable references
                // for each instance variable for a given class. This is different
                // than what the VM does, but it ought to be a small size/performance
                // win.  This change would require Sorbet changes so we have the
                // necessary information at this point.
                auto name = alias.instanceField.shortName(cs);
                auto *cache = buildInstanceVariableCache(cs, name);
                builder.CreateCall(cs.getFunction("sorbet_instanceVariableSet"),
                                   {Payload::varGet(cs, cfg::LocalRef::selfVariable(), builder, irctx, rubyBlockId),
                                    Payload::idIntern(cs, builder, name), var, cache});
                break;
            }
        }
        return;
    }
    if (irctx.escapedVariableIndices.contains(local)) {
        auto *cfp = getCFPForBlock(cs, builder, irctx, rubyBlockId);
        auto [index, level] = escapedVariableIndexAndLevel(cs, local, irctx, rubyBlockId);
        builder.CreateCall(cs.getFunction("sorbet_writeLocal"), {cfp, index, level, var});
        return;
    }

    // normal local variable
    Payload::boxRawValue(cs, builder, irctx.llvmVariables.at(local), var);
}

void Payload::rubyStopInDebugger(CompilerState &cs, llvm::IRBuilderBase &build) {
    auto &builder = builderCast(build);
    builder.CreateCall(cs.getFunction("sorbet_stopInDebugger"), {});
}

void Payload::dbg_p(CompilerState &cs, llvm::IRBuilderBase &build, llvm::Value *val) {
    auto &builder = builderCast(build);
    builder.CreateCall(cs.getFunction("sorbet_dbg_p"), {val});
}

void Payload::pushRubyStackVector(CompilerState &cs, llvm::IRBuilderBase &build, llvm::Value *cfp, llvm::Value *recv,
                                  const std::vector<llvm::Value *> &stack) {
    auto &builder = builderCast(build);
    auto *sorbetPush = cs.getFunction("sorbet_pushValueStack");

    auto *spPtr = builder.CreateCall(cs.getFunction("sorbet_get_sp"), {cfp});
    auto spPtrType = llvm::dyn_cast<llvm::PointerType>(spPtr->getType());
    llvm::Value *sp = builder.CreateLoad(spPtrType->getElementType(), spPtr);
    sp = builder.CreateCall(sorbetPush, {sp, recv});
    for (auto *arg : stack) {
        sp = builder.CreateCall(sorbetPush, {sp, arg});
    }
    builder.CreateStore(sp, spPtr);
}

llvm::Value *Payload::vmBlockHandlerNone(CompilerState &cs, llvm::IRBuilderBase &build) {
    auto &builder = builderCast(build);
    return builder.CreateCall(cs.getFunction("sorbet_vmBlockHandlerNone"), {}, "VM_BLOCK_HANDLER_NONE");
}

llvm::Value *Payload::makeBlockHandlerProc(CompilerState &cs, llvm::IRBuilderBase &build, llvm::Value *block) {
    auto &builder = builderCast(build);
    return builder.CreateCall(cs.getFunction("sorbet_makeBlockHandlerProc"), {block}, "blockHandlerProc");
}

llvm::Value *Payload::getPassedBlockHandler(CompilerState &cs, llvm::IRBuilderBase &build) {
    auto &builder = builderCast(build);
    return builder.CreateCall(cs.getFunction("sorbet_getPassedBlockHandler"), {}, "passedBlockHandler");
}

llvm::Value *Payload::callFuncWithCache(CompilerState &cs, llvm::IRBuilderBase &build, llvm::Value *cache,
                                        llvm::Value *blockHandler) {
    auto &builder = builderCast(build);
    return builder.CreateCall(cs.getFunction("sorbet_callFuncWithCache"), {cache, blockHandler}, "send");
}

llvm::Value *Payload::callFuncBlockWithCache(CompilerState &cs, llvm::IRBuilderBase &build, llvm::Value *cache,
                                             llvm::Value *blockFun, llvm::Value *closure) {
    auto &builder = builderCast(build);
    return builder.CreateCall(cs.getFunction("sorbet_callFuncBlockWithCache"), {cache, blockFun, closure},
                              "sendWithBlock");
}

llvm::Value *Payload::callFuncDirect(CompilerState &cs, llvm::IRBuilderBase &build, llvm::Value *cache, llvm::Value *fn,
                                     llvm::Value *argc, llvm::Value *argv, llvm::Value *recv, llvm::Value *iseq) {
    auto &builder = builderCast(build);
    return builder.CreateCall(cs.getFunction("sorbet_callFuncDirect"), {cache, fn, argc, argv, recv, iseq},
                              "sendDirect");
}

void Payload::afterIntrinsic(CompilerState &cs, llvm::IRBuilderBase &build) {
    auto &builder = builderCast(build);
    builder.CreateCall(cs.getFunction("sorbet_afterIntrinsic"), {});
}

llvm::Value *Payload::getCFPForBlock(CompilerState &cs, llvm::IRBuilderBase &build, const IREmitterContext &irctx,
                                     int rubyBlockId) {
    auto &builder = builderCast(build);
    switch (irctx.rubyBlockType[rubyBlockId]) {
        case FunctionType::Method:
        case FunctionType::StaticInitModule:
        case FunctionType::StaticInitFile:
            return irctx.rubyBlocks2Functions[rubyBlockId]->arg_begin() + 3;
        case FunctionType::Block:
            return builder.CreateLoad(irctx.blockControlFramePtrs.at(rubyBlockId), "cached_cfp");
        case FunctionType::ExceptionBegin:
            return irctx.rubyBlocks2Functions[rubyBlockId]->arg_begin() + 2;
        case FunctionType::Rescue:
        case FunctionType::Ensure:
        case FunctionType::Unused:
            return builder.CreateCall(cs.getFunction("sorbet_getCFP"), {}, "cfp");
    }
}

// Currently this function is an historical artifact. Prior to https://github.com/stripe/sorbet_llvm/pull/353, it was
// used to compute an offset into a global array that was used to hold locals for all method and class static-init
// functions. We now push stack frames for these functions to track the locals. Computing such an offset into the
// locals may become useful for other reasons in the future, however, so we keep the offsets and this function around,
// even though for now the offset is always zero.
llvm::Value *Payload::buildLocalsOffset(CompilerState &cs) {
    return llvm::ConstantInt::get(cs, llvm::APInt(64, 0, true));
}
} // namespace sorbet::compiler
