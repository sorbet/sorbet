// These violate our poisons so have to happen first
#include "llvm/IR/DerivedTypes.h" // FunctionType
#include "llvm/IR/IRBuilder.h"

#include "Payload.h"
#include "ast/Helpers.h"
#include "ast/ast.h"
#include "cfg/CFG.h"
#include "common/sort.h"
#include "compiler/Core/CompilerState.h"
#include "compiler/IREmitter/BasicBlockMap.h"
#include "compiler/IREmitter/IREmitterHelpers.h"
#include "compiler/Names/Names.h"

using namespace std;
namespace sorbet::compiler {

UnorderedMap<core::LocalVariable, llvm::AllocaInst *>
setupLocalVariables(CompilerState &cs, cfg::CFG &cfg,
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
            auto nilValueRaw = Payload::rubyNil(cs, builder);
            Payload::boxRawValue(cs, builder, alloca, nilValueRaw);
        }
    }

    {
        // nill out closure variables

        builder.SetInsertPoint(blockMap.functionInitializersByFunction[0]);
        auto escapedVariablesCount = blockMap.escapedVariableIndeces.size();
        for (auto i = 0; i < escapedVariablesCount; i++) {
            auto store = builder.CreateCall(
                cs.module->getFunction("sorbet_getClosureElem"),
                {blockMap.escapedClosure[0], llvm::ConstantInt::get(cs, llvm::APInt(32, i))}, "nillingOutClosureVars");
            builder.CreateStore(Payload::rubyNil(cs, builder), store);
        }
    }

    {
        // reserve the magical return value
        auto name = Names::returnValue(cs);
        auto var = core::LocalVariable{name, 1};
        auto nameStr = name.data(cs)->toString(cs);
        llvmVariables[var] =
            builder.CreateAlloca(cs.getValueType(), nullptr, llvm::StringRef(nameStr.data(), nameStr.length()));
    }

    return llvmVariables;
}

