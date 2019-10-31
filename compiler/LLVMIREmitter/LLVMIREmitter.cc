// These violate our poisons so have to happen first
#include "llvm/IR/Attributes.h"
#include "llvm/IR/DerivedTypes.h" // FunctionType, StructType
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Verifier.h"
// ^^^ violate our poisons
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "cfg/CFG.h"
#include "common/FileOps.h"
#include "common/sort.h"
#include "common/typecase.h"
#include "compiler/IRHelpers/IRHelpers.h"
#include "compiler/LLVMIREmitter/LLVMIREmitter.h"
#include "compiler/Names/Names.h"
#include <string_view>

using namespace std;
namespace sorbet::compiler {
struct BasicBlockMap {
    core::SymbolRef forMethod;
    vector<llvm::BasicBlock *> functionInitializersByFunction;
    vector<llvm::BasicBlock *> argumentSetupBlocksByFunction;
    vector<llvm::BasicBlock *> userEntryBlockByFunction;
    vector<llvm::BasicBlock *> llvmBlocksBySorbetBlocks;
    vector<int> basicBlockJumpOverrides;
    vector<llvm::AllocaInst *> sendArgArrayByBlock;
    vector<llvm::Value *> escapedClosure;
    UnorderedMap<core::LocalVariable, int> escapedVariableIndeces;
    llvm::BasicBlock *sigVerificationBlock;
};

// https://docs.ruby-lang.org/en/2.6.0/extension_rdoc.html
// and https://silverhammermba.github.io/emberb/c/ are your friends
// use the `demo` module for experiments
namespace {

struct Alias {
    enum class AliasKind { Constant, InstanceField, ClassField, GlobalField };
    AliasKind kind;
    core::SymbolRef constantSym;
    core::NameRef instanceField;
    core::NameRef classField;
    core::SymbolRef globalField;
    static Alias forConstant(core::SymbolRef sym) {
        Alias ret;
        ret.kind = AliasKind::Constant;
        ret.constantSym = sym;
        return ret;
    }
    static Alias forClassField(core::NameRef name) {
        Alias ret;
        ret.kind = AliasKind::ClassField;
        ret.classField = name;
        return ret;
    }
    static Alias forInstanceField(core::NameRef name) {
        Alias ret;
        ret.kind = AliasKind::InstanceField;
        ret.instanceField = name;
        return ret;
    }
    static Alias forGlobalField(core::SymbolRef sym) {
        Alias ret;
        ret.kind = AliasKind::GlobalField;
        ret.globalField = sym;
        return ret;
    }
};

bool isStaticInit(CompilerState &cs, core::SymbolRef sym) {
    auto name = sym.data(cs)->name;
    return (name.data(cs)->kind == core::NameKind::UTF8 ? name : name.data(cs)->unique.original) ==
           core::Names::staticInit();
}

llvm::GlobalValue::LinkageTypes getFunctionLinkageType(CompilerState &cs, core::SymbolRef sym) {
    if (isStaticInit(cs, sym)) {
        // this is top level code that shoudln't be callable externally.
        // Even more, sorbet reuses symbols used for these and thus if we mark them non-private we'll get link errors
        return llvm::Function::InternalLinkage;
    }
    return llvm::Function::ExternalLinkage;
}

core::SymbolRef removeRoot(core::SymbolRef sym) {
    if (sym == core::Symbols::root() || sym == core::Symbols::rootSingleton()) {
        // Root methods end up going on object
        sym = core::Symbols::Object();
    }
    return sym;
}

std::string showClassNameWithoutOwner(const core::GlobalState &gs, core::SymbolRef sym) {
    auto name = sym.data(gs)->name;
    if (name.data(gs)->kind == core::NameKind::UNIQUE) {
        return name.data(gs)->unique.original.data(gs)->show(gs);
    }
    return name.data(gs)->show(gs);
}

std::string showClassName(const core::GlobalState &gs, core::SymbolRef sym) {
    bool includeOwner = sym.data(gs)->owner.exists() && sym.data(gs)->owner != core::Symbols::root();
    string owner = includeOwner ? showClassName(gs, sym.data(gs)->owner) + "::" : "";
    return owner + showClassNameWithoutOwner(gs, sym);
}

llvm::Constant *toCString(string_view str, llvm::IRBuilder<> &builder) {
    llvm::StringRef nameRef(str.data(), str.length());
    return builder.CreateGlobalStringPtr(nameRef, llvm::Twine("str_") + nameRef);
}

// TODO: add more from https://git.corp.stripe.com/stripe-internal/ruby/blob/48bf9833/include/ruby/ruby.h#L1962. Will
// need to modify core sorbet for it.
const vector<pair<core::SymbolRef, string>> knownSymbolMapping = {
    {core::Symbols::Kernel(), "rb_mKernel"},
    {core::Symbols::Enumerable(), "rb_mEnumerable"},
    {core::Symbols::BasicObject(), "rb_cBasicObject"},
    {core::Symbols::Object(), "rb_cObject"},
    {core::Symbols::Array(), "rb_cArray"},
    {core::Symbols::Class(), "rb_cClass"},
    {core::Symbols::FalseClass(), "rb_cFalseClass"},
    {core::Symbols::TrueClass(), "rb_cTrueClass"},
    {core::Symbols::Float(), "rb_cFloat"},
    {core::Symbols::Hash(), "rb_cHash"},
    {core::Symbols::Integer(), "rb_cInteger"},
    {core::Symbols::Module(), "rb_cModule"},
    {core::Symbols::NilClass(), "rb_cNilClass"},
    {core::Symbols::Proc(), "rb_cProc"},
    {core::Symbols::Range(), "rb_cRange"},
    {core::Symbols::Rational(), "rb_cRational"},
    {core::Symbols::Regexp(), "rb_cRegexp"},
    {core::Symbols::String(), "rb_cString"},
    {core::Symbols::Struct(), "rb_cStruct"},
    {core::Symbols::Symbol(), "rb_cSymbol"},
    {core::Symbols::StandardError(), "rb_eStandardError"},
};

llvm::Value *resolveSymbol(CompilerState &cs, core::SymbolRef sym, llvm::IRBuilder<> &builder) {
    sym = removeRoot(sym);
    for (const auto &[knownSym, name] : knownSymbolMapping) {
        if (sym == knownSym) {
            auto tp = llvm::Type::getInt64Ty(cs);
            auto &nm = name; // C++ bindings don't play well with captures
            auto globalDeclaration = cs.module->getOrInsertGlobal(name, tp, [&] {
                auto ret =
                    new llvm::GlobalVariable(*cs.module, tp, true, llvm::GlobalVariable::ExternalLinkage, nullptr, nm);
                return ret;
            });
            return builder.CreateLoad(globalDeclaration);
        }
    }
    auto str = showClassName(cs, sym);
    ENFORCE(str.length() < 2 || (str[0] != ':'), "implementation assumes that strings dont start with ::");
    auto loaderName = "const_load" + str;
    auto continuePath = llvm::BasicBlock::Create(cs, "const_continue", builder.GetInsertBlock()->getParent());
    auto slowPath = llvm::BasicBlock::Create(cs, "const_slowPath", builder.GetInsertBlock()->getParent());
    auto guardEpochName = "guard_epoch_" + str;
    auto guardedConstName = "guarded_const_" + str;

    auto tp = llvm::Type::getInt64Ty(cs);

    auto guardEpochDeclaration = cs.module->getOrInsertGlobal(guardEpochName, tp, [&] {
        auto ret = new llvm::GlobalVariable(*cs.module, tp, false, llvm::GlobalVariable::LinkOnceAnyLinkage,
                                            llvm::ConstantInt::get(tp, 0), guardEpochName);
        return ret;
    });

    auto guardedConstDeclaration = cs.module->getOrInsertGlobal(guardedConstName, tp, [&] {
        auto ret = new llvm::GlobalVariable(*cs.module, tp, false, llvm::GlobalVariable::LinkOnceAnyLinkage,
                                            llvm::ConstantInt::get(tp, 0), guardedConstName);
        return ret;
    });

    auto canTakeFastPath =
        builder.CreateICmpEQ(builder.CreateLoad(guardEpochDeclaration),
                             builder.CreateCall(cs.module->getFunction("sorbet_getConstantEpoch")), "canTakeFastPath");
    auto expected = cs.setExpectedBool(builder, canTakeFastPath, true);

    builder.CreateCondBr(expected, continuePath, slowPath);
    builder.SetInsertPoint(slowPath);
    auto recomputeFunName = "const_recompute_" + str;
    auto recomputeFun = cs.module->getFunction(recomputeFunName);
    if (!recomputeFun) {
        // generate something logically similar to
        // VALUE const_load_FOO() {
        //    if (const_guard == constant_epoch()) {
        //      return guarded_const;
        //    } else {
        //      guardedConst = sorbet_getConstant("FOO");
        //      const_guard = constant_epoch();
        //      return guarded_const;
        //    }
        //
        //    It's not exactly this as I'm doing some tunnings, more specifically, the "else" branch will be marked cold
        //    and shared across all modules

        auto recomputeFunT = llvm::FunctionType::get(llvm::Type::getVoidTy(cs), {}, false /*not varargs*/);
        recomputeFun = llvm::Function::Create(recomputeFunT, llvm::Function::LinkOnceAnyLinkage,
                                              llvm::Twine("const_recompute_") + str, *cs.module);
        recomputeFun->addFnAttr(llvm::Attribute::Cold);
        llvm::IRBuilder<> functionBuilder(cs);

        functionBuilder.SetInsertPoint(llvm::BasicBlock::Create(cs, "", recomputeFun));

        functionBuilder.CreateStore(
            functionBuilder.CreateCall(
                cs.module->getFunction("sorbet_getConstant"),
                {toCString(str, functionBuilder), llvm::ConstantInt::get(cs, llvm::APInt(64, str.length()))}),
            guardedConstDeclaration);
        functionBuilder.CreateStore(functionBuilder.CreateCall(cs.module->getFunction("sorbet_getConstantEpoch")),
                                    guardEpochDeclaration);
        functionBuilder.CreateRetVoid();
    }
    builder.CreateCall(recomputeFun);
    builder.CreateBr(continuePath);
    builder.SetInsertPoint(continuePath);

    return builder.CreateLoad(guardedConstDeclaration);
}

const vector<pair<core::SymbolRef, string>> optimizedTypeTests = {
    {core::Symbols::untyped(), "sorbet_isa_Untyped"},
    {core::Symbols::Array(), "sorbet_isa_Array"},
    {core::Symbols::FalseClass(), "sorbet_isa_FalseClass"},
    {core::Symbols::TrueClass(), "sorbet_isa_TrueClass"},
    {core::Symbols::Float(), "sorbet_isa_Float"},
    {core::Symbols::Hash(), "sorbet_isa_Hash"},
    {core::Symbols::Integer(), "sorbet_isa_Integer"},
    {core::Symbols::NilClass(), "sorbet_isa_NilClass"},
    {core::Symbols::Proc(), "sorbet_isa_Proc"},
    {core::Symbols::Rational(), "sorbet_isa_Rational"},
    {core::Symbols::Regexp(), "sorbet_isa_Regexp"},
    {core::Symbols::String(), "sorbet_isa_String"},
    {core::Symbols::Symbol(), "sorbet_isa_Symbol"},
    {core::Symbols::Proc(), "sorbet_isa_Proc"},

};

llvm::Value *createTypeTestU1(CompilerState &cs, llvm::IRBuilder<> &builder, llvm::Value *val,
                              const core::TypePtr &type) {
    llvm::Value *ret = nullptr;
    typecase(
        type.get(),
        [&](core::ClassType *ct) {
            for (const auto &[candidate, specializedCall] : optimizedTypeTests) {
                if (ct->symbol == candidate) {
                    ret = builder.CreateCall(cs.module->getFunction(specializedCall), {val});
                    return;
                }
            }
            auto attachedClass = ct->symbol.data(cs)->attachedClass(cs);
            // todo: handle attached of attached class
            if (attachedClass.exists()) {
                ret = builder.CreateCall(cs.module->getFunction("sorbet_isa_class_of"),
                                         {val, resolveSymbol(cs, attachedClass, builder)});
                return;
            }
            ret =
                builder.CreateCall(cs.module->getFunction("sorbet_isa"), {val, resolveSymbol(cs, ct->symbol, builder)});
        },
        [&](core::OrType *ct) {
            // TODO: reoder types so that cheap test is done first
            auto left = createTypeTestU1(cs, builder, val, ct->left);
            auto rightBlockStart = llvm::BasicBlock::Create(cs, "orRight", builder.GetInsertBlock()->getParent());
            auto contBlock = llvm::BasicBlock::Create(cs, "orContinue", builder.GetInsertBlock()->getParent());
            auto leftEnd = builder.GetInsertBlock();
            builder.CreateCondBr(left, contBlock, rightBlockStart);
            builder.SetInsertPoint(rightBlockStart);
            auto right = createTypeTestU1(cs, builder, val, ct->right);
            auto rightEnd = builder.GetInsertBlock();
            builder.CreateBr(contBlock);
            builder.SetInsertPoint(contBlock);
            auto phi = builder.CreatePHI(builder.getInt1Ty(), 2, "orTypeTest");
            phi->addIncoming(left, leftEnd);
            phi->addIncoming(right, rightEnd);
            ret = phi;
        },
        [&](core::AndType *ct) {
            // TODO: reoder types so that cheap test is done first
            auto left = createTypeTestU1(cs, builder, val, ct->left);
            auto rightBlockStart = llvm::BasicBlock::Create(cs, "andRight", builder.GetInsertBlock()->getParent());
            auto contBlock = llvm::BasicBlock::Create(cs, "andContinue", builder.GetInsertBlock()->getParent());
            auto leftEnd = builder.GetInsertBlock();
            builder.CreateCondBr(left, rightBlockStart, contBlock);
            builder.SetInsertPoint(rightBlockStart);
            auto right = createTypeTestU1(cs, builder, val, ct->right);
            auto rightEnd = builder.GetInsertBlock();
            builder.CreateBr(contBlock);
            builder.SetInsertPoint(contBlock);
            auto phi = builder.CreatePHI(builder.getInt1Ty(), 2, "andTypeTest");
            phi->addIncoming(left, leftEnd);
            phi->addIncoming(right, rightEnd);
            ret = phi;
        },
        [&](core::Type *_default) { ret = builder.getInt1(true); });
    ENFORCE(ret != nullptr);
    return ret;
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

vector<llvm::Function *> getRubyBlocks2FunctionsMapping(CompilerState &cs, cfg::CFG &cfg, llvm::Function *func) {
    vector<llvm::Function *> res;
    res.emplace_back(func);
    llvm::Type *args[] = {
        llvm::Type::getInt64Ty(cs),    // first yielded argument(first argument is both here and in argArray
        llvm::Type::getInt64Ty(cs),    // data
        llvm::Type::getInt32Ty(cs),    // arg count
        llvm::Type::getInt64PtrTy(cs), // argArray
        llvm::Type::getInt64Ty(cs),    // blockArg
    };
    auto ft = llvm::FunctionType::get(llvm::Type::getInt64Ty(cs), args, false /*not varargs*/);

    for (int i = 1; i <= cfg.maxRubyBlockId; i++) {
        auto fp = llvm::Function::Create(ft, llvm::Function::InternalLinkage,
                                         llvm::Twine{func->getName()} + "$block_" + llvm::Twine(i), *cs.module);
        {
            // setup argument names
            // setup function argument names
            fp->arg_begin()->setName("firstYieldArgRaw");
            (fp->arg_begin() + 1)->setName("captures");
            (fp->arg_begin() + 2)->setName("argc");
            (fp->arg_begin() + 3)->setName("argArray");
            (fp->arg_begin() + 4)->setName("blockArg");
        }
        res.emplace_back(fp);
    }
    return res;
};

llvm::Value *getClassVariableStoreClass(CompilerState &cs, llvm::IRBuilder<> &builder, const BasicBlockMap &blockMap) {
    auto sym = blockMap.forMethod.data(cs)->owner;
    ENFORCE(sym.data(cs)->isClassOrModule());

    return resolveSymbol(cs, sym.data(cs)->topAttachedClass(cs), builder);
};

llvm::Value *varGet(CompilerState &cs, core::LocalVariable local, llvm::IRBuilder<> &builder,
                    const UnorderedMap<core::LocalVariable, llvm::AllocaInst *> &llvmVariables,
                    const UnorderedMap<core::LocalVariable, Alias> &aliases, const BasicBlockMap &blockMap,
                    int rubyBlockId) {
    if (aliases.contains(local)) {
        // alias to a field or constant
        auto alias = aliases.at(local);

        if (alias.kind == Alias::AliasKind::Constant) {
            return resolveSymbol(cs, alias.constantSym, builder);
        } else if (alias.kind == Alias::AliasKind::GlobalField) {
            return builder.CreateCall(cs.module->getFunction("sorbet_globalVariableGet"),
                                      {toCString(alias.globalField.data(cs)->name.data(cs)->shortName(cs), builder)});

        } else if (alias.kind == Alias::AliasKind::ClassField) {
            return builder.CreateCall(cs.module->getFunction("sorbet_classVariableGet"),
                                      {getClassVariableStoreClass(cs, builder, blockMap),
                                       cs.getRubyIdFor(builder, alias.classField.data(cs)->shortName(cs))});
        } else if (alias.kind == Alias::AliasKind::InstanceField) {
            return builder.CreateCall(cs.module->getFunction("sorbet_instanceVariableGet"),
                                      {varGet(cs, core::LocalVariable::selfVariable(), builder, llvmVariables, aliases,
                                              blockMap, rubyBlockId),
                                       cs.getRubyIdFor(builder, alias.instanceField.data(cs)->shortName(cs))});
        }
    }
    if (blockMap.escapedVariableIndeces.contains(local)) {
        auto id = blockMap.escapedVariableIndeces.at(local);
        auto store =
            builder.CreateCall(cs.module->getFunction("sorbet_getClosureElem"),
                               {blockMap.escapedClosure[rubyBlockId], llvm::ConstantInt::get(cs, llvm::APInt(32, id))});
        return builder.CreateLoad(store);
    }

    // normal local variable
    return cs.unboxRawValue(builder, llvmVariables.at(local));
} // namespace

void varSet(CompilerState &cs, core::LocalVariable local, llvm::Value *var, llvm::IRBuilder<> &builder,
            const UnorderedMap<core::LocalVariable, llvm::AllocaInst *> &llvmVariables,
            UnorderedMap<core::LocalVariable, Alias> &aliases, const BasicBlockMap &blockMap, int rubyBlockId) {
    if (aliases.contains(local)) {
        // alias to a field or constant
        auto alias = aliases.at(local);
        if (alias.kind == Alias::AliasKind::Constant) {
            auto sym = aliases.at(local).constantSym;
            auto name = sym.data(cs.gs)->name.show(cs.gs);
            auto owner = sym.data(cs.gs)->owner;
            builder.CreateCall(cs.module->getFunction("sorbet_setConstant"),
                               {resolveSymbol(cs, owner, builder), toCString(name, builder),
                                llvm::ConstantInt::get(cs, llvm::APInt(64, name.length())), var});
        } else if (alias.kind == Alias::AliasKind::GlobalField) {
            builder.CreateCall(cs.module->getFunction("sorbet_globalVariableSet"),
                               {toCString(alias.globalField.data(cs)->name.data(cs)->shortName(cs), builder), var});
        } else if (alias.kind == Alias::AliasKind::ClassField) {
            builder.CreateCall(cs.module->getFunction("sorbet_classVariableSet"),
                               {getClassVariableStoreClass(cs, builder, blockMap),
                                cs.getRubyIdFor(builder, alias.classField.data(cs)->shortName(cs)), var});
        } else if (alias.kind == Alias::AliasKind::InstanceField) {
            builder.CreateCall(cs.module->getFunction("sorbet_instanceVariableSet"),
                               {varGet(cs, core::LocalVariable::selfVariable(), builder, llvmVariables, aliases,
                                       blockMap, rubyBlockId),
                                cs.getRubyIdFor(builder, alias.instanceField.data(cs)->shortName(cs)), var});
        }
        return;
    }
    if (blockMap.escapedVariableIndeces.contains(local)) {
        auto id = blockMap.escapedVariableIndeces.at(local);
        auto store =
            builder.CreateCall(cs.module->getFunction("sorbet_getClosureElem"),
                               {blockMap.escapedClosure[rubyBlockId], llvm::ConstantInt::get(cs, llvm::APInt(32, id))});
        builder.CreateStore(var, store);
        return;
    }

    // normal local variable
    cs.boxRawValue(builder, llvmVariables.at(local), var);
} // namespace

string getFunctionName(CompilerState &cs, core::SymbolRef sym) {
    return "func_" + sym.data(cs)->toStringFullName(cs);
}

llvm::Function *getOrCreateFunction(CompilerState &cs, std::string name, llvm::FunctionType *ft,
                                    llvm::GlobalValue::LinkageTypes linkageType = llvm::Function::InternalLinkage) {
    auto func = cs.module->getFunction(name);
    if (func) {
        return func;
    }
    return llvm::Function::Create(ft, linkageType, name, *cs.module);
}

llvm::Function *getOrCreateFunction(CompilerState &cs, core::SymbolRef sym) {
    return getOrCreateFunction(cs, getFunctionName(cs, sym), cs.getRubyFFIType(), getFunctionLinkageType(cs, sym));
}

llvm::Function *getInitFunction(CompilerState &cs, std::string baseName,
                                llvm::GlobalValue::LinkageTypes linkageType = llvm::Function::InternalLinkage) {
    std::vector<llvm::Type *> NoArgs(0, llvm::Type::getVoidTy(cs));
    auto ft = llvm::FunctionType::get(llvm::Type::getVoidTy(cs), NoArgs, false);
    return getOrCreateFunction(cs, "Init_" + baseName, ft, linkageType);
}

// Create local allocas for local variables, initialize them all to `nil`.
// load arguments, check their count
// load self
UnorderedMap<core::LocalVariable, llvm::AllocaInst *>
setupLocalVariables(CompilerState &cs, cfg::CFG &cfg, vector<llvm::Function *> &rubyBlocks2Functions,
                    const UnorderedMap<core::LocalVariable, optional<int>> &variablesPrivateToBlocks,
                    const BasicBlockMap &blockMap, UnorderedMap<core::LocalVariable, Alias> &aliases) {
    UnorderedMap<core::LocalVariable, llvm::AllocaInst *> llvmVariables;
    llvm::IRBuilder<> builder(cs);
    {
        // nill out block local variables.
        auto valueType = cs.getValueType();
        vector<pair<core::LocalVariable, optional<int>>> variablesPrivateToBlocksSorted;

        for (const auto &entry : variablesPrivateToBlocks) {
            variablesPrivateToBlocksSorted.emplace_back(entry);
        }
        fast_sort(variablesPrivateToBlocksSorted,
                  [](const auto &left, const auto &right) -> bool { return left.first < right.first; });
        for (const auto &entry : variablesPrivateToBlocksSorted) {
            auto var = entry.first;
            if (entry.second == std::nullopt) {
                continue;
            }
            auto svName = var._name.data(cs)->shortName(cs);
            builder.SetInsertPoint(blockMap.functionInitializersByFunction[entry.second.value()]);
            auto alloca = llvmVariables[var] =
                builder.CreateAlloca(valueType, nullptr, llvm::StringRef(svName.data(), svName.length()));
            auto nilValueRaw = cs.getRubyNilRaw(builder);
            cs.boxRawValue(builder, alloca, nilValueRaw);
        }
    }

    {
        // nill out closure variables

        builder.SetInsertPoint(blockMap.functionInitializersByFunction[0]);
        auto escapedVariablesCount = blockMap.escapedVariableIndeces.size();
        for (auto i = 0; i < escapedVariablesCount; i++) {
            auto store =
                builder.CreateCall(cs.module->getFunction("sorbet_getClosureElem"),
                                   {blockMap.escapedClosure[0], llvm::ConstantInt::get(cs, llvm::APInt(32, i))});
            builder.CreateStore(cs.getRubyNilRaw(builder), store);
        }
    }
    return llvmVariables;
}

void setupArguments(CompilerState &cs, cfg::CFG &cfg, unique_ptr<ast::MethodDef> &md,
                    vector<llvm::Function *> &rubyBlocks2Functions,
                    const UnorderedMap<core::LocalVariable, llvm::AllocaInst *> &llvmVariables,
                    const UnorderedMap<core::LocalVariable, optional<int>> &variablesPrivateToBlocks,
                    const BasicBlockMap &blockMap, UnorderedMap<core::LocalVariable, Alias> &aliases) {
    // this function effectively generate an optimized build of
    // https://github.com/ruby/ruby/blob/59c3b1c9c843fcd2d30393791fe224e5789d1677/include/ruby/ruby.h#L2522-L2675
    llvm::IRBuilder<> builder(cs);
    auto funcId = 0;
    auto func = rubyBlocks2Functions[funcId];
    builder.SetInsertPoint(blockMap.argumentSetupBlocksByFunction[funcId]);
    auto maxArgCount = 0;
    auto minArgCount = 0;
    {
        for (auto &arg : md->args) {
            if (ast::isa_tree<ast::OptionalArg>(arg.get())) {
                maxArgCount += 1;
                continue;
            }
            auto local = ast::cast_tree<ast::Local>(arg.get());
            ENFORCE(local);
            if (local->localVariable._name == core::Names::blkArg()) {
                continue;
            }
            maxArgCount += 1;
            minArgCount += 1;
        }
    }
    auto numOptionalArgs = maxArgCount - minArgCount;
    {
        // validate arg count
        auto argCountRaw = func->arg_begin();
        auto argCountFailBlock = llvm::BasicBlock::Create(cs, "argCountFailBlock", func);
        auto argCountSecondCheckBlock = llvm::BasicBlock::Create(cs, "argCountSecondCheckBlock", func);
        auto argCountSuccessBlock = llvm::BasicBlock::Create(cs, "argCountSuccess", func);

        auto tooManyArgs =
            builder.CreateICmpUGT(argCountRaw, llvm::ConstantInt::get(cs, llvm::APInt(32, maxArgCount)), "tooManyArgs");
        auto expected1 = cs.setExpectedBool(builder, tooManyArgs, false);
        builder.CreateCondBr(expected1, argCountFailBlock, argCountSecondCheckBlock);

        builder.SetInsertPoint(argCountSecondCheckBlock);
        auto tooFewArgs =
            builder.CreateICmpULT(argCountRaw, llvm::ConstantInt::get(cs, llvm::APInt(32, minArgCount)), "tooFewArgs");
        auto expected2 = cs.setExpectedBool(builder, tooFewArgs, false);
        builder.CreateCondBr(expected2, argCountFailBlock, argCountSuccessBlock);

        builder.SetInsertPoint(argCountFailBlock);
        cs.emitArgumentMismatch(builder, argCountRaw, minArgCount, maxArgCount);

        builder.SetInsertPoint(argCountSuccessBlock);
    }

    vector<llvm::BasicBlock *> checkBlocks;
    vector<llvm::BasicBlock *> fillFromArgBlocks;
    vector<llvm::BasicBlock *> fillFromDefaultBlocks;
    {
        // create blocks for arg filling
        for (auto i = 0; i < numOptionalArgs + 1; i++) {
            auto suffix = i == numOptionalArgs ? "Done" : to_string(i);
            checkBlocks.emplace_back(llvm::BasicBlock::Create(cs, {"checkBlock", suffix}, func));
            fillFromDefaultBlocks.emplace_back(llvm::BasicBlock::Create(cs, {"fillFromDefaultBlock", suffix}, func));
            // Don't bother making the "Done" block for fillFromArgBlocks
            if (i < numOptionalArgs) {
                fillFromArgBlocks.emplace_back(llvm::BasicBlock::Create(cs, {"fillFromArgBlock", suffix}, func));
            }
        }
    }
    {
        // fill local variables from args
        auto fillRequiredArgs = llvm::BasicBlock::Create(cs, "fillRequiredArgs", func);
        builder.CreateBr(fillRequiredArgs);
        builder.SetInsertPoint(fillRequiredArgs);

        // box `self`
        auto selfArgRaw = func->arg_begin() + 2;
        varSet(cs, core::LocalVariable::selfVariable(), selfArgRaw, builder, llvmVariables, aliases, blockMap, funcId);

        for (auto i = 0; i < maxArgCount; i++) {
            if (i >= minArgCount) {
                // if these are optional, put them in their own BasicBlock
                // because we might not run it
                auto &block = fillFromArgBlocks[i - minArgCount];
                builder.SetInsertPoint(block);
            }
            const auto &arg = md->args[i];
            auto *a = ast::MK::arg2Local(arg.get());
            llvm::Value *indices[] = {llvm::ConstantInt::get(cs, llvm::APInt(32, i, true))};
            auto name = a->localVariable._name.data(cs)->shortName(cs);
            llvm::StringRef nameRef(name.data(), name.length());
            auto argArrayRaw = func->arg_begin() + 1;
            auto rawValue = builder.CreateLoad(builder.CreateGEP(argArrayRaw, indices), {"rawArg_", nameRef});
            varSet(cs, a->localVariable, rawValue, builder, llvmVariables, aliases, blockMap, funcId);
            if (i >= minArgCount) {
                // check if we need to fill in the next variable from the arg
                builder.CreateBr(checkBlocks[i - minArgCount + 1]);
            }
        }

        // make the last instruction in all the required args point at the first check block
        builder.SetInsertPoint(fillRequiredArgs);
        builder.CreateBr(checkBlocks[0]);
    }
    {
        // build check blocks
        for (auto i = 0; i < numOptionalArgs; i++) {
            auto &block = checkBlocks[i];
            builder.SetInsertPoint(block);
            auto argCount =
                builder.CreateICmpEQ(func->arg_begin(), llvm::ConstantInt::get(cs, llvm::APInt(32, i + minArgCount)),
                                     llvm::Twine("default") + llvm::Twine(i));
            auto expected = cs.setExpectedBool(builder, argCount, false);
            builder.CreateCondBr(expected, fillFromDefaultBlocks[i], fillFromArgBlocks[i]);
        }
    }
    {
        // build fillFromDefaultBlocks
        auto optionalMethodIndex = 0;
        for (auto i = 0; i < numOptionalArgs; i++) {
            auto &block = fillFromDefaultBlocks[i];
            builder.SetInsertPoint(block);
            if (md->name.data(cs)->kind == core::NameKind::UNIQUE &&
                md->name.data(cs)->unique.uniqueNameKind == core::UniqueNameKind::DefaultArg) {
                // This method is already a default method so don't fill in
                // another other defaults for it or else it is turtles all the
                // way down
            } else {
                optionalMethodIndex++;
                auto argMethodName =
                    cs.gs.lookupNameUnique(core::UniqueNameKind::DefaultArg, md->name, optionalMethodIndex);
                ENFORCE(argMethodName.exists(), "Default argument method for " + md->name.toString(cs) +
                                                    to_string(optionalMethodIndex) + " does not exist");
                auto argMethod = md->symbol.data(cs)->owner.data(cs)->findMember(cs, argMethodName);
                ENFORCE(argMethod.exists());
                auto fillDefaultFunc = getOrCreateFunction(cs, argMethod);
                auto rawValue = builder.CreateCall(fillDefaultFunc,
                                                   {func->arg_begin(), func->arg_begin() + 1, func->arg_begin() + 2});
                auto argIndex = i + minArgCount;
                auto *a = ast::MK::arg2Local(md->args[argIndex].get());
                cs.boxRawValue(builder, llvmVariables.at(a->localVariable), rawValue);
            }
            builder.CreateBr(fillFromDefaultBlocks[i + 1]);
        }
    }
    {
        // Tie up all the "Done" blocks at the end
        builder.SetInsertPoint(checkBlocks[numOptionalArgs]);
        builder.CreateBr(fillFromDefaultBlocks[numOptionalArgs]);
        builder.SetInsertPoint(fillFromDefaultBlocks[numOptionalArgs]);
    }

    // jump to user body
    builder.CreateBr(blockMap.sigVerificationBlock);

    for (int funcId = 1; funcId <= cfg.maxRubyBlockId; funcId++) {
        // todo: this should be replaced with argument computation for blocks
        builder.SetInsertPoint(blockMap.argumentSetupBlocksByFunction[funcId]);
        builder.CreateBr(blockMap.userEntryBlockByFunction[funcId]);
    }
}

BasicBlockMap getSorbetBlocks2LLVMBlockMapping(CompilerState &cs, cfg::CFG &cfg,
                                               vector<llvm::Function *> rubyBlocks2Functions,
                                               UnorderedMap<core::LocalVariable, int> &&escapedVariableIndices,
                                               int maxSendArgCount) {
    vector<llvm::BasicBlock *> functionInitializersByFunction;
    vector<llvm::BasicBlock *> argumentSetupBlocksByFunction;
    vector<llvm::BasicBlock *> userEntryBlockByFunction(rubyBlocks2Functions.size());
    vector<llvm::AllocaInst *> sendArgArrays;
    vector<llvm::Value *> escapedClosure;
    vector<int> basicBlockJumpOverrides(cfg.maxBasicBlockId);
    llvm::IRBuilder<> builder(cs);
    {
        for (int i = 0; i < cfg.maxBasicBlockId; i++) {
            basicBlockJumpOverrides[i] = i;
        }
    }
    int i = 0;
    for (auto &fun : rubyBlocks2Functions) {
        auto inits = functionInitializersByFunction.emplace_back(llvm::BasicBlock::Create(
            cs, "functionEntryInitializers",
            fun)); // we will build a link for this block later, after we finish building expressions into it
        builder.SetInsertPoint(inits);
        auto sendArgArray = builder.CreateAlloca(llvm::ArrayType::get(llvm::Type::getInt64Ty(cs), maxSendArgCount),
                                                 nullptr, "callArgs");
        llvm::Value *localClosure = nullptr;
        if (i == 0) {
            if (!escapedVariableIndices.empty())
                localClosure =
                    builder.CreateCall(cs.module->getFunction("sorbet_allocClosureAsValue"),
                                       {llvm::ConstantInt::get(cs, llvm::APInt(32, escapedVariableIndices.size()))});
            else {
                localClosure = cs.getRubyNilRaw(builder);
            }
        } else {
            localClosure = fun->arg_begin() + 1;
        }
        escapedClosure.emplace_back(localClosure);
        sendArgArrays.emplace_back(sendArgArray);
        argumentSetupBlocksByFunction.emplace_back(llvm::BasicBlock::Create(cs, "argumentSetup", fun));
        i++;
    }

    llvm::BasicBlock *sigVerificationBlock = llvm::BasicBlock::Create(cs, "checkSig", rubyBlocks2Functions[0]);

    vector<llvm::BasicBlock *> llvmBlocks(cfg.maxBasicBlockId);
    for (auto &b : cfg.basicBlocks) {
        if (b.get() == cfg.entry()) {
            llvmBlocks[b->id] = userEntryBlockByFunction[0] =
                llvm::BasicBlock::Create(cs, "userEntry", rubyBlocks2Functions[0]);
        } else {
            llvmBlocks[b->id] = llvm::BasicBlock::Create(cs, llvm::Twine("BB") + llvm::Twine(b->id),
                                                         rubyBlocks2Functions[b->rubyBlockId]);
        }
    }
    for (auto &b : cfg.basicBlocks) {
        if (b->bexit.cond.variable == core::LocalVariable::blockCall()) {
            userEntryBlockByFunction[b->rubyBlockId] = llvmBlocks[b->bexit.thenb->id];
            basicBlockJumpOverrides[b->id] = b->bexit.elseb->id;
        }
    }

    return BasicBlockMap{cfg.symbol,
                         functionInitializersByFunction,
                         argumentSetupBlocksByFunction,
                         userEntryBlockByFunction,
                         llvmBlocks,
                         basicBlockJumpOverrides,
                         sendArgArrays,
                         escapedClosure,
                         std::move(escapedVariableIndices),
                         sigVerificationBlock

    };
}

void defineMethod(CompilerState &cs, cfg::Send *i, bool isSelf, llvm::IRBuilder<> &builder) {
    ENFORCE(i->args.size() == 2);
    auto ownerSym = typeToSym(cs, i->args[0].type);

    auto lit = core::cast_type<core::LiteralType>(i->args[1].type.get());
    ENFORCE(lit->literalKind == core::LiteralType::LiteralTypeKind::Symbol);
    core::NameRef funcNameRef(cs, lit->value);

    auto lookupSym = isSelf ? ownerSym : ownerSym.data(cs)->attachedClass(cs);
    if (ownerSym == core::Symbols::Object() && !isSelf) {
        // TODO Figure out if this speicial case is right
        lookupSym = core::Symbols::Object();
    }
    auto funcSym = lookupSym.data(cs)->findMember(cs, funcNameRef);
    ENFORCE(funcSym.exists());
    ENFORCE(funcSym.data(cs)->isMethod());
    auto llvmFuncName = getFunctionName(cs, funcSym);
    auto funcHandle = getOrCreateFunction(cs, funcSym);
    auto universalSignature = llvm::PointerType::getUnqual(llvm::FunctionType::get(llvm::Type::getInt64Ty(cs), true));
    auto ptr = builder.CreateBitCast(funcHandle, universalSignature);

    auto rubyFunc = cs.module->getFunction(isSelf ? "sorbet_defineMethodSingleton" : "sorbet_defineMethod");
    ENFORCE(rubyFunc);
    builder.CreateCall(rubyFunc, {resolveSymbol(cs, ownerSym, builder), toCString(funcNameRef.show(cs), builder), ptr,
                                  llvm::ConstantInt::get(cs, llvm::APInt(32, -1, true))});

    builder.CreateCall(getInitFunction(cs, llvmFuncName), {});
}

void defineClass(CompilerState &cs, cfg::Send *i, llvm::IRBuilder<> &builder) {
    auto sym = typeToSym(cs, i->args[0].type);
    auto classNameCStr = toCString(showClassNameWithoutOwner(cs, sym), builder);
    auto isModule = sym.data(cs)->superClass() == core::Symbols::Module();

    if (sym.data(cs)->owner != core::Symbols::root()) {
        auto getOwner = resolveSymbol(cs, sym.data(cs)->owner, builder);
        if (isModule) {
            builder.CreateCall(cs.module->getFunction("sorbet_defineNestedModule"), {getOwner, classNameCStr});
        } else {
            auto rawCall = resolveSymbol(cs, sym.data(cs)->superClass(), builder);
            builder.CreateCall(cs.module->getFunction("sorbet_defineNestedClass"), {getOwner, classNameCStr, rawCall});
        }
    } else {
        if (isModule) {
            builder.CreateCall(cs.module->getFunction("sorbet_defineTopLevelModule"), {classNameCStr});
        } else {
            auto rawCall = resolveSymbol(cs, sym.data(cs)->superClass(), builder);
            builder.CreateCall(cs.module->getFunction("sorbet_defineTopClassOrModule"), {classNameCStr, rawCall});
        }
    }

    auto funcSym = cs.gs.lookupStaticInitForClass(sym.data(cs)->attachedClass(cs));
    auto llvmFuncName = getFunctionName(cs, funcSym);
    builder.CreateCall(getInitFunction(cs, llvmFuncName), {});
}

void trackBlockUsage(CompilerState &cs, cfg::CFG &cfg, core::LocalVariable lv, cfg::BasicBlock *bb,
                     UnorderedMap<core::LocalVariable, optional<int>> &privateUsages,
                     UnorderedMap<core::LocalVariable, int> &escapedIndexes, int &escapedIndexCounter

) {
    auto fnd = privateUsages.find(lv);
    if (fnd != privateUsages.end()) {
        auto &store = fnd->second;
        if (store && store.value() != bb->rubyBlockId) {
            store = nullopt;
            escapedIndexes[lv] = escapedIndexCounter;
            escapedIndexCounter += 1;
        }
    } else {
        privateUsages[lv] = bb->rubyBlockId;
    }
}

/* if local variable is only used in block X, it maps the local variable to X, otherwise, it maps local variable to a
 * negative number */
pair<UnorderedMap<core::LocalVariable, optional<int>>, UnorderedMap<core::LocalVariable, int>>
findCaptures(CompilerState &cs, unique_ptr<ast::MethodDef> &mdef, cfg::CFG &cfg) {
    UnorderedMap<core::LocalVariable, optional<int>> ret;
    UnorderedMap<core::LocalVariable, int> escapedVariableIndexes;
    int idx = 0;
    for (auto &arg : mdef->args) {
        ast::Expression *maybeLocal = arg.get();
        if (auto *opt = ast::cast_tree<ast::OptionalArg>(arg.get())) {
            maybeLocal = opt->expr.get();
        }
        auto local = ast::cast_tree<ast::Local>(maybeLocal);
        ENFORCE(local);
        trackBlockUsage(cs, cfg, local->localVariable, cfg.entry(), ret, escapedVariableIndexes, idx);
    }

    for (auto &bb : cfg.basicBlocks) {
        for (cfg::Binding &bind : bb->exprs) {
            trackBlockUsage(cs, cfg, bind.bind.variable, bb.get(), ret, escapedVariableIndexes, idx);
            typecase(
                bind.value.get(),
                [&](cfg::Ident *i) { trackBlockUsage(cs, cfg, i->what, bb.get(), ret, escapedVariableIndexes, idx); },
                [&](cfg::Alias *i) { /* nothing */
                },
                [&](cfg::SolveConstraint *i) { /* nothing*/ },
                [&](cfg::Send *i) {
                    for (auto &arg : i->args) {
                        trackBlockUsage(cs, cfg, arg.variable, bb.get(), ret, escapedVariableIndexes, idx);
                    }
                    trackBlockUsage(cs, cfg, i->recv.variable, bb.get(), ret, escapedVariableIndexes, idx);
                },
                [&](cfg::Return *i) {
                    trackBlockUsage(cs, cfg, i->what.variable, bb.get(), ret, escapedVariableIndexes, idx);
                },
                [&](cfg::BlockReturn *i) {
                    trackBlockUsage(cs, cfg, i->what.variable, bb.get(), ret, escapedVariableIndexes, idx);
                },
                [&](cfg::LoadSelf *i) { /*nothing*/ /*todo: how does instance exec pass self?*/ },
                [&](cfg::Literal *i) { /* nothing*/ }, [&](cfg::Unanalyzable *i) { /*nothing*/ },
                [&](cfg::LoadArg *i) { /*nothing*/ }, [&](cfg::LoadYieldParams *i) { /*nothing*/ },
                [&](cfg::Cast *i) {
                    trackBlockUsage(cs, cfg, i->value.variable, bb.get(), ret, escapedVariableIndexes, idx);
                },
                [&](cfg::TAbsurd *i) {
                    trackBlockUsage(cs, cfg, i->what.variable, bb.get(), ret, escapedVariableIndexes, idx);
                });
        }
    }
    return {std::move(ret), std::move(escapedVariableIndexes)};
}

void emitUserBody(CompilerState &cs, cfg::CFG &cfg, const BasicBlockMap &blockMap,
                  const UnorderedMap<core::LocalVariable, llvm::AllocaInst *> &llvmVariables,
                  UnorderedMap<core::LocalVariable, Alias> &aliases,
                  const vector<llvm::Function *> &rubyBlocks2Functions) {
    llvm::IRBuilder<> builder(cs);
    for (auto it = cfg.forwardsTopoSort.rbegin(); it != cfg.forwardsTopoSort.rend(); ++it) {
        cfg::BasicBlock *bb = *it;
        auto block = blockMap.llvmBlocksBySorbetBlocks[bb->id];
        cs.functionEntryInitializers = blockMap.functionInitializersByFunction[bb->rubyBlockId];
        bool isTerminated = false;
        builder.SetInsertPoint(block);
        if (bb != cfg.deadBlock()) {
            for (cfg::Binding &bind : bb->exprs) {
                typecase(
                    bind.value.get(),
                    [&](cfg::Ident *i) {
                        auto var = varGet(cs, i->what, builder, llvmVariables, aliases, blockMap, bb->rubyBlockId);
                        varSet(cs, bind.bind.variable, var, builder, llvmVariables, aliases, blockMap, bb->rubyBlockId);
                    },
                    [&](cfg::Alias *i) {
                        if (i->what == core::Symbols::Magic_undeclaredFieldStub()) {
                            auto name = bind.bind.variable._name.data(cs)->shortName(cs);
                            if (name.size() > 2 && name[0] == '@' && name[1] == '@') {
                                aliases[bind.bind.variable] = Alias::forClassField(bind.bind.variable._name);
                            } else {
                                ENFORCE(name.size() > 1 && name[0] == '@');
                                aliases[bind.bind.variable] = Alias::forInstanceField(bind.bind.variable._name);
                            }
                        } else {
                            if (i->what.data(cs)->isField()) {
                                auto name = bind.bind.variable._name.data(cs)->shortName(cs);
                                ENFORCE(name.size() > 1 && name[0] == '$');
                                aliases[bind.bind.variable] = Alias::forGlobalField(i->what);
                            } else {
                                aliases[bind.bind.variable] = Alias::forConstant(i->what);
                            }
                        }
                    },
                    [&](cfg::SolveConstraint *i) {
                        /*uncomment this when we rebase over sorbet master again*/
                        // auto var = varGet(cs, i->send, builder, llvmVariables, aliases, blockMap, bb->rubyBlockId);
                        // varSet(cs, bind.bind.variable, var, builder, llvmVariables, aliases, blockMap,
                        // bb->rubyBlockId);
                    },
                    [&](cfg::Send *i) {
                        auto str = i->fun.data(cs)->shortName(cs);
                        if (i->fun == core::Names::keepForIde() || i->fun == core::Names::keepForTypechecking()) {
                            return;
                        }
                        if (i->fun == core::Names::buildHash()) {
                            auto ret =
                                builder.CreateCall(cs.module->getFunction("sorbet_newRubyHash"), {}, "rawHashLiteral");
                            // TODO(perf): in 2.7 use rb_hash_bulk_insert will give 2x speedup
                            int argc = 0;
                            while (argc < i->args.size()) {
                                auto key = i->args[argc].variable;
                                auto value = i->args[argc + 1].variable;
                                builder.CreateCall(
                                    cs.module->getFunction("sorbet_hashStore"),
                                    {ret, varGet(cs, key, builder, llvmVariables, aliases, blockMap, bb->rubyBlockId),
                                     varGet(cs, value, builder, llvmVariables, aliases, blockMap, bb->rubyBlockId)});
                                argc += 2;
                            }
                            varSet(cs, bind.bind.variable, ret, builder, llvmVariables, aliases, blockMap,
                                   bb->rubyBlockId);
                            return;
                        }
                        if (i->fun == core::Names::buildArray()) {
                            auto ret = builder.CreateCall(
                                cs.module->getFunction("sorbet_newRubyArray"),
                                {llvm::ConstantInt::get(cs, llvm::APInt(64, i->args.size(), true))}, "rawArrayLiteral");
                            for (int argc = 0; argc < i->args.size(); argc++) {
                                auto value = i->args[argc].variable;
                                builder.CreateCall(cs.module->getFunction("sorbet_arrayPush"),
                                                   {ret, varGet(cs, value, builder, llvmVariables, aliases, blockMap,
                                                                bb->rubyBlockId)});
                            }
                            varSet(cs, bind.bind.variable, ret, builder, llvmVariables, aliases, blockMap,
                                   bb->rubyBlockId);
                            return;
                        }
                        if (i->fun == Names::sorbet_defineTopClassOrModule) {
                            defineClass(cs, i, builder);
                            return;
                        }
                        if (i->fun == Names::sorbet_defineMethod) {
                            defineMethod(cs, i, false, builder);
                            return;
                        }
                        if (i->fun == Names::sorbet_defineMethodSingleton) {
                            defineMethod(cs, i, true, builder);
                            return;
                        }
                        auto rawId = cs.getRubyIdFor(builder, str);

                        // fill in args
                        {
                            int argId = -1;
                            for (auto &arg : i->args) {
                                argId += 1;
                                llvm::Value *indices[] = {llvm::ConstantInt::get(cs, llvm::APInt(32, 0, true)),
                                                          llvm::ConstantInt::get(cs, llvm::APInt(64, argId, true))};
                                auto var = varGet(cs, arg.variable, builder, llvmVariables, aliases, blockMap,
                                                  bb->rubyBlockId);
                                builder.CreateStore(var,
                                                    builder.CreateGEP(blockMap.sendArgArrayByBlock[bb->rubyBlockId],
                                                                      indices, "callArgsAddr"));
                            }
                        }
                        llvm::Value *indices[] = {llvm::ConstantInt::get(cs, llvm::APInt(64, 0, true)),
                                                  llvm::ConstantInt::get(cs, llvm::APInt(64, 0, true))};

                        // TODO(perf): call
                        // https://github.com/ruby/ruby/blob/3e3cc0885a9100e9d1bfdb77e136416ec803f4ca/internal.h#L2372
                        // to get inline caching.
                        // before this, perf will not be good
                        auto var =
                            varGet(cs, i->recv.variable, builder, llvmVariables, aliases, blockMap, bb->rubyBlockId);
                        llvm::Value *rawCall;
                        if (i->link != nullptr) {
                            // this send has a block!
                            rawCall = builder.CreateCall(
                                cs.module->getFunction("sorbet_callFuncBlock"),
                                {var, rawId, llvm::ConstantInt::get(cs, llvm::APInt(32, i->args.size(), true)),
                                 builder.CreateGEP(blockMap.sendArgArrayByBlock[bb->rubyBlockId], indices),
                                 rubyBlocks2Functions[i->link->rubyBlockId], blockMap.escapedClosure[bb->rubyBlockId]},
                                "rawSendResult");

                        } else {
                            rawCall = builder.CreateCall(
                                cs.module->getFunction("sorbet_callFunc"),
                                {var, rawId, llvm::ConstantInt::get(cs, llvm::APInt(32, i->args.size(), true)),
                                 builder.CreateGEP(blockMap.sendArgArrayByBlock[bb->rubyBlockId], indices)},
                                "rawSendResult");
                        }
                        varSet(cs, bind.bind.variable, rawCall, builder, llvmVariables, aliases, blockMap,
                               bb->rubyBlockId);
                    },
                    [&](cfg::Return *i) {
                        isTerminated = true;
                        ENFORCE(bb->rubyBlockId == 0, "returns through multiple stacks not implemented");
                        auto var =
                            varGet(cs, i->what.variable, builder, llvmVariables, aliases, blockMap, bb->rubyBlockId);
                        builder.CreateRet(var);
                    },
                    [&](cfg::BlockReturn *i) {
                        ENFORCE(bb->rubyBlockId != 0, "should never happen");

                        isTerminated = true;
                        auto var =
                            varGet(cs, i->what.variable, builder, llvmVariables, aliases, blockMap, bb->rubyBlockId);
                        builder.CreateRet(var);
                    },
                    [&](cfg::LoadSelf *i) {
                        auto var = varGet(cs, i->fallback, builder, llvmVariables, aliases, blockMap, bb->rubyBlockId);
                        varSet(cs, bind.bind.variable, var, builder, llvmVariables, aliases, blockMap, bb->rubyBlockId);
                    },
                    [&](cfg::Literal *i) {
                        if (i->value->derivesFrom(cs, core::Symbols::FalseClass())) {
                            varSet(cs, bind.bind.variable, cs.getRubyFalseRaw(builder), builder, llvmVariables, aliases,
                                   blockMap, bb->rubyBlockId);
                            return;
                        }
                        if (i->value->derivesFrom(cs, core::Symbols::TrueClass())) {
                            varSet(cs, bind.bind.variable, cs.getRubyTrueRaw(builder), builder, llvmVariables, aliases,
                                   blockMap, bb->rubyBlockId);
                            return;
                        }
                        if (i->value->derivesFrom(cs, core::Symbols::NilClass())) {
                            varSet(cs, bind.bind.variable, cs.getRubyNilRaw(builder), builder, llvmVariables, aliases,
                                   blockMap, bb->rubyBlockId);
                            return;
                        }

                        auto litType = core::cast_type<core::LiteralType>(i->value.get());
                        ENFORCE(litType);
                        switch (litType->literalKind) {
                            case core::LiteralType::LiteralTypeKind::Integer: {
                                auto rawInt = cs.getRubyIntRaw(builder, litType->value);
                                varSet(cs, bind.bind.variable, rawInt, builder, llvmVariables, aliases, blockMap,
                                       bb->rubyBlockId);
                                break;
                            }
                            case core::LiteralType::LiteralTypeKind::Symbol: {
                                auto str = core::NameRef(cs, litType->value).data(cs)->shortName(cs);
                                auto rawId = cs.getRubyIdFor(builder, str);
                                auto rawRubySym =
                                    builder.CreateCall(cs.module->getFunction("rb_id2sym"), {rawId}, "rawSym");
                                varSet(cs, bind.bind.variable, rawRubySym, builder, llvmVariables, aliases, blockMap,
                                       bb->rubyBlockId);
                                break;
                            }
                            case core::LiteralType::LiteralTypeKind::String: {
                                auto str = core::NameRef(cs, litType->value).data(cs)->shortName(cs);
                                auto rawRubyString = cs.getRubyStringRaw(builder, str);
                                varSet(cs, bind.bind.variable, rawRubyString, builder, llvmVariables, aliases, blockMap,
                                       bb->rubyBlockId);
                                break;
                            }
                            default:
                                cs.trace("UnsupportedLiteral");
                        }
                    },
                    [&](cfg::Unanalyzable *i) { Exception::raise("unsupported"); },
                    [&](cfg::LoadArg *i) {
                        /* intentionally omitted, it's part of method preambula */
                    },
                    [&](cfg::LoadYieldParams *i) {
                        /* intentionally omitted, it's part of method preambula */
                    },
                    [&](cfg::Cast *i) {
                        auto val =
                            varGet(cs, i->value.variable, builder, llvmVariables, aliases, blockMap, bb->rubyBlockId);
                        auto passedTypeTest = createTypeTestU1(cs, builder, val, bind.bind.type);
                        auto successBlock =
                            llvm::BasicBlock::Create(cs, "typeTestSuccess", builder.GetInsertBlock()->getParent());

                        auto failBlock =
                            llvm::BasicBlock::Create(cs, "typeTestFail", builder.GetInsertBlock()->getParent());

                        auto expected = cs.setExpectedBool(builder, passedTypeTest, true);
                        builder.CreateCondBr(expected, successBlock, failBlock);
                        builder.SetInsertPoint(failBlock);
                        // this will throw exception
                        builder.CreateCall(cs.module->getFunction("sorbet_cast_failure"),
                                           {val, toCString(i->cast.data(cs)->shortName(cs), builder),
                                            toCString(bind.bind.type->show(cs), builder)});
                        builder.CreateUnreachable();
                        builder.SetInsertPoint(successBlock);

                        if (i->cast == core::Names::let() || i->cast == core::Names::cast()) {
                            varSet(cs, bind.bind.variable, val, builder, llvmVariables, aliases, blockMap,
                                   bb->rubyBlockId);
                        } else if (i->cast == core::Names::assertType()) {
                            varSet(cs, bind.bind.variable, cs.getRubyFalseRaw(builder), builder, llvmVariables, aliases,
                                   blockMap, bb->rubyBlockId);
                        }
                    },
                    [&](cfg::TAbsurd *i) { cs.trace("TAbsurd\n"); });
                if (isTerminated) {
                    break;
                }
            }
            if (!isTerminated) {
                if (bb->bexit.thenb != bb->bexit.elseb && bb->bexit.cond.variable != core::LocalVariable::blockCall()) {
                    auto var =
                        varGet(cs, bb->bexit.cond.variable, builder, llvmVariables, aliases, blockMap, bb->rubyBlockId);
                    auto condValue = cs.getIsTruthyU1(builder, var);

                    builder.CreateCondBr(
                        condValue,
                        blockMap.llvmBlocksBySorbetBlocks[blockMap.basicBlockJumpOverrides[bb->bexit.thenb->id]],
                        blockMap.llvmBlocksBySorbetBlocks[blockMap.basicBlockJumpOverrides[bb->bexit.elseb->id]]);
                } else {
                    builder.CreateBr(
                        blockMap.llvmBlocksBySorbetBlocks[blockMap.basicBlockJumpOverrides[bb->bexit.thenb->id]]);
                }
            }
        } else {
            // handle dead block. TODO: this should throw
            builder.CreateRet(cs.getRubyNilRaw(builder));
        }
    }
}

int getMaxSendArgCount(cfg::CFG &cfg) {
    int maxSendArgCount = 0;
    for (auto &bb : cfg.basicBlocks) {
        for (cfg::Binding &bind : bb->exprs) {
            if (auto snd = cfg::cast_instruction<cfg::Send>(bind.value.get())) {
                if (maxSendArgCount < snd->args.size()) {
                    maxSendArgCount = snd->args.size();
                }
            }
        }
    }
    return maxSendArgCount;
}

void emitSigVerification(CompilerState &cs, cfg::CFG &cfg, unique_ptr<ast::MethodDef> &md,
                         const UnorderedMap<core::LocalVariable, llvm::AllocaInst *> &llvmVariables,
                         const UnorderedMap<core::LocalVariable, Alias> &aliases, const BasicBlockMap &blockMap) {
    llvm::IRBuilder<> builder(cs);
    builder.SetInsertPoint(blockMap.sigVerificationBlock);
    builder.CreateBr(blockMap.userEntryBlockByFunction[0]);
}

} // namespace

void LLVMIREmitter::run(CompilerState &cs, cfg::CFG &cfg, unique_ptr<ast::MethodDef> &md, const string &functionName) {
    UnorderedMap<core::LocalVariable, Alias> aliases;
    const int maxSendArgCount = getMaxSendArgCount(cfg);
    auto func = getOrCreateFunction(cs, md->symbol);
    {
        // setup function argument names
        func->arg_begin()->setName("argc");
        (func->arg_begin() + 1)->setName("argArray");
        (func->arg_begin() + 2)->setName("selfRaw");
    }
    func->addFnAttr(llvm::Attribute::AttrKind::StackProtectReq);
    func->addFnAttr(llvm::Attribute::AttrKind::NoUnwind);
    func->addFnAttr(llvm::Attribute::AttrKind::UWTable);
    llvm::IRBuilder<> builder(cs);

    vector<llvm::Function *> rubyBlocks2Functions = getRubyBlocks2FunctionsMapping(cs, cfg, func);

    auto [variablesPrivateToBlocks, escapedVariableIndexes] = findCaptures(cs, md, cfg);

    const BasicBlockMap blockMap = getSorbetBlocks2LLVMBlockMapping(cs, cfg, rubyBlocks2Functions,
                                                                    std::move(escapedVariableIndexes), maxSendArgCount);

    ENFORCE(cs.functionEntryInitializers == nullptr, "modules shouldn't be reused");

    const UnorderedMap<core::LocalVariable, llvm::AllocaInst *> llvmVariables =
        setupLocalVariables(cs, cfg, rubyBlocks2Functions, variablesPrivateToBlocks, blockMap, aliases);

    setupArguments(cs, cfg, md, rubyBlocks2Functions, llvmVariables, variablesPrivateToBlocks, blockMap, aliases);
    emitSigVerification(cs, cfg, md, llvmVariables, aliases, blockMap);

    emitUserBody(cs, cfg, blockMap, llvmVariables, aliases, rubyBlocks2Functions);
    for (int funId = 0; funId < blockMap.functionInitializersByFunction.size(); funId++) {
        builder.SetInsertPoint(blockMap.functionInitializersByFunction[funId]);
        builder.CreateBr(blockMap.argumentSetupBlocksByFunction[funId]);
    }
    /* run verifier */
    ENFORCE(!llvm::verifyFunction(*func, &llvm::errs()), "see above");
    // cs.runCheapOptimizations(func);
}

void LLVMIREmitter::buildInitFor(CompilerState &cs, const core::SymbolRef &sym, string_view objectName) {
    llvm::IRBuilder<> builder(cs);

    auto baseName = getFunctionName(cs, sym);
    auto linkageType = llvm::Function::InternalLinkage;
    auto owner = sym.data(cs)->owner;
    auto isRoot = owner == core::Symbols::rootSingleton();

    if (isStaticInit(cs, sym) && isRoot) {
        baseName = objectName;
        baseName = baseName.substr(0, baseName.rfind(".rb"));
        linkageType = llvm::Function::ExternalLinkage;
    }
    auto entryFunc = getInitFunction(cs, baseName, linkageType);

    auto bb = llvm::BasicBlock::Create(cs, "entry", entryFunc);
    builder.SetInsertPoint(bb);

    if (isStaticInit(cs, sym)) {
        core::SymbolRef staticInit;
        auto attachedClass = owner.data(cs)->attachedClass(cs);
        if (isRoot) {
            staticInit = cs.gs.lookupStaticInitForFile(sym.data(cs)->loc());
        } else {
            staticInit = cs.gs.lookupStaticInitForClass(attachedClass);
        }

        // Call the LLVM method that was made by run() from this Init_ method
        auto staticInitName = getFunctionName(cs, staticInit);
        auto staticInitFunc = cs.module->getFunction(staticInitName);
        ENFORCE(staticInitFunc, staticInitName + " does not exist");
        builder.CreateCall(staticInitFunc,
                           {llvm::ConstantInt::get(cs, llvm::APInt(32, 0, true)),
                            llvm::ConstantPointerNull::get(llvm::Type::getInt64PtrTy(cs)),
                            builder.CreateCall(cs.module->getFunction("sorbet_rb_cObject"))},
                           staticInitName);
    }

    builder.CreateRetVoid();

    ENFORCE(!llvm::verifyFunction(*entryFunc, &llvm::errs()), "see above");
    cs.runCheapOptimizations(entryFunc);
}

} // namespace sorbet::compiler