void trackBlockUsage(CompilerState &cs, cfg::CFG &cfg, core::LocalVariable lv, cfg::BasicBlock *bb,
                     UnorderedMap<core::LocalVariable, optional<int>> &privateUsages,
                     UnorderedMap<core::LocalVariable, int> &escapedIndexes, int &escapedIndexCounter,
                     bool &usesBlockArg, core::LocalVariable blkArg) {
    usesBlockArg = usesBlockArg || lv == blkArg;
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
tuple<UnorderedMap<core::LocalVariable, optional<int>>, UnorderedMap<core::LocalVariable, int>, bool>
findCaptures(CompilerState &cs, unique_ptr<ast::MethodDef> &mdef, cfg::CFG &cfg) {
    UnorderedMap<core::LocalVariable, optional<int>> ret;
    UnorderedMap<core::LocalVariable, int> escapedVariableIndexes;
    bool usesBlockArg = false;
    core::LocalVariable blkArg = core::LocalVariable::noVariable();

    int idx = 0;
    int argId = -1;
    for (auto &arg : mdef->args) {
        argId += 1;
        ast::Expression *maybeLocal = arg.get();
        if (auto *opt = ast::cast_tree<ast::OptionalArg>(arg.get())) {
            maybeLocal = opt->expr.get();
        }
        auto local = ast::cast_tree<ast::Local>(maybeLocal);
        ENFORCE(local);
        trackBlockUsage(cs, cfg, local->localVariable, cfg.entry(), ret, escapedVariableIndexes, idx, usesBlockArg,
                        blkArg);
        if (cfg.symbol.data(cs)->arguments()[argId].flags.isBlock) {
            blkArg = local->localVariable;
        }
    }

    for (auto &bb : cfg.basicBlocks) {
        for (cfg::Binding &bind : bb->exprs) {
            trackBlockUsage(cs, cfg, bind.bind.variable, bb.get(), ret, escapedVariableIndexes, idx, usesBlockArg,
                            blkArg);
            typecase(
                bind.value.get(),
                [&](cfg::Ident *i) {
                    trackBlockUsage(cs, cfg, i->what, bb.get(), ret, escapedVariableIndexes, idx, usesBlockArg, blkArg);
                },
                [&](cfg::Alias *i) { /* nothing */
                },
                [&](cfg::SolveConstraint *i) { /* nothing*/ },
                [&](cfg::Send *i) {
                    for (auto &arg : i->args) {
                        trackBlockUsage(cs, cfg, arg.variable, bb.get(), ret, escapedVariableIndexes, idx, usesBlockArg,
                                        blkArg);
                    }
                    trackBlockUsage(cs, cfg, i->recv.variable, bb.get(), ret, escapedVariableIndexes, idx, usesBlockArg,
                                    blkArg);
                },
                [&](cfg::Return *i) {
                    trackBlockUsage(cs, cfg, i->what.variable, bb.get(), ret, escapedVariableIndexes, idx, usesBlockArg,
                                    blkArg);
                },
                [&](cfg::BlockReturn *i) {
                    trackBlockUsage(cs, cfg, i->what.variable, bb.get(), ret, escapedVariableIndexes, idx, usesBlockArg,
                                    blkArg);
                },
                [&](cfg::LoadSelf *i) { /*nothing*/ /*todo: how does instance exec pass self?*/ },
                [&](cfg::Literal *i) { /* nothing*/ }, [&](cfg::Unanalyzable *i) { /*nothing*/ },
                [&](cfg::LoadArg *i) { /*nothing*/ }, [&](cfg::LoadYieldParams *i) { /*nothing*/ },
                [&](cfg::Cast *i) {
                    trackBlockUsage(cs, cfg, i->value.variable, bb.get(), ret, escapedVariableIndexes, idx,
                                    usesBlockArg, blkArg);
                },
                [&](cfg::TAbsurd *i) {
                    trackBlockUsage(cs, cfg, i->what.variable, bb.get(), ret, escapedVariableIndexes, idx, usesBlockArg,
                                    blkArg);
                });
        }
    }
    return {std::move(ret), std::move(escapedVariableIndexes), usesBlockArg};
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

vector<llvm::Function *> getRubyBlocks2FunctionsMapping(CompilerState &cs, cfg::CFG &cfg, llvm::Function *func) {
    vector<llvm::Function *> res;
    res.emplace_back(func);
    auto ft = cs.getRubyBlockFFIType();
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

BasicBlockMap IREmitterHelpers::getSorbetBlocks2LLVMBlockMapping(CompilerState &cs, cfg::CFG &cfg,
                                                                 unique_ptr<ast::MethodDef> &md,
                                                                 UnorderedMap<core::LocalVariable, Alias> &aliases,
                                                                 llvm::Function *mainFunc) {
    vector<llvm::Function *> rubyBlock2Function = getRubyBlocks2FunctionsMapping(cs, cfg, mainFunc);
    const int maxSendArgCount = getMaxSendArgCount(cfg);
    auto [variablesPrivateToBlocks, escapedVariableIndices, usesBlockArgs] = findCaptures(cs, md, cfg);
    vector<llvm::BasicBlock *> functionInitializersByFunction;
    vector<llvm::BasicBlock *> argumentSetupBlocksByFunction;
    vector<llvm::BasicBlock *> userEntryBlockByFunction(rubyBlock2Function.size());
    vector<llvm::AllocaInst *> sendArgArrays;
    vector<llvm::AllocaInst *> lineNumberPtrsByFunction;
    vector<llvm::AllocaInst *> iseqEncodedPtrsByFunction;
    vector<llvm::Value *> escapedClosure;
    vector<int> basicBlockJumpOverrides(cfg.maxBasicBlockId);
    llvm::IRBuilder<> builder(cs);
    {
        for (int i = 0; i < cfg.maxBasicBlockId; i++) {
            basicBlockJumpOverrides[i] = i;
        }
    }
    int i = 0;
    auto lineNumberPtrType = llvm::PointerType::getUnqual(llvm::Type::getInt64PtrTy(cs));
    auto iseqEncodedPtrType = llvm::Type::getInt64PtrTy(cs);
    for (auto &fun : rubyBlock2Function) {
        auto inits = functionInitializersByFunction.emplace_back(llvm::BasicBlock::Create(
            cs, "functionEntryInitializers",
            fun)); // we will build a link for this block later, after we finish building expressions into it
        builder.SetInsertPoint(inits);
        auto sendArgArray = builder.CreateAlloca(llvm::ArrayType::get(llvm::Type::getInt64Ty(cs), maxSendArgCount),
                                                 nullptr, "callArgs");
        llvm::Value *localClosure = nullptr;
        if (i == 0) {
            if (!escapedVariableIndices.empty())
                localClosure = builder.CreateCall(
                    cs.module->getFunction("sorbet_allocClosureAsValue"),
                    {llvm::ConstantInt::get(cs, llvm::APInt(32, escapedVariableIndices.size()))}, "captures");
            else {
                localClosure = Payload::rubyNil(cs, builder);
            }
        } else {
            localClosure = fun->arg_begin() + 1;
        }
        ENFORCE(localClosure != nullptr);
        escapedClosure.emplace_back(localClosure);
        sendArgArrays.emplace_back(sendArgArray);
        auto lineNumberPtr = builder.CreateAlloca(lineNumberPtrType, nullptr, "lineCountStore");
        lineNumberPtrsByFunction.emplace_back(lineNumberPtr);
        auto iseqEncodedPtr = builder.CreateAlloca(iseqEncodedPtrType, nullptr, "iseqEncodedStore");
        iseqEncodedPtrsByFunction.emplace_back(iseqEncodedPtr);
        argumentSetupBlocksByFunction.emplace_back(llvm::BasicBlock::Create(cs, "argumentSetup", fun));
        i++;
    }

    llvm::BasicBlock *sigVerificationBlock = llvm::BasicBlock::Create(cs, "checkSig", rubyBlock2Function[0]);

    vector<llvm::BasicBlock *> llvmBlocks(cfg.maxBasicBlockId);
    for (auto &b : cfg.basicBlocks) {
        if (b.get() == cfg.entry()) {
            llvmBlocks[b->id] = userEntryBlockByFunction[0] =
                llvm::BasicBlock::Create(cs, "userEntry", rubyBlock2Function[0]);
        } else {
            llvmBlocks[b->id] = llvm::BasicBlock::Create(cs, llvm::Twine("BB") + llvm::Twine(b->id),
                                                         rubyBlock2Function[b->rubyBlockId]);
        }
    }
    vector<shared_ptr<core::SendAndBlockLink>> blockLinks(rubyBlock2Function.size());
    vector<vector<core::LocalVariable>> rubyBlockArgs(rubyBlock2Function.size());

    {
        // fill in data about args for main function
        for (auto &treeArg : md->args) {
            auto *a = ast::MK::arg2Local(treeArg.get());
            rubyBlockArgs[0].emplace_back(a->localVariable);
        }
    }

    for (auto &b : cfg.basicBlocks) {
        if (b->bexit.cond.variable == core::LocalVariable::blockCall()) {
            userEntryBlockByFunction[b->rubyBlockId] = llvmBlocks[b->bexit.thenb->id];
            basicBlockJumpOverrides[b->id] = b->bexit.elseb->id;
            auto backId = -1;
            for (auto bid = 0; bid < b->backEdges.size(); bid++) {
                if (b->backEdges[bid]->rubyBlockId < b->rubyBlockId) {
                    backId = bid;
                    break;
                };
            }
            ENFORCE(backId >= 0);
            auto &expectedSendBind = b->backEdges[backId]->exprs[b->backEdges[backId]->exprs.size() - 2];
            auto expectedSend = cfg::cast_instruction<cfg::Send>(expectedSendBind.value.get());
            ENFORCE(expectedSend);
            ENFORCE(expectedSend->link);
            blockLinks[b->rubyBlockId] = expectedSend->link;

            rubyBlockArgs[b->rubyBlockId].resize(expectedSend->link->argFlags.size(),
                                                 core::LocalVariable::noVariable());
            for (auto &maybeCallOnLoadYieldArg : b->bexit.thenb->exprs) {
                auto maybeCast = cfg::cast_instruction<cfg::Send>(maybeCallOnLoadYieldArg.value.get());
                if (maybeCast == nullptr || maybeCast->recv.variable._name != core::Names::blkArg() ||
                    maybeCast->fun != core::Names::squareBrackets() || maybeCast->args.size() != 1) {
                    continue;
                }
                auto litType = core::cast_type<core::LiteralType>(maybeCast->args[0].type.get());
                if (litType == nullptr) {
                    continue;
                }
                rubyBlockArgs[b->rubyBlockId][litType->value] = maybeCallOnLoadYieldArg.bind.variable;
            }
        }
    }

    llvm::BasicBlock *postProcessBlock = llvm::BasicBlock::Create(cs, "postProcess", mainFunc);

    BasicBlockMap approximation{cfg.symbol,
                                functionInitializersByFunction,
                                argumentSetupBlocksByFunction,
                                userEntryBlockByFunction,
                                llvmBlocks,
                                move(basicBlockJumpOverrides),
                                move(sendArgArrays),
                                escapedClosure,
                                std::move(escapedVariableIndices),
                                sigVerificationBlock,
                                postProcessBlock,
                                move(blockLinks),
                                move(rubyBlockArgs),
                                move(rubyBlock2Function),
                                {},
                                move(lineNumberPtrsByFunction),
                                move(iseqEncodedPtrsByFunction),
                                usesBlockArgs};
    approximation.llvmVariables = setupLocalVariables(cs, cfg, variablesPrivateToBlocks, approximation, aliases);

    return approximation;
}

namespace {

string getFunctionNamePrefix(CompilerState &cs, core::SymbolRef sym) {
    auto maybeAttached = sym.data(cs)->attachedClass(cs);
    if (maybeAttached.exists()) {
        return getFunctionNamePrefix(cs, maybeAttached) + ".singleton_class";
    }
    string suffix;
    auto name = sym.data(cs)->name;
    if (name.data(cs)->kind == core::NameKind::CONSTANT &&
        name.data(cs)->cnst.original.data(cs)->kind == core::NameKind::UTF8) {
        suffix = (string)name.data(cs)->shortName(cs);
    } else {
        suffix = name.toString(cs);
    }
    string prefix =
        sym.data(cs)->owner == core::Symbols::root() ? "" : getFunctionNamePrefix(cs, sym.data(cs)->owner) + "::";

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
    if (name.data(cs)->kind == core::NameKind::UTF8) {
        suffix = (string)name.data(cs)->shortName(cs);
    } else {
        suffix = name.toString(cs);
    }

    return prefix + suffix;
}

bool IREmitterHelpers::isStaticInit(CompilerState &cs, core::SymbolRef sym) {
    auto name = sym.data(cs)->name;
    return (name.data(cs)->kind == core::NameKind::UTF8 ? name : name.data(cs)->unique.original) ==
           core::Names::staticInit();
}

namespace {
llvm::GlobalValue::LinkageTypes getFunctionLinkageType(CompilerState &cs, core::SymbolRef sym) {
    if (IREmitterHelpers::isStaticInit(cs, sym)) {
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
    ENFORCE(sym.data(cs)->name != core::Names::staticInit(), "use special helper instead");
    auto func = cs.module->getFunction(IREmitterHelpers::getFunctionName(cs, sym));
    return func;
}
llvm::Function *IREmitterHelpers::getOrCreateFunctionWeak(CompilerState &cs, core::SymbolRef sym) {
    ENFORCE(sym.data(cs)->name != core::Names::staticInit(), "use special helper instead");
    return getOrCreateFunctionWithName(cs, IREmitterHelpers::getFunctionName(cs, sym), cs.getRubyFFIType(),
                                       llvm::Function::WeakAnyLinkage);
}

llvm::Function *IREmitterHelpers::getOrCreateFunction(CompilerState &cs, core::SymbolRef sym) {
    ENFORCE(sym.data(cs)->name != core::Names::staticInit(), "use special helper instead");
    return getOrCreateFunctionWithName(cs, IREmitterHelpers::getFunctionName(cs, sym), cs.getRubyFFIType(),
                                       getFunctionLinkageType(cs, sym), true);
}

llvm::Function *IREmitterHelpers::getOrCreateStaticInit(CompilerState &cs, core::SymbolRef sym, core::Loc loc) {
    ENFORCE(sym.data(cs)->name == core::Names::staticInit(), "use general helper instead");
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

} // namespace sorbet::compiler
